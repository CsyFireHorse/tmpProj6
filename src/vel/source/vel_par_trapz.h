#ifndef _VEL_PAR_TRAPZ_H_
#define _VEL_PAR_TRAPZ_H_

#include "scurve_base.h"
// #include "zal_common.h"
#include "math.h"
#include "VelBase.h"
#include "vel_relax_param.hpp"

#define ABS(a) (((a)>=0)?(a):(-(a)))
#define SQUARE(x) ((x)*(x))
#define CUBE(x) ((x)*(x)*(x))
#define POWER4(x) ((x)*(x)*(x)*(x))

namespace zmc{



struct LadderRelaxParam{
    uint8_t nseg;
    double _vm;
};

inline LadderRelaxParam gen_LadderRelaxParam(LadderSegmentType nseg, double vm)
{
    LadderRelaxParam ret = {(uint8_t) nseg, vm};
    return ret;
}

struct TrapzVelParam:public VelParam
{
  public:
    inline double get_S()const { return S_d - S_0;}
    inline void set_pos_rate(double s0, double sd, double start_speed, double end_speed){
        S_0 = s0;
        S_d = sd;
        V_0 = start_speed;
        V_d = end_speed;
    };

    double nseg20_vm2s(double vm) const{
        double vm2 = SQUARE(vm);
        double S = (vm2 - SQUARE(V_0))/2/A_m + (vm2 - SQUARE(V_d))/2/A_n;
        return S;
    }

    double nseg20_s2vm(double S) const{
        return sqrt((2*S*A_m*A_n +  SQUARE(V_0)*A_n +  SQUARE(V_d)*A_m)/(A_m + A_n));
    }

    double nseg22_s2vn2(double S) const{
        return sqrt((SQUARE(V_0)*A_m - 2*(S_d - S_0) *A_m*A_n)/(A_m + A_n));
    }

    double nseg24_s2vn2(double S) const{
        return sqrt((SQUARE(V_0)*A_m - 2*(S_d - S_0) *A_m*A_n)/(A_m + A_n));
    }

    double nseg1_vm2s() const{
        if (V_d > V_0)
            return (SQUARE(V_d) - SQUARE(V_0)) / 2 / A_m;
        else if (V_d < V_0)
            return (SQUARE(V_0) - SQUARE(V_d)) / 2 / A_n;
        else
            return 0;
    }

    double nseg11_vm2s() const{
        return (SQUARE(V_d) - SQUARE(V_0)) / 2 / A_m;
    }
    double nseg12_vm2s() const{
        return (SQUARE(V_0) - SQUARE(V_d)) / 2 / A_n;
    }
    double nseg11_s2vd(double S) const{
        return sqrt(SQUARE(V_0) + 2 * A_m * S);
    }
    double nseg12_s2vd(double S) const{
        return sqrt(SQUARE(V_0) - 2 * A_n * S);
    }
    double nseg23_s2vn(double S, double vd) const{
        return -sqrt( (A_m * SQUARE(V_0) + A_n * SQUARE(vd) - 2 * A_m * A_n * S)/ (A_m + A_n));
    }

    double nseg23_vm2s(double vm)const {
        // S>=0 , |V_0| <= vm <= |V_d|
        // S>=0 , |V_d| <= vm <= |V_0|
        double vm2 = SQUARE(vm);
        return (SQUARE(V_0) - vm2)/2/A_n + (SQUARE(V_d) - vm2)/2/A_m;
    }
    double nseg23_s2vm(double S)const {
        double S12 =  SQUARE(V_0)*A_m + SQUARE(V_d)*A_n;
        double ds = S12 - 2*S*A_m*A_n;
        if (ds <= 0)
            return 0; 
        else
            return sqrt(ds/(A_m + A_n));
    }

    double nseg20_vm2t(double vm)const {
        return (vm - V_0)/ A_m + (vm - V_d)/ A_n;
    }
    double nseg20_s2t(double s)const {
        double vm = nseg20_s2vm(s);
        return nseg20_vm2t(vm);
    }
    double nseg23_vm2t(double vm)const {
        return (V_0 - vm)/ A_n + (V_d - vm)/ A_m;
    }

    double nseg22_s2t()const { // V_0 > V_d, T12 T22 T31
        double S12 = nseg12_vm2s(); 
        return (get_S() - S12) / V_0 + (V_0 - V_d)/ A_n;
    }
    double nseg23_s2t()const {
        double vm = nseg23_s2vm(get_S());
        if (vm <= 0)
            return 1e9;
        return nseg23_vm2t(vm);
    }
    double nseg24_s2t()const { // V_0 > V_d,  T12 T24 T31 , longest time
        double S12 = nseg12_vm2s(); 
        return (get_S() - S12) / V_d + (V_0 - V_d)/ A_n;
    }
    double nseg21_s2t()const { // V_0 < V_d, T11 T21 T32
        double S11 = nseg11_vm2s(); 
        return (V_d - V_0)/ A_m + (get_S() - S11) / V_d;
    }
    double nseg25_s2t()const { // V_0 < V_d, T11 T25 T32 , longest time
        double S11 = nseg11_vm2s(); 
        return (V_d - V_0)/ A_m + (get_S() - S11) / V_0;
    }
    double nseg2x_s2t()const {
        if (V_0 < V_d){
            return nseg25_s2t();
        }else if(V_0 > V_d){
            return nseg24_s2t();
        }else {
            return get_S()/ V_0;
        }
    }
};



struct TrapzParams:public TrapzVelParam{
    double nseg30_t2vm(double T) const;
    double nseg31_t2vm(double T) const;
    double nseg32_t2vm(double T) const;
    double nseg34_t2vm(double T) const;
    double nseg30_vm2t(double vm) const;
    double nseg31_vm2t(double vm) const;
    double nseg32_vm2t(double vm) const;
    double nseg34_vm2t(double vm) const;

    int relaxation(TrapzRelaxParam& rParam) const;
    int relaxation(double& _vm, double& _acc, double& _dec) const;
    // int relaxation(double& _vm, double& _acc, double& _dec, double& V_d2) const;
    int relaxation2(double& _vm, double& _acc, double& _dec, double& V_d2) const;
    LadderRelaxParam relaxation3(double& _vm, double& _acc, double& _dec, double& V_d2) const;
    int time_relaxation(double& _vm, double& _acc, double& _dec, double& V_d2) const;    

    bool is_feasible()const;
    double get_max_duration()const;
    int posttune_duration(int nseg, double T, double& _vm, double& _acc, double& _dec) const;
    bool is_symmetry()const;
    
};

}
#endif