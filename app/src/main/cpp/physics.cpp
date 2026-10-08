#include "physics.h"
namespace apex {

static float tireForce(float slipAngle,float cornerStiffness,float availableGrip){
    const float linear=cornerStiffness*slipAngle;
    const float normalized=linear/std::max(1.0f,availableGrip);
    return -availableGrip*(normalized/(1.0f+std::fabs(normalized)));
}

void step(Car& c,const Input& in,float dt){
    dt=clamp(dt,0.0f,0.05f); if(dt<=0)return;
    constexpr float g=9.81f,lf=1.48f,lr=1.32f;
    constexpr float Iz=820.0f*2.8f*2.8f*0.22f;
    constexpr float Cf=118000.0f,Cr=132000.0f,mu=1.62f;
    constexpr float cgHeight=0.32f,trackWidth=1.60f,maxBrake=11500.0f;
    constexpr float airDensity=1.225f,dragArea=1.02f;

    const float throttle=clamp(in.throttle,0.0f,1.0f);
    const float brake=clamp(in.brake,0.0f,1.0f);
    const float steer=clamp(in.steer,-1.0f,1.0f);
    const float forwardSpeed=std::max(0.5f,c.vx);

    // Responsive at low speed, progressively calmer at high speed.
    const float maxSteer=clamp(0.55f/(1.0f+forwardSpeed*0.018f),0.105f,0.55f);
    const float steerRate=steer==0.0f?4.5f:3.8f;
    c.steeringAngle+=clamp(steer*maxSteer-c.steeringAngle,-steerRate*dt,steerRate*dt);

    // The previous drag term was far too small, producing unrealistic speed and
    // making the car impossible to judge against braking/cornering.
    const float aero=1.35f*forwardSpeed*forwardSpeed;
    const float baseDrag=0.5f*airDensity*dragArea*forwardSpeed*forwardSpeed;
    c.downforce=aero;
    c.drsActive=in.drs&&forwardSpeed>20.0f;
    const float aeroDrag=baseDrag*(c.drsActive?0.86f:1.0f);

    const float totalLoad=c.mass*g+aero;
    const float longitudinalTransfer=c.mass*c.longitudinalAccel*cgHeight/c.wheelBase;
    const float frontAxle=clamp(totalLoad*c.frontWeight-longitudinalTransfer,0.12f*totalLoad,0.76f*totalLoad);
    const float rearAxle=std::max(0.12f*totalLoad,totalLoad-frontAxle);
    const float lateralTransferF=clamp(c.mass*c.lateralAccel*cgHeight/trackWidth,-0.65f*frontAxle*0.5f,0.65f*frontAxle*0.5f);
    const float lateralTransferR=clamp(c.mass*c.lateralAccel*cgHeight/trackWidth,-0.65f*rearAxle*0.5f,0.65f*rearAxle*0.5f);

    c.wheel[0].load=std::max(1.0f,frontAxle*0.5f-lateralTransferF);
    c.wheel[1].load=std::max(1.0f,frontAxle*0.5f+lateralTransferF);
    c.wheel[2].load=std::max(1.0f,rearAxle*0.5f-lateralTransferR);
    c.wheel[3].load=std::max(1.0f,rearAxle*0.5f+lateralTransferR);

    const float alphaF=std::atan2(c.vy+lf*c.yawRate,forwardSpeed)-c.steeringAngle;
    const float alphaR=std::atan2(c.vy-lr*c.yawRate,forwardSpeed);
    const float frontGrip=mu*(c.wheel[0].load+c.wheel[1].load);
    const float rearGrip=mu*(c.wheel[2].load+c.wheel[3].load);
    float fyF=tireForce(alphaF,Cf,frontGrip);
    float fyR=tireForce(alphaR,Cr,rearGrip);

    const float driveForce=c.powertrain.step({throttle,brake},forwardSpeed,dt);
    c.rpm=c.powertrain.state().rpm;
    c.gear=c.powertrain.state().gear;
    c.engineTorque=c.powertrain.state().engineTorqueNm;

    // Rear-wheel drive with front-biased braking.
    float fxRear=driveForce-aeroDrag-brake*maxBrake*0.42f;
    float fxFront=-brake*maxBrake*0.58f;
    const float rearLatRatio=std::fabs(fyR)/std::max(1.0f,rearGrip);
    const float frontLatRatio=std::fabs(fyF)/std::max(1.0f,frontGrip);
    const float rearLongCap=rearGrip*std::sqrt(std::max(0.0f,1.0f-rearLatRatio*rearLatRatio));
    const float frontLongCap=frontGrip*std::sqrt(std::max(0.0f,1.0f-frontLatRatio*frontLatRatio));
    fxRear=clamp(fxRear,-rearLongCap,rearLongCap);
    fxFront=clamp(fxFront,-frontLongCap,frontLongCap);

    const float rolling=105.0f+5.5f*forwardSpeed;
    const float fxBody=fxRear+fxFront-rolling;
    const float fyBody=fyF*std::cos(c.steeringAngle)+fyR;
    c.longitudinalAccel=fxBody/std::max(1.0f,c.mass);
    c.lateralAccel=fyBody/std::max(1.0f,c.mass);

    const float yawMoment=lf*fyF*std::cos(c.steeringAngle)-lr*fyR;
    c.yawRate+=(yawMoment/Iz)*dt;
    c.yawRate*=std::pow(0.985f,dt*60.0f);
    c.yaw+=c.yawRate*dt;

    const float sy=std::sin(c.yaw),cy=std::cos(c.yaw);
    c.vx=std::max(0.0f,c.vx+c.longitudinalAccel*dt);
    c.vy+=c.lateralAccel*dt;
    const float maxLateral=std::max(2.0f,c.vx*0.42f);
    c.vy=clamp(c.vy,-maxLateral,maxLateral);
    c.x+=(sy*c.vy+cy*c.vx)*dt;
    c.y+=(cy*c.vx-sy*c.vy)*dt;
    c.speed=std::sqrt(c.vx*c.vx+c.vy*c.vy);

    const float targetPitch=clamp(-c.longitudinalAccel*0.035f,-0.12f,0.12f);
    const float targetRoll=clamp(c.lateralAccel*0.018f,-0.14f,0.14f);
    c.pitch+=(targetPitch-c.pitch)*std::min(1.0f,dt*7.0f);
    c.roll+=(targetRoll-c.roll)*std::min(1.0f,dt*7.0f);

    for(int i=0;i<4;i++){
        const bool front=i<2;
        const float load=c.wheel[i].load;
        c.wheel[i].slipAngle=front?alphaF:alphaR;
        c.wheel[i].lateralForce=front?fyF*0.5f:fyR*0.5f;
        c.wheel[i].longitudinalForce=front?fxFront*0.5f:fxRear*0.5f;
        c.wheel[i].slipRatio=c.wheel[i].longitudinalForce/std::max(1.0f,mu*load);
    }
}
}
