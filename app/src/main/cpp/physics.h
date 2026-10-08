#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include "powertrain.h"

namespace apex {
struct Input { float steer=0, throttle=0, brake=0; bool drs=false; };
struct WheelTelemetry { float load=0, slipAngle=0, slipRatio=0, lateralForce=0, longitudinalForce=0; };
struct Car {
    float x=0,y=0,yaw=0;
    float speed=0,yawRate=0,vx=0,vy=0;
    float pitch=0,roll=0,longitudinalAccel=0,lateralAccel=0,steeringAngle=0;
    float wheelBase=2.8f,mass=820.0f,frontWeight=0.47f,downforce=0,drag=0;
    float rpm=4000.0f; int gear=1; float engineTorque=0.0f; bool drsActive=false;
    WheelTelemetry wheel[4];
    Powertrain powertrain{};
};
inline float clamp(float v,float lo,float hi){return std::max(lo,std::min(hi,v));}
void step(Car& c,const Input& in,float dt);
}
