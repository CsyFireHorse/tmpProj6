#if 0
#include "VelUnCWrap.h"
#include "VelWrapImpl.h"

using namespace zmc;
using zmc::VelocityWrapImpl;

vel_handle vel_new(void){ return new VelocityWrapImpl;}
void vel_delete(vel_handle handle) { delete handle;}
ZMC_DLLPORT vel_handle vel_copy(vel_handle handle){
    return new VelocityWrapImpl(*(zmc::VelocityWrapImpl*)handle);
}
void vel_set_config(vel_handle vel, double vm, double acc, double dec, double jerk){
    return ((VelocityWrapImpl *)vel)->set_config(vm, acc, dec, jerk);
};
void vel_set_begin_end_pos_vel(vel_handle handle, double s0, double sd, double v0, double vd){
    return ((VelocityWrapImpl *)handle)->set_begin_end_pos_vel(s0, sd, v0, vd);
};
void vel_set_begin_end_acc(vel_handle handle, double a0, double ad){
    return ((VelocityWrapImpl *)handle)->set_begin_end_acc(a0, ad);
};
void vel_set_end_vel(vel_handle handle, double vd){
    return ((VelocityWrapImpl *)handle)->set_end_vel(vd);
};
void vel_set_total_time(vel_handle handle, double Tc){
    return ((VelocityWrapImpl *)handle)->set_total_time(Tc);
};

void vel_set_scurve(vel_handle handle, bool v){
    return ((VelocityWrapImpl *)handle)->set_scurve(v);
};
void vel_set_continuous_vel(vel_handle handle, bool v){
    return ((VelocityWrapImpl *)handle)->set_continuous_vel(v);
};

void vel_set_inversable(vel_handle handle, bool v){
    return ((VelocityWrapImpl *)handle)->set_inversable(v);
};
void vel_set_no_dest(vel_handle handle, bool v){
    return ((VelocityWrapImpl *)handle)->set_no_dest(v);
};
bool vel_is_continuous_vel(vel_handle handle){
    return ((VelocityWrapImpl *)handle)->is_continuous_vel();
};
bool vel_is_no_dest(vel_handle handle){
    return ((VelocityWrapImpl *)handle)->is_no_dest();
};
bool vel_is_scurve(vel_handle handle){
    return ((VelocityWrapImpl *)handle)->is_scurve();
};

void vel_chop_left_time(vel_handle handle, double tm){
    return ((VelocityWrapImpl *)handle)->chop_left_time(tm);
}
void vel_shift_time(vel_handle handle, double tm){
    return ((VelocityWrapImpl *)handle)->shift_time(tm);
}
int vel_build(vel_handle handle){
    return ((VelocityWrapImpl *)handle)->build();
}
void vel_shift_pos(vel_handle handle, double ofs){
    return ((VelocityWrapImpl *)handle)->shift_pos(ofs);
}
int vel_get_plan(vel_handle handle, double* total_time, double* rVmax, double* rAcc, double* rDec){
    return ((VelocityWrapImpl *)handle)->get_plan(total_time, rVmax, rAcc, rDec);
};
int vel_get_slice(vel_handle handle, double time, double* pos, double* vel, double* acc){
    return ((VelocityWrapImpl *)handle)->get_slice(time, pos, vel, acc);
};
#endif