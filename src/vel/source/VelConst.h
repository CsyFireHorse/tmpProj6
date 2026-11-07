/*
 * @Author: ChenShengyong 
 * @Date: 2024-05-20 17:52:00 
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-12-12 09:36:39
 */
#ifndef _VEL_CONST_H_
#define _VEL_CONST_H_

#include "VelBase.h"

namespace  zmc{

class VelForwarderConst:public VelForwarderBase{
public:
    inline double get_rmax_vel() const { return vm; }
    int get_plan(double* total_time, double* rVmax)const{
        if (total_time) *total_time = td;
        if (rVmax) *rVmax = vm;
        return 0;
    };
    int get_plan(double* total_time, double* rVmax, double* rAcc, double* rDec)const{
        if (total_time) *total_time = td;
        if (rVmax) *rVmax = vm;
        return 0;
    };
    int get_slice(double time, double* pos, double* vel)const{
        if (time <= 0){
            if (pos) *pos = s0;
            if (vel) *vel = 0;
        }else if (time < td){
            if (pos) *pos = s0 + vm * time;
            if (vel) *vel = vm;
        }else {
            if (pos) *pos = sd;
            if (vel) *vel = 0;
            return E_VEL_COVERED;
        }
        return E_VEL_UNCOVERED;
    };
    int get_slice(double time, double* pos, double* vel, double* acc)const{
        return get_slice(time, pos, vel);
    };
    void shift_pos(double ofs){
        s0 += ofs;
        sd += ofs;
    }
    void shift_time(double ofs){
        t0 += ofs;
        td += ofs;
    }
    double vm;
    double s0;
    double sd;
    double t0;
    double td;
};

class FuncVelConst:public VelFunctorBase{
public:
    int operator()(const VelParam& param, VelForwarderConst& fw){
        fw.s0 = param.S_0;
        fw.sd = param.S_d;
        fw.vm = (param.S_0 < param.S_d)? param.V_m: -param.V_m;
        fw.t0 = 0;
        fw.td = (param.S_d - param.S_0)/ fw.vm;
        fw.pPar = (VelParam*) &param;
        return 0;
    }
};
class VelToyConst:public VelTmplBase<FuncVelConst, VelForwarderConst>{

};
}
#endif
