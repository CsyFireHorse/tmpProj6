#pragma once
#include "vel/VelWrapCommon.h"
// #include "cubic_poly.h"

namespace zmc {
class FifthOrderPolyWrap {
public:
    FifthOrderPolyWrap();
    ~FifthOrderPolyWrap();

    // Initialize with position, velocity and acceleration at start and end points
    void init(double t0, double t1, double p0, double p1, double v0, double v1, double a0, double a1);
    
    PosVelAcc get_pva(double x) const;
    // Get position, velocity and acceleration at time t
    void get_pva(double x, PosVelAcc* pva) const;
    
    // Get position, velocity, acceleration and jerk at time t
    void get_pvaj(double x, PosVelAccJerk* pvaj) const;

private:
    static constexpr int STORAGE_SIZE = sizeof(double) * 8;
    char storage_[STORAGE_SIZE];
};

}