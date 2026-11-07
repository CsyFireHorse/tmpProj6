/*
 * @Author: ChenShengyong
 * @LastEditors: ChenSy
 * @Date: 2022-02-08 09:28:54
 * @LastEditTime: 2023-08-14 09:44:04
 * @Description: Modify here please
 */
/*******************************************************************************
*                                    ZS
*                       ----------------------------
*                        ZHIYUAN Software Interface
*
* Copyright (c) 2001-2020 Guangzhou ZHIYUAN Electronics Co., Ltd.
* All rights reserved.
*
* Contact information:
* web site:    http://www.zlg.cn/
*******************************************************************************/
 
/*----------------------------------------------------------------------------*/
/**
 * \file
 * \brief S型速度规划
 *
 * \internal
 * \par Modification history
 * - 1.00 2022-3-12  CSY, 初次实现
 * \endinternal
 */
/*----------------------------------------------------------------------------*/
#ifndef ZS_VEL_PAR_SJ_H_
#define ZS_VEL_PAR_SJ_H_

#include "VelBase.h"
#include "scurve_base.h"
#include "common/zal_common.h"
#include "lookaheadImpl.hpp"
#include "math.h"
#include "vel_relax_param.hpp"

namespace zmc
{

/*----------------------------------------------------------------------------*/
/**
  \brief 7段S型速度规划 J约束
  acc
              ___________
            /             \
          /                \
        /                   \
      s  l  s      l     s l \___________                 
                                         \               /            
                                          \             /             
                                           \___________/              


*/
/*----------------------------------------------------------------------------*/

// input parameter
typedef struct SLadderParams:public VelParam
{
  public:
    SLadderSymmetry get_symmetry() const;
    SLadderSortType sort_type() const;
    SLadderSegmentType relaxation_Z_Symmetry(SjRelaxParam& param) const;
    SLadderSegmentType relaxation_V_Symmetry(SjRelaxParam& param) const;
    SLadderSegmentType relaxation_A_Symmetry(SjRelaxParam& param) const;
    SLadderSegmentType _relaxation_A_Symmetry(SjRelaxParam& param) const;
    SLadderSegmentType relaxation(SjRelaxParam& param) const;

    inline double get_T2(double V_m, double A_m, double A_n) const;
    inline double get_T6(double V_m, double A_m, double A_n) const;
    inline double get_T6(double vm, double A_m, double an, double j) const;
    inline double get_T2(double V_m, double A_m, double A_n, double j) const;

    inline double fAm_2Vm(double A_m) const;
    inline double fAn_2Vm(double A_n) const;
    inline double fVm_2Am(double V_m) const;
    inline double fVm_2An(double V_m) const;
    inline double fAm_2An(double V_m) const;
    inline double fAn_2Am(double V_m) const;
    inline double fAn_2Vn(double An) const;
    inline double fVn_2An(double Vn) const;
    inline double fVn_2Am(double Vn) const;
    inline double fAm_2Vn(double Am) const{
        double A0s = (A_0 >= 0 ? (A_0*A_0):(-A_0*A_0));
        return V_0 + (A0s/2 - Am*Am)/J;
    }

    inline double fV0_2VmMin() const;
    inline double fVd_2VmMin() const;
    inline double fV0Vd2VmMin() const;

    double s60_2vm(double S, double A_m, double A_n) const ;
    double vaa2s60(double V_m, double A_m, double A_n) const ;
    double vaa2s51(double V_m, double A_n) const ;
    double _vaa2s51(double V_m, double A_m, double A_n) const;
    double _vaa2s52(double V_m, double A_m, double A_n) const;
    double vaa2s52(double V_m, double A_m) const ;
    double s51_2vm(double S, double A_n, double vmin=0, double vmax=1e9)const ;
    double s51_2am(double S, double V_m, double vmin=0, double vmax=1e9)const ;
    double s52_2vm(double S, double A_m, double vmin=0, double vmax=1e9)const ;
    double s52_2an(double S, double V_m, double vmin=0, double vmax=1e9)const ;
    double vaa2s40(double V_m, double A_m, double A_n) const;
    double vm2s40(double V_m) const;
    double am2s40(double A_m) const;
    double an2s40(double A_n) const;
    double s40_2vm(double S, double vmin=0, double vmax=1e9)const;
    double s40_2am(double S, double vmin=0, double vmax=1e9)const;
    double s40_2an(double S, double vmin=0, double vmax=1e9)const;
    double s40_2vm_q(double S, double vmin=0, double vmax=1e9)const;
    double vaa2s31() const;
    double vaa2s32()const;
    double vaa2s3()const;
    double vaa2s21() const;
    double vaa2s22()const;
    double vaa2s2()const;
    double s31s21_2vm(double S) const;
    double s31_2vm(double S) const;
    double s21_2am(double S) const;
    double vs2s31(double vs) const;
    double vs2s21(double vs) const;

    inline double cal_reachable_pre_vel(double vd) const {
        if (!is_scurve())
            return cal_reachable_trapz_vel(get_abs_S(), vd, A_n);
        else
            return cal_reachable_scurve_vel(get_abs_S(), vd, A_n, J);
    }
    inline double cal_reachable_pro_vel(double v0) const {
        if (!is_scurve())
            return cal_reachable_trapz_vel(get_abs_S(),v0, A_m);
        else
            return cal_reachable_scurve_vel(get_abs_S(), v0, A_m, J);
    }
    inline double zsymmetry_T2() const{
        return V_m/A_m - A_m/J;
    }
    inline double zsymmetry_T4() const{
        return get_S()/V_m - V_m/A_m - A_m/J;
    }
    inline double vsymmetry_T2() const{
        return (V_m-V_0)/A_m - A_m/J;
    }
    inline double vsymmetry_T4() const{
        return get_S()/V_m - V_m/A_m - A_m/J+ (J*V_0- A_m*A_m)/(V_m*A_m*J)*V_0;
    }
    inline double vsymmetry_nseg60_am2vm(double aM) const{
        return (sqrt(POWER4(aM) - 4*aM*aM*J*V_0 + 4*get_S()*aM*J*J + 4*J*J*V_0*V_0) - aM*aM)/(2*J);
    }
    inline double vsymmetry_nseg66_am2vm(double aM) const{
        double pp[3] = {1, -aM*aM/J,  2*get_S()/2*aM - V_0*V_0 -aM*aM*V_0/J};
        return quadtric_equation_solver(pp);
    }

    double vsymmetry_nseg40_s2am(double S) const;
    inline double zsymmetry_nseg60_am2vm(double aM) const{
        return (sqrt(aM*(CUBE(aM) + 4*get_S()*J*J)) - aM*aM)/2/J;
    }
    inline double nseg21_am2s(double am) const{ // assume A_0 = 0
        return (2*J*V_0 + SQUARE(am))*am/(J*J);
    }
    inline double nseg22_an2s(double an) const{
        // 2JVx - x^3 = J^2S
        //  => x^3 - 2JVx +J^2 S = 0
        return (2*J*V_0- SQUARE(an))*an/(J*J);
    }
    double nseg21_s2am(double S) const;
    double nseg22_s2an(double S) const;
    inline double get_acc_plain_time(double v0, double vm, double am, double J, double a0=0, double ad=0) const{
    // if (am > 0 && A_0 < am)  (am^2 -a0^2 + am^2 -0)/2J  
    // am > 0 && A_0 > am    (a0^2 - am^2 + am^2 -0)/2J  
    // am < 0 && A_0 > am    -(am^2 -a0^2 + am^2 -0)/2J  
    // am < 0 && A_0 < am   -(a0^2 - am^2 + am^2 -0)/2J 
        double sumA;
        if (am >= 0 && A_0 <= am)  sumA = (2*am*am - A_0*A_0)/2/J;
        else if (am >= 0 && A_0 > am) sumA = (A_0*A_0 - 2*am*am)/2/J;
        else if (am < 0 && A_0 >= am) sumA = -(2*am*am - A_0*A_0)/2/J;
        else if (am < 0 && A_0 < am) sumA = -(A_0*A_0 - 2*am*am)/2/J;
        return (vm - v0 - sumA)/am;
    }
    static inline double get_acc_plain_time_2(double dv, double am, double J, double a0=0, double ad=0){
        // if (am > 0 && A_0 < am)  (am^2 -a0^2 + am^2 -0)/2J  
        // am > 0 && A_0 > am    (a0^2 - am^2 + am^2 -0)/2J  
        // am < 0 && A_0 > am    -(am^2 -a0^2 + am^2 -0)/2J  
        // am < 0 && A_0 < am   -(a0^2 - am^2 + am^2 -0)/2J 
            double sumA;
            if (am >= 0 && a0 <= am)  sumA = (2*am*am - a0*a0 - ad*ad)/2/J;
            else if (am >= 0 && a0 > am) sumA = (a0*a0 - 2*am*am - ad*ad)/2/J;
            else if (am < 0 && a0 >= am) sumA = -(2*am*am - a0*a0 + ad*ad)/2/J;
            else if (am < 0 && a0 < am) sumA = -(a0*a0 - 2*am*am + ad* ad)/2/J;
            return (dv - sumA)/am;
        }

    inline double get_dec_plain_time(double vm, double vd, double am, double J) const{
        double sumA;
        if (am >= 0 )  sumA = (am*am)/J;
        else if (am < 0 ) sumA = -(am*am)/J;
        return (vd - vm - sumA)/am;
    }

}SLadderParams;

double SLadderParams::get_T6(double vm, double A_m, double an, double j) const{
    return (vm - an * an / j - V_d) / an;
}
double SLadderParams::get_T6(double V_m, double A_m, double A_n) const{
    return (V_m-A_n*A_n/J - V_d)/A_n;
}

    double SLadderParams::fAm_2Vm(double A_m) const{
        return V_0 + (A_m*A_m-A_0*A_0/2)/J;
    }

    double SLadderParams::fAm_2An(double Am) const{
        return sqrt(J*(V_0 - V_d) + Am*Am - A_0*A_0/2);
    }

    double SLadderParams::fAn_2Vm(double A_n) const{
        return V_d + A_n*A_n/J;
    }
    double SLadderParams::fAn_2Vn(double An) const{
        return V_d - An*An/J;
    }
    double SLadderParams::fVn_2An(double Vn) const{
        return sqrt((V_d - Vn) *J);
    }

    double SLadderParams::fVn_2Am(double Vn) const{
        double A0s = A_0 >= 0 ? (A_0*A_0):(-A_0*A_0);
        return sqrt(A0s/2 + (V_0 - Vn)*J );
    }

    double SLadderParams::fAn_2Am(double An) const{
        return sqrt(J*(V_d - V_0) + A_0*A_0/2 + An*An);
    }

    double SLadderParams::fVm_2Am(double V_m) const{
        return sqrt(J*(V_m -V_0)+A_0*A_0/2);
    }

    double SLadderParams::fVm_2An(double V_m) const{
        return sqrt(J*(V_m -V_d));
    }

    double SLadderParams::fV0_2VmMin() const{
        return V_0+A_0*A_0/2/J;
    }

    double SLadderParams::fVd_2VmMin() const{
        return V_d;
    }

    double SLadderParams::fV0Vd2VmMin() const{
        return MAX(fVd_2VmMin(), fV0_2VmMin());
    }
double SLadderParams::get_T2(double V_m, double A_m, double A_n) const{
    return (A_0*A_0 - 2*A_m*A_m - 2*J*V_0 + 2*J*V_m)/(2*J*A_m);
}
double SLadderParams::get_T2(double vm, double am, double A_n, double j) const{
    return (A_0*A_0 - 2*am*am - 2*j*V_0 + 2*j*vm)/(2*j*am);
}


SLadderSegmentType sLadderRelaxationZsymmetry(SLadderParams& slp, SjRelaxParam& param);
SLadderSegmentType sLadderRelaxationVsymmetry(SLadderParams& slp, SjRelaxParam& param);
SLadderSegmentType sLadderRelaxationAsymmetry(SLadderParams& slp, SjRelaxParam& param);


typedef struct SLadderRelaxer:public SLadderParams{
    inline double nseg11_a2s(double am) const{
        double T = (am - A_0)/J;
        return T*(V_0 + A_0*T/2 + J*T*T/6) ;
    }
    inline double nseg11_a2v(double am) const{
        return V_0 + (am* am - A_0*A_0)/2/J;
    }
    inline double nseg12_a2s(double am) const{
        double T = (A_0 - am)/J;
        return T*(V_0 + A_0*T/2 - J*T*T/6) ;
    }
    inline double nseg12_a2v(double am) const{
        return V_0 - (am* am - A_0*A_0)/2/J;
    }

    double nseg21_vm2s(double vm) const;
    double nseg22_vm2s(double vm) const;
    double nseg23_vn2s(double vn) const;
    double nseg24_vn2s(double vn) const;
    double nseg31_vm2s(double vm) const;
    double nseg35_vn2s(double vn) const;
    double nseg36_vn2s(double vn) const;
    inline double nseg31za_vm2s(double v0, double vm, double am) const{
        return ((v0 + vm)*(am*am - J*v0 + J*vm))/(2*am*J);
    }
    double nseg32_vm2s(double vm) const;
    double nseg21_v2s(double v0, double vm) const;
    double nseg31_v2s(double v0, double vm, double am) const;

    bool is_reachable() const;
    bool has_dv_plain(double dv, double am, double a0=0) const;
    bool has_plain_acc(double vm, double am) const;
    bool has_plain_dec(double vm, double an) const;
    // Placeholders aligned with SinSquareRelaxer branches

    SLadderSegmentType relaxation(SjRelaxParam& param) const;
    
    SLadderSegmentType _relaxation(SjRelaxParam& param) const;
    SLadderSegmentType _relaxation_sym(SjRelaxParam& param) const;
    SLadderSegmentType _relaxation_sym_zero(SjRelaxParam& param) const;    
    SLadderSegmentType _relaxation_sym_overlimit(SjRelaxParam& param) const;
    SLadderSegmentType _relaxation_sym_inv_db(SjRelaxParam& param) const;
    SLadderSegmentType _relaxation_sym_main(SjRelaxParam& param) const;
    SLadderSegmentType _relaxation_asym(SjRelaxParam& param) const;
    SLadderSegmentType _relaxation_asym_main(SjRelaxParam& param) const;
    SLadderSegmentType _relaxation_asym_main0(SjRelaxParam& param) const;
    SLadderSegmentType _relaxation_asym_overlimit_db(SjRelaxParam& param) const;
    SLadderSegmentType _relaxation_asym_overlimit_sg(SjRelaxParam& param) const;
    SLadderSegmentType _relaxation_asym_main_inlimit(SjRelaxParam& param) const;
    SLadderSegmentType _relaxation_asym_main_inlimit0(SjRelaxParam& param) const;
    SLadderSegmentType _relaxation_asym_inv_db(SjRelaxParam& param) const;
    SLadderSegmentType _relaxation_asym_inv_sg(SjRelaxParam& param) const;

    SLadderSegmentType _relaxation_asym_inv_overlimit(SjRelaxParam& param) const;
    SLadderSegmentType _relaxation_unreachable(SjRelaxParam& param) const;

    bool is_reachable_asym_overlimit(SjRelaxParam& param, double V_m) const;
    bool is_reachable_asym_main_vm(SjRelaxParam& param, double Vm) const;
    SLadderSegmentType _shrink_Vm(SjRelaxParam& param) const;
    SLadderSegmentType _shrink_Vm_Am(SjRelaxParam& param) const;
    SLadderSegmentType _shrink_Vm_An(SjRelaxParam& param) const;
    SLadderSegmentType _shrink_Vm_Am_An(SjRelaxParam& param) const;

    double nseg35_23_s2vm(double S, double xMin, double xMax) const;
};

struct SLadderRelaxer2:public SLadderRelaxer{
    SLadderSegmentType relaxation(SjRelaxParam& param) const;
};

}
#endif  // ZS_VELOCITY_LADDER_H_
/* end of file */
