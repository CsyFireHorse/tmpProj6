#ifndef _ZMC_VEL_FOLLOWER_IMPL_HPP_
#define _ZMC_VEL_FOLLOWER_IMPL_HPP_

#include "math.h"
#include "stdint.h"
// #include "VelGearFollower.h"
// #include "TrapzVel.h"
#include "vel_enum_def.h"
#include "vel/VelWrapCommon.h"

namespace zmc {

template <typename T>
inline T sat(T val, T low, T up) {
    if (val < low)
        return low;
    else if (val > up)
        return up;
    else
        return val;
}
struct VelGearFollowerConfig {
    double V_m;  // velocity max limit
    double A_m;  // acceleration max limit
    double A_n;  // deceleration max limit
    double J;    // jerk max limit
    int num = 1;
    unsigned int den = 1;
    double Ts = 0.01;
};
struct GearFollowerImplBase {
    VelGearFollowerConfig config;
    PosVelAcc slavePva = {0, 0, 0};  // slave initial position or setposition
    EnumGearStatus nGearStatus = GEAR_STATUS_IDLE;
    bool is_vel_mode = false;  // method to calulate position, false for diffence of position ,
                               // true for sum of velocity
    uint8_t uGearType = 0;    // acc,vel, pos
    double synMasterPos = 0;
    double synSlavePos = 0;
    void set_num_den(int num, unsigned int den) {
        config.num = num;
        config.den = den;
    }
    void set_config(double vm, double acc, double dec, double jerk) {
        config.V_m = vm;
        config.A_m = acc;
        config.A_n = dec;
        config.J = jerk;
    }
    void set_begin_pva(double s0, double v0, double a0) {
        slavePva.pos = s0;
        slavePva.vel = v0;
        slavePva.acc = a0;
    }
    void reset() {
        nGearStatus = GEAR_STATUS_IDLE;
        synMasterPos = 0;
        synSlavePos = 0;
        slavePva.pos = 0;
        slavePva.vel = 0;
        slavePva.acc = 0;
    }
};

}  // namespace zmc
#endif