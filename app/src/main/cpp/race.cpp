#include "race.h"
#include "real_tracks.h"
#include "track_profiles.h"
#include <algorithm>

namespace apex {

static std::vector<TrackPoint> buildTrack(int trackIndex){
    const auto& raw=REAL_TRACKS[trackIndex%REAL_TRACK_COUNT];
    // Normalize the open-source geometry to the published circuit centreline length,
    // while retaining the source curvature and measured track widths.
    static constexpr float officialLength[REAL_TRACK_COUNT]={7004.0f,5891.0f,5793.0f,5807.0f,5412.0f,4309.0f,5513.0f};
    // raw.length is the source metadata; compute the actual closed polyline length
    // so normalization cannot inherit a source-length rounding error.
    float measuredLength=0.0f;
    for(int i=0;i<raw.count;i++){
        const float* a=&raw.data[i*3];
        const float* b=&raw.data[((i+1)%raw.count)*3];
        measuredLength+=std::hypot(b[0]-a[0],b[1]-a[1]);
    }
    const float scale=officialLength[trackIndex%REAL_TRACK_COUNT]/std::max(1.0f,measuredLength);
    std::vector<TrackPoint> out; out.reserve(raw.count);
    for(int i=0;i<raw.count;i++){
        const float* d=&raw.data[i*3];
        const float* dn=&raw.data[((i+1)%raw.count)*3];
        const float* dp=&raw.data[((i+raw.count-1)%raw.count)*3];
        TrackPoint p{};
        p.x=d[0]*scale; p.z=d[1]*scale; p.width=d[2];
        const float dx=dn[0]-dp[0], dz=dn[1]-dp[1], len=std::max(.001f,std::sqrt(dx*dx+dz*dz));
        p.tx=dx/len; p.tz=dz/len;
        p.progress=float(i)/float(raw.count);
        // Elevation envelope is circuit-specific. It is anchored to published
        // elevation-change figures; telemetry-derived profiles can replace it later.
        p.elevation=trackElevation(trackIndex,p.progress);
        out.push_back(p);
    }
    return out;
}
void Race::rebuildTrackMetrics(){
 trackLength=0.0f;
 if(points.empty()){trackLength=1.0f;return;}
 for(size_t i=0;i<points.size();i++){const auto&a=points[i],&b=points[(i+1)%points.size()];trackLength+=std::hypot(b.x-a.x,b.z-a.z);}
 trackLength=std::max(1.0f,trackLength);
 float cumulative=0.0f; points[0].progress=0.0f;
 for(size_t i=1;i<points.size();i++){const auto&a=points[i-1],&b=points[i];cumulative+=std::hypot(b.x-a.x,b.z-a.z);points[i].progress=cumulative/trackLength;}
}
Race::Race():points(buildTrack(trackIndex)),ai(7){rebuildTrackMetrics();}
void Race::reset(Car&car){auto&p=points[0];car.x=p.x;car.y=p.z;car.yaw=std::atan2(p.tx,p.tz);car.speed=0;car.vx=0;car.vy=0;car.yawRate=0;playerProgress=lastProgress=0;playerElevation=p.elevation;offTrackDistance=dist;raceTime=0;finishTime=-1;startTimer=3;goTimer=0;lap=1;position=1;previousPosition=1;positionDelta=0;playerPoint=0;started=false;finished=false;for(int i=0;i<(int)ai.size();i++){ai[i].progress=.012f+float(i)*.006f;ai[i].laps=0;ai[i].previousProgress=ai[i].progress;ai[i].lapTime=0;ai[i].lastLapTime=0;ai[i].bestLapTime=-1;ai[i].finished=false;ai[i].speed=29+float(i%3)*1.6f;ai[i].lineOffset=(float(i)-3)*.38f;ai[i].elevation=points[int(ai[i].progress*points.size())%points.size()].elevation;}}
const char* Race::trackName() const { return REAL_TRACKS[trackIndex%REAL_TRACK_COUNT].name; }

void Race::nextTrack(Car& car){
    trackIndex=(trackIndex+1)%REAL_TRACK_COUNT;
    points=buildTrack(trackIndex);
    rebuildTrackMetrics();
    reset(car);
}

void Race::update(Car&car,float dt){
 if(!started){startTimer-=dt;if(startTimer<=0){startTimer=0;started=true;goTimer=1;}else return;} if(goTimer>0)goTimer=std::max(0.0f,goTimer-dt);if(!finished)raceTime+=dt;completedLaps=started&&!finished?std::max(0,lap-1):completedLaps;
 const int count=int(points.size());const int searchRadius=28;
 int bestSeg=playerPoint; float bestDistSq=1e30f; float bestT=0.0f;
 for(int off=-searchRadius;off<=searchRadius;off++){
     int i=(playerPoint+off+count)%count, j=(i+1)%count;
     const auto&a=points[i]; const auto&b=points[j];
     const float sx=b.x-a.x, sz=b.z-a.z, segLenSq=sx*sx+sz*sz;
     const float t=segLenSq>1e-6f?clamp(((car.x-a.x)*sx+(car.y-a.z)*sz)/segLenSq,0.0f,1.0f):0.0f;
     const float qx=a.x+sx*t, qz=a.z+sz*t, dx=car.x-qx, dz=car.y-qz, d=dx*dx+dz*dz;
     if(d<bestDistSq){bestDistSq=d;bestSeg=i;bestT=t;}
 }
 playerPoint=bestSeg;
 const auto&segA=points[bestSeg]; const auto&segB=points[(bestSeg+1)%count];
 const float span=bestSeg==count-1?1.0f-segA.progress:segB.progress-segA.progress;
 playerProgress=segA.progress+span*bestT; playerProgress-=std::floor(playerProgress);
 playerElevation=segA.elevation+(segB.elevation-segA.elevation)*bestT;
 const float dist=std::sqrt(bestDistSq);
 if(dist>7){
     const float pull=std::min(.18f,dt*2.5f);
     const float qx=segA.x+(segB.x-segA.x)*bestT, qz=segA.z+(segB.z-segA.z)*bestT;
     car.x+=(qx-car.x)*pull; car.y+=(qz-car.y)*pull;
     car.speed*=std::max(0.0f,1-dt*1.8f);
 }
 if(lastProgress>.9f&&playerProgress<.1f&&car.speed>5){if(lap<totalLaps)lap++;else{finished=true;finishTime=raceTime;}}lastProgress=playerProgress;
 for(auto&a:ai){a.previousProgress=a.progress;a.progress+=a.speed*dt/trackLength;a.lapTime+=dt;if(a.progress>=1){a.progress-=1;a.laps++;a.lastLapTime=a.lapTime;a.lapTime=0;if(a.bestLapTime<0||a.lastLapTime<a.bestLapTime)a.bestLapTime=a.lastLapTime;if(a.laps>=totalLaps)a.finished=true;}int j=int(a.progress*points.size())%int(points.size()),jm=(j+points.size()-2)%int(points.size()),jp=(j+2)%int(points.size()),jl=(j+12)%int(points.size());auto&q=points[j],&qm=points[jm],&qp=points[jp],&look=points[jl];float local=1-std::clamp(qm.tx*qp.tx+qm.tz*qp.tz,-1.0f,1.0f),up=1-std::clamp(q.tx*look.tx+q.tz*look.tz,-1.0f,1.0f),curv=std::max(local,up*.82f);float target=42-std::min(24.0f,curv*42);float response=a.speed>target?4.4f:2.0f;a.speed+=(target-a.speed)*std::min(1.0f,dt*response);float off=a.lineOffset,pa=playerProgress-a.progress;if(pa>0&&pa<.045f)off+=(a.lineOffset>=0?.38f:-.38f);float nx=q.tz,nz=-q.tx,offset=std::clamp(off,-q.width*.55f,q.width*.55f);a.x=q.x+nx*offset;a.z=q.z+nz*offset;a.yaw=std::atan2(q.tx,q.tz);a.elevation=q.elevation;}
 float playerDist=float(lap-1)+playerProgress,leaderDist=playerDist,ahead=1e9f;for(auto&a:ai){float d=float(a.laps)+a.progress;leaderDist=std::max(leaderDist,d);if(d>playerDist)ahead=std::min(ahead,d);}previousPosition=position;position=1;for(auto&a:ai){float d=float(a.laps)+a.progress;if(d>playerDist)position++;}float s=std::max(8.0f,car.speed);gapToLeader=std::max(0.0f,(leaderDist-playerDist)*trackLength/s);intervalToAhead=ahead<1e8f?std::max(0.0f,(ahead-playerDist)*trackLength/s):0;positionDelta=previousPosition-position;
}
}
