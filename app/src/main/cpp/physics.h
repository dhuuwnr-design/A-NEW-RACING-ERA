#pragma once
#include <algorithm>
#include <cmath>

namespace apex {

struct Input { float throttle=0, brake=0, steer=0; };

struct WheelTelemetry {
    float load=0;
    float slipAngle=0;
    float slipRatio=0;
    float lateralForce=0;
    float longitudinalForce=0;
};

struct Car {
    float x=0, y=0, yaw=0;
    float speed=0, yawRate=0;
    float pitch=0, roll=0;
    float longitudinalAccel=0, lateralAccel=0, steeringAngle=0;
    float wheelBase=2.8f, mass=820.0f;
    float frontWeight=0.47f;
    float downforce=0;
    float drag=0;
    WheelTelemetry wheel[4];
};

inline float clamp(float v,float lo,float hi){return std::max(lo,std::min(hi,v));}
void step(Car& c,const Input& in,float dt);

}
