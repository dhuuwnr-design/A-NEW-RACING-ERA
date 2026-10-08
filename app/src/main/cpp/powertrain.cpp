#include "powertrain.h"
namespace apex {
Powertrain::Powertrain(const PowertrainConfig& config):config_(config){reset();}
void Powertrain::reset(){state_={};state_.gear=1;state_.rpm=config_.idleRpm;}
float Powertrain::torqueAtRpm(float rpm) const {
    constexpr float pi=3.14159265359f;
    (void)pi;
    const float t=std::clamp(rpm/config_.redlineRpm,0.0f,1.0f);
    const float rise=std::clamp(t/0.25f,0.0f,1.0f);
    const float fall=1.0f-0.28f*std::clamp((t-0.68f)/0.32f,0.0f,1.0f);
    return config_.maxTorqueNm*(0.84f+0.16f*rise)*fall;
}
void Powertrain::updateGear(float vehicleSpeedMps,float throttle){
    constexpr float pi=3.14159265359f;
    const float wheelRpm=vehicleSpeedMps/std::max(0.05f,2.0f*pi*config_.wheelRadiusM)*60.0f;
    float ratio=config_.gearRatios[static_cast<size_t>(state_.gear-1)]*config_.finalDrive;
    state_.rpm=std::max(config_.idleRpm,wheelRpm*ratio);
    if(throttle>0.12f&&state_.rpm>config_.shiftUpRpm&&state_.gear<8) ++state_.gear;
    else if(state_.gear>1&&state_.rpm<config_.idleRpm+1800.0f) --state_.gear;
    ratio=config_.gearRatios[static_cast<size_t>(state_.gear-1)]*config_.finalDrive;
    state_.rpm=std::clamp(wheelRpm*ratio,config_.idleRpm,config_.redlineRpm);
}
float Powertrain::step(const PowertrainInput& input,float vehicleSpeedMps,float dt){
    (void)input.brake;(void)dt;
    const float throttle=std::clamp(input.throttle,0.0f,1.0f);
    updateGear(std::max(0.0f,vehicleSpeedMps),throttle);
    state_.engineTorqueNm=torqueAtRpm(state_.rpm)*throttle;
    const float ratio=config_.gearRatios[static_cast<size_t>(state_.gear-1)]*config_.finalDrive;
    state_.wheelTorqueNm=state_.engineTorqueNm*ratio*config_.drivelineEfficiency;
    state_.wheelForceN=state_.wheelTorqueNm/std::max(0.05f,config_.wheelRadiusM);
    return state_.wheelForceN;
}
} // namespace apex
