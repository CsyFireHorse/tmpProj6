#ifndef _VEL_TOY_TRAPZ_H_
#define _VEL_TOY_TRAPZ_H_

#include "VelBase.h"
// #include "zal_common.h"
#include "math.h"
#include "scurve_base.h"
#include "vel_relax_param.hpp"

namespace zmc{

class VelForwarderTrapz:public VelForwarderBase{
    enum {NUM_LADDER_SEGMENT = 3};
public:
    int get_plan(double* total_time, double* rVmax, double* rAcc=nullptr, double* rDec=nullptr)const;
    int get_slice(double slice, double *position, double *velocity, double *acc=nullptr) const;

    double get_distance(double tm) const;
    double get_rate(double tm) const;
    double get_acc(double tm) const;

    void build(TrapzRelaxParam& rParam, const VelParam& param){
        nseg = rParam.seg_num;
        vm = rParam.vMax;
        am  = rParam.aMax;
        an  = rParam.aMin;
        _S = param.S_d - param.S_0;
        pPar = (VelParam*) &param;
        // if (nseg == LADDER_23_SEGMENT)
        //     _vn = vm;
        _buildTimeSegment();
        _buildPosVecSegment();
        _cumsum();
    }
    inline double get_rmax_vel() const { return vm; }
private:
    inline double get_begin_time()const { return cTarr[0];}
    inline double get_end_time()const { return cTarr[NUM_LADDER_SEGMENT];}
    inline double get_begin_distance()const { return cSarr[0];}
    void _buildTimeSegment();
    void _buildTimeSegment_s20();
    void _buildTimeSegment_s30();
    void _buildTimeSegment_s10();
    void _buildTimeSegment_s11();
    void _buildTimeSegment_s12();
    void _buildTimeSegment_s31();
    void _buildTimeSegment_s23();

    void _buildPosVecSegment();
    void _buildPosVecSegment_s30();
    void _buildPosVecSegment_s20();
    void _buildPosVecSegment_s10();
    void _buildPosVecSegment_s31();
    void _buildPosVecSegment_s23();
    void _cumsum();

    double _get_distance_s30(double tm) const;
    double _get_distance_s20(double tm) const;
    double _get_distance_s31(double tm) const;
    double _get_distance_s23(double tm) const;

    double _get_rate_s30(double tm) const;
    double _get_rate_s20(double tm) const;
    double _get_rate_s31(double tm) const;
    double _get_rate_s23(double tm) const;
    double _get_rate_s24(double tm) const;

    double _get_acc_s30(double tm) const;
    double _get_acc_s20(double tm) const;
    double _get_acc_s21(double tm) const;
    double _get_acc_s22(double tm) const;
    double _get_acc_s23(double tm) const;
    double _get_acc_s24(double tm) const;
    double _get_acc_s31(double tm) const;
    double _get_acc_s10(double tm) const;
    double _get_acc_s11(double tm) const;
    double _get_acc_s12(double tm) const;

    double         _S;
    double         vm;
    double         am;
    double         an;
    double         _vn;
    double         _vs; // FIXME
    double         Varr[NUM_LADDER_SEGMENT + 1];
    union{
        double         Tarr[NUM_LADDER_SEGMENT + 1];
        double         cTarr[NUM_LADDER_SEGMENT + 1];
    };
    union{
        double         cSarr[NUM_LADDER_SEGMENT + 1];
        double         Sarr[NUM_LADDER_SEGMENT + 1];
    };

    friend class FuncVelToyTrapz;
};

class FuncVelToyTrapz:public VelFunctorBase{
public:
    int operator()(const VelParam& param, VelForwarderTrapz& fw);
};


class VelToyTrapz:public VelTmplBase<FuncVelToyTrapz, VelForwarderTrapz>{

};


}
#endif
