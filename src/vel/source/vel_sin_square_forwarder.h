#ifndef _VEL_SIN_SQUARE_FORWARDER_H_
#define _VEL_SIN_SQUARE_FORWARDER_H_

#include "vel/VelWrapCommon.h"
#include "vel_relax_param.hpp"
#include "VelBase.h"

#include <cmath>

#define M_PI (3.141592653589793)

namespace zmc {

typedef VelParam SinSquareParam;

typedef TrapzRelaxParam SinSquareRelaxParam;

struct VelSinSquareForwarder{
    int nseg;
    double vMax;
    double tArr[4];
    double sArr[4];
    double vArr[4];
    double aArr[4];

    void invert() {
        for (int i=0; i<4; i++){
            sArr[i] = -sArr[i];
            vArr[i] = -vArr[i];
            aArr[i] = -aArr[i];
        }
    }

    double get_end_time() const {
        return tArr[3];
    }
    void build(SinSquareRelaxParam& rParam, const SinSquareParam& param );
    
    int get_nseg() const {
        return nseg;
    }
    
    double get_rmax_vel() const {
        return vMax;
    }
    
    int check() const {
        return 0;
    }
    
    void shift_pos(double ofs) {
        for (int i=0; i<4; i++){
            sArr[i] += ofs;
        }
    }
    
    void shift_time(double ofs) {
        for (int i=0; i<4; i++){
            tArr[i] += ofs;
        }
    }
    
    int get_slice(double time, double* pos, double* vel) const {
        PosVelAcc pva;
        int ret = get_slice(time, pva);
        if (pos) *pos = pva.pos;
        if (vel) *vel = pva.vel;
        return ret;
    }
    
    int get_slice(double time, double* pos, double* vel, double* acc) const {
        PosVelAcc pva;
        int ret = get_slice(time, pva);
        if (pos) *pos = pva.pos;
        if (vel) *vel = pva.vel;
        if (acc) *acc = pva.acc;
        return ret;
    }

    void _build(SinSquareRelaxParam& rParam, const SinSquareParam& param );
    void _cumsum_time_pos();
    PosVelAcc get_slice(double t) const;
    int get_slice(double t, PosVelAcc& state) const;
    int get_slice_continuous(double tm, PosVelAcc& pva) const;
};

} // namespace zmc

#endif

