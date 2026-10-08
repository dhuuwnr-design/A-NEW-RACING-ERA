#include "../app/src/main/cpp/physics.h"
#include "../app/src/main/cpp/race.h"
#include <cassert>
#include <cmath>
#include <cstdio>
int main(){
 apex::Car c; apex::Input in; in.throttle=1;
 for(int i=0;i<1200;i++) apex::step(c,in,1.0f/120.0f);
 assert(c.speed>35.0f&&c.speed<130.0f);
 assert(c.wheel[2].load>0&&c.wheel[2].longitudinalForce>0);
 assert(c.rpm>3500.0f&&c.rpm<=15000.0f&&c.gear>=1&&c.gear<=8);
 assert(std::isfinite(c.engineTorque));
 float before=c.speed; in.throttle=0;in.brake=1;
 for(int i=0;i<240;i++) apex::step(c,in,1.0f/120.0f);
 assert(c.speed<before);
 float yaw0=c.yaw;in.brake=0;in.throttle=.65f;in.steer=.55f;
 for(int i=0;i<120;i++) apex::step(c,in,1.0f/120.0f);
 assert(std::fabs(c.yaw-yaw0)>.01f&&std::fabs(c.lateralAccel)>.01f&&std::fabs(c.roll)>.0001f);
 assert(std::isfinite(c.vy)&&std::fabs(c.vy)<std::max(8.0f,c.speed*.20f));
 const float yawTurning=c.yaw; in.steer=0;
 for(int i=0;i<240;i++) apex::step(c,in,1.0f/120.0f);
 assert(std::isfinite(c.vy)&&std::fabs(c.vy)<std::max(5.0f,c.speed*.12f));
 assert(std::fabs(c.yaw-yawTurning)<2.5f);
 apex::Race race;race.reset(c);
 assert(race.points.size()==320&&race.ai.size()==7&&race.position==1&&!race.finished&&race.raceTime==0);
 assert(race.trackLength>6900.0f&&race.trackLength<7100.0f);
 for(int i=0;i<240;i++){apex::Input t;t.throttle=1;apex::step(c,t,1.0f/60.0f);race.update(c,1.0f/60.0f);}
 assert(race.started&&race.raceTime>0);
 for(int i=0;i<360;i++){apex::Input t;t.throttle=1;apex::step(c,t,1.0f/60.0f);race.update(c,1.0f/60.0f);}
 assert(race.position>=1&&race.position<=8);
 std::printf("APEX_NEXT_PHYSICS_POWERTRAIN_TEST PASS speed=%.2f rpm=%.0f gear=%d yaw=%.3f\n",c.speed,c.rpm,c.gear,c.yaw);
}
