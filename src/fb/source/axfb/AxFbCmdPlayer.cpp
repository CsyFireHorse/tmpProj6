#include "axfb/AxFbCmdPlayer.h"
#include <iostream>
#include <thread>

namespace zmc {

AxFbCmdPlayer::AxFbCmdPlayer() {
    // _last_real_time = std::chrono::steady_clock::now();
}

AxFbCmdPlayer::~AxFbCmdPlayer() {

}

void AxFbCmdPlayer::preRunCycle(){
    if (_delay_by_real_time) {
        updateSimTimeByRealTime();
    }
}

void AxFbCmdPlayer::step() {
    preRunCycle();
    runCycle();
}

void AxFbCmdPlayer::steps(int num) {
    preRunCycle();
    for (int i = 0; i < num; i++) {
        runCycle();
    }
}

void AxFbCmdPlayer::tickto(int targetTick) {
    uint32_t currentTick = getCurrentTick();
    if (targetTick <= currentTick) {
        return;
    }
    
    int ticksToRun = targetTick - currentTick;
    steps(ticksToRun);
}

void AxFbCmdPlayer::delay(double time) {
    steps(int(time/_runCfg.cycleTime));
}

void AxFbCmdPlayer::timeto(double targetTime) {
    double currentTime = getCurrentTick() * _runCfg.cycleTime;
    if (targetTime <= currentTime) {
        return;
    }
    
    double timeToRun = targetTime - currentTime;
    int ticksToRun = static_cast<int>(timeToRun / _runCfg.cycleTime + 0.5);
    steps(ticksToRun);
}

void AxFbCmdPlayer::set_real_time_num(int num) {
    if (num <= 0) {
        _real_time_num = 1;
    } else {
        _real_time_num = num;
    }
}

void AxFbCmdPlayer::set_real_time_den(int den) {
    if (den <= 0) {
        _real_time_den = 1;
    } else {
        _real_time_den = den;
    }
}

void AxFbCmdPlayer::set_delay_by_real_time(bool v) {
    if (v){
    _delay_by_real_time = false;
    bLastTimeValid_ = false;
    }else{
    _delay_by_real_time = true;
    bLastTimeValid_ = true;
    _last_real_time = std::chrono::steady_clock::now();
    }
}

void AxFbCmdPlayer::delay_by_real_time() {
    updateSimTimeByRealTime();
}

int AxFbCmdPlayer::realTimeToTick(double real_time)const{
    return simuTimeToTick(realTimeToSimuTime(real_time));
}

int AxFbCmdPlayer::simuTimeToTick(double simu_time)const{
    return static_cast<int>(simu_time / _runCfg.cycleTime + 0.5);
}

double AxFbCmdPlayer::realTimeToSimuTime(double real_time)const{
    return real_time * _real_time_num / _real_time_den;
}

void AxFbCmdPlayer::updateSimTimeByRealTime() {
    auto current_real_time = std::chrono::steady_clock::now();
    
    if (!bLastTimeValid_) {
        bLastTimeValid_ = true;
        _last_real_time = current_real_time;
        return;
    }

    double elapsed_real_time = std::chrono::duration<double>(current_real_time - _last_real_time).count();

    runCycles(realTimeToTick(elapsed_real_time));
    _last_real_time = current_real_time;
}

} // namespace zmc 