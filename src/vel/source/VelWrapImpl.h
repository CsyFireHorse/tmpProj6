/*
 * @Author: ChenShengyong
 * @Date: 2024-03-28 17:47:49
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-12-12 16:38:20
 */
#ifndef ZS_VELOCITY_WRAP_H_
#define ZS_VELOCITY_WRAP_H_

#include "VelFollower.h"
#include "VelParStopper.h"
#include "VelSj.h"
#include "VelStopper.h"
#include "VelToyTrapz.h"
#include "vel/VelWrapCommon.h"
#include "vel_forward_trapz.h"
#include "vel_forward_trapz_rev.h"
#include "VelSinSquare.h"

namespace zmc {
#define TOL (1e-6)

union VelocityWrapImpl {
    // VelTrapzRev velTr;
    VelTrapz3 velTr;
    VelocitySj velSj;
    VelAcceler velStp;
    VelGearFollower velSlv;
    VelSinSquareRev velSin;

    static_assert(sizeof(VelocitySj) > sizeof(VelTrapzRev), "sizeof not right");
    static_assert(sizeof(VelocitySj) > sizeof(VelAcceler), "sizeof not right");
    static_assert(sizeof(VelocitySj) > sizeof(VelGearFollower), "sizeof not right");
    static_assert(sizeof(VelocitySj) > sizeof(VelSinSquareRev), "VelSinSquareRev sizeof not right");
    VelocityWrapImpl() : velSj() {}
    // Same pattern as TrajUn: copy the active velSj member through the union.
    VelocityWrapImpl(const VelocityWrapImpl &other) : velSj(other.velSj) {}
    VelocityWrapImpl &operator=(const VelocityWrapImpl &other) {
        if (this != &other) {
            velSj = other.velSj;
        }
        return *this;
    }
    ~VelocityWrapImpl() { velSj.~VelocitySj(); }

   public:
    void set_config(double vm, double acc, double dec, double jerk) {
        velSj.param.set_config(vm, acc, dec, jerk);
    };
    void set_begin_end_pos_vel(double s0, double sd, double v0, double vd) {
        velSj.param.set_begin_end_pos_vel(s0, sd, v0, vd);
    };
    void set_begin_end_pos(double s0, double sd) {
        velSj.param.set_begin_end_pos(s0, sd);
    };
    void set_begin_end_vel(double v0, double vd) {
        velSj.param.set_begin_end_vel(v0, vd);
    };
    void set_begin_end_acc(double a0 = 0, double ad = 0) {
        velSj.param.set_begin_end_acc(a0, ad);
    };
    void set_begin_pva(double s0, double v0, double a0 = 0) {
        velSj.param.set_begin_pva(s0, v0, a0);
    };
    void set_end_pva(double sd, double vd, double ad = 0) {
        velSj.param.set_end_pva(sd, vd, ad);
    };
    void set_begin_pos(double s0){
        velSj.param.S_0 = s0;
    }
    void set_begin_vel(double v0){
        velSj.param.V_0 = v0;
    }
    void set_end_pos(double sd) { velSj.param.S_d = sd; };
    void set_end_vel(double vd) { velSj.param.V_d = vd; };
    void set_total_time(double t) { velSj.param.set_total_time(t); };
    inline void set_scurve(bool v) { velSj.param.set_scurve(v); };
    inline void set_inversable(bool v) { velSj.param.set_inversable(v); };
    inline void set_continuous_vel(bool val) { velSj.set_continuous_vel(val); }
    inline void set_no_dest(bool v) { velSj.param.set_no_dest(v); };
    inline void set_slave(bool v) { velSj.param.set_slave(v); };
    inline void set_sinsquare(bool v) { velSj.param.set_sinsquare(v); };

    inline void set_mode(int mode) { velSj.param.set_mode(mode); };
    inline void set_vel_flag(unsigned int v) { velSj.param.policy = v; };

    inline bool is_continuous_vel() const { return velSj.param.is_continuous_vel(); }
    inline bool is_no_dest() const { return velSj.param.is_no_dest(); }
    inline bool is_online() const { return velSj.param.is_online(); }
    inline bool is_slave() const { return velSj.param.is_slave(); }
    inline bool is_scurve() const { return velSj.param.is_scurve(); }
    inline bool is_sinsquare() const { return velSj.param.is_sinsquare(); }

    void get_config(double& vm, double& acc, double& dec, double& jerk) {
        velSj.param.get_config(vm, acc, dec, jerk);
    };
    double cal_reachable_backward_vel(double vd)const{
        if (is_scurve())
            return velSj.cal_reachable_pre_vel(vd);
        else
            return velTr.cal_reachable_pre_vel(vd);
    }
    double cal_reachable_forward_vel(double v0)const{
        if (is_scurve())
            return velSj.cal_reachable_pro_vel(v0);
        else
            return velTr.cal_reachable_pro_vel(v0);
    }

    double cal_reachable_pre_vel(double vd)const{
        return cal_reachable_backward_vel(vd);
    }
    double cal_reachable_pro_vel(double v0)const{
        return cal_reachable_forward_vel(v0);
    }
    double get_end_pos()const{
        return velSj.param.S_d;
    }
    double get_delta_pos()const{
        return velSj.param.get_S();
    }
    int get_nseg() const { return velSj.get_nseg(); }
    void init() {
        if (is_slave()) return velSlv.init();
    }
    void set_num_den(int n, int d) {
        if (is_slave()) return velSlv.set_num_den(n, d);
    }
    void set_time_sample(double v) {
        if (is_slave()) return velSlv.set_time_sample(v);
    }
    void follow(double sc, double vc, double ac) {
        if (is_slave()) return velSlv.follow(sc, vc, ac);
    }

    int build() {
        if (is_slave())
            return velSlv.build();
        else if (is_sinsquare())
            return velSin.build();
        else if (is_no_dest())
            return velStp.build();
        else if (is_scurve())
            return velSj.build();
        else
            return velTr.build();
    }
    int check() const {
        if (is_slave())
            return velSlv.check();
        else if (is_sinsquare())
            return velSin.check();
        else if (is_no_dest())
            return velStp.check();
        else if (is_scurve())
            return velSj.check();
        else
            return velTr.check();
    }
    double get_begin_rate() const { return velSj.param.V_0; }
    double get_end_rate() const { return velSj.param.V_d; }
    double get_begin_acc() const { return velSj.param.A_0; }
    double get_end_acc() const { return velSj.param.A_d; }
    double get_rend_time() const { return get_end_time(); }
    inline double get_end_time() const {
        if (is_slave())
            return 0;
        else if (is_sinsquare())
            return velSin.get_end_time();
        else if (is_no_dest())
            return velStp.get_end_time();
        else if (!is_scurve())
            return velTr.get_end_time();
        else
            return velSj.get_end_time();
    }
    inline double get_max_vel() const { return velSj.param.V_m; }
    inline double get_rmax_vel() const { 
        if (is_slave())
            return 0; // velSlv.get_rmax_vel(); 
        else if (is_no_dest())
            return velStp.get_rmax_vel(); 
        else if (is_sinsquare())
            return velSin.get_rmax_vel(); 
        else if (is_scurve())
            return velSj.get_rmax_vel(); 
        else
            return velTr.get_rmax_vel(); 
    }

    int get_plan(double* total_time, double* rVmax, double* rAcc = nullptr,
                 double* rDec = nullptr) const {
        if (is_slave())
            return velSlv.get_plan(total_time, rVmax, rAcc, rDec);
        else if (is_sinsquare())
            return velSin.get_plan(total_time, rVmax, rAcc, rDec);
        else if (is_no_dest())
            return velStp.get_plan(total_time, rVmax, rAcc, rDec);
        else if (is_scurve())
            return velSj.get_plan(total_time, rVmax, rAcc, rDec);
        else
            return velTr.get_plan(total_time, rVmax, rAcc, rDec);
    };
    /**
     *
     * \return  output flag
     * \retval  0 for isDone, inGear, inVel, isStopped;
     * \retval  1 for notDone, not in gear, not in vel, not stopped.
     */
    int get_slice(double time, double* pos, double* vel, double* acc = nullptr) const {
        if (is_slave())
            return velSlv.get_slice(time, pos, vel, acc);
        else if (is_sinsquare())
            return velSin.get_slice(time, pos, vel, acc);
        else if (is_no_dest())
            return velStp.get_slice(time, pos, vel, acc);
        else if (is_scurve())
            return velSj.get_slice(time, pos, vel, acc);
        else
            return velTr.get_slice(time, pos, vel, acc);
    };
    int get_slice(double time, PosVelAcc& pva)const{
        return get_slice(time, &pva.pos, &pva.vel, &pva.acc);
    }
    int get_slice_continuous(double time, PosVelAcc& pva)const{
        if (is_scurve())
            return velSj.get_slice_continuous(time, pva);
        else if (is_sinsquare())
            return velSin.get_slice_continuous(time, pva);
        else
            return velTr.get_slice_continuous(time, pva);
    }
    double get_param(VelInnerParam par) const {
        double val;
        switch (par) {
            case VEL_INNER_NUM_SEG:
                return get_nseg();
            case VEL_INNER_TOTAL_DISTANCE:
            case VEL_INNER_BEGIN_VEL:
            case VEL_INNER_END_VEL:
            case VEL_INNER_BEGIN_ACC:
            case VEL_INNER_END_ACC:
            case VEL_INNER_MAX_VEL:
            case VEL_INNER_MAX_ACC:
            case VEL_INNER_MAX_DEC:
            case VEL_INNER_MAX_JERK:
            case VEL_INNER_MAX_JERK2:
            case VEL_INNER_TOTAL_TIME:
                return velSj.param.get_param(par);
            case VEL_INNER_REAL_END_TIME:
                return get_end_time();
            case VEL_INNER_REAL_MAX_VEL:
                return get_rmax_vel();
            case VEL_INNER_REAL_MAX_ACC:
                return get_rmax_vel(); // FIXME
            case VEL_INNER_REAL_MAX_DEC:
                return get_rmax_vel(); // FIXME
            default:
                if (par >= 32 && par < 32 + sizeof(VelocitySj)) {
                    uint8_t* pDat = (uint8_t*)&velSj;
                    return pDat[par - 32];
                }
                return 0;
        }
    }
    void shift_time(double tm) {
        if (is_slave())
            ;  // return velSlv.shift_time(tm);
        else if (is_no_dest())
            return velStp.shift_time(tm);
        else if (is_sinsquare())
            return velSin.shift_time(tm);
        else if (is_scurve())
            return velSj.shift_time(tm);
        else
            return velTr.shift_time(tm);
    }
    void chop_left_time(double tm) {
        if (is_slave())
            ;  // return velSlv.shift_time(tm);
        else if (is_sinsquare())
            return velSin.chop_left_time(tm);
        else if (is_no_dest())
            return velStp.chop_left_time(tm);
        else if (is_scurve())
            return velSj.chop_left_time(tm);
        else
            return velTr.chop_left_time(tm);
    }
    void shift_pos(double ofs) {
        if (is_slave())
            ;  // return velSlv.shift_pos(ofs);
        else if (is_sinsquare())
            return velSin.shift_pos(ofs);
        else if (is_no_dest())
            return velStp.shift_pos(ofs);
        else if (is_scurve())
            return velSj.shift_pos(ofs);
        else
            return velTr.shift_pos(ofs);
    }

    inline static double calc_stop_dist(double v0, double a0, double dec, double jerk) {
        return VelParamsStopper::calc_stop_dist(v0, a0, dec, jerk);
    }
    inline static double calc_stop_dist(double v0, double dec) {
        return VelParamsStopper::calc_stop_dist(v0, dec);
    }
    inline static double calc_vel_dist(double v0, double vd, double acc, double dec) {
        if (vd > v0 + 1e-6) {
            return (vd * vd - v0 * v0) / acc / 2;
            ;
        } else if (vd < v0 - 1e-6) {
            return -(vd * vd - v0 * v0) / dec / 2;
            ;
        } else {
            return 0;
        }
    }
    double cal_stop_pos()const {
        return calc_stop_dist(velSj.param.V_0, velSj.param.A_n);
    }
};

}  // namespace zmc

#endif  // ZS_VELOCITY_SLADDER_T_IMPL_H_
        /* end of file */