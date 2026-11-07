#ifndef _VELOCITY_UTILS_HPP_
#define _VELOCITY_UTILS_HPP_

#include "math.h"
#include "stdio.h"
#include "vel/VelWrapCommon.h"
#include "vel_enum_def.h"

namespace zmc{
enum EnumVelCheckCode{
    VEL_CHECK_NOP,
    VEL_CHECK_VEL_LIMIT,
    VEL_CHECK_ACC_LIMIT,
    VEL_CHECK_DEC_LIMIT,
    VEL_CHECK_JERK_LIMIT,
};


class ObsDif2Pos{
public:
    double pos = 0;
    double vel = 0;
    double acc = 0;
    double Ts = 0.01;
    void calc(double posInp){
        pos = posInp;
        vel = (pos - _posBak)/Ts;
        acc = (vel - _velBak)/Ts;
        _velBak = vel;
        _posBak = pos;
    }
private:
    double _posBak = 0;
    double _velBak = 0;
};


template <typename T>
int prt_vel(T&& vt, double Ts = 0.05, int count=0){
    // static_assert(std::is_base_of<VelBase, T>::value, "Type T should be Herit of VelBase");
    double plan_time = vt.get_end_time();

    int CNT = count>0 ? count: ceil(plan_time/Ts);
    double sc, vc, ac;
    printf("idx,time,pos,vel,acc\n");
    for (int i = 0; i <= CNT; i++){
        double tm = Ts * i;
        vt.get_slice(tm, &sc, &vc, &ac);
        printf("%d,%f,%f,%f,%f\n", i, tm, sc, vc, ac);
    }
    return 0;
}

template<typename T>
void printVel(const T& vel, double Tc, double Ts=0.01){
    printf("idx,time,pos,vel,acc,diPos,posDif, posDif2\n");
    PosVelAcc pva;
    int N = ceil(Tc/Ts);
    ObsDif2Pos obs;
    obs.Ts = Ts;
    for (int i=0; i<=N+2; i++){
        double tm = i*Ts;
        vel.get_slice(tm, pva);
        obs.calc(round(pva.pos*51200));
        printf("%d,%f,%f,%f,%f,%f,%f,%f\n", i, tm, pva.pos, pva.vel, pva.acc, obs.pos, obs.vel, obs.acc);
    }
}


template <typename T, typename P>
int build_print_velocity(const P& par, double Ts = 0.05, int count=0){
    auto vt = T();
    vt.set_config(par.V_m, par.A_m, par.A_n, par.J);
    vt.set_begin_pva(par.S_0, par.V_0, par.A_0);
    vt.set_end_pva(par.S_d, par.V_d, par.A_d);
    int errCode = vt.build();
    if (errCode)
        return errCode;
    // vt.check();
    prt_vel(vt, Ts, count);
    return 0;
}

#define MAX(a,b) ((a)>=(b)?(a):(b))
#define MIN(a,b) ((a)<=(b)?(a):(b))

#if 1

template<typename T>
int pos_vel_dis_check(const T& vp, double Ts, double tol=1e-6){
    // static_assert(std::is_base_of<VelBase, T>::value, "Type T should be Herit of VelBase");
    double  plan_time = vp.get_end_time();
    int CNT = ceil(plan_time/Ts);
    double sc, vc, ac;
    double sBak, vBak, aBak;
    double J = vp.get_param(VEL_INNER_MAX_JERK);
    bool is_scurve = vp.is_scurve();
    for (int i = 0; i <= CNT; i++)
    {
        double tm = Ts * i;
        vp.get_slice(tm, &sc, &vc, &ac);
        if (i != 0 && i!= CNT){
            double mV = (sc - sBak) / Ts;
            double mAcc = (vc - vBak) / Ts;
            if (mV > MAX(vBak, vc)+tol || mV < MIN(vBak, vc)-tol)
                return VEL_CHECK_VEL_LIMIT;
            if (!is_scurve){
                if (mAcc > MAX(aBak, ac)+tol || mAcc < MIN(aBak, ac)-tol)
                    return VEL_CHECK_ACC_LIMIT;
            }else {
                double dvj = J*Ts/2;
                if (mAcc > MAX(aBak, ac)+tol + dvj || mAcc < MIN(aBak, ac)-tol - dvj)
                    return VEL_CHECK_ACC_LIMIT;
                if (i>= 2){
                    double mJerk = (ac - aBak) / Ts;
                    if (mJerk > J + tol || mJerk < -J-tol)
                        return VEL_CHECK_JERK_LIMIT;
                }
            }
        }
        // printf("%d,%f,%f,%f,%f\n", i, tm, sc, vc, ac);
        sBak = sc;
        vBak = vc;
        aBak = ac;
    }
    return VEL_CHECK_NOP;
}

template<typename T>
int pos_vel_continuous_check(const T& vp, double Ts, double tol=1e-6){
    // static_assert(std::is_base_of<VelBase, T>::value, "Type T should be Herit of VelBase");
    double plan_time = vp.get_end_time();
    int CNT = ceil(plan_time/Ts) + 5;
    double sc, vc, ac;
    double sBak, vBak, aBak;
    for (int i = 0; i <= CNT; i++)
    {
        double tm = Ts * i;
        vp.get_slice(tm, &sc, &vc, &ac);
        if (i != 0){
            double mV = (sc - sBak) / Ts;
            double mAcc = (vc - vBak) / Ts;
            if (mV > MAX(vBak, vc)+tol || mV < MIN(vBak, vc)-tol)
                return VEL_CHECK_VEL_LIMIT;
            if (mAcc > MAX(aBak, ac)+tol || mAcc < MIN(aBak, ac)-tol)
                return VEL_CHECK_ACC_LIMIT;
        }
        // printf("%d,%f,%f,%f,%f\n", i, tm, sc, vc, ac);
        sBak = sc;
        vBak = vc;
        aBak = ac;
    }
    return VEL_CHECK_NOP;
}

template<typename T>
int pos_vel_check(const T& vp, double Ts, double tol=1e-6){
    if (vp.is_continuous_vel()){
        return pos_vel_continuous_check(vp, Ts, tol);
    }else
        return pos_vel_dis_check(vp, Ts, tol);
}
#endif


// Matches MC_AXIS_STATES::mcErrorStop without depending on fb headers.
static const int VEL_ONLINE_AXIS_ERROR_STOP = 7;

class VelOnlineChecker{
private:
    // ErrorStop cycle-internal stop: reachable constant decel to zero within Ts.
    bool _is_errorstop_cycle_stop(double pos, double vel, double acc) const {
        if (axis_stat_ != VEL_ONLINE_AXIS_ERROR_STOP)
            return false;
        if (fabs(vel) > tol || fabs(acc) > tol)
            return false;
        if (fabs(velBak) <= tol)
            return false;
        const double ds = pos - posBak;
        if (ds * velBak > 0.0) {
            const double t_stop = 2.0 * ds / velBak;
            if (t_stop > 0.0 && t_stop <= Ts + tol)
                return true;
        }
        // Large deceleration can finish inside Ts with negligible displacement.
        if (fabs(ds) <= tol)
            return true;
        return false;
    }
    EnumVelCheckCode _check0(double pos, double vel, double acc){
        if (_idx != 0){
            double mV = (pos - posBak) / Ts;
            double mAcc = (vel - velBak) / Ts;
            if (mV > MAX(velBak, vel)+tol || mV < MIN(velBak, vel)-tol)
                return VEL_CHECK_VEL_LIMIT;
            if (mAcc > MAX(accBak, acc)+tol || mAcc < MIN(accBak, acc)-tol)
                return VEL_CHECK_ACC_LIMIT;
        }
        // printf("%d,%f,%f,%f,%f\n", i, tm, sc, vc, ac);
        _idx ++;
        posBak = pos;
        velBak = vel;
        accBak = acc;
        return VEL_CHECK_NOP;
    }
    EnumVelCheckCode _check(double pos, double vel, double acc){
        EnumVelCheckCode ret = VEL_CHECK_NOP;
        if (_idx != 0){
            if (_is_errorstop_cycle_stop(pos, vel, acc)) {
                _idx++;
                posBak = pos;
                velBak = vel;
                accBak = acc;
                return VEL_CHECK_NOP;
            }
            if (acc == accBak){
                double v2 = velBak + accBak * Ts;
                double s2 = posBak + velBak * Ts + acc * Ts*Ts /2;
                if (fabs(v2 - vel) > tol)
                    ret = VEL_CHECK_ACC_LIMIT;
                if (fabs(s2 - pos) > tol)
                    ret = VEL_CHECK_VEL_LIMIT;
            }else{
                double v2 =  velBak + accBak * Ts;
                if (fabs(v2 - vel) > tol + fabs(accBak - acc) * Ts){
                    ret = VEL_CHECK_ACC_LIMIT;
                }
                double s2 = posBak + (velBak + vel)/2* Ts + (acc+accBak)/2 * Ts*Ts /2;
                if (fabs(s2 - pos) > tol + fabs(velBak - vel) * Ts + fabs(acc-accBak)/2 * Ts*Ts /2){
                    ret = VEL_CHECK_VEL_LIMIT;
                }
            }
        }
        _idx ++;
        posBak = pos;
        velBak = vel;
        accBak = acc;
        return ret;
    }
public:
    // Independent axis-state input for one check() call. Values follow MC_AXIS_STATES.
    void set_state(int axis_stat) {
        axis_stat_ = axis_stat;
    }
    EnumVelCheckCode check(double pos, double vel, double acc){
        return _check(pos, vel, acc);
    }
    void reset(){
        _idx = 0;
    }
    int _idx = 0;
    int axis_stat_ = 0;
    double Ts;
    double tol = 1e-6;
    double posBak;
    double velBak;
    double accBak;
};


}
#endif