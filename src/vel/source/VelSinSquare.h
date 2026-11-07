#ifndef _VEL_FORWARD_SIN_SQUARE_REV_H_
#define _VEL_FORWARD_SIN_SQUARE_REV_H_

#include "VelBase.h"
#include "vel_relax_param.hpp"
#include "vel_sin_square_forwarder.h"
#include "TrapzVelForwarder.hpp"

namespace zmc {

class FuncVelSinSquare:public VelFunctorBase{
public:
    int operator()(const VelParam& param, VelSinSquareForwarder& fw);
};


class VelSinSquareRev:public VelTmplBase<FuncVelSinSquare, VelSinSquareForwarder>{
public:
    int get_slice_continuous(double tm, PosVelAcc& pva)const{
        return fw.get_slice_continuous(tm, pva);
    }
};

class FuncVelTrapz3:public VelFunctorBase{
    public:
        int operator()(const VelParam& param, TrapzForwarder& fw);
};

class VelTrapz3:public VelTmplBase<FuncVelTrapz3, TrapzForwarder>{
public:
    double cal_reachable_pre_vel(double vd) const{
        return param.cal_reachable_backward_vel(vd);
    }
    double cal_reachable_pro_vel(double v0) const{
        return param.cal_reachable_forward_vel(v0);
    }
    int get_slice_continuous(double tm, PosVelAcc& pva)const{
        return fw.get_slice_continuous(tm, pva);
    }
};


} /* namespace zmc */

#endif

