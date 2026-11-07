#include "VelCWrap.h"
#include "TrapzVel.h"

using namespace zmc;
using zmc::TrapzVel;

vel_handle vel_new(void){ return new TrapzVel;}
void vel_delete(vel_handle handle) { delete handle;}
ZMC_DLLPORT vel_handle vel_copy(vel_handle handle){
    return new TrapzVel(*(zmc::TrapzVel*)handle);
}
void vel_set_config(vel_handle vel, double vm, double acc, double dec, double jerk){
    return ((TrapzVel *)vel)->set_config(vm, acc, dec, jerk);
};
void vel_set_begin_pos_vel(vel_handle handle, double s0, double v0){
    return ((TrapzVel *)handle)->set_begin_pos_vel(s0,  v0);
};
void vel_set_end_pos_vel(vel_handle handle, double sd, double vd){
    return ((TrapzVel *)handle)->set_end_pos_vel( sd, vd);
};

int vel_build(vel_handle handle){
    return ((TrapzVel *)handle)->build();
}
double vel_get_end_time(vel_handle handle){
    return ((TrapzVel *)handle)->get_end_time();
}
// int vel_get_plan(vel_handle handle, double* total_time, double* rVmax, double* rAcc, double* rDec){
//     return ((TrapzVel *)handle)->get_plan(total_time, rVmax, rAcc, rDec);
// };
int vel_get_slice(vel_handle handle, double time, double* pos, double* vel, double* acc){
    PosVelAcc pva;
    int ret = ((TrapzVel *)handle)->get_slice(time, pva);
    *pos = pva.pos;
    *vel = pva.vel;
    *acc = pva.acc;
    return ret;
};