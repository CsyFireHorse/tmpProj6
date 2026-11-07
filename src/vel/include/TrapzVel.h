#ifndef _ZMC_VEL_TRAPZ_H_
#define _ZMC_VEL_TRAPZ_H_

#include "vel/VelWrapCommon.h"


namespace zmc {

// class TrapzForwarder;

class VelImplParam {
protected:
    void* pImpl;
public:
    // int policy = 0;
    double S_0, S_d;
    double V_0, V_d;
    double A_0, A_d;
    double V_m;
    double A_m;
    double A_n;
    double J;
    double Tc;
    int policy = 0;
};

class TrapzVel:public VelImplParam{
    public: 
    TrapzVel();
    ~TrapzVel();
    TrapzVel(const TrapzVel& other);
    TrapzVel& operator=(const TrapzVel& other);
    void set_inversable(bool v);
    void set_scurve(bool);
    void set_continuous_vel(bool);
    void set_config(double vm, double acc, double dec, double jerk);
    void set_begin_pos_vel(double pos, double vel);
    void set_end_pos_vel(double pos, double vel);

    void set_begin_pva(double s0, double v0, double a0 = 0);
    void set_end_pva(double sd, double vd, double ad = 0);

    void set_begin_pos(double pos);
    void set_begin_vel(double vel);
    void set_end_pos(double pos);
    void set_end_vel(double vel);
    int build();
    double get_end_time() const;
    double get_end_pos() const;
    double get_delta_pos() const;
    double get_max_vel() const;
    int get_slice(double tm, PosVelAcc& pva) const;
    int get_slice(double tm, double* pos, double* vel, double* acc) const;
    PosVelAcc get_slice(double tm) const;
    int get_slice_continuous(double tm, PosVelAcc& pva) const;
    void chop_left_time(double tm);
    void shift_time(double dt);
    void shift_pos(double pos);
    double cal_reachable_backward_vel(double target_vel) const;
    double cal_reachable_forward_vel(double current_vel) const;
    double cal_stop_pos() const;
};



}  // namespace zmc

#endif