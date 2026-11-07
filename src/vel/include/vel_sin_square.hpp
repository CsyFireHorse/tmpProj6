#ifndef _ZMC_VEL_SIN_SQUARE_HPP_
#define _ZMC_VEL_SIN_SQUARE_HPP_

#include <cmath>
#include "vel/VelWrapCommon.h"


#define M_PI (3.141592653589793)
namespace zmc {


class VelSinSquarePlanner {
   private:
    void* pImpl;
    // 速度参数
    // A_m, A_n;
    double S0 = 0, Sd;
    double V0, Vd; 
    double A0, Ad;
    double Vmax; 
    double Amax;
    double Amin;
    double Jerk;
    double Tc;
    int policy;

    // depreciated
    // double T1, T2, T3;
    // double T_total;
    // double S1, S2, S3;
   public:
   VelSinSquarePlanner();
   ~VelSinSquarePlanner();    
    void set_config(double vmax, double acc, double dec, double jerk);
    void set_begin_pva(double s0, double v0, double a0=0);
    void set_end_pva(double s, double vd, double ad=0);
    int build();
    void build_by_phasing(double v0, double vd, double vm, double t1, double t2, double t3);
    int build_by_s(double s0, double sd, double v0, double vd, double Vmax, double A_m, double A_n);
    
    PosVelAcc get_slice(double t) const;
    int get_slice(double tm, PosVelAcc& pva) const;
    int get_slice(double tm, double* pos, double* vel, double* acc) const;
    // PosVelAcc get_slice0(double t) const;
    double get_end_time() const;
};

}  // namespace zmc
#endif