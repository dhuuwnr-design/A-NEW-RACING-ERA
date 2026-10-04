#include "physics.h"
namespace apex {
void step(Car& c,const Input& in,float dt){
 dt=clamp(dt,0.0f,0.05f); if(dt<=0)return;
 constexpr float g=9.81f,lf=1.48f,lr=1.32f,Iz=820.0f*2.8f*2.8f*0.22f,Cf=105000.0f,Cr=125000.0f,mu=1.70f,cgHeight=0.32f,trackWidth=1.60f,maxDrive=10500.0f,maxBrake=14500.0f;
 const float throttle=clamp(in.throttle,0.0f,1.0f),brake=clamp(in.brake,0.0f,1.0f),steer=clamp(in.steer,-1.0f,1.0f);
 const float speedForSteer=std::max(0.0f,c.vx);
 const float maxSteer=clamp(0.48f/(1.0f+speedForSteer*0.024f),0.085f,0.48f);
 c.steeringAngle+=clamp(steer*maxSteer-c.steeringAngle,-2.8f*dt,2.8f*dt);
 const float vxf=std::max(2.0f,std::fabs(c.vx));
 const float aero=1.35f*vxf*vxf,totalLoad=c.mass*g+aero; c.downforce=aero;c.drag=0.0016f*vxf*vxf;
 const float longitudinalTransfer=c.mass*c.longitudinalAccel*cgHeight/c.wheelBase;
 const float frontAxle=clamp(totalLoad*c.frontWeight-longitudinalTransfer,0.08f*totalLoad,0.78f*totalLoad),rearAxle=std::max(0.08f*totalLoad,totalLoad-frontAxle);
 const float latTransferF=clamp(c.mass*c.lateralAccel*cgHeight/trackWidth,-0.70f*frontAxle*0.5f,0.70f*frontAxle*0.5f),latTransferR=clamp(c.mass*c.lateralAccel*cgHeight/trackWidth,-0.70f*rearAxle*0.5f,0.70f*rearAxle*0.5f);
 c.wheel[0].load=std::max(1.0f,frontAxle*0.5f-latTransferF);c.wheel[1].load=std::max(1.0f,frontAxle*0.5f+latTransferF);c.wheel[2].load=std::max(1.0f,rearAxle*0.5f-latTransferR);c.wheel[3].load=std::max(1.0f,rearAxle*0.5f+latTransferR);
 const float alphaF=std::atan2(c.vy+lf*c.yawRate,vxf)-c.steeringAngle,alphaR=std::atan2(c.vy-lr*c.yawRate,vxf);
 const float frontGrip=mu*(c.wheel[0].load+c.wheel[1].load),rearGrip=mu*(c.wheel[2].load+c.wheel[3].load);
 float fyF=clamp(-Cf*alphaF,-frontGrip,frontGrip),fyR=clamp(-Cr*alphaR,-rearGrip,rearGrip);
 const float rolling=95.0f+8.0f*vxf,requestedDrive=maxDrive*throttle,requestedBrake=maxBrake*brake;
 float fxR=requestedDrive-requestedBrake*rearAxle/totalLoad,fxF=-requestedBrake*frontAxle/totalLoad;
 const float rearLongLimit=std::sqrt(std::max(0.0f,rearGrip*rearGrip-fyR*fyR)),frontLongLimit=std::sqrt(std::max(0.0f,frontGrip*frontGrip-fyF*fyF));
 fxR=clamp(fxR,-rearLongLimit,rearLongLimit);fxF=clamp(fxF,-frontLongLimit,frontLongLimit);
 for(int i=0;i<4;i++){const bool front=i<2;const float load=c.wheel[i].load,grip=mu*load,lat=front?fyF*.5f:fyR*.5f,lon=front?fxF*.5f:fxR*.5f;c.wheel[i].slipAngle=front?alphaF:alphaR;c.wheel[i].lateralForce=lat;c.wheel[i].longitudinalForce=lon;c.wheel[i].slipRatio=lon/std::max(1.0f,grip);}
 const float fx=fxF+fxR,fyBody=fyR+fyF*std::cos(c.steeringAngle),fxBody=fx*std::cos(c.steeringAngle)-fyF*std::sin(c.steeringAngle);
 c.longitudinalAccel=(fxBody-c.drag-rolling)/std::max(1.0f,c.mass);c.lateralAccel=(fyBody-c.mass*c.vx*c.yawRate)/std::max(1.0f,c.mass);
 c.vx=std::max(0.0f,c.vx+c.longitudinalAccel*dt);c.vy+=c.lateralAccel*dt;
 c.yawRate+=(lf*fyF*std::cos(c.steeringAngle)-lr*fyR)/Iz*dt;c.yawRate*=std::pow(0.985f,dt*60.0f);c.yaw+=c.yawRate*dt;
 const float sy=std::sin(c.yaw),cy=std::cos(c.yaw);c.x+=(sy*c.vx+cy*c.vy)*dt;c.y+=(cy*c.vx-sy*c.vy)*dt;c.speed=std::sqrt(c.vx*c.vx+c.vy*c.vy);
 const float targetPitch=clamp(-c.longitudinalAccel*.035f,-.12f,.12f),targetRoll=clamp(c.lateralAccel*.018f,-.14f,.14f);c.pitch+=(targetPitch-c.pitch)*std::min(1.0f,dt*7.0f);c.roll+=(targetRoll-c.roll)*std::min(1.0f,dt*7.0f);
}}
