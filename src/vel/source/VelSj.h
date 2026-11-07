#ifndef ZS_VELOCITY_FOO_S_H_
#define ZS_VELOCITY_FOO_S_H_


#include "scurve_base.h"
#include "VelBase.h"
#include "vel_forward_sj.h"
#include "vel_forward_sj2.h"
#include "VelForwarder2.hpp"

namespace zmc
{

class FuncVelSj2:public VelFunctorBase{
public:
    int operator()(const VelParam& param, VelForwardSj2& fw);
};
class FuncVelSjOrig:public VelFunctorBase{
public:
    int operator()(const VelParam& param, VelForwardSj2& fw);
};

class VelocitySj0:public VelTmplBase<FuncVelSjOrig, VelForwardSj2>{
public:
    double cal_reachable_pre_vel(double vd) const;
    double cal_reachable_pro_vel(double v0) const;
};

class VelocitySj2:public VelTmplBase<FuncVelSj2, VelForwardSj2>{
public:
    double cal_reachable_pre_vel(double vd) const;
    double cal_reachable_pro_vel(double v0) const;
};



typedef VelForwarderGeneric<7> VelForwardSj3;
class FuncVelSj3:public VelFunctorBase{
public:
    int operator()(const VelParam& param, VelForwardSj3& fw);
};
class VelocitySj3:public VelTmplBase<FuncVelSj3, VelForwardSj3>{
public:
    double cal_reachable_pre_vel(double vd) const;
    double cal_reachable_pro_vel(double v0) const;
    int get_slice_continuous(double, PosVelAcc&)const;
};

typedef VelocitySj3 VelocitySj;

// class SLadder_J_Impl{
// public:
//     SLadder_J_Impl();
//     SLadder_J_Impl(const SLadder_J_Impl& ) = default;

//     void relaxation(void);
//     int build(void);

//     // inline double cal_deceleration_distance(double v_start, double acc_start=0) const {return 0;};
//     // inline double cal_min_distance(double v_start, double acc_start=0, double v_end=0) const {return 0;};
// private:
//     SLadderSymmetry is_symmetry;
// };


}


#endif  // ZS_VELOCITY_SLADDER_T_IMPL_H_
/* end of file */


