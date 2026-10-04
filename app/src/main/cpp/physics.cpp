#include "physics.h"

namespace apex {

void step(Car& c,const Input& in,float dt){
    dt=clamp(dt,0.0f,0.05f);
    if(dt<=0) return;

    const float throttle=clamp(in.throttle,0.0f,1.0f);
    const float brake=clamp(in.brake,0.0f,1.0f);
    const float steer=clamp(in.steer,-1.0f,1.0f);

    constexpr float g=9.81f;
    constexpr float maxDrive=10500.0f;
    constexpr float maxBrake=14500.0f;
    constexpr float cornerStiff=52000.0f;
    constexpr float mu=1.55f;
    constexpr float cgHeight=0.32f;
    constexpr float trackWidth=1.60f;

    const float aero=c.speed*c.speed*1.35f;
    const float totalLoad=c.mass*g+aero;
    c.downforce=aero;
    c.drag=0.0016f*c.speed*c.speed;

    const float drive=maxDrive*throttle;
    const float brakeForce=maxBrake*brake;
    const float rolling=95.0f+8.0f*c.speed;
    const float longitudinal=drive-brakeForce-rolling-c.drag;
    c.longitudinalAccel=longitudinal/std::max(1.0f,c.mass);

    // F1-style steering gets progressively less aggressive at high speed.
    const float steerScale=clamp(0.50f/(1.0f+c.speed*0.025f),0.16f,0.50f);
    c.steeringAngle=steer*steerScale;

    // Longitudinal load transfer: braking loads the front axle, acceleration
    // loads the rear. This is deliberately bounded for stable mobile driving.
    const float longitudinalTransfer=c.mass*c.longitudinalAccel*cgHeight/c.wheelBase;
    const float frontAxle=clamp(totalLoad*c.frontWeight-longitudinalTransfer,0.1f,20000.0f);
    const float rearAxle=clamp(totalLoad-frontAxle,0.1f,22000.0f);

    const float wheelSpeed=std::max(1.5f,c.speed);
    const float betaFront=c.steeringAngle-std::atan2(c.yawRate*(c.wheelBase*(1.0f-c.frontWeight)),wheelSpeed);
    const float betaRear=-std::atan2(c.yawRate*(c.wheelBase*c.frontWeight),wheelSpeed);

    // Approximate lateral acceleration for left/right load transfer.
    c.lateralAccel=c.speed*c.speed*std::tan(c.steeringAngle)/std::max(1.0f,c.wheelBase);
    const float lateralTransfer=c.mass*c.lateralAccel*cgHeight/trackWidth;

    const float baseFront=.5f*frontAxle;
    const float baseRear=.5f*rearAxle;
    const float maxTransferFront=baseFront*.70f;
    const float maxTransferRear=baseRear*.70f;
    const float frontShift=clamp(lateralTransfer,-maxTransferFront,maxTransferFront);
    const float rearShift=clamp(lateralTransfer,-maxTransferRear,maxTransferRear);

    // Wheel order: front-left, front-right, rear-left, rear-right.
    c.wheel[0].load=std::max(0.1f,baseFront-frontShift);
    c.wheel[1].load=std::max(0.1f,baseFront+frontShift);
    c.wheel[2].load=std::max(0.1f,baseRear-rearShift);
    c.wheel[3].load=std::max(0.1f,baseRear+rearShift);

    float frontLat=0,rearLat=0;
    const float drivenPerWheel=std::max(0.0f,drive-brakeForce*.55f)*.5f;
    for(int i=0;i<4;i++){
        const bool front=i<2;
        const float slip=front?betaFront:betaRear;
        const float load=c.wheel[i].load;
        const float rawLat=cornerStiff*slip;
        const float requestedLong=(i>=2)?drivenPerWheel:(-brakeForce*.25f);
        const float grip=mu*load;
        const float longLimited=clamp(requestedLong,-grip,grip);
        const float remaining=std::sqrt(std::max(0.0f,grip*grip-longLimited*longLimited));
        const float lat=clamp(rawLat,-remaining,remaining);

        c.wheel[i].slipAngle=slip;
        c.wheel[i].longitudinalForce=longLimited;
        c.wheel[i].slipRatio=longLimited/std::max(1.0f,grip);
        c.wheel[i].lateralForce=lat;
        if(front) frontLat+=lat; else rearLat+=lat;
    }

    const float accel=longitudinal/std::max(1.0f,c.mass);
    c.speed=clamp(c.speed+accel*dt,0.0f,105.0f);

    const float yawMoment=frontLat*(c.wheelBase*(1.0f-c.frontWeight))-rearLat*(c.wheelBase*c.frontWeight);
    const float yawInertia=std::max(100.0f,c.mass*c.wheelBase*c.wheelBase*0.22f);
    c.yawRate += (yawMoment/yawInertia)*dt*18.0f;
    c.yawRate *= std::pow(0.92f,dt*60.0f);
    c.yaw += c.yawRate*dt;

    c.x += std::sin(c.yaw)*c.speed*dt;
    c.y += std::cos(c.yaw)*c.speed*dt;

    // Visual body attitude: quick response, then damped like a real car.
    const float targetPitch=clamp(-c.longitudinalAccel*.035f,-.12f,.12f);
    const float targetRoll=clamp(c.lateralAccel*.018f,-.14f,.14f);
    c.pitch+=(targetPitch-c.pitch)*std::min(1.0f,dt*7.0f);
    c.roll+=(targetRoll-c.roll)*std::min(1.0f,dt*7.0f);
}

}
