package com.apexenginenext;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.view.*;
import android.widget.FrameLayout;
import android.graphics.*;

public final class MainActivity extends Activity implements SurfaceHolder.Callback {
    private GameView game; private Ui ui;
    static { System.loadLibrary("apex"); }

    @Override public void onCreate(Bundle s){
        super.onCreate(s); setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        getWindow().setNavigationBarColor(Color.BLACK);
        FrameLayout root=new FrameLayout(this);
        game=new GameView(); game.getHolder().addCallback(this); ui=new Ui();
        root.addView(game,new FrameLayout.LayoutParams(-1,-1)); root.addView(ui,new FrameLayout.LayoutParams(-1,-1));
        setContentView(root);
    }
    @Override public void surfaceCreated(SurfaceHolder h){nativeStart(h.getSurface(),getAssets());game.running=true;game.post(game.frame);ui.loading();}
    @Override public void surfaceDestroyed(SurfaceHolder h){game.running=false;nativeStop();}
    @Override public void surfaceChanged(SurfaceHolder h,int f,int w,int z){nativeResize(w,z);}

    private static native void nativeStart(Surface s,android.content.res.AssetManager a);
    private static native void nativeStop(); private static native void nativeResize(int w,int h);
    private static native void nativeFrame(float dt); private static native void nativeToggleCamera();
    private static native String nativeTrackName(); private static native void nativeTouch(float s,float t,float b);
    private static native float nativeSpeed(); private static native float nativeFps(); private static native int nativeGear(); private static native float nativeRpm();
    private static native int nativeLap(); private static native int nativePosition();
    private static native boolean nativeFinished();

    final class GameView extends SurfaceView {
        boolean running;
        final Runnable frame=new Runnable(){long last=System.nanoTime();public void run(){
            if(!running)return; long n=System.nanoTime(); float dt=Math.min(.05f,(n-last)*1e-9f);last=n;
            nativeFrame(dt);postOnAnimation(this);
        }};
        GameView(){super(MainActivity.this);setFocusable(true);}
        boolean hit(float x,float y,float l,float t,float r,float b){return x>=l&&x<=r&&y>=t&&y<=b;}
        @Override public boolean onTouchEvent(MotionEvent e){
            int a=e.getActionMasked();
            if(a==MotionEvent.ACTION_CANCEL || a==MotionEvent.ACTION_UP){nativeTouch(0,0,0);return true;}
            float w=getWidth(),h=getHeight(),ss=Math.min(w*.115f,h*.19f),gap=Math.min(w*.018f,h*.025f),sy=h*.765f;
            float ll=w*.035f,lr=ll+ss,rl=lr+gap,rr=rl+ss,pl=w*.795f,pr=w*.955f;
            float st=0,th=0,br=0;
            for(int i=0;i<e.getPointerCount();i++){float x=e.getX(i),y=e.getY(i);
                if(hit(x,y,ll,sy,lr,h*.96f))st=-1;
                if(hit(x,y,rl,sy,rr,h*.96f))st=1;
                if(hit(x,y,pl,h*.60f,pr,h*.77f))th=1;
                if(hit(x,y,pl,h*.79f,pr,h*.96f))br=1;
            }
            nativeTouch(st,th,br);return true;
        }
    }

    final class Ui extends View {
        final Paint p=new Paint(Paint.ANTI_ALIAS_FLAG); int screen=0; long loadAt;
        final int RED=0xffe31b2d, WHITE=0xfff4f5f7, MUTED=0xff9aa3ad, PANEL=0xdd0b1016, LINE=0xff303944;
        Ui(){super(MainActivity.this);setLayerType(View.LAYER_TYPE_SOFTWARE,null);}
        void loading(){screen=0;loadAt=System.currentTimeMillis();postDelayed(()->{screen=1;invalidate();},1500);}
        void text(Canvas c,String s,float x,float y,float size,int color,boolean bold){p.setStyle(Paint.Style.FILL);p.setColor(color);p.setTextSize(size);p.setTypeface(bold?Typeface.DEFAULT_BOLD:Typeface.DEFAULT);c.drawText(s,x,y,p);}
        void box(Canvas c,float l,float t,float r,float b,float rad,int fill,int stroke){p.setStyle(Paint.Style.FILL);p.setColor(fill);c.drawRoundRect(l,t,r,b,rad,rad,p);if(stroke!=0){p.setStyle(Paint.Style.STROKE);p.setStrokeWidth(1.5f);p.setColor(stroke);c.drawRoundRect(l,t,r,b,rad,rad,p);p.setStyle(Paint.Style.FILL);}}
        boolean hit(float x,float y,float l,float t,float r,float b){return x>=l&&x<=r&&y>=t&&y<=b;}
        void car(Canvas c,float x,float y,float s){p.setColor(RED);Path q=new Path();q.moveTo(x-130*s,y+8*s);q.lineTo(x-65*s,y-25*s);q.lineTo(x-28*s,y-31*s);q.lineTo(x-12*s,y-47*s);q.lineTo(x+12*s,y-47*s);q.lineTo(x+28*s,y-31*s);q.lineTo(x+65*s,y-25*s);q.lineTo(x+130*s,y+8*s);q.lineTo(x+68*s,y+3*s);q.lineTo(x-68*s,y+3*s);q.close();c.drawPath(q,p);p.setColor(0xff07090b);c.drawCircle(x-80*s,y+25*s,27*s,p);c.drawCircle(x+80*s,y+25*s,27*s,p);p.setColor(WHITE);c.drawRect(x-136*s,y+14*s,x+136*s,y+20*s,p);}
        void header(Canvas c,float w,float h){text(c,"APEX",w*.045f,h*.095f,43,WHITE,true);text(c,"ENGINE NEXT",w*.17f,h*.095f,16,0xffb8c0c9,true);text(c,"A NEW RACING ERA",w*.045f,h*.135f,10,0xff727c88,true);}
        void loadingDraw(Canvas c){float w=getWidth(),h=getHeight();p.setShader(new LinearGradient(0,0,w,h,0xff020407,0xff111820,Shader.TileMode.CLAMP));c.drawRect(0,0,w,h,p);p.setShader(null);header(c,w,h);car(c,w*.59f,h*.51f,1.25f);text(c,"RACING SYSTEMS",w*.045f,h*.70f,11,0xff7e8994,true);p.setColor(0xff313943);c.drawRect(w*.045f,h*.73f,w*.72f,h*.736f,p);float pr=Math.min(1f,(System.currentTimeMillis()-loadAt)/1500f);p.setColor(RED);c.drawRect(w*.045f,h*.73f,w*.045f+w*.675f*pr,h*.736f,p);text(c,(int)(pr*100)+"%",w*.75f,h*.742f,13,WHITE,true);text(c,"INITIALIZING TRACK / PHYSICS / RACE CONTROL",w*.045f,h*.80f,12,MUTED,true);}
        void menuButton(Canvas c,String label,String sub,float l,float t,float r,float b,boolean active){box(c,l,t,r,b,8,active?0xff151b22:PANEL,active?RED:LINE);if(active){p.setColor(RED);c.drawRect(l,t,l+4,b,p);}text(c,label,l+20,t+(b-t)*.43f,17,active?WHITE:0xff6f7882,true);text(c,sub,l+20,t+(b-t)*.74f,9,active?0xffaeb7c0:0xff5f6872,true);if(!active)text(c,"LOCKED",r-58,t+(b-t)*.56f,8,0xff606a74,true);}
        void menuDraw(Canvas c){float w=getWidth(),h=getHeight();p.setShader(new LinearGradient(0,0,w,h,0xff030507,0xff131a22,Shader.TileMode.CLAMP));c.drawRect(0,0,w,h,p);p.setShader(null);header(c,w,h);text(c,"RACE",w*.045f,h*.205f,28,WHITE,true);text(c,"CHOOSE YOUR NEXT SESSION",w*.045f,h*.235f,10,MUTED,true);float l=w*.045f,r=w*.36f,bh=h*.105f,g=h*.022f,t=h*.275f;menuButton(c,"QUICK RACE","START A GRAND PRIX",l,t,r,t+bh,true);menuButton(c,"CAREER","PROGRESSION",l,t+bh+g,r,t+2*bh+g,false);menuButton(c,"GARAGE","CAR & SETUP",l,t+2*(bh+g),r,t+3*bh+2*g,false);menuButton(c,"MULTIPLAYER","ONLINE RACING",l,t+3*(bh+g),r,t+4*bh+3*g,false);box(c,w*.42f,h*.22f,w*.96f,h*.86f,12,0xdd090e14,0xff303943);text(c,"NEXT GRAND PRIX",w*.46f,h*.29f,10,RED,true);text(c,nativeTrackName(),w*.46f,h*.37f,31,WHITE,true);text(c,"5 LAPS   •   DRY   •   QUICK RACE",w*.46f,h*.415f,10,MUTED,true);car(c,w*.68f,h*.58f,1.25f);p.setColor(RED);c.drawRect(w*.46f,h*.69f,w*.78f,h*.695f,p);text(c,"CONVENTIONAL TOUCH CONTROLS",w*.46f,h*.785f,9,MUTED,true);box(c,w*.78f,h*.73f,w*.92f,h*.81f,7,RED,0);text(c,"RACE  ›",w*.81f,h*.782f,12,WHITE,true);text(c,"LEVEL 01",w*.76f,h*.10f,10,MUTED,true);text(c,"12,450 CR",w*.85f,h*.10f,12,0xffffc844,true);text(c,"SETTINGS  ⚙",w*.045f,h*.89f,11,WHITE,true);}
        void raceHud(Canvas c){float w=getWidth(),h=getHeight();box(c,w*.025f,h*.035f,w*.23f,h*.125f,7,0xcc080d13,0xff39434d);text(c,"P"+nativePosition(),w*.045f,h*.078f,24,WHITE,true);text(c,"LAP "+nativeLap()+"/5",w*.12f,h*.070f,13,WHITE,true);text(c,nativeTrackName(),w*.12f,h*.102f,9,MUTED,true);box(c,w*.38f,h*.025f,w*.62f,h*.105f,7,0xcc080d13,0xff39434d);text(c,"RACE",w*.405f,h*.057f,9,RED,true);text(c,"APEX GP",w*.405f,h*.085f,12,WHITE,true);text(c,"DRY",w*.555f,h*.057f,9,0xff66d47a,true);text(c,"120 HZ SIM",w*.535f,h*.085f,8,MUTED,true);box(c,w*.73f,h*.025f,w*.965f,h*.145f,9,0xcc080d13,0xff39434d);text(c,"GEAR",w*.755f,h*.058f,8,MUTED,true);text(c,""+nativeGear(),w*.752f,h*.112f,35,WHITE,true);text(c,String.format(java.util.Locale.US,"%.0f",nativeSpeed()),w*.835f,h*.088f,27,WHITE,true);text(c,"KM/H",w*.895f,h*.118f,8,MUTED,true);float rpm=Math.max(0,Math.min(1,nativeRpm()/15000f));p.setColor(0xff2b333d);c.drawRect(w*.835f,h*.128f,w*.94f,h*.134f,p);p.setColor(rpm>.90f?0xffffc33d:RED);c.drawRect(w*.835f,h*.128f,w*.835f+w*.105f*rpm,h*.134f,p);box(c,w*.025f,h*.145f,w*.105f,h*.205f,8,0xaa080d13,0xff39434d);text(c,"CAM",w*.048f,h*.180f,10,WHITE,true);float ss=Math.min(w*.115f,h*.19f),gap=Math.min(w*.018f,h*.025f),sy=h*.765f,ll=w*.035f,lr=ll+ss,rl=lr+gap,rr=rl+ss;box(c,ll,sy,lr,h*.96f,18,0x66313a44,0xff737e89);box(c,rl,sy,rr,h*.96f,18,0x66313a44,0xff737e89);text(c,"‹",ll+ss*.34f,sy+(h*.96f-sy)*.66f,35,WHITE,true);text(c,"›",rl+ss*.34f,sy+(h*.96f-sy)*.66f,35,WHITE,true);float pl=w*.795f,pr=w*.955f;box(c,pl,h*.60f,pr,h*.77f,16,0x66313a44,0xff737e89);box(c,pl,h*.79f,pr,h*.96f,16,0x66313a44,0xff737e89);text(c,"THROTTLE",pl+10,h*.70f,10,WHITE,true);text(c,"BRAKE",pl+22,h*.89f,10,WHITE,true);if(nativeFinished()){box(c,w*.38f,h*.18f,w*.62f,h*.26f,8,0xdd080d13,RED);text(c,"FINISH  P"+nativePosition(),w*.435f,h*.235f,18,0xffffd34d,true);}}
        @Override protected void onDraw(Canvas c){super.onDraw(c);float w=getWidth(),h=getHeight();if(screen==0){loadingDraw(c);return;}if(screen==1){menuDraw(c);return;}if(screen==3){c.drawColor(0xff070a0e);header(c,w,h);text(c,"SETTINGS",w*.045f,h*.20f,27,WHITE,true);box(c,w*.045f,h*.27f,w*.72f,h*.78f,10,PANEL,LINE);text(c,"DRIVING",w*.08f,h*.35f,11,RED,true);text(c,"CONVENTIONAL TOUCH CONTROLS",w*.08f,h*.405f,17,WHITE,true);text(c,"LEFT / RIGHT   •   THROTTLE / BRAKE",w*.08f,h*.445f,11,MUTED,true);text(c,"LANDSCAPE LOCKED",w*.08f,h*.50f,11,MUTED,true);text(c,"CAMERA",w*.08f,h*.59f,11,RED,true);text(c,"Tap CAM during a race to switch view",w*.08f,h*.64f,13,WHITE,true);box(c,w*.08f,h*.69f,w*.30f,h*.755f,7,RED,0);text(c,"BACK",w*.16f,h*.732f,12,WHITE,true);return;}raceHud(c);}
        @Override public boolean onTouchEvent(MotionEvent e){if(e.getActionMasked()!=MotionEvent.ACTION_UP)return screen!=2;float x=e.getX(),y=e.getY(),w=getWidth(),h=getHeight();if(screen==1){float l=w*.045f,r=w*.36f,bh=h*.105f,g=h*.022f,t=h*.275f;int index=(int)((y-t)/(bh+g));if(index>=0&&index<4){float a=t+index*(bh+g);if(hit(x,y,l,a,r,a+bh)){if(index==0)screen=2;invalidate();return true;}}if(hit(x,y,w*.045f,h*.84f,w*.22f,h*.94f)){screen=3;invalidate();return true;}if(hit(x,y,w*.78f,h*.72f,w*.94f,h*.84f)){screen=2;invalidate();return true;}return true;}if(screen==3){if(hit(x,y,w*.07f,h*.66f,w*.34f,h*.80f)){screen=1;invalidate();}return true;}if(screen==2&&x>w*.02f&&x<w*.12f&&y<h*.23f){nativeToggleCamera();return true;}return false;}
    }
}