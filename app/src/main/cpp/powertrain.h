#pragma once
#include <array>
#include <algorithm>
#include <cmath>

namespace apex {
struct PowertrainInput { float throttle=0.0f; float brake=0.0f; };
struct PowertrainConfig {
    float wheelRadiusM=0.33f, finalDrive=3.20f, drivelineEfficiency=0.95f;
    float idleRpm=4000.0f, shiftUpRpm=14750.0f, redlineRpm=15000.0f, maxTorqueNm=605.0f;
    std::array<float,8> gearRatios{{3.20f,2.42f,1.88f,1.56f,1.30f,1.10f,0.94f,0.82f}};
};
struct PowertrainState { int gear=1; float rpm=4000.0f; float engineTorqueNm=0.0f; float wheelTorqueNm=0.0f; float wheelForceN=0.0f; };
class Powertrain {
public:
    explicit Powertrain(const PowertrainConfig& config={});
    void reset();
    const PowertrainState& state() const { return state_; }
    float step(const PowertrainInput& input,float vehicleSpeedMps,float dt);
private:
    float torqueAtRpm(float rpm) const;
    void updateGear(float vehicleSpeedMps,float throttle);
    PowertrainConfig config_;
    PowertrainState state_;
};
} // namespace apex
