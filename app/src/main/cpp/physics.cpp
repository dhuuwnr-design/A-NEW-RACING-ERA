#include "physics.h"
namespace apex {
void step(Car& c,const Input& in,float dt){
    dt=clamp(dt,0.0f,0.05f); if(dt<=0)return;
    constexpr float g=9.81f,lf=1.48f,lr=1.32f,Iz=820.0f*2.8f*2.8f*0.22f;
    constexpr float Cf=105000.0f,Cr=125000.0f,mu=1.70f,cgHeight=0.32f,trackWidth=1.60f,maxBrake=14500.0f;
    const float throttle=clamp(in.throttle,0.0f,1.0f),brake=clamp(in.brake,0.0f,1.0f),steer=clamp(in.steer,-1.0f,1.0f);
    const float speedForSteer=std::max(0.0f,c.vx);
    const float maxSteer=clamp(0.48f/(1.0f+speedForSteer*0.024f),0.085f,0.48f);
    c.steeringAngle+=clamp(steer*maxSteer-c.steeringAngle,-2.8f*dt,2.8f*dt);
    const float vxf=std::max(2.0f,std::fabs(c.vx));
    const float aero=1.35f*vxf*vxf;
    const float totalLoad=c.mass*g+aero;
    c.downforce=aero; c.drag=0.0016f*vxf*vxf;
    const float longitudinalTransfer=c.mass*c.longitudinalAccel*cgHeight/c.wheelBase;
    const float frontAxle=clamp(totalLoad*c.frontWeight-longitudinalTransfer,0.08f*totalLoad,0.78f*totalLoad);
    const float rearAxle=std::max(0.08f*totalLoad,totalLoad-frontAxle);
    const float lateralTransferF=clamp(c.mass*c.lateralAccel*cgHeight/trackWidth,-0.70f*frontAxle*0.5f,0.70f*frontAxle*0.5f);
    const float lateralTransferR=clamp(c.mass*c.lateralAccel*cgHeight/trackWidth,-0.70f*rearAxle*0.5f,0.70f*rearAxle*0.5f);
    c.wheel[0].load=std::max(1.0f,frontAxle*0.5f-lateralTransferF);
    c.wheel[1].load=std::max(1.0f,frontAxle*0.5f+lateralTransferF);
    c.wheel[2].load=std::max(1.0f,rearAxle*0.5f-lateralTransferR);
    c.wheel[3].load=std::max(1.0f,rearAxle*0.5f+lateralTransferR);
    const float alphaF=std::atan2(c.vy+lf*c.yawRate,vxf)-c.steeringAngle;
    const float alphaR=std::atan2(c.vy-lr*c.yawRate,vxf);
    const float frontGrip=mu*(c.wheel[0].load+c.wheel[1].load),rearGrip=mu*(c.wheel[2].load+c.wheel[3].load);
    float fyF=clamp(-Cf*alphaF,-frontGrip,frontGrip),fyR=clamp(-Cr*alphaR,-rearGrip,rearGrip);
    const float driveForce=c.powertrain.step({throttle,brake},vxf,dt);
    c.rpm=c.powertrain.state().rpm;c.gear=c.powertrain.state().gear;c.engineTorque=c.powertrain.state().engineTorqueNm;
    c.drsActive=in.drs&&vxf>20.0f;
    float fxRear=driveForce-(c.drsActive?0.00055f*vxf*vxf:0.0f)-brake*maxBrake;
    const float rearLatRatio=std::fabs(fyR)/std::max(1.0f,rearGrip);
    const float rearLongCap=rearGrip*std::sqrt(std::max(0.0f,1.0f-rearLatRatio*rearLatRatio));
    fxRear=clamp(fxRear,-rearLongCap,rearLongCap);
    const float frontLong=-brake*maxBrake*0.5f;
    const float frontLatRatio=std::fabs(fyF)/std::max(1.0f,frontGrip);
    const float frontLongCap=frontGrip*0.5f*std::sqrt(std::max(0.0f,1.0f-frontLatRatio*frontLatRatio));
    const float fxFront=clamp(frontLong,-frontLongCap,frontLongCap);
    const float rolling=95.0f+8.0f*vxf;
    const float fxBody=fxRear+fxFront-c.drag-rolling;
    const float fyBody=fyF*std::cos(c.steeringAngle)+fyR;
    c.longitudinalAccel=fxBody/std::max(1.0f,c.mass);
    c.lateralAccel=fyBody/std::max(1.0f,c.mass);
    const float yawMoment=lf*fyF*std::cos(c.steeringAngle)-lr*fyR;
    c.yawRate+=(yawMoment/Iz)*dt;c.yawRate*=std::pow(0.985f,dt*60.0f);c.yaw+=c.yawRate*dt;
    const float sy=std::sin(c.yaw),cy=std::cos(c.yaw);
    c.vx=std::max(0.0f,c.vx+c.longitudinalAccel*dt);c.vy+=c.lateralAccel*dt;
    c.x+=(sy*c.vy+cy*c.vx)*dt;c.y+=(cy*c.vx-sy*c.vy)*dt;c.speed=std::sqrt(c.vx*c.vx+c.vy*c.vy);
    const float targetPitch=clamp(-c.longitudinalAccel*0.035f,-0.12f,0.12f),targetRoll=clamp(c.lateralAccel*0.018f,-0.14f,0.14f);
    c.pitch+=(targetPitch-c.pitch)*std::min(1.0f,dt*7.0f);c.roll+=(targetRoll-c.roll)*std::min(1.0f,dt*7.0f);
    for(int i=0;i<4;i++){
        const bool front=i<2;const float load=c.wheel[i].load;
        c.wheel[i].slipAngle=front?alphaF:alphaR;
        c.wheel[i].lateralForce=front?fyF*0.5f:fyR*0.5f;
        c.wheel[i].longitudinalForce=front?fxFront*0.5f:fxRear*0.5f;
        c.wheel[i].slipRatio=c.wheel[i].longitudinalForce/std::max(1.0f,mu*load);
    }
}
}
