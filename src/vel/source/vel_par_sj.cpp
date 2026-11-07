/*
 * @Author: ChenShengyong
 * @LastEditors: Chen Shengyong
 * @Date: 2022-02-11 13:54:22
 * @LastEditTime: 2023-02-10 11:57:50
 * @Description: Modify here please
 */

#include <math.h>
#include "vel_par_sj.h"
#include "stdlib.h"
#include "base/include/solver.h"
#include "VelSj.h"
#include "stdio.h"

// std::qsort
#define secantItermethod secantItermethod2

namespace zmc
{

inline SLadderSegmentType setSegSym(SjRelaxParam& param, SLadderSegmentType t){
    param.aMin = param.aMax;
    param.seg_type = t;
    return t;
}

inline SLadderSegmentType setSeg(SjRelaxParam& param, SLadderSegmentType t){
    param.seg_type = t;
    return t;
}
inline SLadderSegmentType setSegAndInvAn(SjRelaxParam& param, SLadderSegmentType t){
    param.seg_type = t;
    param.aMin = - param.aMin;
    return t;
}

#define EPS (1e-9)
#define TOL (1e-8)
#define V_EPS (1e-9)

    inline double  speed_f3(double x, double v0, double vd){
        return (x+v0)*sqrt(x-v0)+ (x + vd)*sqrt(x-vd);
    }

    inline double  speed_f3s_diff(double x, double v0){
        return sqrt(x-v0)+ (x + v0)/(2*sqrt(x-v0));
    }
    inline double polyval8(const double* pp, double v){
        return ((((((pp[0]*v + pp[1])*v + pp[2])*v + pp[3])*v + pp[4])*v + pp[5])*v + pp[6])*v + pp[7];
    }

    double cheby_fit_cuberoot(double x){
        double v;
        if (x<=4){
            static const double pp[] = {0.0157752931686585,0.00566025580158995,-0.0716064541337475,0.0841610051134782,-0.0632046950949161,0.0115194088936252,0.814588747358816,1.59446411040191};
            return polyval8(pp,(x-2)/2);}
        else if (x<=25){
            static const double pp[] = {-0.000196257436967026,-0.00554561774238865,0.0179614811301102,-0.0364764504590093,0.0928084300114162,-0.319942111784364,2.82940942204228,5.69318146843736};
            return polyval8(pp,(x-14.5)/10.5);}
        else if (x<=100){
            static const double pp[] = {0.00652949930395723,-0.0149400017756596,0.0213058332046089,-0.0534937536344770,0.164824411573261,-0.624544626672955,6.28768711302755,15.4447149373816};
            return polyval8(pp,(x-62.5)/37.5);}
        else if (x<=1000){
            static const double pp[] = {0.665496434204695,-0.849965652984333,-0.0189170277598125,-0.402367837862923,1.97148652399738,-5.09633775902531,36.5990611319650,66.8056609245607};
            return polyval8(pp,(x-550)/450);
        }
        else if (x<=10000){
            static const double pp[] = {3.11088572574184,-3.96728901096797,-0.103770203551276,-1.85741833117089,9.16587446086293,-23.6701168025030,169.893412191571,311.268903411318};
            return polyval8(pp,(x-5500)/4500);
        }
        else{
            return 463.85* pow((x/10000),2.0/3);
        }
    }
    inline double solve_third_equ(double s){
        return cheby_fit_cuberoot(sqrt(s));
    }
    inline double solve_third_equ2(double S, double a){
        // sqrt(x-a)*(x+a)-S = 0
        if (a==0)
            return cbrt(S*S);
        else
            return solve_third_equ(S*S/a/a/a)*a;
    }

    inline double cheby_fit_cuberoot3(double S, double a, double b){
        if (a==b)
            return solve_third_equ2(S/2, a);
        else if (b==0 || a/b>= 100) 
            return solve_third_equ(S*S/4/a/a/a)*a;
        else if (a==0 || b/a>= 100)
            return solve_third_equ(S*S/4/b/b/b)*b;
        else
            return (solve_third_equ(S*S/4/a/a/a)*a+solve_third_equ(S*S/4/b/b/b)*b)/2;
    }



SLadderSymmetry SLadderParams::get_symmetry() const{
    if (A_0 == 0 && A_m==A_n && V_0 == V_d){
        if (V_0 == 0)
            return SLadderSymmetry::SLADDER_Z_SYMMETRY;
        else
            return SLadderSymmetry::SLADDER_V_SYMMETRY;
    }else
        return SLadderSymmetry::SLADDER_AYMMETRY;
}

SLadderSortType SLadderParams::sort_type() const{
        double Vm2 = fAm_2Vm(A_m);
        double Vm3 = fAn_2Vm(A_n);
        if (V_m >= Vm2 && V_m >= Vm3){
            if (Vm2 > Vm3)
                return SLadderSortType::SLADDER_Vm_Am_An;
            else
                return SLadderSortType::SLADDER_Vm_An_Am;
        }else if (V_m > Vm2 && V_m < Vm3)
            return SLadderSortType::SLADDER_An_Vm_Am;
        else if (V_m < Vm2 && V_m > Vm3)
            return SLadderSortType::SLADDER_Am_Vm_An;
        else if (V_m > Vm2 && V_m < Vm3) { 
            if (Vm2 > Vm3)
                return SLadderSortType::SLADDER_Am_An_Vm;
            else
                return SLadderSortType::SLADDER_An_Am_Vm;
        }
        return SLadderSortType::SLADDER_An_Am_Vm ;
} 


SLadderSegmentType SLadderParams::relaxation_Z_Symmetry(SjRelaxParam& param) const{
    double& V_t = param.vMax;
    double& A_t = param.aMax;
    param.plainTime = 0;
    double S = get_S();
    double T_2 = zsymmetry_T2();
    double T_4 = zsymmetry_T4();
    double aMn = MIN(A_m, A_n);
    if (T_2 >= 0 && T_4 >= 0){// 7 segments
        A_t = aMn;
        V_t = V_m;
        param.plainTime = T_4;
        return setSegSym(param, SLADDER_70_SEGMENT);
        // [V_m, A_m, A_n]
    }else if (T_2 < 0 && T_4 >= 0){// 5 segments
        V_t = V_m;
        A_t = sqrt(V_m*J);
        param.plainTime = get_S()/V_m - V_m/A_t - A_t/J;;
        return setSegSym(param, SLADDER_50_SEGMENT);
    }else if (T_2 >= 0 && T_4 < 0){// shrink Vm
        double S21 = nseg21_am2s(aMn);
        if (S > S21*2 - TOL){
            V_t = zsymmetry_nseg60_am2vm(aMn);
            A_t = aMn;
            return setSegSym(param, SLADDER_60_SEGMENT);
        }else{
            V_t = pow(S*S*J/4, 1.0/3);
            A_t = sqrt(J*V_t);
            return setSegSym(param, SLADDER_40_SEGMENT);
        }
    }else { // if (T_2 < 0 && T_4 < 0)  
        // shrink Vm Am
        V_t = pow(S*S*J/4, 1.0/3);
        A_t = sqrt(J*V_t);
        return setSegSym(param, SLADDER_40_SEGMENT);
    }
}

SLadderSegmentType SLadderParams::relaxation_V_Symmetry(SjRelaxParam& param) const {
    double& V_t = param.vMax;
    double& A_t = param.aMax;
    param.plainTime = 0;
    double S = get_S();
    double T_2 = vsymmetry_T2();
    double T_4 = vsymmetry_T4();
    double aMn = MIN(A_m, A_n);
    if (T_2 >= 0 && T_4 >= 0){// 7 segments
        A_t = aMn;
        V_t = V_m;
        param.plainTime = T_4;
        return setSegSym(param, SLADDER_70_SEGMENT);
        // [V_m, A_m, A_m]
    }else if (T_2 < 0 && T_4 >= 0){// 5 segments
        V_t = V_m;
        A_t = fVm_2An(V_m); 
        if (A_t< TOL){
            param.plainTime = get_S()/V_m;
            return setSegSym(param, SLADDER_10_SEGMENT);
        }
        param.plainTime = get_S()/V_m - V_m/A_t - A_t/J+ (J*V_0- A_t*A_t)/(V_m*A_t*J)*V_0;
        return setSegSym(param, SLADDER_50_SEGMENT);//, [V_m, A_t, A_t]
    }if (T_2 >= 0 && T_4 < 0){ // shrink Vm
        double S21 = nseg21_am2s(aMn);
        if (S < S21*2){
            A_t = nseg21_s2am(S/2);
            V_t = fAn_2Vm(A_t);
            return setSegSym(param, SLADDER_40_SEGMENT);
        }else{
            V_t = vsymmetry_nseg60_am2vm(aMn);
            A_t = aMn;
            return setSegSym(param, SLADDER_60_SEGMENT);
        }
    }else { //  if (T_2 < 0 && T_4 < 0) 
        // shrink Vm, Am
        A_t = vsymmetry_nseg40_s2am(S);
        V_t = fAn_2Vm(A_t);
        return setSegSym(param, SLADDER_40_SEGMENT);
    }
}
SLadderSegmentType SLadderParams::relaxation_A_Symmetry(SjRelaxParam& param) const{
    auto ret = _relaxation_A_Symmetry(param);
    param.seg_type = ret;
    return ret;
}

SLadderSegmentType SLadderParams::_relaxation_A_Symmetry(SjRelaxParam& param) const{
    double& V_t = param.vMax;
    double& A_t = param.aMax;
    double& An_t = param.aMin;
    int& errCode = param.err;
    double T2 = get_T2(V_m, A_m, A_n);
    double T6 = get_T6(V_m, A_m, A_n);
    double vm_min = fV0Vd2VmMin();
    double S_d = this->S_d - this->S_0;
    errCode = 0;
    if (T2 >= 0 && T6 >= 0){
        double S60 = vaa2s60(V_m, A_m, A_n);
        if (S_d >= S60){
            V_t = V_m;
            A_t = A_m;
            An_t = A_n;
            return SLADDER_70_SEGMENT;
        }else {
            double Vm2 = fAm_2Vm(A_m);
            double Vm3 = fAn_2Vm(A_n);
            if (Vm2 > Vm3){
                double S51 =  vaa2s51(Vm2, A_n);
                if (S_d >= S51){
                    V_t = s60_2vm(S_d, A_m, A_n);
                    // assert Vmt <= V_m
                    A_t = A_m;
                    An_t = A_n;
                    return SLADDER_60_SEGMENT;
                }else{
                    if (Vm3 < vm_min){
                        double S32 = vaa2s51(vm_min, A_n); // vaa2s32()
                        if (S_d > S32 + TOL){
                            // errCode = 1;
                            // return SLADDER_0_SEGMENT;
                            A_t = s51_2am(S_d, A_n, fVm_2Am(vm_min), V_m);
                            V_t = fAm_2Vm(A_t);
                            An_t = A_n;
                            return SLADDER_51_SEGMENT;
                        }else if (S_d > S32 - TOL){
                            V_t = fV0_2VmMin();
                            A_t = 0;
                            An_t = A_n;
                            return SLADDER_32_SEGMENT;
                        }else{
                            V_t = fV0_2VmMin();
                            A_t = 0;
                            An_t = A_n;
                            errCode = 1;
                            return SLADDER_32_SEGMENT;
                        }
                    }else{
                        double S40 = an2s40(A_n);
                        if (S_d >= S40 + TOL){
                            // V_t = s51_2vm(S_d, A_n, vm_min, V_m);
                            // A_t = fVm_2Am(V_t);
                            // An_t = A_n;

                            A_t = s51_2am(S_d, A_n, fVm_2Am(vm_min), V_m);
                            V_t = fAm_2Vm(A_t);
                            An_t = A_n;

                            return SLADDER_51_SEGMENT;
                        }else if (S_d >= S40 - TOL){
                            V_t = fV0_2VmMin();
                            A_t = 0;
                            An_t = A_n;
                            return SLADDER_32_SEGMENT;
                        }else{
                            double S22 = vaa2s2();
                            if (S_d >= S22 + TOL){
                                // V_t = s40_2vm(S_d, vm_min, V_m);
                                // A_t = fVm_2Am(V_t);
                                // An_t = fVm_2An(V_t);

                                A_t = s40_2am(S_d, fVm_2Am(vm_min), A_m);
                                V_t = fAm_2Vm(A_t);
                                An_t = fAm_2An(A_t);
                                return SLADDER_40_SEGMENT;// T2=0 T4=0 T6=0
                            }else if (S_d >= S22 - TOL){
                                V_t = vm_min;
                                A_t = 0;
                                An_t = fVm_2An(V_t);
                                return SLADDER_22_SEGMENT;
                            }else{
                                errCode = 1;
                                V_t = vm_min;
                                A_t = 0;
                                An_t = fVm_2An(V_t);
                                return SLADDER_22_SEGMENT;
                                // return SLADDER_0_SEGMENT;
                            }
                        }
                    }
                }
            }else{
                double S52 = vaa2s52(Vm3, A_m);
                if (S_d >= S52){
                    V_t = s60_2vm(S_d, A_m, A_n);
                    A_t = A_m;
                    An_t = A_n;
                    return SLADDER_60_SEGMENT;
                }
                else{
                    if (Vm2 < vm_min){
                        double S31 = vaa2s52(vm_min, A_m); // vaa2s31()
                        if (S_d > S31 + TOL){
                            An_t = s52_2an(S_d, A_m, fVm_2An(vm_min), A_n);
                            A_t = A_m;
                            V_t = fAn_2Vm(An_t);
                            if (An_t < TOL){
                                return SLADDER_31_SEGMENT;
                            }
                            return SLADDER_52_SEGMENT;//, (Vmt, A_m, Ant) // T6 = 0
                        }else if (S_d > S31 - TOL){
                            V_t = s31_2vm(S_d);
                            A_t = A_m;
                            An_t = 0;
                            return SLADDER_31_SEGMENT;
                        }else{
                            V_t = s31_2vm(S_d);
                            A_t = A_m;
                            An_t = 0;
                            errCode = 1;
                            return SLADDER_0_SEGMENT;
                        }
                    }else{
                        double S40 = am2s40(A_m);
                        if (S_d >= S40 + TOL){
                            // V_t = s52_2vm(S_d, A_m, vm_min, V_m);
                            // A_t = A_m;
                            // An_t = fVm_2An(V_t);
                            An_t = s52_2an(S_d, A_m, fVm_2An(vm_min), A_n);
                            A_t = A_m;
                            V_t = fAn_2Vm(An_t);
                            if (An_t < TOL){
                                return SLADDER_31_SEGMENT;
                            }
                            return SLADDER_52_SEGMENT;//, (Vmt, A_m, Ant) // T6 = 0
                        }else if (S_d >= S40 - TOL){
                            V_t = V_d;
                            A_t = A_m;
                            An_t = 0;
                            return SLADDER_31_SEGMENT; 
                        }else{
                            double S21 = vaa2s2();
                            if (S_d >= S21 + TOL){
                                // V_t = s40_2vm(S_d, vm_min, V_m);
                                // A_t = fVm_2Am(V_t);
                                // An_t = fVm_2An(V_t);
                                An_t = s40_2an(S_d, fVm_2An(vm_min), A_n);
                                V_t = fAn_2Vm(An_t);
                                A_t = fAn_2Am(An_t);
                                return SLADDER_40_SEGMENT;//, (Vmt, Amt, Ant) // T2=0 T4=0 T6=0
                            }else if (S_d >= S21 - TOL){
                                V_t = vm_min;
                                A_t = fVm_2Am(V_t);
                                An_t = 0;
                                return SLADDER_21_SEGMENT;//, (Vmt, Amt, Ant) // T2=0 T4=0 T6=0
                            }else{
                                A_t = s21_2am(S_d);
                                V_t = fAm_2Vm(A_t);
                                An_t = 0;
                                errCode = 1;
                                if (A_0 > A_t || V_t > V_m || A_t > A_m){
                                    errCode = 4;
                                    return SLADDER_21_SEGMENT;
                                }
                                return SLADDER_21_SEGMENT;
                            }
                        }
                    }
                }
            }
        }
    }else if (T2 < 0 && T6 >= 0){
        double Amt = fVm_2Am(V_m); // 收缩 Am
        // assert Amt <= A_m
        double S51 =  vaa2s51(V_m, A_n);
        if (S_d >= S51){
            V_t = V_m;
            A_t = fVm_2Am(V_t);
            An_t = A_n;
            if (A_t < TOL)
                return SLADDER_42_SEGMENT;
            return SLADDER_61_SEGMENT;// (V_m, Amt, A_n) // T2=0   
        }else{
            // 收缩 Vm Am
            if (vm_min > fAm_2Vm(A_m) || vm_min > fAn_2Vm(A_n)){
                double S32 = vaa2s51(vm_min, A_n); // vaa2s32()
                if (S_d >= S32 + TOL){
                    V_t = s51_2vm(S_d, A_n, vm_min, V_m);
                    A_t = fVm_2Am(V_t);
                    An_t = A_n;
                    return SLADDER_51_SEGMENT;//, (Vmt, Amt, A_n);
                }else if (S_d >= S32 - TOL){
                    V_t = vm_min;
                    A_t = 0;
                    An_t = A_n;
                    return SLADDER_32_SEGMENT;// (Vmt, Amt, A_n);
                }else{
                    errCode = 1;
                    V_t = vm_min;
                    A_t = 0;
                    An_t = A_n;
                    // return SLADDER_32_SEGMENT;
                    return SLADDER_0_SEGMENT;
                }
            }else{
                double S40 = an2s40(A_n);
                if (S_d >= S40 + TOL){
                    V_t = s51_2vm(S_d, A_n, vm_min, V_m);
                    A_t = fVm_2Am(V_t);
                    An_t = A_n;
                    return SLADDER_51_SEGMENT;//, (Vmt, Amt, A_n);
                }else {
                    double S22 = vaa2s2();
                    if (S_d >= S22 + TOL){
                        // V_t = s40_2vm(S_d, vm_min, V_m);
                        // A_t = fVm_2Am(V_t);
                        // An_t = fVm_2An(V_t);
                        A_t = s40_2am(S_d, fVm_2Am(vm_min), A_m);
                        V_t = fAm_2Vm(A_t);
                        An_t = fAm_2An(A_t);
                        return SLADDER_40_SEGMENT;
                    }else if (S_d >= S22 - TOL){
                        V_t = V_0;
                        A_t = 0;
                        An_t = fVm_2An(V_t);
                        return SLADDER_22_SEGMENT;
                    }
                    else{ 
                        errCode = 1;
                        V_t = V_0;
                        A_t = 0;
                        An_t = fVm_2An(V_t);
                        return SLADDER_22_SEGMENT;
                    }
                }
            }  
        }      
    }else if (T2 >= 0 && T6 < 0){
        double Ant = fVm_2An(V_m); // 收缩 A_n
        // assert Ant <= A_n;
        double S52 =  vaa2s52(V_m, A_m);
        if (S_d >= S52){
            V_t = V_m;
            A_t = A_m;
            An_t = fVm_2An(V_t);
            if (An_t < TOL)
                return SLADDER_41_SEGMENT;
            return SLADDER_62_SEGMENT;// , (V_m, A_m, Ant)  // T6=0  
        }else{
            // 收缩 Vm An
            if (vm_min > fAn_2Vm(A_n) || vm_min > fAm_2Vm(A_m)){// S4 = vaa2s31()
                double S31 = vaa2s52(vm_min, A_m);
                if (S_d > S31 + TOL){
                    An_t = s52_2an(S_d, A_m, fVm_2An(vm_min), A_n);
                    A_t = A_m;
                    V_t = fAn_2Vm(An_t);
                    if (An_t < TOL)
                        return SLADDER_31_SEGMENT;
                    return SLADDER_52_SEGMENT;//, (Vmt, A_m, Ant) // T4=0 T6=0
                }else if (S_d > S31 - TOL){
                    V_t = s31_2vm(S_d);
                    A_t = A_m;
                    An_t = 0;
                    if (V_t < fAm_2Vm(A_t))
                        return SLADDER_0_SEGMENT;
                    return SLADDER_31_SEGMENT;
                }else{
                    V_t = s31_2vm(S_d);
                    A_t = A_m;
                    An_t = 0;
                    if (V_t < fAm_2Vm(A_t))
                        return SLADDER_0_SEGMENT;
                    return SLADDER_0_SEGMENT;
                }
            }else{
                double S40 = am2s40(A_m);
                if (S_d >= S40){
                    // V_t = s52_2vm(S_d, A_m, vm_min, V_m);
                    // A_t = A_m;
                    // An_t = fVm_2An(V_t);
                    An_t = s52_2an(S_d, A_m, fVm_2An(vm_min), A_n);
                    A_t = A_m;
                    V_t = fAn_2Vm(An_t);
                    if (An_t < TOL)
                        return SLADDER_31_SEGMENT;
                    return SLADDER_52_SEGMENT;//, (Vmt, A_m, Ant) // T4=0 T6=0
                }else{
                    double S21 = vaa2s2();
                    if (S_d >= S21 + TOL){
                        // V_t = s40_2vm(S_d, vm_min, V_m);
                        // A_t = fVm_2Am(V_t);
                        // An_t = fVm_2An(V_t);
                        An_t = s40_2an(S_d, fVm_2An(vm_min), A_n);
                        V_t = fAn_2Vm(An_t);
                        A_t = fAn_2Am(An_t);
                        return SLADDER_40_SEGMENT;//, (Vmt, Amt, Ant) // T2=0 T4=0 T6=0
                    }else if (S_d >= S21 - TOL){
                        V_t = V_d;
                        A_t = s21_2am(S_d);
                        An_t = 0;
                        return SLADDER_21_SEGMENT;//, (Vmt, Amt, Ant) // T2=0 T4=0 T6=0
                    }else{
                        A_t = s21_2am(S_d);
                        V_t = fAm_2Vm(A_t);
                        An_t = 0;
                        errCode = 1;
                        if (A_0 > A_t || V_t > V_m || A_t > A_m){
                            errCode = 4;
                            return SLADDER_21_SEGMENT;
                        }
                        return SLADDER_21_SEGMENT;
                    }
                }
            }
        }
    }else if (T2 < 0 && T6 < 0){
        double Amt = fVm_2Am(V_m); // 收缩 Am
        double Ant = fVm_2An(V_m); // 收缩 An
        // assert Amt <= A_m and Ant <= A_n
        if (Amt < A_0){
            errCode = 4;
            return SLADDER_0_SEGMENT;
        }
        double S40 = vm2s40(V_m);
        if (S_d >= S40 + TOL){
            V_t = V_m;
            A_t = Amt;
            An_t = Ant;
            if (A_t < TOL){
                return SLADDER_34_SEGMENT;
            }else if (An_t < TOL){
                return SLADDER_33_SEGMENT;
            }
            return SLADDER_50_SEGMENT;
        }if (S_d >= S40 - TOL){
            V_t = V_m;
            A_t = Amt;
            An_t = Ant;
            if (A_t < TOL){
                return SLADDER_22_SEGMENT;
            }else if (An_t < TOL){
                return SLADDER_21_SEGMENT;
            }
            return SLADDER_40_SEGMENT;
        }
        else{
            double S2 = vaa2s2();
            if (S_d >= S2 + TOL){
                V_t = s40_2vm(S_d, vm_min, V_m);
                A_t = fVm_2Am(V_t);
                An_t = fVm_2An(V_t);        
                return SLADDER_40_SEGMENT;
            }else if (S_d >= S2 - TOL){
                if (fV0_2VmMin() < fVd_2VmMin()){
                    V_t = fVd_2VmMin();
                    A_t = fVm_2Am(V_t);
                    An_t = 0;        
                    return SLADDER_21_SEGMENT;
                }else{
                    V_t = fV0_2VmMin();
                    A_t = 0;
                    An_t = fVm_2An(V_t);      
                    return SLADDER_22_SEGMENT;
                }
            }else{
                if (vm_min > V_d){
                    errCode = 4;  
                    return SLADDER_22_SEGMENT;
                    // return SLADDER_0_SEGMENT;
                }else{
                    A_t = s21_2am(S_d);
                    V_t = fAm_2Vm(A_t);
                    An_t = 0;
                    errCode = 1;  
                    if (A_0 > A_t || V_t > V_m || A_t > A_m){
                        errCode = 4;  
                        return SLADDER_21_SEGMENT;
                    }
                    return SLADDER_21_SEGMENT;
                }
            }
        }
    }

    V_t = V_m;
    A_t = A_m;
    An_t = A_n;
    return SLADDER_70_SEGMENT;
}

// Placeholder branches (no concrete segment logic unless forwarding)


SLadderSegmentType SLadderParams::relaxation(SjRelaxParam& param) const{
    // velBuild treats nseg <= 0 as failure; zero acc/dec cannot form a profile.
    if (A_m <= 0 || A_n <= 0)
        return SLADDER_0_SEGMENT;
    if (get_S() < 0){
        auto param2 = *this;
        param2.invert();
        param2.A_m = A_n;
        param2.A_n = A_m;
        auto ret = param2.relaxation(param);
        param.invert();
        return ret;
    }
    if (get_symmetry() == SLadderSymmetry::SLADDER_Z_SYMMETRY){
        return relaxation_Z_Symmetry(param);
    }else if (get_symmetry() == SLadderSymmetry::SLADDER_V_SYMMETRY){
        return relaxation_V_Symmetry(param);
    }else{
        return relaxation_A_Symmetry(param);
    };
};


    double SLadderParams::s60_2vm(double S, double Am, double An) const {
        double J2=J*J;
        
        if (Am == An){
            double V2sq = (3*A_0*CUBE(A_0) - 8*Am*CUBE(A_0) + 24*J*A_0*Am*V_0 + 6*(Am*Am - 2*J *V_0)*A_0*A_0 + 6*(Am*CUBE(Am) + 4*Am*S*J2 - 2*J*(V_0 + V_d)*Am*Am + 2*(V_0*V_0 + V_d*V_d)*J2));
            double v_max = (sqrt(6*V2sq)-6*Am*Am)/(12*J);
            return v_max;
        }
        double A_m2 = Am*Am;
        double A_n2 = An*An;
        double A_02 = A_0*A_0;
        double pp[3] = {
            (12*Am*J2 + 12*An*J2),
            (12*J*A_m2*An + 12*J*Am*A_n2),
            - 3*A_02*A_02*An + 8*A_0*A_02*Am*An - 6*A_02*A_m2*An + 12*A_02*An*J*V_0 - 24*A_0*Am*An*J*V_0 + 12*A_m2*An*J*V_0 + 12*Am*A_n2*J*V_d - 24*S*Am*An*J2 - 12*Am*J2*V_d*V_d - 12*An*J2*V_0*V_0
        };

        double vm = quadtric_equation_solver(pp);
        return vm;
    };

    double SLadderParams::vaa2s60(double Vm, double Am, double An) const {
        double T_2 = get_T2(Vm, Am, An);
        double T_6 = get_T6(Vm, Am, An);
        double J2 = J*J;

        double s = (2*CUBE(A_0) - 6*SQUARE(A_0)*Am - 3*SQUARE(A_0)*J*T_2 - 6*V_0*A_0*J + 6*CUBE(Am) + 9*SQUARE(Am)*J*T_2 + 3*Am*J2*T_2*T_2 + 12*V_0*Am*J - 6*CUBE(An) - 9*SQUARE(An)*J*T_6 - 3*An*J2*T_6*T_6 + 12*Vm*An*J + 6*V_0*J2*T_2 + 6*Vm*J2*T_6)/(6*J2);
        return s;
    };

    double SLadderParams::vaa2s51(double Vm, double An) const{
        double am = fVm_2Am(Vm); 
        // sqrt(J*(Vm-V_0)+SQUARE(A_0)/2);
        return _vaa2s51(Vm, am, An);
    };

    double SLadderParams::_vaa2s51(double Vm, double Am, double An) const{
        double J2 = SQUARE(J);
        double S1 = (2*Am-A_0)*J*V_0 + CUBE(Am) - Am*SQUARE(A_0)+ CUBE(A_0)/3;
        double S2 = (V_d+Vm)* SQUARE(An)+(SQUARE(Vm)-SQUARE(V_d))*J;
        return S1/J2+S2/(2*An*J);
    };

    double SLadderParams::vaa2s52(double Vm, double Am) const {
        double an = fVm_2An(Vm); 
        return _vaa2s52(Vm, Am, an);
    };

    double SLadderParams::_vaa2s52(double Vm, double Am, double An)const{
        double J2 = SQUARE(J);
        double S1 = 0.5*J*V_0*SQUARE(A_0) + 0.5*(SQUARE(Vm)-SQUARE(V_0))*J2-POWER4(A_0)/8;
        double S2 = CUBE(A_0)/3 - A_0*J*V_0 - (CUBE(An) - 2*An*J*Vm);
        double S3 = 0.5*J*(V_0 + Vm)-0.25*SQUARE(A_0);
        return S1/(Am*J2)+S2/J2+Am*S3/J2;
    };

    double SLadderParams::s51_2vm(double S, double An, double vmin, double vmax) const {
        double vm = secantItermethod(
            [this, S, An](double x){return this->vaa2s51(x, An) - S;},
            vmin, vmax, 1e-5);
        return vm;
    };

    double SLadderParams::s51_2am(double S, double An, double vmin, double vmax) const {
        double vm = secantItermethod(
            [this, S, An](double x){return this->_vaa2s51(fAm_2Vm(x), x, An) - S;},
            vmin, vmax, 1e-5);
        return vm;
    };

    double SLadderParams::s52_2vm(double S, double Am, double vmin, double vmax) const {
        double vm = secantItermethod(
            [this, S, Am](double x){return this->vaa2s52(x, Am) - S;},
             vmin, vmax, 1e-5);
        return vm;
    };

    double SLadderParams::s52_2an(double S, double Am, double vmin, double vmax) const {
        double vm = secantItermethod(
            [this, S, Am](double x){return this->_vaa2s52(fAn_2Vm(x), Am, x) - S;},
             vmin, vmax, 1e-5);
        return vm;
    };

    double SLadderParams::vaa2s40(double Vm, double Am, double An) const {
        double S = ((2*An*Vm+2*Am*V_0 - A_0*V_0)*J + CUBE(Am) +CUBE(A_0)/3 - SQUARE(A_0)*Am - CUBE(An))/J/J;
        return S;
    };
    
    double SLadderParams::vm2s40(double Vm) const {
        double am = fVm_2Am(Vm);        
        double an = fVm_2An(Vm);
        return vaa2s40(Vm, am, an);
    };

    double SLadderParams::am2s40(double Am) const {
        double vm = fAm_2Vm(Am);
        double an = fVm_2An(vm);
        return vaa2s40(vm, Am, an);
    };

    double SLadderParams::an2s40(double An) const {
        double vm = fAn_2Vm(An);
        double am = fVm_2Am(vm);   
        return vaa2s40(vm, am, An);
    };

    double SLadderParams::s40_2vm(double S, double vmin, double vmax) const {
        if (get_symmetry() == SLADDER_Z_SYMMETRY){
            double Vm = pow(S*S*J/4,1.0/3);
            return Vm;
        }else{
            
        }
        // [=](int x)-> double {return x+10;};
        // [this, S](double x)-> double {return this->vm2s40(x) - S;};
        double vm = secantItermethod(
            [this, S](double x){return this->vm2s40(x) - S;}, 
            vmin, vmax, EPS);
        return vm;
    };

    double SLadderParams::s40_2am(double S, double vmin, double vmax) const {
        if (get_symmetry() == SLADDER_Z_SYMMETRY){
            double am = pow(S*J*J/2, 1.0/3);
            return am;
        }else if(get_symmetry() == SLADDER_V_SYMMETRY){
           // todo 
        }
        // [=](int x)-> double {return x+10;};
        // [this, S](double x)-> double {return this->vm2s40(x) - S;};
        double am = secantItermethod(
            [this, S](double x){return this->am2s40(x) - S;}, 
            vmin, vmax, EPS);
        return am;
    };

    double SLadderParams::s40_2an(double S, double vmin, double vmax) const {
        if (get_symmetry() == SLADDER_Z_SYMMETRY){
            double an = pow(S*J*J/2, 1.0/3);
            return an;
        }else if(get_symmetry() == SLADDER_V_SYMMETRY){
           // todo 
        }
        // [=](int x)-> double {return x+10;};
        // [this, S](double x)-> double {return this->vm2s40(x) - S;};
        double an = secantItermethod(
            [this, S](double x){return this->an2s40(x) - S;}, 
            vmin, vmax, EPS);
        return an;
    };

    /// @brief 
    /// @param S 
    /// @param vmin 
    /// @param vmax 
    /// @return 
    double SLadderParams::s40_2vm_q(double S, double vmin, double vmax) const {
        double SJ = S*sqrt(J);
        double v = cheby_fit_cuberoot3(SJ, V_0, V_d);
        if (v < V_0 || v < V_d)
            v = MAX(V_0, V_d) + 1e-8;
    
        double s2, ds;
        s2 = speed_f3(v, V_0, V_d) - SJ;
        ds = speed_f3s_diff(v, V_0) + speed_f3s_diff(v, V_d);
        v = v - s2/ds;
        if (v < V_0 || v < V_d)
            v = MAX(V_0, V_d)+1e-8;
        s2 = speed_f3(v, V_0, V_d) - SJ;
        ds = speed_f3s_diff(v, V_0) + speed_f3s_diff(v, V_d);

        v = v - s2/ds;
        return v;

    };
    double SLadderParams::vaa2s31() const{
        return vs2s31(V_m);
    }

    double SLadderParams::vs2s31(double vs) const{
        double S1 = 0.5*J*V_0*A_0*A_0 + 0.5*(vs*vs-V_0*V_0)*J*J-POWER4(A_0)/8;
        double S2 = CUBE(A_0)/3 - A_0*J*V_0;
        double S3 = 0.5*J*(V_0 + vs)-0.25*A_0*A_0; 
        return S1/(A_m*J*J)+S2/J/J+A_m*S3/J/J;

    };

    double SLadderParams::vs2s21(double vs) const{
        double Vm = vs;
        double am = fVm_2Am(Vm); 
        if (A_0 == 0){
            return 2*am*V_0/J + CUBE(am)/J/J;
        }else{
            return (CUBE(A_0) - 3*A_0*A_0*am - 3*J*V_0*A_0 + 3*CUBE(am) + 6*J*V_0*am)/(3*J*J);
        }
    };

    double SLadderParams::vaa2s32() const {
        // todo
        return 0;
    };

    double SLadderParams::vaa2s3() const {
        // todo
        return 0;
    };

    double SLadderParams::vaa2s2() const {
        if (fV0_2VmMin() > fVd_2VmMin())
            return vaa2s22();
        else if (fV0_2VmMin() < fVd_2VmMin())
            return vaa2s21();
        return 0;
    };

    double SLadderParams::vaa2s21() const{
        return vs2s21(V_d);
    };

    double SLadderParams::vaa2s22() const {
        double vm = fV0_2VmMin();
        double am = A_0;
        double an = sqrt(J*(vm - V_d));

        if (A_0 == 0){
            return (2*J*V_0*an- CUBE(an))/(J*J);
        }else{
            return (CUBE(A_0) + 3*A_0*A_0*an + 3*J*V_0*A_0 - 3*CUBE(an) + 6*J*V_0*an)/(3*J*J);
        }
    };

    double SLadderParams::s31s21_2vm(double S) const{
        double VM_exact = fAm_2Vm(A_m);
        if (vs2s21(VM_exact) >= S){
            return fAm_2Vm(s21_2am(S));
        }else{
            return s31_2vm(S);
        }
    }

    double SLadderParams::s31_2vm(double S) const{
        if (get_symmetry() == SLADDER_Z_SYMMETRY){
            double pp[3] = {
               1, A_m*A_m/J, - 2*S*A_m
            };
            return quadtric_equation_solver(pp);
        }else if (get_symmetry() == SLADDER_V_SYMMETRY){
            double pp[3] = {
                1, A_m*A_m/J, A_m*A_m*V_0/J - 2*S*A_m - V_0*V_0
            };
            return quadtric_equation_solver(pp);
        }else{
            double pp[3] = {
                1,
                A_m*A_m/J, 
                - (3*POWER4(A_0) - 8*CUBE(A_0)*A_m + 6*A_0*A_0*A_m*A_m - 12*A_0*A_0*J*V_0 + 24*A_0*A_m*J*V_0 - 12*A_m*A_m*J*V_0 + 24*S*A_m*J*J + 12*J*J*V_0*V_0)/(12*J*J)
            };
            return quadtric_equation_solver(pp);
        }
    };

    double SLadderParams::s21_2am(double S) const{
        if (get_symmetry() == SLADDER_Z_SYMMETRY){
            return pow(S*J*J, 1.0/3);
        }else if (get_symmetry() == SLADDER_V_SYMMETRY){
            return nseg21_s2am(S);
        }else{
            return cube_equation_solver(2*J*V_0 - A_0*A_0, CUBE(A_0)/3 - J*J*S- A_0*V_0*J);
        }
    };

double SLadderParams::nseg21_s2am(double S) const{
    // inverse to nseg21_am2s
    // Am^3 + 2*J*V_0*Am - J*J*S = 0
    return cube_equation_solver(2*J*V_0, - J*J*S);
}

double SLadderParams::nseg22_s2an(double S) const{
    // inverse to nseg22_an2s
    // An^3 - 2*J*V_0*An + J*J*S = 0
    return cube_equation_solver(-2*J*V_0, J*J*S);
}

double SLadderParams::vsymmetry_nseg40_s2am(double S) const{
    // nseg40 = nseg21 + nseg22
    return nseg21_s2am(S/2);
}

SLadderSegmentType sLadderRelaxationZsymmetry(SLadderParams& slp, SjRelaxParam& param)
{
    return slp.relaxation_Z_Symmetry(param);
};

SLadderSegmentType sLadderRelaxationVsymmetry(SLadderParams& slp, SjRelaxParam& param)
{
    return slp.relaxation_V_Symmetry(param);
};

SLadderSegmentType sLadderRelaxationAsymmetry(SLadderParams& slp, SjRelaxParam& param)
{
    return slp.relaxation_A_Symmetry(param);
};

int FuncVelSjOrig::operator()(const VelParam& param, VelForwardSj2& fw){
    SLadderParams& par = * (SLadderParams*) &param;
    // SjRelaxParam rp{};
    // auto ret = par.relaxation(rp);
    // if (ret <= LADDER_0_SEGMENT)
    //     return 1;
    // fw.build(rp, (VelParam&) param);
    return velBuild<SjRelaxParam>(par, fw);
}

int FuncVelSj2::operator()(const VelParam& param, VelForwardSj2& fw){
    auto& par = * (SLadderRelaxer*) &param;
    return velBuild<SjRelaxParam>(par, fw);
}

double VelocitySj2::cal_reachable_pre_vel(double vd) const{
    return ((SLadderParams*) &param)->cal_reachable_pre_vel(vd);
}
double VelocitySj2::cal_reachable_pro_vel(double v0) const{
    return ((SLadderParams*) &param)->cal_reachable_pro_vel(v0);
}

int FuncVelSj3::operator()(const VelParam& param, VelForwardSj3& fw){
    auto& par = * (SLadderRelaxer2*) &param;
    return velBuild<SjRelaxParam>(par, fw);
}
double VelocitySj3::cal_reachable_pre_vel(double vd) const{
    return ((SLadderParams*) &param)->cal_reachable_pre_vel(vd);
}
double VelocitySj3::cal_reachable_pro_vel(double v0) const{
    return ((SLadderParams*) &param)->cal_reachable_pro_vel(v0);
}
int VelocitySj3::get_slice_continuous(double tm, PosVelAcc& pva)const{
    return fw.get_slice_continuous(tm, pva);
}
    bool SLadderRelaxer::is_reachable() const{
        double _S = get_S();
        if (A_0 == 0){
            if (V_0 < V_d){
                if (has_plain_acc(V_d, A_m)){
                    return nseg31_vm2s(V_d) <= _S;
                }else{
                    return nseg21_vm2s(V_d) <= _S;
                }
            }else if (V_0 > V_d){
                if (has_plain_dec(V_0, A_n)){
                    return nseg32_vm2s(V_0) <= _S;
                }else{
                    return nseg22_vm2s(V_0) <= _S;
                }
            }else {
                return true;
            }
        }else if (A_0 > 0){
            double V1 = V_0 + A_0*A_0/2/J;
            if (V1 <= V_d){
                if (has_dv_plain(V_d - V_0, A_m, A_0)){
                    return nseg31_vm2s(V_d) < _S;
                }else{
                    return nseg21_vm2s(V_d) < _S;
                }
            }else {
                if (has_dv_plain(V_0 - V_d, A_n, A_0)){
                    return nseg35_vn2s(V_d) < _S;
                }else{
                    return nseg23_vn2s(V_d) < _S;
                }
            }
        }else{
            double V1 = V_0 - A_0*A_0/2/J;
            if (V1 <= V_d){
                if (has_dv_plain(V_d - V_0, A_m, A_0)){
                    return nseg31_vm2s(V_d) < _S;
                }else{
                    return nseg21_vm2s(V_d) < _S;
                }
            }else { // if (V1 > V_d)
                if (has_dv_plain(V1 - V_d, A_n, A_0)){
                    return nseg35_vn2s(V_d) < _S;
                }else{
                    return nseg23_vn2s(V_d) < _S;
                }
            }
        }
    }

    bool SLadderRelaxer::has_dv_plain(double dv, double am, double a0) const{
        if (a0 >= 0)
            return J* dv  >= am * am - a0* a0 / 2;
        else
            return J* dv  >= am * am + a0* a0 / 2;
    }
    bool SLadderRelaxer::has_plain_acc(double vm, double am) const{
        return J* (vm - V_0) >= am * am ;
    }
    bool SLadderRelaxer::has_plain_dec(double vm, double an) const{
        return J* (vm - V_d) >= an * an ;
    }
    double SLadderRelaxer::nseg21_vm2s(double vm) const{
        if (A_0 == 0){
            return (vm+ V_0)*sqrt((V_m - V_0)/J);
        }else{
            double am = sqrt(J*(vm -V_0) + A_0* A_0/2);
            return (CUBE(am)+ (2*J*V_0 - A_0*A_0)*am + CUBE(A_0)/3 - V_0*A_0*J)/(J*J);
        }
    }


    double SLadderRelaxer::nseg22_vm2s(double vm) const{
        return (vm+ V_d)*sqrt((vm - V_d)/J);
    }
    double SLadderRelaxer::nseg24_vn2s(double vn) const{
        // assume V_d > vm
        return (vn + V_d)*sqrt((V_d - vn)/J);
    }

    double SLadderRelaxer::nseg23_vn2s(double vn) const{
        // assume V_0 > vm
        if (A_0 == 0){
            return (vn + V_0)*sqrt((V_0 - vn)/J);
        }else{
            double an = sqrt(J*(V_0- vn) + A_0* A_0/2);
            return ((2*J*V_0 + A_0*A_0)*an -CUBE(an) + CUBE(A_0)/3 + V_0*A_0*J)/(J*J);
        }
    }

    double SLadderRelaxer::nseg21_v2s(double v0, double vm) const{
        if (A_0 == 0){
            return (vm+ v0)*sqrt((vm - v0)/J);
        }else{
            // TODO
            return 0;
        }
    }

    double SLadderRelaxer::nseg31_vm2s(double vm) const{
        if (A_0 == 0){
            return nseg31za_vm2s(V_0, vm, A_m);
        }else{
            if (A_0 > 0){
                // use prev point { Sn1, Vn1, 0}
                double T = (A_0 - 0)/J;
                double Sn1 = T*(V_0 - A_0*T/2 + J*T*T/6);
                double Vn1 = V_0 - (A_0*A_0)/2/J;
                double S31z = nseg31za_vm2s(Vn1, vm, A_m);
                return S31z - Sn1;
            }else{
                // use next point { S1, V1, 0}
                double T = (0-A_0)/J;
                double V1 = V_0 + (0 - A_0*A_0)/2/J;
                double S1 = T*(V_0 + A_0*T/2 + J*T*T/6);
                double S31z = nseg31za_vm2s(V1, vm, A_m);
                return S31z + S1;
            }
        }
    }
    double SLadderRelaxer::nseg35_vn2s(double vn) const{
        if (A_0 == 0){
            return nseg31za_vm2s(vn, V_0, A_n);
        }else if (A_0 < 0){
            // use prev point { Sn1, Vn1, 0}
            double J = -this->J;
            double T = (A_0 - 0)/J;
            double Sn1 = T*(V_0 - A_0*T/2 + J*T*T/6);
            double Vn1 = V_0 - (A_0*A_0)/2/J;
            double S31z = nseg31za_vm2s(vn, Vn1, A_n);
            return S31z - Sn1;
        }else{
            // use next point { S1, V1, 0}
            double T = -(0-A_0)/J;
            double V1 = V_0 + (0 + A_0*A_0)/2/J;
            double S1 = T*(V_0 + A_0*T/2 - J*T*T/6);
            double S31z = nseg31za_vm2s(vn, V1, A_n);
            return S31z + S1;
        }
    }

    double SLadderRelaxer::nseg36_vn2s(double vn) const{
        // assume Vn < V_d
        return nseg31za_vm2s(vn, V_d, A_n);
    }
    double SLadderRelaxer::nseg32_vm2s(double vm) const{
        return nseg31za_vm2s(V_d, vm, A_n);
        // double S = ((V_d + vm)*(A_n*A_n - J*V_d + J*vm))/(2*A_n*J);
        // return S;
    }

    double SLadderRelaxer::nseg31_v2s(double v0, double vm, double am) const{
        if (A_0 == 0){
            double S = ((v0 + vm)*(am*am - J*v0 + J*vm))/(2*am*J);
            return S;
        }else{
            return 0; // TODO
        }
    }

SLadderSegmentType SLadderRelaxer::relaxation(SjRelaxParam& param) const{
    // Shared by FuncVelSj2/Sj3 via velBuild; nseg <= 0 becomes build() == -1.
    if (A_m <= 0 || A_n <= 0)
        return SLADDER_0_SEGMENT;
    if (get_S() >= 0){
        return _relaxation(param);
    }else{
        auto relaxer2 = * this;
        relaxer2.A_m = A_n;
        relaxer2.A_n = A_m;
        relaxer2.invert();
        auto ret = relaxer2._relaxation(param);
        param.invert();
        return ret;
    }
}

SLadderSegmentType SLadderRelaxer::_relaxation(SjRelaxParam& param) const{
    if (get_symmetry() == SLadderSymmetry::SLADDER_V_SYMMETRY || get_symmetry() == SLadderSymmetry::SLADDER_Z_SYMMETRY){
        return _relaxation_sym(param);
    } else{
        return _relaxation_asym(param);
    };
};
    

SLadderSegmentType SLadderRelaxer::_relaxation_sym(SjRelaxParam& param) const{
    if (V_0 > V_m + V_EPS) {
        return _relaxation_sym_overlimit(param);
    }else if (V_0 < - V_EPS){
        return _relaxation_sym_inv_db(param);
    }else if (V_0 == 0){
        return _relaxation_sym_zero(param);    
    } else {
        return _relaxation_sym_main(param);
    }
}

SLadderSegmentType SLadderRelaxer::_relaxation_sym_zero(SjRelaxParam& param) const{
    return relaxation_Z_Symmetry(param);
}

SLadderSegmentType SLadderRelaxer::_relaxation_sym_overlimit(SjRelaxParam& param) const{
    // assume V_0 = V_d > V_m
    // nseg22 + nseg21 + (plain vel)
    // nseg32 + nseg31 + (plain vel)
    double S = get_S();
    double aMn = MIN(A_m, A_n);
    // Initialize to full range
    param.vMax = V_m;
    param.aMax = aMn;
    param.aMin = aMn;
    param.plainTime = 0;
    // Check if we have plain acceleration/deceleration segments
    // In symmetric case, V_0 = V_d, so check from V_0 to V_m
    bool is_plain_acc = has_dv_plain(V_0 - V_m, aMn);    
    // Calculate single trip distance (from V_0 to V_m)
    // Due to symmetry, total distance = 2 × single trip distance
    double S_total;
    if (is_plain_acc) {
        // nseg32 + nseg31
        S_total = nseg31_v2s(V_m, V_0, aMn) * 2;
    } else {
        // nseg22 + nseg21
        S_total = nseg21_v2s(V_m, V_0) * 2;
    }

    if (S >= S_total - TOL) {
        // Have enough distance: use V_m with plain velocity segment (T4 exists)
        // Vm is the given value V_m, no relaxation needed
        param.plainTime = (S - S_total)/V_m;
        if (is_plain_acc) {
            return setSegSym(param, SLADDER_72_SEGMENT);
        } else {
            param.aMax = sqrt(J*(V_d - V_m));
            return setSegSym(param, SLADDER_54_SEGMENT);
        }
    } else {
        // Not enough distance: no plain velocity segment (T4 doesn't exist), need to shrink        
        if (is_plain_acc) {
            // Pure acceleration and deceleration: use vsymmetry_nseg60_am2vm
            double S44 = nseg21_am2s(aMn) *2;
            if (S > S44 - TOL){
                param.vMax = vsymmetry_nseg66_am2vm(aMn);
                param.aMax = aMn;
                return setSegSym(param, SLADDER_66_SEGMENT);
            }else{
                param.aMax = nseg22_s2an(S/2);
                param.vMax = fAn_2Vn(param.aMax);
                return setSegSym(param, SLADDER_44_SEGMENT);
            }
        } else {
            param.aMax = nseg22_s2an(S/2);
            param.vMax = fAn_2Vn(param.aMax);
            return setSegSym(param, SLADDER_44_SEGMENT);
        }
    }
}

SLadderSegmentType SLadderRelaxer::_relaxation_sym_inv_db(SjRelaxParam& param) const{
    return relaxation_V_Symmetry(param);
}

SLadderSegmentType SLadderRelaxer::_relaxation_sym_main(SjRelaxParam& param) const{
    return relaxation_V_Symmetry(param);
}


SLadderSegmentType SLadderRelaxer::_relaxation_asym(SjRelaxParam& param) const{
    if (V_0 > -V_EPS && V_d > -V_EPS) {
        return _relaxation_asym_main(param);
    }else if (V_0 < 0 && V_d < 0) {
        return _relaxation_asym_inv_db(param);
    }else { // (V0 < 0 && Vd >= 0) || (V0 >= 0 && Vd < 0)
        return _relaxation_asym_inv_sg(param);
    }
}

SLadderSegmentType SLadderRelaxer::_relaxation_asym_main(SjRelaxParam& param) const{
    if (V_0 < V_m + V_EPS && V_d < V_m +V_EPS) {
        auto ret = _relaxation_asym_main_inlimit(param);
        return setSeg(param, ret);
    }else if (V_0 > V_m + V_EPS && V_d > V_m + V_EPS) {
        return _relaxation_asym_overlimit_db(param);
    }else {
        return _relaxation_asym_overlimit_sg(param);
    }
}

SLadderSegmentType SLadderRelaxer::_relaxation_asym_overlimit_db(SjRelaxParam& param) const{
    if (!is_reachable()){
        return _relaxation_unreachable(param);
    }
    // TODO
    return setSeg(param, SLADDER_0_SEGMENT);
}

double SLadderRelaxer::nseg35_23_s2vm(double S, double xMin, double xMax) const{
    // Determine search range
    // double vm_min = fV0Vd2VmMin();
    // double vm_max = V_m > vm_min ? V_m : vm_min + 1e6; // Use V_m if available, otherwise use a large value
    
    // Create target function: f(x) = Sacc + Sdec - S
    auto target_func = [this, S](double x) -> double {
        bool is_plain_acc = has_dv_plain(V_0 - x, A_n, A_0);
        bool is_plain_dec = has_plain_dec(x, A_n);
        double Sacc = is_plain_acc ? nseg35_vn2s(x) : nseg23_vn2s(x);
        double Sdec = is_plain_dec ? nseg32_vm2s(x) : nseg22_vm2s(x);
        return Sacc + Sdec - S;
    };
    
    // Setup bisection arguments
    SolverArgs args;
    args.xMin = xMin;
    args.xMax = xMax;
    args.epsilon = EPS;
    
    // Solve using bisection
    double root = 0.0;
    int result = bisection(target_func, args, &root);
    
    // Return the solution if found, otherwise return a default value
    if (result > 0) {
        return root;
    } else {
        // If no solution found, return the minimum value as fallback
        return xMin;
    }
}


bool SLadderRelaxer::is_reachable_asym_overlimit(SjRelaxParam& param, double V_m) const{
    double S = get_S();
    if (V_0 > V_d) {
        // V_0 > V_m, V_d < V_m (deceleration case)
        // Check if V_m is reachable
        bool is_plain_acc = has_dv_plain(V_0 - V_m, A_n, A_0);
        bool is_plain_dec = has_plain_dec(V_m, A_n);
        double Sacc = is_plain_acc ? nseg35_vn2s(V_m) : nseg23_vn2s(V_m);
        double Sdec = is_plain_dec ? nseg32_vm2s(V_m) : nseg22_vm2s(V_m);
        
        if (S > Sacc + Sdec - TOL) {
            // V_m is reachable: use V_m with plain velocity segment
            param.vMax = V_m;
            param.aMax = -A_n;
            param.aMin = -A_n;
            param.plainTime = 0;
            
            SLadderSegmentType nseg;
            if (is_plain_acc && is_plain_dec){
                nseg = SLADDER_70_SEGMENT;
            }else if (is_plain_acc && !is_plain_dec){
                param.aMin = -fVm_2An(V_m);
                nseg = SLADDER_62_SEGMENT;
            }else if (!is_plain_acc && is_plain_dec){
                param.aMax = -fVn_2Am(V_m);
                nseg = SLADDER_61_SEGMENT;
            }else{
                param.aMax = -fVm_2Am(V_m);
                param.aMin = -fVm_2An(V_m);
                nseg = SLADDER_50_SEGMENT;
            }
            param.plainTime = (S - Sacc - Sdec)/V_m;
            setSegAndInvAn(param, nseg);
            return true;
        }else{
            return false;
        }
    }else {
// V_0 < V_m < V_d (acceleration case)
        // Check if V_m is reachable
        bool is_plain_acc = has_dv_plain(V_m - V_0, A_m, A_0);
        bool is_plain_dec = has_dv_plain(V_d - V_m, A_m, 0);  // from V_m to V_d is acceleration
        double Sacc = is_plain_acc ? nseg31_vm2s(V_m) : nseg21_vm2s(V_m);
        double Sdec = is_plain_dec ? nseg36_vn2s(V_m) : nseg24_vn2s(V_m);
        printf("Sacc: %f, Sdec: %f\n", Sacc, Sdec);
        printf("S: %f\n", S);
        if (S > Sacc + Sdec - TOL) {
            // V_m is reachable: use V_m with plain velocity segment
            param.vMax = V_m;
            param.aMax = A_m;
            param.aMin = A_m;
            param.plainTime = 0;
            
            SLadderSegmentType nseg;
            if (is_plain_acc && is_plain_dec){
                nseg = SLADDER_70_SEGMENT;
            }else if (is_plain_acc && !is_plain_dec){
                param.aMin = fVn_2An(V_m);
                nseg = SLADDER_62_SEGMENT;
            }else if (!is_plain_acc && is_plain_dec){
                param.aMax = fVm_2Am(V_m);
                nseg = SLADDER_61_SEGMENT;
            }else{
                param.aMax = fVm_2Am(V_m);
                param.aMin = fVn_2An(V_m);
                nseg = SLADDER_50_SEGMENT;
            }
            param.plainTime = (S - Sacc - Sdec)/V_m;
            setSegAndInvAn(param, nseg);
            return true;
        }else{
            return false;
        }
    }
}
SLadderSegmentType SLadderRelaxer::_relaxation_asym_overlimit_sg(SjRelaxParam& param) const{
    if (!is_reachable()){
        return _relaxation_unreachable(param);
        auto par2 = *this;
        par2.invert();
        auto ret = par2._relaxation_asym_overlimit_sg(param);
        param.invert();
        return ret;
    }
    
    double S = get_S();
    
    if (V_0 > V_d){
        if (is_reachable_asym_overlimit(param, V_m)){
            return (SLadderSegmentType) param.seg_type;
        } else {
            // V_m is not reachable: use max(V_0, V_d) as target velocity with vel_plain
            bool is_plain_to_vd = has_dv_plain(V_0 - V_d, A_n, A_0);
            double S_to_vd = is_plain_to_vd ? nseg35_vn2s(V_d) : nseg23_vn2s(V_d);
            double S_remaining = S - S_to_vd;
            
            param.vMax = V_0;
            param.plainTime = 0;
            
            if (S_remaining > TOL) {
                param.plainTime = S_remaining / V_0;
            }
            
            SLadderSegmentType nseg;
            if (is_plain_to_vd) {
                // nseg35: from V_0 to V_d with plain segment, using A_n for deceleration
                param.aMax = 0;
                param.aMin = -A_n;
                nseg = SLADDER_42_SEGMENT;
            } else {
                // nseg23: from V_0 to V_d without plain segment, using A_n for deceleration
                param.aMax = 0;
                param.aMin = -fVm_2An(V_0);
                nseg = SLADDER_50_SEGMENT; // SLADDER_22_SEGMENT
            }
            return setSegAndInvAn(param, nseg);
        }
    } else {
        if (is_reachable_asym_overlimit(param, V_m)){
            return (SLadderSegmentType) param.seg_type;
        } else {
            // V_m is not reachable: use max(V_0, V_d) as target velocity with vel_plain
            bool is_plain_to_vd = has_dv_plain(V_d - V_0, A_m, A_0);
            double S_to_vd = is_plain_to_vd ? nseg31_vm2s(V_d) : nseg21_vm2s(V_d);
            double S_remaining = S - S_to_vd;
            
            param.vMax = V_d;
            param.plainTime = 0;
            
            if (S_remaining > TOL) {
                param.plainTime = S_remaining / V_d;
            }
            
            SLadderSegmentType nseg;
            if (is_plain_to_vd) {
                param.aMax = A_m;
                param.aMin = 0;
                nseg = SLADDER_41_SEGMENT;
            } else {
                param.aMax = fVm_2Am(V_d);
                param.aMin = 0;
                nseg = SLADDER_50_SEGMENT; // SLADDER_21_SEGMENT
            }
            return setSegAndInvAn(param, nseg);
        }
    }
}

// shrink Vm Am
SLadderSegmentType SLadderRelaxer::_shrink_Vm_Am(SjRelaxParam& param) const{
    double vm_min = fV0Vd2VmMin();
    double S = get_S();
    double vm3 = fAn_2Vm(A_n);
    if (vm_min < vm3){
        // assume V_0 < vm3 < vm2
        double S40 = an2s40(A_n);
        if (S >= S40 - TOL){
            param.vMax = s51_2vm(S, A_n, vm_min, V_m);
            param.aMax = fVm_2Am(param.vMax);
            return SLADDER_51_SEGMENT;
        }else {
            return _shrink_Vm_Am_An(param);
        }
    }else{ // assume V_d < vm3 < V_0
        double S32 = nseg32_vm2s(vm_min);
        if (S > S32 - TOL){
            param.aMax = s51_2am(S, A_n, fVm_2Am(V_0), A_m);
            param.vMax = fAm_2Vm(param.aMax);
            return SLADDER_51_SEGMENT;
        }else{
            return _relaxation_unreachable(param);
        }
    }
}

SLadderSegmentType SLadderRelaxer::_shrink_Vm_An(SjRelaxParam& param) const{
    double vm_min = fV0Vd2VmMin();
    double S = get_S();
    double vm2 = fAm_2Vm(A_m);
    // freeze Am & shrink An Vm 
    if (vm2 > vm_min){
        double S40 = am2s40(A_m);
        if (S > S40 - TOL){
            param.aMin = s52_2an(S, A_m, fVm_2An(vm_min), A_n);
            param.vMax = fAn_2Vm(param.aMin);
            return SLADDER_52_SEGMENT;
        }else{
            return _shrink_Vm_Am_An(param);
        }
    }else{ 
        // assume V_0 < VM2 < V_d < Vm3
        double S31 = nseg31_vm2s(V_d);
        if (S > S31 - TOL){
            param.aMin = s52_2an(S, A_m, fVm_2An(vm_min), A_n);
            param.vMax = fAn_2Vm(param.aMin);
            return SLADDER_52_SEGMENT;
        }else{
            return _relaxation_unreachable(param);
        }
    }
}

SLadderSegmentType SLadderRelaxer::_shrink_Vm_Am_An(SjRelaxParam& param) const{
    double S2x;
    double S = get_S();
    double vm_min = fV0Vd2VmMin();
    if (V_0 > V_d){
        S2x = nseg22_vm2s(V_0);
        if (S > S2x - TOL){
            param.aMax = s40_2am(S, fVm_2Am(V_0), A_m);
            param.vMax = fAm_2Vm(param.aMax);
            param.aMin = fAm_2An(param.aMax);
            return SLADDER_40_SEGMENT;
        }else{
            return _relaxation_unreachable(param);
        }
    }else{
        S2x = nseg21_vm2s(V_d);
        if (S > S2x - TOL){
            param.aMin = s40_2an(S, fVm_2An(vm_min), A_n);
            param.vMax = fAn_2Vm(param.aMin);
            param.aMax = fVm_2Am(param.vMax);
            return SLADDER_40_SEGMENT;
        }else{
            return _relaxation_unreachable(param);
        }
    }
}

SLadderSegmentType SLadderRelaxer::_relaxation_asym_main_inlimit(SjRelaxParam& param) const{
    auto ret = _relaxation_asym_main_inlimit0(param);
    return setSeg(param, ret);
}

bool SLadderRelaxer::is_reachable_asym_main_vm(SjRelaxParam& param, double V_m) const{
    double S = get_S();
    param.vMax = V_m;
    param.aMax = A_m;
    param.aMin = A_n;
    param.plainTime = 0;

    bool is_plain_acc = has_dv_plain(V_m - V_0, A_m, A_0);
    bool is_plain_dec = has_plain_dec(V_m, A_n);
    double Sacc = is_plain_acc ? nseg31_vm2s(V_m) : nseg21_vm2s(V_m);
    double Sdec = is_plain_dec ? nseg32_vm2s(V_m) : nseg22_vm2s(V_m);
    if (S > Sacc + Sdec - TOL){ 
        // use V_m and plain velocity segment
        SLadderSegmentType nseg;
        if (is_plain_acc && is_plain_dec){
            nseg = SLADDER_70_SEGMENT;
        }else if (is_plain_acc && !is_plain_dec){
            param.aMin = fVm_2An(V_m);
            nseg = SLADDER_62_SEGMENT;
        }else if (!is_plain_acc && is_plain_dec){
            param.aMax = fVm_2Am(V_m);
            nseg = SLADDER_61_SEGMENT;
        }else{
            param.aMax = fVm_2Am(V_m);
            param.aMin = fVm_2An(V_m);
            nseg = SLADDER_50_SEGMENT;
        }
        param.plainTime = (S - Sacc - Sdec)/V_m;
        setSeg(param, nseg);
        return true;
    }else{
        return false;
    }
}

SLadderSegmentType SLadderRelaxer::_relaxation_asym_main_inlimit0(SjRelaxParam& param) const{
    if (is_reachable_asym_main_vm(param, V_m)){
        return (SLadderSegmentType) param.seg_type;
    }else{ // shrink vMax and no plain velocity segment
        return _shrink_Vm(param);
    }
}

SLadderSegmentType SLadderRelaxer::_shrink_Vm(SjRelaxParam& param) const{
    double S = get_S();
    bool is_plain_acc = has_dv_plain(V_m - V_0, A_m, A_0);
    bool is_plain_dec = has_plain_dec(V_m, A_n);
        if (is_plain_acc && is_plain_dec){
            // assert max(vm2, vm3) <  V_m
            double vm2 = fAm_2Vm(A_m);
            double vm3 = fAn_2Vm(A_n);
            if (vm2 > vm3){
                double S51 = nseg21_vm2s(vm2) + nseg32_vm2s(vm2);
                if (S >= S51){
                    param.vMax = s60_2vm(S, A_m, A_n);
                    return setSeg(param, SLADDER_60_SEGMENT);
                }else{
                    return _shrink_Vm_Am(param);
                }
            }else{ 
                double S52 = nseg31_vm2s(vm3) + nseg22_vm2s(vm3);
                if (S >= S52){
                    param.vMax = s60_2vm(S, A_m, A_n);
                    return setSeg(param, SLADDER_60_SEGMENT);
                }else{
                    return _shrink_Vm_An(param);
                }
            }
        }else if (is_plain_acc && !is_plain_dec){
            // assert V0 < Vm2 < Vm < Vm3
            double S52 = nseg31_vm2s(V_m) + nseg22_vm2s(V_m);
            if (S >= S52){
                param.aMin = fVm_2An(V_m);  // shrink A_n
                return SLADDER_62_SEGMENT;
            }else{
                return _shrink_Vm_An(param);
            }
        }else if (!is_plain_acc && is_plain_dec){
            // assert  Vd < Vm3 < Vm < Vm2
            double S51 =  vaa2s51(V_m, A_n);
            if (S >= S51){
                param.aMax = fVm_2Am(V_m);
                return SLADDER_61_SEGMENT;
            }else{
                return _shrink_Vm_Am(param);
            }
        }else{
            return _shrink_Vm_Am_An(param);
        }
    }


SLadderSegmentType SLadderRelaxer::_relaxation_asym_inv_db(SjRelaxParam& param) const{
    return _relaxation_asym_main(param);
}

SLadderSegmentType SLadderRelaxer::_relaxation_asym_inv_sg(SjRelaxParam& param) const{
    // TODO: overlimit_sg, inlimit,  unreachable, overlimit_inv, 
    if (V_0 < -V_m || V_d < -V_m)
        return _relaxation_asym_overlimit_sg(param);
    return _relaxation_asym_main(param);
}

SLadderSegmentType SLadderRelaxer::_relaxation_asym_inv_overlimit(SjRelaxParam& param) const{
    if (is_reachable_asym_overlimit(param, -V_m)){ // should always be true
        return (SLadderSegmentType) param.seg_type;
    }else{
        // should not happen
        return _relaxation_unreachable(param);
    }
}


SLadderSegmentType SLadderRelaxer::_relaxation_unreachable(SjRelaxParam& param) const{
    if (V_0 < -V_m && V_d > -V_m || (V_0 > -V_m && V_d < -V_m))
        return _relaxation_asym_inv_overlimit(param);
    auto par2 = *this;
    par2.invert();
    auto ret = par2._relaxation_asym_main_inlimit(param);
    param.invert();
    return ret;
}

SLadderSegmentType SLadderRelaxer2::relaxation(SjRelaxParam& param) const{
    auto ret = SLadderRelaxer::relaxation(param);
    switch (param.seg_type){
        case SLADDER_72_SEGMENT:
            param.aMax = - param.aMax;
            param.aMin = - param.aMin;
            param.seg_type = SLADDER_70_SEGMENT;
            break;
            case SLADDER_66_SEGMENT:
            param.aMax = - param.aMax;
            param.aMin = - param.aMin;
            param.seg_type = SLADDER_60_SEGMENT;
            break;
        case SLADDER_54_SEGMENT:
            param.aMax = - param.aMax;
            param.aMin = - param.aMin;
            param.seg_type = SLADDER_50_SEGMENT;
            break;
        case SLADDER_44_SEGMENT:
            param.aMax = - param.aMax;
            param.aMin = - param.aMin;
            param.seg_type = SLADDER_40_SEGMENT;
            break;
        default:
            break;
    }
    param.aMin = - param.aMin;
    return ret;
} 
}
/* end of file */
