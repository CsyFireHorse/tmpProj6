/*
 * @Author: ChenShengyong
 * @Date: 2024-07-03 17:47:54
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-12-20 16:17:56
 */
#ifndef _VEL_STOPPER_H_
#define _VEL_STOPPER_H_

#include "VelBase.h"
#include "scurve_base.h"

namespace zmc {

class VelForwarderAccDec : public VelForwarderBase {
   public:
    enum { NUM_LADDER_SEGMENT = 1 };
    inline double get_rmax_vel() const { return vm; }
    int get_plan(double* total_time, double* rVmax) const {
        if (total_time) *total_time = td;
        if (rVmax) *rVmax = vm;
        return 0;
    };
    int get_plan(double* total_time, double* rVmax, double* rAcc, double* rDec) const {
        if (total_time) *total_time = td;
        if (rVmax) *rVmax = vm;
        return 0;
    };
    int get_slice(double time, double* pos, double* vel, double* acc = nullptr) const {
        if (time <= t0) {
            if (pos) *pos = s0;
            if (vel) *vel = v0;
            if (acc) *acc = 0;
        } else if (time <= td) {
            time -= t0;
            if (pos) *pos = s0 + (v0 + am * time / 2) * time;
            if (vel) *vel = v0 + am * time;
            if (acc) *acc = am;
        } else {
            if (pPar->is_continuous_vel()){
                if (pos) *pos = sd + (time - td) * vd;
            }else
                if (pos) *pos = sd;
            if (vel) *vel = vd;
            if (acc) *acc = 0;
            return E_VEL_COVERED;
        }
        return E_VEL_UNCOVERED;
    };
    int check() const { return 0; }
    inline double get_end_time() const { return td; }
    void shift_pos(double ofs){
        s0 += ofs;
        sd += ofs;
    }
    void shift_time(double ofs){
        t0 += ofs;
        td += ofs;
    }
    double vm;
    double am;
    double an;
    double s0;
    double sd;
    double v0;
    double vd;
    double t0;
    double td;
};

class FuncVelAccDec : public VelFunctorBase {
   public:
    int operator()(const VelParam& param, VelForwarderAccDec& fw) {
        fw.s0 = param.S_0;
        fw.v0 = param.V_0;
        fw.vd = param.V_d;
        fw.vm = fw.v0 > fw.vd ? fw.v0 : fw.vd;
        fw.am = fw.v0 > fw.vd ? -param.A_n : param.A_m;
        fw.an = fw.am;
        // fw.an = fw.v0 > fw.vd ? param.A_n: 0;
        // fw.am = fw.v0 > fw.vd ? 0 : param.A_m;
        fw.t0 = 0;
        fw.td = (fw.vd - fw.v0) / fw.am;
        fw.sd = param.S_0 + (param.V_d * param.V_d - param.V_0 * param.V_0) / fw.am / 2;
        fw.pPar = (VelParam*)&param;
        if (fw.v0 > fw.vd || fw.vd == 0)
            fw.nseg = LADDER_12_SEGMENT;
        else
            fw.nseg = LADDER_11_SEGMENT;
        return 0;
    }
};
class VelAccDec : public VelTmplBase<FuncVelAccDec, VelForwarderAccDec> {
   public:
    // inline double get_rend_time() const { return fw.get_end_time(); }
};
class VelAcceler : public VelAccDec {
   public:
    VelAcceler() { param.set_continuous_vel(true); }
};
class VelDeceler : public VelAccDec {
   public:
    VelDeceler() { param.V_d = 0; }
};

}  // namespace zmc
#endif
