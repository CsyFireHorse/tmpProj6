/*
 * @Author: ChenSy
 * @LastEditors: ChenSy
 * @Date: 2024-03-21 15:39:48
 * @LastEditTime: 2024-03-21 15:40:24
 * @Description: Modify here please
 */
#ifndef _VEL_BASE_H_
#define _VEL_BASE_H_

#include "stdint.h"
#include <type_traits>
#include "vel/VelWrapCommon.h"
#include "VelMixerTmpl.hpp"
#include "vel/include/vel_enum_def.h"
#include "math.h"

namespace zmc{

struct VelParam{
    double S_0;
    double S_d;
    double V_0;
    double V_d;
    double A_0;
    double A_d;
    double V_m;
    double A_m;
    double A_n;
    double J;
    // double J2;
    double Tc;
    unsigned int policy;
    void init(){ 
        Tc = 0; 
        policy = E_VEL_POLICY_DISCETE;
        V_0 = 0; V_d = 0; A_0 = 0; A_d = 0; S_0 = 0; S_d = 0;
        V_m = 0; A_m = 0; A_n = 0; J = 0;
    }
    inline double get_max_vel() const { return V_m;}
    inline double get_abs_S()const { return S_d >= S_0 ? S_d - S_0: S_0 - S_d;}
    inline double get_S()const { return S_d - S_0;}
    inline double get_signed_jerk()const { return S_d >= S_0 ? J: -J;}
    // void set_config(double vm, double acc, double dec, double jerk){
        // set_config(vm, acc, dec, jerk, jerk);
    // };
    void set_config(double vm, double acc, double dec, double jerk){
        V_m = vm;
        A_m = acc;
        A_n = dec;
        J = jerk;
        // J2 = jerk2;
    };
    void set_begin_end_pos_vel(double s0, double sd, double v0, double vd){
        S_0 = s0;
        S_d = sd;
        V_0 = v0;
        V_d = vd;
    };
    void set_begin_end_pos(double s0, double sd) {
        S_0 = s0;
        S_d = sd;
    };
    void set_begin_end_vel(double v0, double vd) {
        V_0 = v0;
        V_d = vd;
    }
    void set_begin_end_acc(double a0=0, double ad=0){ A_0=a0; A_d = ad;};

    void set_begin_pva(double s0, double v0, double a0 = 0) {
        S_0 = s0;
        V_0 = v0;
        A_0 = a0;
    };
    void set_end_pva(double sd, double vd, double ad = 0) {
        S_d = sd;
        V_d = vd;
        A_d = ad;
    };
    void set_total_time(double t){ Tc=t;};

    void get_config(double& vm, double& acc, double& dec, double& jerk)const{
        vm = V_m;
        acc = A_m;
        dec = A_n;
        jerk = J;
    };
    inline double get_param(VelInnerParam par)const{
        switch (par)
        {
        case VEL_INNER_TOTAL_DISTANCE:
            return get_S();
        case VEL_INNER_BEGIN_POS:
            return S_0;
        case VEL_INNER_BEGIN_VEL:
            return V_0;
        case VEL_INNER_END_VEL:
            return V_d;
        case VEL_INNER_END_POS:
            return S_d;
        case VEL_INNER_BEGIN_ACC:
            return A_0;
        case VEL_INNER_END_ACC:
            return A_d;
        case VEL_INNER_MAX_VEL:
            return V_m;
        case VEL_INNER_MAX_ACC:
            return A_m;
        case VEL_INNER_MAX_DEC:
            return A_n;
        case VEL_INNER_MAX_JERK:
            return J;
        case VEL_INNER_MAX_JERK2:
            return J;
        case VEL_INNER_TOTAL_TIME:
            return Tc;
        default:
            return 0;
        }
    }
    inline int set_param(VelInnerParam par, double val){
        switch (par)
        {
        case VEL_INNER_TOTAL_DISTANCE:
            return -1;
        case VEL_INNER_BEGIN_POS:
            S_0 = val;
            return 0;
        case VEL_INNER_BEGIN_VEL:
            V_0 = val;
            return 0;
        case VEL_INNER_END_POS:
            S_d = val;
            return 0;
        case VEL_INNER_END_VEL:
            V_d = val;
            return 0;
        case VEL_INNER_BEGIN_ACC:
            A_0 = val;
            return 0;
        case VEL_INNER_END_ACC:
            A_d = val;
            return 0;
        case VEL_INNER_MAX_VEL:
            V_m = val;
            return 0;
        case VEL_INNER_MAX_ACC:
            A_m = val;
            return 0;
        case VEL_INNER_MAX_DEC:
            A_n = val;
            return 0;
        case VEL_INNER_MAX_JERK:
            J = val;
            return 0;
        case VEL_INNER_MAX_JERK2:
            J = val;
            return 0;
        case VEL_INNER_TOTAL_TIME:
            Tc = val;
            return 0;
        default:
            return -1;
        }
    }
    inline bool is_vel_symmetry() const {
        return V_0 == V_d;
    }
    void invert(){
        S_0 = -S_0;
        S_d = -S_d;
        // S_d = S_0 - (S_d - S_0);
        V_0 = -V_0;
        V_d = -V_d;
        A_0 = -A_0;
        A_d = -A_d;
    }
    void shift_pos(double ofs){
        S_0 += ofs;
        S_d += ofs;
    }
    void shift_time(double ofs){}
    inline bool is_scurve()const{
        return  (policy & E_VEL_POLICY_SCURVE ) && (J > 1e-4 && J < 1e7);
    }
    inline void set_policy_mask(bool val, uint32_t mask){
        if (val)
            policy |= mask;
        else 
            policy &= ~((unsigned int) mask);
    }
    inline void set_policy_mask2(uint32_t val, uint32_t mask){
        policy = ((~(unsigned int) mask) & policy) | (mask & val);
    }
    inline void set_scurve(bool val) { 
        set_policy_mask(val, E_VEL_POLICY_SCURVE);
    }
    inline void set_continuous_vel(bool val) { 
        set_policy_mask(val, E_VEL_POLICY_CONSTINUOUS);
    }
    inline void set_online(bool val) { 
        set_policy_mask(val, E_VEL_POLICY_ONLINE);
    }  
    inline void set_slave(bool val) { 
        set_policy_mask(val, E_VEL_POLICY_SLAVE);
    }  
    inline void set_sinsquare(bool val) { 
        set_policy_mask(val, E_VEL_POLICY_SINSQUARE);
    }  
    inline void set_inversable(bool val) { 
        set_policy_mask(val, E_VEL_POLICY_DISABLE_INVERT);
    }
    inline void set_no_dest(bool val) {
        set_policy_mask(val, E_VEL_POLICY_NO_DEST);
    }    

    inline void set_mode(int mode) { 
        switch (mode)
        {
        case VEL_MODE_TRAPEZOID:
            /* code */
            set_scurve(false);
            set_sinsquare(false);
            break;
        case VEL_MODE_SINSQUARE:
            set_sinsquare(true);
            set_scurve(false);
            break;
        case VEL_MODE_QUADRATIC:
            set_scurve(true);
            set_sinsquare(false);
            break;
        case VEL_MODE_QUADRATIC_SMOOTH:
            set_scurve(true);
            set_sinsquare(true);
            break;
        default:
            break;
        }
    }
    inline bool is_continuous_vel() const{ return policy & E_VEL_POLICY_CONSTINUOUS;}
    inline bool is_no_dest() const{ return policy & E_VEL_POLICY_NO_DEST;}
    inline bool is_online() const{ return policy & E_VEL_POLICY_ONLINE;}
    inline bool is_slave() const{ return policy & E_VEL_POLICY_SLAVE;}   
    inline bool is_inversable() const{ return (policy & E_VEL_POLICY_DISABLE_VD_INC) == E_VEL_POLICY_DISABLE_INVERT;}   
    inline bool is_sinsquare() const{ return policy & E_VEL_POLICY_SINSQUARE;}

    double cal_reachable_forward_vel(double current_vel) const {
        double fDistance_ = get_S();        
        if (fDistance_ > 1e-6) {
            return sqrt(current_vel * current_vel + 2 * A_m * fDistance_);
        } else if (fDistance_ < -1e-6) {
            return -sqrt(current_vel * current_vel + 2 * A_n * -fDistance_);
        }else {
            return current_vel;
        }
    }
    
    double cal_reachable_backward_vel(double target_vel) const {
        double fDistance_ = get_S();
        if (fDistance_ > 1e-6) {
            return sqrt(target_vel * target_vel + 2 * A_n * fDistance_);
        } else if (fDistance_ < -1e-6) {
            return -sqrt(target_vel * target_vel + 2 * A_m * -fDistance_);
        }else{
            return target_vel;
        }
    }
};

struct VelForwarderBase{
    inline int get_nseg()const { return nseg;}
    uint8_t nseg = 0;
    uint8_t niche;
    VelParam* pPar;
    int check()const { return 0;}
    inline double get_rmax_vel() const { return 0; }
};

class VelBase{
public:
    VelBase(){
        param.init();
    }
    void set_config(double vm, double acc, double dec, double jerk){
        param.set_config(vm, acc, dec, jerk);
    };
    void set_begin_end_pos_vel(double s0, double sd, double v0, double vd){
        param.set_begin_end_pos_vel(s0, sd, v0, vd);
    };
    void set_begin_end_acc(double a0=0, double ad=0){
        param.set_begin_end_acc(a0, ad);
    };
    void set_continuous_vel(bool val) { param.set_continuous_vel(val);}
    inline bool is_continuous_vel() const { return param.is_continuous_vel();}
    inline double get_param(VelInnerParam par)const{ return param.get_param(par);}
    inline int set_param(VelInnerParam par, double val){ return param.set_param(par, val);}
    VelParam param;
};

class VelFunctorBase{};


// template <typename Func, typename T>
// class VelTmplBase:public VelMixerTmpl<VelBase, T, Func>{
// };
// // typename = typename std::enable_if_t<std::is_base_of_v<VelForwarderBase, T>>
template <typename Func, typename T>
class VelTmplBase:public VelBase{

public:
    inline void chop_left_time(double tm){
        double pos, vel, acc;
        fw.get_slice(tm, &pos, &vel, &acc);
        this->param.S_0 = pos;
        this->param.V_0 = vel;
        this->param.A_0 = acc;
        return;
    }
    int build(){
        // static_assert(std::is_base_of<VelForwarderBase, T>::value, "Type T should be Herit of VelForwarderBase");
        static_assert(std::is_base_of<VelFunctorBase, Func>::value, "Type T should be Herit of VelFunctorBase");
        return Func()(this->param, fw);
    }
    double get_end_time()const{
        return fw.get_end_time();
    }
    double get_rmax_vel()const{
        return fw.get_rmax_vel();
    }
    void shift_pos(double ofs){
        this->param.shift_pos(ofs);
        fw.shift_pos(ofs);
    }
    void shift_time(double ofs){
        this->param.shift_time(ofs);
        fw.shift_time(ofs);
    }
    int check()const{ return fw.check(); }

    int get_plan(double* total_time, double* rVmax)const{
        if (total_time) *total_time = fw.get_end_time();
        if (rVmax) *rVmax = fw.get_rmax_vel();
        return 0;
    };
    int get_plan(double* total_time, double* rVmax, double* rAcc, double* rDec)const{
        if (total_time) *total_time = fw.get_end_time();
        if (rVmax) *rVmax = fw.get_rmax_vel();
        return 0;
    };
    int get_slice(double time, double* pos, double* vel)const{
        return fw.get_slice(time, pos, vel);
    };
    int get_slice(double time, double* pos, double* vel, double* acc)const{
        return fw.get_slice(time, pos, vel, acc);
    };
    int get_nseg()const{
        return fw.get_nseg();
    }
    inline bool is_scurve()const {
        return this->param.is_scurve();
    }
    T fw;
};


}
#endif
