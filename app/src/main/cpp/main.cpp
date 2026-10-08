#include <jni.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>
#include <cmath>
#include <mutex>
#include <algorithm>
#include <vector>
#include <string>
#include <cstdio>
#include "physics.h"
#include "race.h"
#include "track_profiles.h"
#include "glb_mesh.h"
#include <android/asset_manager_jni.h>

static EGLDisplay display=EGL_NO_DISPLAY; static EGLSurface surface=EGL_NO_SURFACE; static EGLContext context=EGL_NO_CONTEXT;
static int width=1,height=1; static std::mutex mutex;
static AAssetManager* assetManager=nullptr;
static apex::GlbMesh playerGlb,grandstandGlb,pitGlb,barrierGlb,signageGlb,vegetationGlb,landmarkGlb;
static bool glbAssetsReady=false; static apex::Car car; static apex::Input input; static apex::Race race;
static float cameraYaw=0,cameraHeight=3.2f,renderFps=0;

static const char* VS=R"(#version 300 es
layout(location=0) in vec3 aPos;
uniform mat4 uMVP;
out vec3 vWorldPos;
void main(){ vWorldPos=aPos; gl_Position=uMVP*vec4(aPos,1.0); })";

static const char* FS=R"(#version 300 es
precision mediump float;
uniform vec4 uColor;
uniform vec3 uLightDir;
in vec3 vWorldPos;
out vec4 outColor;
void main(){
    vec3 n=normalize(cross(dFdx(vWorldPos),dFdy(vWorldPos)));
    float d=max(dot(n,normalize(-uLightDir)),0.0);
    float light=0.30+0.70*d;
    outColor=vec4(uColor.rgb*light,uColor.a);
})";

static GLuint program=0,vbo=0; static GLint uMvp=-1,uColor=-1,uLightDir=-1; static bool cockpitCamera=false;

static GLuint compile(GLenum type,const char*src){
    GLuint s=glCreateShader(type);glShaderSource(s,1,&src,nullptr);glCompileShader(s);GLint ok=0;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
    if(!ok){GLchar log[512];GLsizei n=0;glGetShaderInfoLog(s,512,&n,log);__android_log_print(ANDROID_LOG_ERROR,"Apex","shader: %.*s",(int)n,log);glDeleteShader(s);return 0;}return s;
}
static bool shaders(){
    GLuint a=compile(GL_VERTEX_SHADER,VS),b=compile(GL_FRAGMENT_SHADER,FS);if(!a||!b)return false;
    program=glCreateProgram();glAttachShader(program,a);glAttachShader(program,b);glLinkProgram(program);glDeleteShader(a);glDeleteShader(b);
    GLint ok=0;glGetProgramiv(program,GL_LINK_STATUS,&ok);
    if(!ok){GLchar log[512];GLsizei n=0;glGetProgramInfoLog(program,512,&n,log);__android_log_print(ANDROID_LOG_ERROR,"Apex","link: %.*s",(int)n,log);glDeleteProgram(program);program=0;return false;}
    uMvp=glGetUniformLocation(program,"uMVP");uColor=glGetUniformLocation(program,"uColor");uLightDir=glGetUniformLocation(program,"uLightDir");glGenBuffers(1,&vbo);return true;
}
static bool initEgl(ANativeWindow*w){
    display=eglGetDisplay(EGL_DEFAULT_DISPLAY);if(display==EGL_NO_DISPLAY||!eglInitialize(display,nullptr,nullptr))return false;
    const EGLint a[]={EGL_RENDERABLE_TYPE,EGL_OPENGL_ES3_BIT,EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_DEPTH_SIZE,24,EGL_NONE};
    EGLConfig cfg;EGLint n=0;if(!eglChooseConfig(display,a,&cfg,1,&n)||!n)return false;
    const EGLint c[]={EGL_CONTEXT_CLIENT_VERSION,3,EGL_NONE};context=eglCreateContext(display,cfg,EGL_NO_CONTEXT,c);surface=eglCreateWindowSurface(display,cfg,w,nullptr);
    if(context==EGL_NO_CONTEXT||surface==EGL_NO_SURFACE||!eglMakeCurrent(display,surface,surface,context))return false;
    glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);glDisable(GL_CULL_FACE);return shaders();
}
static void shutdownEgl(){
    if(display!=EGL_NO_DISPLAY){eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);if(vbo)glDeleteBuffers(1,&vbo);if(program)glDeleteProgram(program);
        if(surface!=EGL_NO_SURFACE)eglDestroySurface(display,surface);if(context!=EGL_NO_CONTEXT)eglDestroyContext(display,context);eglTerminate(display);}
    display=EGL_NO_DISPLAY;surface=EGL_NO_SURFACE;context=EGL_NO_CONTEXT;vbo=0;program=0;
}
static void perspective(float*m,float fovy,float aspect,float zn,float zf){
    for(int i=0;i<16;i++)m[i]=0;float f=1/std::tan(fovy*.5f),q=zf/(zn-zf);m[0]=f/aspect;m[5]=f;m[10]=q;m[11]=-1;m[14]=q*zn;
}
static void lookAt(float*m,float ex,float ey,float ez,float cx,float cy,float cz){
    float fx=cx-ex,fy=cy-ey,fz=cz-ez,fl=std::sqrt(fx*fx+fy*fy+fz*fz);if(fl<.0001f)fl=1;
    fx/=fl;fy/=fl;fz/=fl;float sx=-fz,sy=0,sz=fx,sl=std::sqrt(sx*sx+sy*sy+sz*sz);
    if(sl<.0001f){sx=1;sy=0;sz=0;}else{sx/=sl;sy/=sl;sz/=sl;}
    float tx=sy*fz-sz*fy,ty=sz*fx-sx*fz,tz=sx*fy-sy*fx;
    m[0]=sx;m[4]=sy;m[8]=sz;m[12]=-(sx*ex+sy*ey+sz*ez);m[1]=tx;m[5]=ty;m[9]=tz;m[13]=-(tx*ex+ty*ey+tz*ez);
    m[2]=-fx;m[6]=-fy;m[10]=-fz;m[14]=fx*ex+fy*ey+fz*ez;m[3]=m[7]=m[11]=0;m[15]=1;
}
static void mul(float*o,const float*a,const float*b){
    float r[16];for(int c=0;c<4;c++)for(int rr=0;rr<4;rr++)r[c*4+rr]=a[rr]*b[c*4]+a[4+rr]*b[c*4+1]+a[8+rr]*b[c*4+2]+a[12+rr]*b[c*4+3];
    for(int i=0;i<16;i++)o[i]=r[i];
}
static void draw(const std::vector<float>&v,const float*m,float r,float g,float b){
    if(v.empty())return;
    glUseProgram(program);glUniformMatrix4fv(uMvp,1,GL_FALSE,m);glUniform4f(uColor,r,g,b,1);glUniform3f(uLightDir,-.35f,-.85f,-.25f);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);glBufferData(GL_ARRAY_BUFFER,v.size()*sizeof(float),v.data(),GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,nullptr);
    glDrawArrays(GL_TRIANGLES,0,(GLsizei)v.size()/3);glDisableVertexAttribArray(0);
}
static std::vector<float> box(float x,float y,float z,float yaw,float sx,float sy,float sz){
    const float q[]={-1,-1,-1,1,-1,-1,1,-1,1,-1,-1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,1,-1,1,1,1,-1,1,-1,1,1,1,1,1,-1,-1,1,-1,1,-1,1,1,1,-1,-1,1,-1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,1,-1,-1,1,1,-1,-1,1,1,1,-1,1,-1,-1,-1,-1,1,1,-1,1,1,-1,-1,1,1,-1,1,1,1,1,1,1,-1,-1,-1,-1,-1,1,1,-1,1,1,-1,-1,1,1,1,1,-1,1,1};
    std::vector<float>out;out.reserve(108);float c=std::cos(yaw),s=std::sin(yaw);
    for(int i=0;i<108;i+=3){float lx=q[i]*sx,ly=q[i+1]*sy,lz=q[i+2]*sz;out.push_back(x+c*lx+s*lz);out.push_back(y+ly);out.push_back(z-s*lx+c*lz);}return out;
}
static void addBox(std::vector<float>&dst,float x,float y,float z,float yaw,float sx,float sy,float sz){
    auto b=box(x,y,z,yaw,sx,sy,sz);dst.insert(dst.end(),b.begin(),b.end());
}
// Apply chassis attitude in the car's local coordinate frame.
// The physics model already produces pitch/roll; this makes that state visible
// instead of leaving the rendered car visually flat on the road.
static void applyCarAttitude(std::vector<float>&v,float cx,float cy,float cz,float yaw,float pitch,float roll){
    const float cyaw=std::cos(yaw),syaw=std::sin(yaw);
    const float cp=std::cos(pitch),sp=std::sin(pitch);
    const float cr=std::cos(roll),sr=std::sin(roll);
    for(size_t i=0;i+2<v.size();i+=3){
        const float dx=v[i]-cx,dy=v[i+1]-cy,dz=v[i+2]-cz;
        // World -> car-local (inverse yaw).
        const float lx=cyaw*dx-syaw*dz;
        const float lz=syaw*dx+cyaw*dz;
        // Pitch about local X.
        const float py=cp*dy-sp*lz;
        const float pz=sp*dy+cp*lz;
        // Roll about local Z.
        const float rx=cr*lx-sr*py;
        const float ry=sr*lx+cr*py;
        // Car-local -> world.
        v[i]=cx+cyaw*rx+syaw*pz;
        v[i+1]=cy+ry;
        v[i+2]=cz-syaw*rx+cyaw*pz;
    }
}
static void addQuad(std::vector<float>&dst,float ax,float ay,float az,float bx,float by,float bz,float cx,float cy,float cz,float dx,float dy,float dz){
    dst.insert(dst.end(),{ax,ay,az,bx,by,bz,cx,cy,cz,ax,ay,az,cx,cy,cz,dx,dy,dz});
}
static void drawGlbAt(const apex::GlbMesh&m,float x,float y,float z,float yaw,float scale,float r,float g,float b,const float*pv){if(m.triangles.empty())return;std::vector<float>v=m.triangles;float c=std::cos(yaw),s=std::sin(yaw);for(size_t i=0;i+2<v.size();i+=3){float lx=v[i]*scale,lz=v[i+2]*scale;v[i]=x+c*lx+s*lz;v[i+1]=y+v[i+1]*scale;v[i+2]=z-s*lx+c*lz;}draw(v,pv,r,g,b);}
static void drawGlbCar(const apex::GlbMesh&m,float x,float z,float y,float yaw,float pitch,float roll,float scale,float r,float g,float b,const float*pv){if(m.triangles.empty())return;std::vector<float>v=m.triangles;for(size_t i=0;i+2<v.size();i+=3){v[i]*=scale;v[i+1]*=scale;v[i+2]*=scale;}applyCarAttitude(v,0,0,0,yaw,pitch,roll);for(size_t i=0;i+2<v.size();i+=3){v[i]+=x;v[i+1]+=y;v[i+2]+=z;}draw(v,pv,r,g,b);}
static void drawCar(float x,float z,float baseY,float yaw,float pitch,float roll,float steering,float r,float g,float b,const float*pv){
    std::vector<float>body,wheels;float fx=std::sin(yaw),fz=std::cos(yaw);
    addBox(body,x,baseY+.27f,z,yaw,1.02f,.25f,1.78f);addBox(body,x+fx*.55f,baseY+.34f,z+fz*.55f,yaw,.60f,.18f,.68f);
    addBox(body,x-fx*.18f,baseY+.49f,z-fz*.18f,yaw,.43f,.17f,.62f);addBox(body,x+fx*1.02f,baseY+.22f,z+fz*1.02f,yaw,1.25f,.07f,.22f);
    addBox(body,x-fx*.95f,baseY+.47f,z-fz*.95f,yaw,1.05f,.08f,.16f);
    float rx=std::cos(yaw),rz=-std::sin(yaw);
    for(int side=-1;side<=1;side+=2)for(int end=-1;end<=1;end+=2){
        float wx=x+rx*side*.82f+fx*end*.55f,wz=z+rz*side*.82f+fz*end*.55f;
        const float wheelYaw=yaw+(end>0?steering:0.0f);
        addBox(wheels,wx,baseY+.19f,wz,wheelYaw,.13f,.19f,.30f);
    }
    applyCarAttitude(body,x,baseY+.27f,z,yaw,pitch,roll);
    applyCarAttitude(wheels,x,baseY+.19f,z,yaw,pitch,roll);
    draw(body,pv,r,g,b);draw(wheels,pv,.025f,.028f,.032f);
}
static float physicsAccumulator=0.0f;
static void drawWorld(float dt){
    // Keep simulation deterministic even when Android delivers an uneven render frame.
    // Rendering may vary with refresh rate, but vehicle physics and race state advance at
    // a fixed 120 Hz with a bounded catch-up budget.
    dt=std::min(std::max(dt,0.0f),0.10f);if(dt>0)renderFps+=((1.0f/dt)-renderFps)*std::min(1.0f,dt*3.0f);
    physicsAccumulator=std::min(physicsAccumulator+dt,0.10f);
    constexpr float fixedStep=1.0f/120.0f;
    int steps=0;
    while(physicsAccumulator>=fixedStep && steps<8){
        if(race.started) apex::step(car,input,fixedStep);
        else { apex::Input idle{}; apex::step(car,idle,fixedStep); }
        race.update(car,fixedStep);
        physicsAccumulator-=fixedStep;
        ++steps;
    }
    float p[16],v[16],pv[16];
    // Speed-sensitive FOV makes acceleration visually readable without changing physics.
    // Keep cockpit wider so the driver can see the apex and track exit on a phone display.
    const float speedKmh=car.speed*3.6f;
    const float baseFov=cockpitCamera?60.0f:40.0f;
    const float speedFov=cockpitCamera?std::min(7.0f,speedKmh*.035f):std::min(9.0f,speedKmh*.045f);
    const float targetFov=baseFov+speedFov;
    static float cameraFov=40.0f;
    cameraFov+=(targetFov-cameraFov)*std::min(1.0f,dt*6.0f);
    perspective(p,cameraFov*0.0174532925f,float(width)/float(height),.1f,400);
    float fx=std::sin(car.yaw),fz=std::cos(car.yaw),dy=car.yaw-cameraYaw;
    while(dy>3.14159f)dy-=6.28318f;while(dy<-3.14159f)dy+=6.28318f;cameraYaw+=dy*std::min(1.0f,dt*5.0f);
    float targetRoadY=race.playerElevation;
    float targetH=targetRoadY+3.0f+std::min(2.0f,car.speed*.025f);
    cameraHeight+=(targetH-cameraHeight)*std::min(1.0f,dt*4.0f);
    float ex,ey,ez,cx,cy,cz;
    if(cockpitCamera){
        ex=car.x+fx*.18f; ey=targetRoadY+.88f; ez=car.y+fz*.18f;
        cx=car.x+fx*6.5f; cy=targetRoadY+.72f; cz=car.y+fz*6.5f;
    }else{
        // Pull the chase camera back as speed rises so braking points and corner exits
        // stay readable instead of filling the phone display with the car body.
        const float chaseDistance=9.5f+std::min(4.0f,speedKmh*0.018f);
        ex=car.x-std::sin(cameraYaw)*chaseDistance; ey=cameraHeight; ez=car.y-std::cos(cameraYaw)*chaseDistance;
        const float lookAhead=2.0f+std::min(2.2f,speedKmh*0.010f);
        cx=car.x+fx*lookAhead; cy=targetRoadY+.35f; cz=car.y+fz*lookAhead;
    }
    lookAt(v,ex,ey,ez,cx,cy,cz);mul(pv,p,v);
    // Circuit-aware atmosphere gives each venue a distinct visual identity
    // without adding extra geometry or draw calls.
    float skyR=.010f,skyG=.020f,skyB=.034f;
    switch(race.trackIndex%7){
        case 4: skyR=.045f;skyG=.026f;skyB=.018f; break; // Bahrain desert
        case 5: skyR=.012f;skyG=.030f;skyB=.038f; break; // Interlagos
        case 6: skyR=.018f;skyG=.028f;skyB=.042f; break; // COTA
        default: break;
    }
    glViewport(0,0,width,height);glClearColor(skyR,skyG,skyB,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    // Keep the world floor below the deepest circuit terrain (Spa has a large descent).
    std::vector<float>ground={-220,-70.0f,-220,220,-70.0f,-220,220,-70.0f,220,-220,-70.0f,-220,220,-70.0f,220,-220,-70.0f,220};
    draw(ground,pv,.025f,.075f,.035f);

    const auto& profile=apex::TRACK_PROFILES[race.trackIndex%7];
    std::vector<float>grassL,grassR,road,runoff,gravel,kerbRed,kerbWhite,edgeLines,barriers,trees,bridge;
    for(size_t i=0;i<race.points.size();i++){
        auto&a=race.points[i];auto&b=race.points[(i+1)%race.points.size()];
        float anx=a.tz,anz=-a.tx,bnx=b.tz,bnz=-b.tx;
        float aw=a.width,bw=b.width,outerA=aw+18.0f,outerB=bw+18.0f;
        addQuad(grassL,a.x+anx*aw,a.elevation-.035f,a.z+anz*aw,a.x+anx*outerA,a.elevation-.08f,a.z+anz*outerA,b.x+bnx*outerB,b.elevation-.08f,b.z+bnz*outerB,b.x+bnx*bw,b.elevation-.035f,b.z+bnz*bw);
        addQuad(grassR,a.x-anx*aw,a.elevation-.035f,a.z-anz*aw,b.x-bnx*bw,b.elevation-.035f,b.z-bnz*bw,b.x-bnx*outerB,b.elevation-.08f,b.z-bnz*outerB,a.x-anx*outerA,a.elevation-.08f,a.z-anz*outerA);
        addQuad(road,a.x+anx*aw,a.elevation+.008f,a.z+anz*aw,a.x-anx*aw,a.elevation+.008f,a.z-anz*aw,b.x-bnx*bw,b.elevation+.008f,b.z-bnz*bw,b.x+bnx*bw,b.elevation+.008f,b.z+bnz*bw);

        // Realistic circuit shoulders: painted kerb, paved runoff, then gravel before grass.
        const float runoffOuter=profile.runoffOuter;
        const float gravelOuter=profile.gravelOuter;
        addQuad(runoff,
                a.x+anx*(aw+.58f),a.elevation-.002f,a.z+anz*(aw+.58f),
                a.x+anx*runoffOuter,a.elevation-.014f,a.z+anz*runoffOuter,
                b.x+bnx*runoffOuter,b.elevation-.014f,b.z+bnz*runoffOuter,
                b.x+bnx*(bw+.58f),b.elevation-.002f,b.z+bnz*(bw+.58f));
        addQuad(runoff,
                a.x-anx*(aw+.58f),a.elevation-.002f,a.z-anz*(aw+.58f),
                b.x-bnx*(bw+.58f),b.elevation-.002f,b.z-bnz*(bw+.58f),
                b.x-bnx*runoffOuter,b.elevation-.014f,b.z-bnz*runoffOuter,
                a.x-anx*runoffOuter,a.elevation-.014f,a.z-anz*runoffOuter);
        addQuad(gravel,
                a.x+anx*runoffOuter,a.elevation-.018f,a.z+anz*runoffOuter,
                a.x+anx*gravelOuter,a.elevation-.028f,a.z+anz*gravelOuter,
                b.x+bnx*gravelOuter,b.elevation-.028f,b.z+bnz*gravelOuter,
                b.x+bnx*runoffOuter,b.elevation-.018f,b.z+bnz*runoffOuter);
        addQuad(gravel,
                a.x-anx*runoffOuter,a.elevation-.018f,a.z-anz*runoffOuter,
                b.x-bnx*runoffOuter,b.elevation-.018f,b.z-bnz*runoffOuter,
                b.x-bnx*gravelOuter,b.elevation-.028f,b.z-bnz*gravelOuter,
                a.x-anx*gravelOuter,a.elevation-.028f,a.z-anz*gravelOuter);

        float cw=.58f;
        float lxA=a.x+anx*(aw+cw),lzA=a.z+anz*(aw+cw),lxB=b.x+bnx*(bw+cw),lzB=b.z+bnz*(bw+cw);
        float rxA=a.x-anx*(aw+cw),rzA=a.z-anz*(aw+cw),rxB=b.x-bnx*(bw+cw),rzB=b.z-bnz*(bw+cw);

        // Kerbs should describe corners rather than paint the entire circuit.
        // Use the change in heading across a short look-ahead window as a
        // geometry-derived corner mask. This keeps long straights clean while
        // retaining segmented kerbs through braking/turn-in/apex/exit zones.
        const size_t n=race.points.size();
        const auto& prev=race.points[(i+n-3)%n];
        const auto& next=race.points[(i+3)%n];
        const float turn=1.0f-std::clamp(prev.tx*next.tx+prev.tz*next.tz,-1.0f,1.0f);
        const bool corner=turn>0.0025f;
        if(corner){ /* striped kerb geometry emitted below */ }
        addQuad(edgeLines,a.x+anx*(aw+.05f),a.elevation+.035f,a.z+anz*(aw+.05f),
                b.x+bnx*(bw+.05f),b.elevation+.035f,b.z+bnz*(bw+.05f),
                b.x+bnx*(bw+.16f),b.elevation+.035f,b.z+bnz*(bw+.16f),
                a.x+anx*(aw+.16f),a.elevation+.035f,a.z+anz*(aw+.16f));
        addQuad(edgeLines,a.x-anx*(aw+.05f),a.elevation+.036f,a.z-anz*(aw+.05f),
                b.x-bnx*(bw+.05f),b.elevation+.036f,b.z-bnz*(bw+.05f),
                b.x-bnx*(bw+.16f),b.elevation+.036f,b.z-bnz*(bw+.16f),
                a.x-anx*(aw+.16f),a.elevation+.036f,a.z-anz*(aw+.16f));
        if(i%8==0){
            float barrierOffset=std::max(aw,bw)+7.0f;
            float yaw=std::atan2(a.tx,a.tz);
            addBox(barriers,a.x+anx*barrierOffset,a.elevation+.55f,a.z+anz*barrierOffset,yaw,.10f,.38f,1.55f);
            addBox(barriers,a.x-anx*barrierOffset,a.elevation+.55f,a.z-anz*barrierOffset,yaw,.10f,.38f,1.55f);
        }
        if(i%18==0 && profile.treeSpacing>0){
            float treeOffset=std::max(aw,bw)+profile.treeSpacing;
            float tx=a.x+anx*treeOffset,tz=a.z+anz*treeOffset;
            addBox(trees,tx,a.elevation+.85f,tz,0,.18f,.85f,.18f);
            addBox(trees,tx,a.elevation+1.95f,tz,0,.85f,1.0f,.85f);
            tx=a.x-anx*(treeOffset+3.0f);tz=a.z-anz*(treeOffset+3.0f);
            addBox(trees,tx,a.elevation+.85f,tz,0,.18f,.85f,.18f);
            addBox(trees,tx,a.elevation+1.95f,tz,0,.85f,1.0f,.85f);
        }
        // Kerbs are a racing-line feature, not a decorative stripe around the
        // entire circuit. Keep them concentrated in actual corner geometry.
        if(corner){
            const int kerbStripe=(i/3)%2;
            if(kerbStripe==0){
                addQuad(kerbWhite,lxA,a.elevation+.025f,lzA,lxB,b.elevation+.025f,lzB,b.x+bnx*bw,b.elevation+.025f,b.z+bnz*bw,a.x+anx*aw,a.elevation+.025f,a.z+anz*aw);
                addQuad(kerbWhite,rxA,a.elevation+.026f,rzA,rxB,b.elevation+.026f,rzB,b.x-bnx*bw,b.elevation+.026f,b.z-bnz*bw,a.x-anx*aw,a.elevation+.026f,a.z-anz*aw);
            }else{
                addQuad(kerbRed,lxA,a.elevation+.025f,lzA,lxB,b.elevation+.025f,lzB,b.x+bnx*bw,b.elevation+.025f,b.z+bnz*bw,a.x+anx*aw,a.elevation+.025f,a.z+anz*aw);
                addQuad(kerbRed,rxA,a.elevation+.026f,rzA,rxB,b.elevation+.026f,rzB,b.x-bnx*bw,b.elevation+.026f,b.z-bnz*bw,a.x-anx*aw,a.elevation+.026f,a.z-anz*aw);
            }
        }
    }
    // Circuit-specific materials keep the seven venues visually distinct instead of
    // painting every location with the same generic green/grey palette.
    draw(grassL,pv,profile.grass[0],profile.grass[1],profile.grass[2]);draw(grassR,pv,profile.grass[0],profile.grass[1],profile.grass[2]);
    draw(gravel,pv,profile.gravel[0],profile.gravel[1],profile.gravel[2]);draw(runoff,pv,profile.runoff[0],profile.runoff[1],profile.runoff[2]);draw(road,pv,profile.road[0],profile.road[1],profile.road[2]);
    draw(kerbRed,pv,profile.kerbB[0],profile.kerbB[1],profile.kerbB[2]);draw(kerbWhite,pv,profile.kerbA[0],profile.kerbA[1],profile.kerbA[2]);draw(edgeLines,pv,.88f,.88f,.84f);draw(barriers,pv,.22f,.24f,.27f);draw(trees,pv,.12f,.28f,.10f);

    // Suzuka flyover: the back straight passes over Degner 2/130R.
    if(profile.bridge){
        for(int k=0;k<5;k++){
            const int idx=(270+k*2)%int(race.points.size());
            const auto&q=race.points[idx];
            float nx=q.tz,nz=-q.tx,span=q.width+1.8f;
            float supportHeight=std::max(2.5f,q.elevation+0.6f);
            float supportY=q.elevation-supportHeight*.5f;
            addBox(bridge,q.x+nx*span,supportY,q.z+nz*span,std::atan2(q.tx,q.tz),.65f,supportHeight,.65f);
            addBox(bridge,q.x-nx*span,supportY,q.z-nz*span,std::atan2(q.tx,q.tz),.65f,supportHeight,.65f);
        }
        draw(bridge,pv,.35f,.36f,.38f);
    }

    // Track-specific landmark pass. These are deliberately low-poly/mobile-friendly, but
    // their placement, scale and silhouette are tied to the seven real circuits rather than
    // using one generic scenery layout everywhere. This is the bridge to the eventual GLB
    // landmark assets: the same placement hooks can be replaced asset-by-asset later.
    std::vector<float>landmarkDark,landmarkLight,landmarkAccent;
    auto placeStructure=[&](int idx,float lateral,float along,float sx,float sy,float sz,float yaw,float r,float g,float b){
        if(race.points.empty())return;
        const size_t n=race.points.size(); size_t k=(static_cast<size_t>(idx)+n)%n;
        const auto&q=race.points[k];
        const float tx=q.tx,tz=q.tz,nx=q.tz,nz=-q.tx;
        const float x=q.x+nx*lateral+tx*along, z=q.z+nz*lateral+tz*along;
        addBox(landmarkDark,x,q.elevation+sy,z,yaw,sx,sy,sz);
        (void)r;(void)g;(void)b;
    };
    auto addTrackSign=[&](int idx,float lateral,float height,float width,float yaw){
        if(race.points.empty())return;
        const auto&q=race.points[(static_cast<size_t>(idx)+race.points.size())%race.points.size()];
        float x=q.x+q.tz*lateral,z=q.z-q.tx*lateral;
        addBox(landmarkLight,x,q.elevation+height,z,yaw,width,.55f,.08f);
        addBox(landmarkDark,x,q.elevation+height-.75f,z,yaw,.07f,.75f,.07f);
    };
    switch(race.trackIndex%7){
        case 0: // Spa-Francorchamps: forest, armco and the Eau Rouge/Raidillon visual wall.
            for(int j=0;j<12;j++)placeStructure(12+j*4,10.5f,0,1.0f,1.0f,1.0f,0,.02f,.05f,.02f);
            for(int j=0;j<4;j++){addTrackSign(22+j*8,7.8f,2.2f,1.4f,0);}
            break;
        case 1: // Silverstone: long pit complex and marshal/signage structures.
            for(int j=0;j<8;j++)placeStructure(5+j*3,11.0f,0,2.8f,1.0f,1.8f,0,.18f,.18f,.20f);
            for(int j=0;j<5;j++)addTrackSign(30+j*10,8.0f,2.4f,1.7f,0);
            break;
        case 2: // Monza: park trees plus tall historic-style grandstand silhouettes.
            for(int j=0;j<6;j++)placeStructure(18+j*8,14.0f,0,3.8f,1.8f,2.0f,0,.16f,.17f,.18f);
            for(int j=0;j<5;j++)addTrackSign(45+j*9,8.5f,2.6f,1.8f,0);
            break;
        case 3: // Suzuka: distinctive signage around the figure-eight and flyover supports.
            for(int j=0;j<7;j++)addTrackSign(245+j*5,8.0f,2.5f,1.8f,0);
            for(int j=0;j<4;j++)placeStructure(250+j*3,11.0f,0,1.6f,1.2f,2.8f,0,.20f,.20f,.22f);
            break;
        case 4: // Bahrain: floodlight masts and desert-event buildings.
            for(int j=0;j<10;j++){
                int idx=j*24; placeStructure(idx,10.5f,0,.16f,5.0f,.16f,0,.28f,.29f,.31f);
                addTrackSign(idx,9.8f,3.7f,2.0f,0);
            }
            for(int j=0;j<4;j++)placeStructure(90+j*7,14.0f,0,3.0f,1.0f,2.0f,0,.22f,.20f,.16f);
            break;
        case 5: // Interlagos: dense hillside structures and spectator blocks.
            for(int j=0;j<8;j++)placeStructure(25+j*6,13.0f,0,3.0f,1.5f,2.0f,0,.16f,.18f,.20f);
            for(int j=0;j<6;j++)addTrackSign(80+j*7,8.0f,2.3f,1.6f,0);
            break;
        default: // COTA: observation tower / paddock blocks and prairie signage.
            for(int j=0;j<5;j++)placeStructure(8+j*8,14.0f,0,2.0f,4.0f,2.0f,0,.18f,.19f,.22f);
            for(int j=0;j<7;j++)addTrackSign(35+j*8,8.5f,2.5f,1.8f,0);
            break;
    }
    draw(landmarkDark,pv,.18f,.19f,.21f);
    draw(landmarkLight,pv,.82f,.82f,.78f);
    draw(landmarkAccent,pv,.72f,.08f,.05f);

    std::vector<float>markers;
    for(size_t i=0;i<race.points.size();i+=12){
        auto&q=race.points[i];float off=q.width+4.5f;
        float x=q.x+q.tz*off,z=q.z-q.tx*off;addBox(markers,x,q.elevation+.42f,z,std::atan2(q.tx,q.tz),.24f,.8f,.24f);
        x=q.x-q.tz*off;z=q.z+q.tx*off;addBox(markers,x,q.elevation+.42f,z,std::atan2(q.tx,q.tz),.24f,.8f,.24f);
    }
    draw(markers,pv,.06f,.28f,.10f);

    // Starting grid and a compact spectator grandstand make the first sector read like a real GP venue.
    std::vector<float> gridLines,grandstands;
    const auto&grid0=race.points[0];
    const float gridYaw=std::atan2(grid0.tx,grid0.tz);
    const float gx=grid0.tz, gz=-grid0.tx;
    for(int row=0;row<8;row++){
        const float along=5.0f+row*4.5f;
        const float px=grid0.x+std::cos(gridYaw)*along;
        const float pz=grid0.z+std::sin(gridYaw)*along;
        const float side=(row%2==0)?-1.0f:1.0f;
        addBox(gridLines,px+gx*(side*(1.6f+row*.15f)),grid0.elevation+.025f,
               pz+gz*(side*(1.6f+row*.15f)),gridYaw,.055f,.025f,2.35f);
    }
    // Three stepped spectator tiers outside the start straight; low-poly on purpose for mobile.
    const float standSide=1.0f;
    const float standOffset=16.0f;
    const float standX=grid0.x+gx*standSide*standOffset;
    const float standZ=grid0.z+gz*standSide*standOffset;
    for(int tier=0;tier<3;tier++){
        const float lateral=tier*1.65f;
        addBox(grandstands,standX+gx*standSide*lateral,grid0.elevation+.65f+tier*.72f,
               standZ+gz*standSide*lateral,gridYaw,8.5f,1.15f,22.0f);
    }
    draw(gridLines,pv,.92f,.92f,.88f); draw(grandstands,pv,.22f,.24f,.28f);
    if(glbAssetsReady&&!race.points.empty()){
        auto P=[&](const apex::GlbMesh&m,int i,float lat,float along,float y,float yaw,float sc,float r,float g,float b){const auto&q=race.points[(size_t(i)+race.points.size())%race.points.size()];drawGlbAt(m,q.x+q.tz*lat+q.tx*along,q.elevation+y,q.z-q.tx*lat+q.tz*along,yaw,sc,r,g,b,pv);};
        switch(race.trackIndex%7){
        case 0:for(int j=0;j<10;j++){P(vegetationGlb,10+j*5,12,0,0,0,1.1f,.10f,.32f,.08f);P(barrierGlb,14+j*8,8,0,0,0,.8f,.2f,.22f,.24f);}P(pitGlb,35,15,0,0,0,1,.26f,.27f,.29f);P(landmarkGlb,52,-14,0,0,0,1.2f,.18f,.2f,.22f);break;
        case 1:for(int j=0;j<4;j++)P(pitGlb,8+j*7,13,0,0,0,1,.24f,.25f,.28f);for(int j=0;j<5;j++)P(grandstandGlb,42+j*9,15,0,0,0,1,.3f,.31f,.34f);for(int j=0;j<8;j++)P(signageGlb,28+j*10,9,0,0,0,1,.86f,.86f,.8f);break;
        case 2:for(int j=0;j<14;j++)P(vegetationGlb,12+j*6,14+(j%2)*2,0,0,0,1,.1f,.3f,.08f);for(int j=0;j<4;j++)P(grandstandGlb,34+j*10,-15,0,3.14f,0,1.1f,.27f,.28f,.31f);break;
        case 3:for(int j=0;j<7;j++)P(signageGlb,238+j*5,9,0,0,0,1,.88f,.88f,.82f);for(int j=0;j<10;j++)P(vegetationGlb,225+j*7,13+(j%2)*2,0,0,0,1,.08f,.28f,.07f);P(landmarkGlb,270,13,0,0,0,1.2f,.24f,.25f,.27f);break;
        case 4:for(int j=0;j<5;j++)P(pitGlb,10+j*8,14,0,0,0,.95f,.35f,.31f,.25f);for(int j=0;j<10;j++)P(signageGlb,25+j*14,10,0,0,0,1,.9f,.88f,.7f);break;
        case 5:for(int j=0;j<6;j++)P(grandstandGlb,25+j*7,14,0,0,0,1,.25f,.27f,.31f);for(int j=0;j<12;j++)P(vegetationGlb,18+j*6,-13-(j%3),0,0,0,1,.08f,.3f,.07f);for(int j=0;j<8;j++)P(barrierGlb,30+j*8,9,0,0,0,.82f,.2f,.22f,.24f);break;
        default:for(int j=0;j<5;j++)P(landmarkGlb,8+j*8,14,0,0,0,1.2f,.22f,.23f,.26f);for(int j=0;j<12;j++)P(vegetationGlb,20+j*7,15+(j%2)*3,0,0,0,.9f,.09f,.28f,.07f);for(int j=0;j<7;j++)P(signageGlb,35+j*9,9,0,0,0,1,.88f,.88f,.82f);break;
        }
    }

    // Start/finish infrastructure follows the actual first track sample.
    const auto&s= race.points[0];float syaw=std::atan2(s.tx,s.tz),snx=s.tz,snz=-s.tx;
    std::vector<float>startLine,gantry,darkLights,redLights;
    float half=s.width;
    addQuad(startLine,s.x+snx*half,s.elevation+.032f,s.z+snz*half,s.x-snx*half,s.elevation+.032f,s.z-snz*half,
            s.x-snx*half+ s.tx*.22f,s.elevation+.032f,s.z-snz*half+s.tz*.22f,s.x+snx*half+s.tx*.22f,s.elevation+.032f,s.z+snz*half+s.tz*.22f);
    draw(startLine,pv,.94f,.94f,.94f);
    float poleOffset=half*.9f;
    addBox(gantry,s.x+snx*poleOffset,s.elevation+1.8f,s.z+snz*poleOffset,syaw,.12f,1.8f,.12f);
    addBox(gantry,s.x-snx*poleOffset,s.elevation+1.8f,s.z-snz*poleOffset,syaw,.12f,1.8f,.12f);
    addBox(gantry,s.x,s.elevation+3.55f,s.z,syaw,half*.95f,.12f,.12f);
    for(int i=0;i<5;i++){
        float localX=(-.72f+i*.36f)*half;
        float x=s.x+snx*localX,z=s.z+snz*localX;
        addBox(darkLights,x,s.elevation+3.35f,z,syaw,.11f,.11f,.11f);
        if(i<5)addBox(redLights,x,s.elevation+3.35f,z,syaw,.075f,.075f,.075f);
    }
    draw(gantry,pv,.10f,.10f,.12f);draw(darkLights,pv,.02f,.02f,.025f);draw(redLights,pv,.95f,.03f,.02f);

    const float aiColors[7][3]={{.08f,.18f,.8f},{.95f,.75f,.05f},{.1f,.55f,.9f},{.75f,.08f,.12f},{.55f,.15f,.7f},{.1f,.75f,.35f},{.95f,.3f,.08f}};
    for(size_t i=0;i<race.ai.size();i++){auto&a=race.ai[i];
        // Let AI cars visually conform to the same 3D track surface as the player.
        // The physics model owns their progress; this only derives a stable visual attitude
        // from neighbouring track samples, avoiding a separate or conflicting AI physics model.
        const size_t n=race.points.size();
        size_t k=n?static_cast<size_t>(a.progress*float(n)):0; if(n) k%=n;
        const auto& pm=race.points[(k+n-1)%n]; const auto& pp=race.points[(k+1)%n];
        float ds=std::max(1.0f,std::sqrt((pp.x-pm.x)*(pp.x-pm.x)+(pp.z-pm.z)*(pp.z-pm.z)));
        float pitch=std::atan2(pp.elevation-pm.elevation,ds);
        float h0=std::atan2(pm.tx,pm.tz),h1=std::atan2(pp.tx,pp.tz),dh=h1-h0;
        while(dh>3.14159265f)dh-=6.28318531f; while(dh<-3.14159265f)dh+=6.28318531f;
        float roll=std::max(-0.055f,std::min(0.055f,dh*2.2f));
        if(glbAssetsReady)drawGlbCar(playerGlb,a.x,a.z,a.elevation,a.yaw,pitch,roll,.72f,aiColors[i][0],aiColors[i][1],aiColors[i][2],pv);else drawCar(a.x,a.z,a.elevation,a.yaw,pitch,roll,0.0f,aiColors[i][0],aiColors[i][1],aiColors[i][2],pv);}
    if(glbAssetsReady)drawGlbCar(playerGlb,car.x,car.y,race.playerElevation,car.yaw,car.pitch,car.roll,.72f,.78f,.03f,.025f,pv);else drawCar(car.x,car.y,race.playerElevation,car.yaw,car.pitch,car.roll,car.steeringAngle,.9f,.035f,.02f,pv);

    if(eglSwapBuffers(display,surface)!=EGL_TRUE)__android_log_print(ANDROID_LOG_WARN,"Apex","eglSwapBuffers failed: %d",eglGetError());
}
extern "C" JNIEXPORT void JNICALL Java_com_apexenginenext_MainActivity_nativeStart(JNIEnv*e,jclass,jobject js,jobject jam){
    std::lock_guard<std::mutex>l(mutex);cockpitCamera=false;cameraHeight=3.2f;assetManager=jam?AAssetManager_fromJava(e,jam):nullptr;glbAssetsReady=false;
    if(assetManager){bool a=apex::loadGlbMesh(assetManager,"cars/player.glb",playerGlb),b=apex::loadGlbMesh(assetManager,"environment/grandstand.glb",grandstandGlb),c=apex::loadGlbMesh(assetManager,"environment/pit_building.glb",pitGlb),d=apex::loadGlbMesh(assetManager,"environment/barrier.glb",barrierGlb),s=apex::loadGlbMesh(assetManager,"environment/signage.glb",signageGlb),v=apex::loadGlbMesh(assetManager,"environment/vegetation.glb",vegetationGlb),k=apex::loadGlbMesh(assetManager,"environment/landmark.glb",landmarkGlb);glbAssetsReady=a&&b&&c&&d&&s&&v&&k;__android_log_print(ANDROID_LOG_INFO,"Apex","GLB assets ready=%d",glbAssetsReady);}
    ANativeWindow*w=ANativeWindow_fromSurface(e,js);if(!w)return;shutdownEgl();if(initEgl(w)){physicsAccumulator=0.0f;race.reset(car);}ANativeWindow_release(w);
}
extern "C" JNIEXPORT void JNICALL Java_com_apexenginenext_MainActivity_nativeStop(JNIEnv*,jclass){std::lock_guard<std::mutex>l(mutex);shutdownEgl();assetManager=nullptr;glbAssetsReady=false;}
extern "C" JNIEXPORT void JNICALL Java_com_apexenginenext_MainActivity_nativeResize(JNIEnv*,jclass,jint w,jint h){std::lock_guard<std::mutex>l(mutex);width=std::max(1,(int)w);height=std::max(1,(int)h);}
extern "C" JNIEXPORT void JNICALL Java_com_apexenginenext_MainActivity_nativeTouch(JNIEnv*,jclass,jfloat s,jfloat t,jfloat b){std::lock_guard<std::mutex>l(mutex);input.steer=s;input.throttle=t;input.brake=b;}
extern "C" JNIEXPORT void JNICALL Java_com_apexenginenext_MainActivity_nativeToggleCamera(JNIEnv*,jclass){std::lock_guard<std::mutex>l(mutex);cockpitCamera=!cockpitCamera;}
extern "C" JNIEXPORT void JNICALL Java_com_apexenginenext_MainActivity_nativeNextTrack(JNIEnv*,jclass){std::lock_guard<std::mutex>l(mutex);race.nextTrack(car);}
extern "C" JNIEXPORT void JNICALL Java_com_apexenginenext_MainActivity_nativeFrame(JNIEnv*,jclass,jfloat dt){std::lock_guard<std::mutex>l(mutex);if(display!=EGL_NO_DISPLAY&&surface!=EGL_NO_SURFACE)drawWorld(dt);}
extern "C" JNIEXPORT jfloat JNICALL Java_com_apexenginenext_MainActivity_nativeSpeed(JNIEnv*,jclass){return car.speed*3.6f;}
extern "C" JNIEXPORT jfloat JNICALL Java_com_apexenginenext_MainActivity_nativeSteering(JNIEnv*,jclass){return car.steeringAngle;}
extern "C" JNIEXPORT jfloat JNICALL Java_com_apexenginenext_MainActivity_nativeThrottle(JNIEnv*,jclass){return input.throttle;}
extern "C" JNIEXPORT jfloat JNICALL Java_com_apexenginenext_MainActivity_nativeBrake(JNIEnv*,jclass){return input.brake;}
extern "C" JNIEXPORT jfloat JNICALL Java_com_apexenginenext_MainActivity_nativeSteerInput(JNIEnv*,jclass){return input.steer;}
extern "C" JNIEXPORT jfloat JNICALL Java_com_apexenginenext_MainActivity_nativeOffTrackDistance(JNIEnv*,jclass){return race.offTrackDistance;}
extern "C" JNIEXPORT jfloat JNICALL Java_com_apexenginenext_MainActivity_nativeProgress(JNIEnv*,jclass){return race.playerProgress;}
extern "C" JNIEXPORT jfloat JNICALL Java_com_apexenginenext_MainActivity_nativeFps(JNIEnv*,jclass){return renderFps;}
extern "C" JNIEXPORT jint JNICALL Java_com_apexenginenext_MainActivity_nativeGear(JNIEnv*,jclass){return car.gear;}
extern "C" JNIEXPORT jfloat JNICALL Java_com_apexenginenext_MainActivity_nativeRpm(JNIEnv*,jclass){return car.rpm;}
extern "C" JNIEXPORT jfloat JNICALL Java_com_apexenginenext_MainActivity_nativeLateralG(JNIEnv*,jclass){return car.lateralAccel/9.81f;}
extern "C" JNIEXPORT jint JNICALL Java_com_apexenginenext_MainActivity_nativeLap(JNIEnv*,jclass){return race.lap;}
extern "C" JNIEXPORT jint JNICALL Java_com_apexenginenext_MainActivity_nativePosition(JNIEnv*,jclass){return race.position;}
extern "C" JNIEXPORT jfloat JNICALL Java_com_apexenginenext_MainActivity_nativeRaceTime(JNIEnv*,jclass){return race.finished?race.finishTime:race.raceTime;}
extern "C" JNIEXPORT jfloat JNICALL Java_com_apexenginenext_MainActivity_nativeGap(JNIEnv*,jclass){return race.gapToLeader;}
extern "C" JNIEXPORT jfloat JNICALL Java_com_apexenginenext_MainActivity_nativeInterval(JNIEnv*,jclass){return race.intervalToAhead;}
extern "C" JNIEXPORT jstring JNICALL Java_com_apexenginenext_MainActivity_nativeTimingTower(JNIEnv* env,jclass){
    struct Entry{float dist;int id;};
    Entry e[8]; e[0]={float(race.lap-1)+race.playerProgress,0};
    for(int i=0;i<7;i++) e[i+1]={float(race.ai[i].laps)+race.ai[i].progress,i+1};
    std::sort(e,e+8,[](const Entry&a,const Entry&b){return a.dist>b.dist;});
    const char* names[8]={"RIT","VEX","NOV","ARC","LYN","ORX","ZEN","KAI"};
    std::string out; float leader=e[0].dist;
    for(int row=0;row<8;row++){
        if(row)out+=';';
        float gap=std::max(0.0f,(leader-e[row].dist)*race.trackLength/std::max(8.0f,car.speed));
        float interval=row?std::max(0.0f,(e[row-1].dist-e[row].dist)*race.trackLength/std::max(8.0f,car.speed)):0.0f;
        char b[96];
        std::snprintf(b,sizeof(b),"%d,%s,%d,%.2f,%.2f,%d",row+1,names[e[row].id],e[row].id==0?race.position:0,gap,interval,e[row].id==0?race.positionDelta:0);
        out+=b;
    }
    return env->NewStringUTF(out.c_str());
}
extern "C" JNIEXPORT jboolean JNICALL Java_com_apexenginenext_MainActivity_nativeFinished(JNIEnv*,jclass){return race.finished?JNI_TRUE:JNI_FALSE;}
extern "C" JNIEXPORT jstring JNICALL Java_com_apexenginenext_MainActivity_nativeTrackName(JNIEnv*env,jclass){std::lock_guard<std::mutex>l(mutex);return env->NewStringUTF(race.trackName());}

extern "C" JNIEXPORT jint JNICALL Java_com_apexenginenext_MainActivity_nativeStartState(JNIEnv*,jclass){
    if(!race.started)return (jint)std::ceil(race.startTimer);
    if(race.goTimer>0)return -1;
    return 0;
}
