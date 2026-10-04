#include "../app/src/main/cpp/physics.h"
#include "../app/src/main/cpp/race.h"
#include <cassert>
#include <cmath>
#include <cstdio>

int main(){
    apex::Car c; apex::Input in; in.throttle=1;
    for(int i=0;i<1200;i++) apex::step(c,in,1.0f/120.0f);
    assert(c.speed>35.0f&&c.speed<=105.0f);
    assert(c.wheel[2].load>0&&c.wheel[2].longitudinalForce>0);
    assert(std::fabs(c.steeringAngle)<.51f);
    assert(std::isfinite(c.pitch)&&std::isfinite(c.roll));
    assert(c.wheel[0].load>0&&c.wheel[3].load>0);

    float before=c.speed;in.throttle=0;in.brake=1;
    for(int i=0;i<240;i++) apex::step(c,in,1.0f/120.0f);
    assert(c.speed<before);

    float yaw0=c.yaw;in.brake=0;in.throttle=.65f;in.steer=.55f;
    for(int i=0;i<120;i++) apex::step(c,in,1.0f/120.0f);
    assert(std::fabs(c.yaw-yaw0)>.01f);
    assert(std::fabs(c.lateralAccel)>0.01f);
    assert(std::fabs(c.roll)>0.0001f);

    apex::Race race;race.reset(c);
    assert(race.points.size()==320&&race.ai.size()==7);
    assert(race.position==1);assert(!race.finished&&race.raceTime==0);
    assert(race.trackLength>6900.0f&&race.trackLength<7100.0f);
    assert(!race.started&&race.startTimer>2.9f);
    float minElevation=999,maxElevation=-999;
    for(const auto&p:race.points){minElevation=std::min(minElevation,p.elevation);maxElevation=std::max(maxElevation,p.elevation);assert(p.width>4.4f&&p.width<5.6f);}
    assert(maxElevation-minElevation>2.0f);
    assert(std::fabs(race.playerElevation-race.points[0].elevation)<.001f);

    for(int i=0;i<240;i++){apex::Input t;t.throttle=1;apex::step(c,t,1.0f/60.0f);race.update(c,1.0f/60.0f);}
    assert(race.started&&race.raceTime>0);
    for(int i=0;i<360;i++){apex::Input t;t.throttle=1;apex::step(c,t,1.0f/60.0f);race.update(c,1.0f/60.0f);}
    assert(race.position>=1&&race.position<=8);
    for(const auto&a:race.ai){assert(std::isfinite(a.elevation));assert(std::isfinite(a.speed));assert(std::fabs(a.lineOffset)<2.0f);}

    std::printf("APEX_NEXT_TRACK_0.9.2_TEST PASS speed=%.2f yaw=%.3f elevation=%.2f..%.2f\n",c.speed,c.yaw,minElevation,maxElevation);
}
