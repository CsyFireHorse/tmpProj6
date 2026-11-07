#include "VelUnCppWrap.h"
#include "mc_vel_api.h"

using namespace zmc;

mc_vel_handle mc_vel_new(void) { return new VelocityWrap(); }

void mc_vel_delete(mc_vel_handle handle) { delete (VelocityWrap *)handle; }

void mc_vel_set_config(mc_vel_handle handle, double vm, double acc, double dec, double jerk) {
    ((VelocityWrap *)handle)->set_config(vm, acc, dec, jerk);
}

void mc_vel_set_begin_pva(mc_vel_handle handle, double s0, double v0, double a0) {
    ((VelocityWrap *)handle)->set_begin_pva(s0, v0, a0);
}

void mc_vel_set_end_pva(mc_vel_handle handle, double sd, double vd, double ad) {
    ((VelocityWrap *)handle)->set_end_pva(sd, vd, ad);
}

int mc_vel_build(mc_vel_handle handle) { return ((VelocityWrap *)handle)->build(); }

double mc_vel_get_end_time(mc_vel_handle handle) { return ((VelocityWrap *)handle)->get_end_time(); }

int mc_vel_get_slice(mc_vel_handle handle, double time, double *pos, double *vel, double *acc) {
    return ((VelocityWrap *)handle)->get_slice(time, pos, vel, acc);
}
