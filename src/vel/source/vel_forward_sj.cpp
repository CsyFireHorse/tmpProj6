#include <math.h>

#include "stdlib.h"
#include "base/include/solver.h"
#include "vel_forward_sj.h"
#include "common/zal_common.h"
#include "math.h"

// std::qsort
#define secantItermethod secantItermethod2
#define EPS (1e-9)
#define TOL (1e-8)
namespace zmc
{

int VelForwardSj::get_plan(double* total_time, double* rVmax, double* rAcc, double* rDec)const{
    if (total_time) *total_time = get_end_time();
    if (rVmax) *rVmax = vm;
    return 0;
};

int VelForwardSj::get_slice(double slice, double *position, double *velocity, double *acc) const{
    int ret = E_VEL_UNCOVERED;
    if (slice < get_begin_time()) slice = 0;
    else if (slice >= get_end_time()) {
        slice = get_end_time();
        ret = E_VEL_COVERED;
    }

    if (position) *position = get_distance(slice);
    if (velocity) *velocity = get_rate(slice);
    if (acc) *acc = get_acc(slice);
    return ret;
};
    int VelForwardSj::build(){
        _buildTimeSegment();
        _buildPosVecSegment();
        _cumsum();
        return 0;
    }
    void VelForwardSj::_cumsum()
    {
        cTarr[0] = Tarr[0];
        for (int i=1; i<8; i++){
            cTarr[i] = cTarr[i-1] + Tarr[i];
        }

        double ans = 0;
        for (int i=0; i<8; i++){
            ans += Sarr[i];
            cSarr[i] = ans;
        }
        for (int i=0; i<8; i++){
            Sarr[i] = round(Sarr[i]*1.0e8)/1.0e8;
            cSarr[i] = round(cSarr[i]*1.0e8)/1.0e8;
            Tarr[i] = round(Tarr[i]*1.0e8)/1.0e8;
            cTarr[i] = round(cTarr[i]*1.0e8)/1.0e8;
            Varr[i] = round(Varr[i]*1.0e8)/1.0e8;
        }
    }
    
    int VelForwardSj::_check(){
        int errCode = 0;
        auto& pms = *pPar;
        if (ABS(cSarr[7] - pms.S_d) > 1.0e-8){
            errCode = (zmc::LadderSegmentExceptionType)( (int) errCode | 8);
        }
        if (ABS(Varr[7] - pms.V_d) > 1.0e-8){
            errCode = (zmc::LadderSegmentExceptionType)( (int) errCode | 16);
        }
        return errCode;
    }

    void VelForwardSj::_buildTimeSegment(void){
        switch(nseg){
        case SLADDER_70_SEGMENT:
            _buildTimeSegment_s70();
            break;
        case SLADDER_60_SEGMENT:
            _buildTimeSegment_s60();
            break;
        case SLADDER_61_SEGMENT:
            _buildTimeSegment_s61();
            break;
        case SLADDER_62_SEGMENT:
            _buildTimeSegment_s62();
            break;
        case SLADDER_50_SEGMENT:
            _buildTimeSegment_s50();
            break;
        case SLADDER_51_SEGMENT:
            _buildTimeSegment_s51();
            break;
        case SLADDER_52_SEGMENT:
            _buildTimeSegment_s52();
            break;
        case SLADDER_40_SEGMENT:
            _buildTimeSegment_s40();
            break;
        case SLADDER_41_SEGMENT:
            _buildTimeSegment_s41();
            break;
        case SLADDER_42_SEGMENT:
            _buildTimeSegment_s42();
            break;
        case SLADDER_31_SEGMENT:
            _buildTimeSegment_s31();
            break;
        case SLADDER_32_SEGMENT:
            _buildTimeSegment_s32();
            break; 
        case SLADDER_33_SEGMENT:
        case SLADDER_34_SEGMENT:
            _buildTimeSegment_s50();  
            break;
        case SLADDER_21_SEGMENT:
            _buildTimeSegment_s21();
            break;
        case SLADDER_22_SEGMENT:
            _buildTimeSegment_s22();
            break;
        case SLADDER_10_SEGMENT:
            _buildTimeSegment_s10();
            break;
        default:
            _buildTimeSegment_s70();
        }
    };  

    void VelForwardSj::_buildTimeSegment_s70(void){
        auto& pms = *pPar;
        double V_0 = pms.V_0;
        double A_0 = pms.A_0;
        double V_d = pms.V_d;
        double J = pms.J;
        double S_0 = pms.S_0;
        double Sm = pms.S_d;
        double J2 = SQUARE(J);

        double T_0 = 0;
        double T_1 = (am- A_0) / J;
        double T_2 = (SQUARE(A_0) - 2*SQUARE(am) - 2*J*V_0 + 2*J*vm)/(2*J*am);
        double T_3 = am / J;
        double T_4;
        if (A_0 == 0){
            T_4 = Sm/vm + (am*J*SQUARE(V_d) + an*J*SQUARE(V_0) - SQUARE(am)*an*V_0 - am*SQUARE(an)*V_d)/(2*am*an*J*vm) - ((am*an + J*vm)*(am + an))/(2*am*an*J);

            // (am + an)/(2*J) + vm*(am + an)/(2*am*an);
        }else{
            T_4 = (3*POWER4(A_0)*an - 8*CUBE(A_0)*am*an + 6*SQUARE(A_0)*SQUARE(am)*an - 12*SQUARE(A_0)*an*J*V_0 + 24*A_0*am*an*J*V_0 - 12*SQUARE(am)*an*J*V_0 - 12*am*SQUARE(an)*J*V_d + 24*Sm*am*an*J2 + 12*am*J2*SQUARE(V_d) + 12*an*J2*SQUARE(V_0))/(24*am*an*J2*vm) - ((am*an + J*vm)*(am + an))/(2*am*an*J);
        }
        double T_5 = an / J;
        double T_6 = (vm-SQUARE(an)/J - V_d)/an;
        double T_7 = an / J;
        // Tarr = {T_0, T_1, T_2, T_3, T_4, T_5, T_6, T_7};
        Tarr[0] = T_0;Tarr[1] = T_1;Tarr[2] = T_2;Tarr[3] = T_3;
        Tarr[4] = T_4;Tarr[5] = T_5;Tarr[6] = T_6;Tarr[7] = T_7;
    };

    void VelForwardSj::_buildTimeSegment_s60(void){
        auto& pms = *pPar;
        double J = pms.J;
        double V_0 = pms.V_0;
        double V_d = pms.V_d;
        double A_0 = pms.A_0;

        Tarr[0] = 0;
        Tarr[1] = (am- A_0) / J;
        Tarr[2] = (A_0*A_0 - 2*am*am - 2*J*V_0 + 2*J*vm)/(2*J*am);
        Tarr[3] = am / J;
        Tarr[4] = 0;
        Tarr[5] = an / J;
        Tarr[6] = (vm-an*an/J - V_d)/an;
        Tarr[7] = an / J;
    };
    
    void VelForwardSj::_buildTimeSegment_s61(void){
        _buildTimeSegment_s70();
    };

    void VelForwardSj::_buildTimeSegment_s62(void){
        _buildTimeSegment_s70();
    };

    void VelForwardSj::_buildTimeSegment_s50(void){
        auto& pms = *pPar;
        double V_0 = pms.V_0;
        double A_0 = pms.A_0;
        double V_d = pms.V_d;
        double J = pms.J;
        double S_0 = pms.S_0;
        double Sm = pms.S_d;
        double J2 = SQUARE(J);

        double T_0 = 0;
        double T_1 = (am- A_0) / J;
        double T_2 = 0.0;
        double T_3 = am / J;
        // double T_4 = (3*POWER4(A_0)*an - 8*CUBE(A_0)*am*an + 6*SQUARE(A_0)*SQUARE(am)*an - 12*SQUARE(A_0)*an*J*V_0 + 24*A_0*am*an*J*V_0 - 12*SQUARE(am)*an*J*V_0 - 12*am*SQUARE(an)*J*V_d + 24*Sm*am*an*J2 + 12*am*J2*SQUARE(V_d) + 12*an*J2*SQUARE(V_0))/(24*am*an*J2*vm) - ((am*an + J*vm)*(am + an))/(2*am*an*J);

        double Ss;
        if (A_0 == 0){
            Ss = (CUBE(am) + 2*J*V_0*am - CUBE(an) + 2*J*vm*an)/SQUARE(J);
        }else{
            Ss = (CUBE(A_0) - 3*SQUARE(A_0)*am - 3*J*V_0*A_0 + 3*CUBE(am) + 6*J*V_0*am - 3*CUBE(an) + 6*J*vm*an)/(3*SQUARE(J));
        }
        double T_4 = (Sm-Ss)/vm;

        double T_5 = an / J;
        double T_6 = 0.0;
        double T_7 = an / J;
        // Tarr = {T_0, T_1, T_2, T_3, T_4, T_5, T_6, T_7};
        Tarr[0] = T_0;Tarr[1] = T_1;Tarr[2] = T_2;Tarr[3] = T_3;
        Tarr[4] = T_4;Tarr[5] = T_5;Tarr[6] = T_6;Tarr[7] = T_7;
    };

    void VelForwardSj::_buildTimeSegment_s51(void){
        auto& pms = *pPar;
        double V_0 = pms.V_0;
        double A_0 = pms.A_0;
        double V_d = pms.V_d;
        double J = pms.J;
        double S_0 = pms.S_0;
        double Sm = pms.S_d;
        double J2 = SQUARE(J);

        double T_0 = 0;
        double T_1 = (am- A_0) / J;
        double T_2 = 0.0;
        double T_3 = am / J;
        double T_4 = 0.0;

        double T_5 = an / J;
        double T_6 = (vm-SQUARE(an)/J - V_d)/an;
        double T_7 = an / J;
        Tarr[0] = T_0;Tarr[1] = T_1;Tarr[2] = T_2;Tarr[3] = T_3;
        Tarr[4] = T_4;Tarr[5] = T_5;Tarr[6] = T_6;Tarr[7] = T_7;
    };

    void VelForwardSj::_buildTimeSegment_s52(void){
        auto& pms = *pPar;
        double V_0 = pms.V_0;
        double A_0 = pms.A_0;
        double V_d = pms.V_d;
        double J = pms.J;
        double S_0 = pms.S_0;
        double Sm = pms.S_d;
        double J2 = SQUARE(J);

        double T_0 = 0;
        double T_1 = (am- A_0) / J;
        double T_2 = (SQUARE(A_0) - 2*SQUARE(am) - 2*J*V_0 + 2*J*vm)/(2*J*am);
        double T_3 = am / J;
        double T_4 = 0.0;

        double T_5 = an / J;
        double T_6 = 0.0;
        double T_7 = an / J;
        Tarr[0] = T_0;Tarr[1] = T_1;Tarr[2] = T_2;Tarr[3] = T_3;
        Tarr[4] = T_4;Tarr[5] = T_5;Tarr[6] = T_6;Tarr[7] = T_7;
        // Tarr = {T_0, T_1, T_2, T_3, T_4, T_5, T_6, T_7};
    };
    void VelForwardSj::_buildTimeSegment_s40(void){
        auto& pms = *pPar;
        double J = pms.J;
        double A_0 = pms.A_0;

        Tarr[0] = 0;
        Tarr[1] = (am- A_0) / J;
        Tarr[2] = 0;
        Tarr[3] = am / J;
        Tarr[4] = 0;
        Tarr[5] = an / J;
        Tarr[6] = 0;
        Tarr[7] = an / J;
    };

    void VelForwardSj::_buildTimeSegment_s41(void){
        auto& pms = *pPar;
        double J = pms.J;
        double A_0 = pms.A_0;
        double V_0 = pms.V_0;
        double Sm = pms.S_d;

        Tarr[0] = 0;
        Tarr[1] = (am- A_0) / J;
        Tarr[2] = (SQUARE(A_0) - 2*SQUARE(am) - 2*J*V_0 + 2*J*vm)/(2*J*am);
        Tarr[3] = am / J;
        double S1 = 0.5*J*V_0*SQUARE(A_0) + 0.5*(SQUARE(vm) - SQUARE(V_0))*SQUARE(J)-POWER4(A_0)/8;
        double S2 = CUBE(A_0)/3 - A_0*J*V_0;
        double S3 = 0.5*J*(V_0 + vm)-0.25*SQUARE(A_0);
        double S =  S1/(am*SQUARE(J))+S2/SQUARE(J)+am*S3/SQUARE(J);
        double T_4 = (Sm - S)/vm;
        Tarr[4] = T_4;
        Tarr[5] = 0;
        Tarr[6] = 0;
        Tarr[7] = 0;
    };

    void VelForwardSj::_buildTimeSegment_s42(void){
        auto& pms = *pPar;
        double J = pms.J;
        double A_0 = pms.A_0;
        double V_0 = pms.V_0;
        double V_d = pms.V_d;
        double Sm = pms.S_d;

        Tarr[0] = 0;
        Tarr[1] = 0;
        Tarr[2] = 0;
        Tarr[3] = am / J;
        double S1 = CUBE(A_0)/3 + A_0*J*V_0;
        double S2 = (V_d+vm)* SQUARE(an)+J*( SQUARE(vm)- SQUARE(V_d));
        double S = S1/SQUARE(J) + S2/(2*an*J);
        double T_4 = (Sm - S)/vm;

        Tarr[4] = T_4;
        Tarr[5] = an / J;
        Tarr[6] = (vm- SQUARE(an)/J - V_d)/an;
        Tarr[7] = an / J;
    };

    void VelForwardSj::_buildTimeSegment_s31(void){
        auto& pms = *pPar;
        double J = pms.J;
        double A_0 = pms.A_0;
        double V_0 = pms.V_0;
        Tarr[0] = 0;
        Tarr[1] = (am- A_0) / J;
        Tarr[2] = (A_0*A_0 - 2*am*am - 2*J*V_0 + 2*J*vm)/(2*J*am);
        Tarr[3] = am / J;
        Tarr[4] = 0;
        Tarr[5] = 0;
        Tarr[6] = 0;
        Tarr[7] = 0;
    };

    void VelForwardSj::_buildTimeSegment_s32(void){
        auto& pms = *pPar;
        double J = pms.J;
        double A_0 = pms.A_0;
        double V_0 = pms.V_0;
        Tarr[0] = 0;
        Tarr[1] = 0;
        Tarr[2] = 0;
        Tarr[3] = 0;
        Tarr[4] = 0;
        Tarr[5] = (an + A_0) / J;
        Tarr[6] = ((A_0*A_0/2 - an*an)/J + (vm - pms.V_d))/an;
        Tarr[7] = an / J;
    };

    void VelForwardSj::_buildTimeSegment_s21(void){
        auto& pms = *pPar;
        double J = pms.J;
        double A_0 = pms.A_0;

        Tarr[0] = 0;
        Tarr[1] = (am- A_0) / J;
        Tarr[2] = 0;
        Tarr[3] = am / J;
        Tarr[4] = 0;
        Tarr[5] = 0;
        Tarr[6] = 0;
        Tarr[7] = 0;
    };

    void VelForwardSj::_buildTimeSegment_s22(void){
        auto& pms = *pPar;
        double J = pms.J;
        double A_0 = pms.A_0;

        Tarr[0] = 0;
        Tarr[1] = 0;
        Tarr[2] = 0;
        Tarr[3] = 0;
        Tarr[4] = 0;
        Tarr[5] = (an + A_0) / J;
        Tarr[6] = 0;
        Tarr[7] = an / J;
    };

void VelForwardSj::_buildTimeSegment_s10(void){
        auto& pms = *pPar;
        double V_0 = pms.V_0;
        double A_0 = pms.A_0;
        double V_d = pms.V_d;
        double J = pms.J;
        double S_0 = pms.S_0;
        double Sm = pms.S_d;
        double J2 = SQUARE(J);

        double T_0 = 0;
        double T_1 = 0;
        double T_2 = 0.0;
        double T_3 = 0;
        double T_4 = Sm/vm;

        double T_5 = 0;
        double T_6 = 0.0;
        double T_7 = 0;
        // Tarr = {T_0, T_1, T_2, T_3, T_4, T_5, T_6, T_7};
        Tarr[0] = T_0;Tarr[1] = T_1;Tarr[2] = T_2;Tarr[3] = T_3;
        Tarr[4] = T_4;Tarr[5] = T_5;Tarr[6] = T_6;Tarr[7] = T_7;
    };
    void VelForwardSj::_buildPosVecSegment(void){
        if (nseg == SLADDER_70_SEGMENT){
            _buildPosVecSegment_s70();
        }else if (nseg == SLADDER_60_SEGMENT){
            _buildPosVecSegment_s60();
        }else if (nseg == SLADDER_61_SEGMENT){
            _buildPosVecSegment_s61();
        }else if (nseg == SLADDER_62_SEGMENT){
            _buildPosVecSegment_s62();
        }else if (nseg == SLADDER_50_SEGMENT || nseg == SLADDER_33_SEGMENT || nseg == SLADDER_34_SEGMENT){
            _buildPosVecSegment_s50();
        }else if (nseg == SLADDER_51_SEGMENT){
            _buildPosVecSegment_s51();
        }else if (nseg == SLADDER_52_SEGMENT){
            _buildPosVecSegment_s52();
        }else if (nseg == SLADDER_40_SEGMENT){
            _buildPosVecSegment_s40();
        }else if (nseg == SLADDER_31_SEGMENT){
            _buildPosVecSegment_s31();
        }else if (nseg == SLADDER_32_SEGMENT){
            _buildPosVecSegment_s32();  
        }else if (nseg == SLADDER_21_SEGMENT){
            _buildPosVecSegment_s21();
        }else if (nseg == SLADDER_22_SEGMENT){
            _buildPosVecSegment_s22();   
        }else if (nseg == SLADDER_10_SEGMENT){
            _buildPosVecSegment_s50();  
        }else{
            _buildPosVecSegment_s70();
        }
    };

    void VelForwardSj::_buildPosVecSegment_s70(void){
        auto& pms = *pPar;
        double J = pms.J;
        double A_0 = pms.A_0;

        Varr[0] = pms.V_0;
        Varr[1] = Varr[0] +A_0 * Tarr[1] + J*SQUARE(Tarr[1])/2;
        Varr[2] = Varr[1] +am * Tarr[2];
        Varr[3] = Varr[2] +am * Tarr[3] - J*SQUARE(Tarr[3])/2;
        Varr[4] = Varr[3]; //  vm
        Varr[5] = Varr[4] -J * SQUARE(Tarr[5])/2;
        Varr[6] = Varr[5] -an * Tarr[6];
        Varr[7] = Varr[6] -an * Tarr[7] +J*SQUARE(Tarr[7])/2; // V_d

        Sarr[0] = pms.S_0;
        Sarr[1] = Varr[0] * Tarr[1] + A_0*SQUARE(Tarr[1])/2 + J*CUBE(Tarr[1])/6;
        Sarr[2] = Varr[1] * Tarr[2] + am*SQUARE(Tarr[2])/2;
        Sarr[3] = Varr[2] * Tarr[3] + am*SQUARE(Tarr[3])/2 - J*CUBE(Tarr[3])/6;
        Sarr[4] = Varr[3] * Tarr[4];
        Sarr[5] = Varr[4] * Tarr[5] - J*CUBE(Tarr[5])/6;
        Sarr[6] = Varr[5] * Tarr[6] - an*SQUARE(Tarr[6])/2;
        Sarr[7] = Varr[6] * Tarr[7] - an*SQUARE(Tarr[7])/2 + J*CUBE(Tarr[7])/6;
    };

    void VelForwardSj::_buildPosVecSegment_s60(void){
        auto& pms = *pPar;
        double J = pms.J;
        double S_0 = pms.S_0, V_0=pms.V_0, A_0=pms.A_0;

        Varr[0] = V_0;
        Varr[1] = Varr[0] +A_0 * Tarr[1] + J*SQUARE(Tarr[1])/2;
        Varr[2] = Varr[1] +am * Tarr[2];
        Varr[3] = Varr[2] +am * Tarr[3] - J*SQUARE(Tarr[3])/2;
        Varr[4] = Varr[3]; //  vm
        Varr[5] = Varr[4] -J * SQUARE(Tarr[5])/2;
        Varr[6] = Varr[5] -an * Tarr[6];
        Varr[7] = Varr[6] -an * Tarr[7] +J*SQUARE(Tarr[7])/2; // V_d

        Sarr[0] = S_0;
        Sarr[1] = Varr[0] * Tarr[1] + A_0*SQUARE(Tarr[1])/2 + J*CUBE(Tarr[1])/6;
        Sarr[2] = Varr[1] * Tarr[2] + am*SQUARE(Tarr[2])/2;
        Sarr[3] = Varr[2] * Tarr[3] + am*SQUARE(Tarr[3])/2 - J*CUBE(Tarr[3])/6;
        Sarr[4] = 0;
        Sarr[5] = Varr[4] * Tarr[5] - J*CUBE(Tarr[5])/6;
        Sarr[6] = Varr[5] * Tarr[6] - an*SQUARE(Tarr[6])/2;
        Sarr[7] = Varr[6] * Tarr[7] - an*SQUARE(Tarr[7])/2 + J*CUBE(Tarr[7])/6;
    };

    void VelForwardSj::_buildPosVecSegment_s61(void){
        return _buildPosVecSegment_s70();
    };

    void VelForwardSj::_buildPosVecSegment_s62(void){
        return _buildPosVecSegment_s70();
    };

    void VelForwardSj::_buildPosVecSegment_s50(void){
        auto& pms = *pPar;
        double V_0 = pms.V_0;
        double A_0 = pms.A_0;
        // double V_d = pms.V_d;
        double J = pms.J;
        double S_0 = pms.S_0;
        // double Sm = pms.S_d;
        double J2 = SQUARE(J);
        // double vm = vm;
        // double am = am;
        // double an = an;

        double T_1 = Tarr[1];
        double T_2 = Tarr[2];
        double T_3 = Tarr[3];
        double T_4 = Tarr[4];
        double T_5 = Tarr[5];
        double T_6 = Tarr[6];
        double T_7 = Tarr[7];

        // double V_0 = V_0;
        double V_1 = V_0+A_0*T_1+J*SQUARE(T_1)/2;
        double V_2 = V_1+am*T_2;
        double V_3 = V_2+am*T_3-J*SQUARE(T_3)/2;
        double V_4 = V_3;
        double V_5 = V_4-J*SQUARE(T_5)/2;
        double V_6 = V_5-an*T_6;
        double V_7 = V_6-an*T_7+J*SQUARE(T_7)/2;
        // Varr = {V_0, V_1, V_2, V_3, V_4, V_5, V_6, V_7};
        Varr[0] = V_0;Varr[1] = V_1;Varr[2] = V_2;Varr[3] = V_3;
        Varr[4] = V_4;Varr[5] = V_5;Varr[6] = V_6;Varr[7] = V_7;

        // S_0 = S_0
        Sarr[1] = V_0*T_1+A_0*SQUARE(T_1)/2+J*CUBE(T_1)/6;
        Sarr[2] = V_1*T_2+am*SQUARE(T_2)/2;
        Sarr[3] = V_2*T_3+am*SQUARE(T_3)/2-J*CUBE(T_3)/6;
        Sarr[4] = V_3*T_4;
        Sarr[5] = V_4*T_5-J*CUBE(T_5)/6;
        Sarr[6] = V_5*T_6-an*SQUARE(T_6)/2;
        Sarr[7] = V_6*T_7-an*SQUARE(T_7)/2+J*CUBE(T_7)/6;
    };

    void VelForwardSj::_buildPosVecSegment_s51(void){
        auto& pms = *pPar;
        double V_0 = pms.V_0;
        double A_0 = pms.A_0;
        // double V_d = pms.V_d;
        double J = pms.J;
        double S_0 = pms.S_0;
        // double Sm = pms.S_d;
        double J2 = SQUARE(J);
        // double vm = vm;
        // double am = am;
        // double an = an;

        double T_1 = Tarr[1];
        double T_2 = Tarr[2];
        double T_3 = Tarr[3];
        double T_4 = Tarr[4];
        double T_5 = Tarr[5];
        double T_6 = Tarr[6];
        double T_7 = Tarr[7];

        // double V_0 = V_0;
        double V_1 = V_0+A_0*T_1+J*SQUARE(T_1)/2;
        double V_2 = V_1+am*T_2;
        double V_3 = V_2+am*T_3-J*SQUARE(T_3)/2;
        double V_4 = V_3;
        double V_5 = V_4-J*SQUARE(T_5)/2;
        double V_6 = V_5-an*T_6;
        double V_7 = V_6-an*T_7+J*SQUARE(T_7)/2;
        // Varr = {V_0, V_1, V_2, V_3, V_4, V_5, V_6, V_7};
        Varr[0] = V_0;Varr[1] = V_1;Varr[2] = V_2;Varr[3] = V_3;
        Varr[4] = V_4;Varr[5] = V_5;Varr[6] = V_6;Varr[7] = V_7;

        // S_0 = S_0
        Sarr[1] = V_0*T_1+A_0*SQUARE(T_1)/2+J*CUBE(T_1)/6;
        Sarr[2] = V_1*T_2+am*SQUARE(T_2)/2;
        Sarr[3] = V_2*T_3+am*SQUARE(T_3)/2-J*CUBE(T_3)/6;
        Sarr[4] = V_3*T_4;
        Sarr[5] = V_4*T_5-J*CUBE(T_5)/6;
        Sarr[6] = V_5*T_6-an*SQUARE(T_6)/2;
        Sarr[7] = V_6*T_7-an*SQUARE(T_7)/2+J*CUBE(T_7)/6;
    };

    void VelForwardSj::_buildPosVecSegment_s52(void){
        auto& pms = *pPar;
        double V_0 = pms.V_0;
        double A_0 = pms.A_0;
        // double V_d = pms.V_d;
        double J = pms.J;
        double S_0 = pms.S_0;
        // double Sm = pms.S_d;
        double J2 = SQUARE(J);

        double T_1 = Tarr[1];
        double T_2 = Tarr[2];
        double T_3 = Tarr[3];
        double T_4 = Tarr[4];
        double T_5 = Tarr[5];
        double T_6 = Tarr[6];
        double T_7 = Tarr[7];

        // double V_0 = V_0;
        double V_1 = V_0+A_0*T_1+J*SQUARE(T_1)/2;
        double V_2 = V_1+am*T_2;
        double V_3 = V_2+am*T_3-J*SQUARE(T_3)/2;
        double V_4 = V_3;
        double V_5 = V_4-J*SQUARE(T_5)/2;
        double V_6 = V_5-an*T_6;
        double V_7 = V_6-an*T_7+J*SQUARE(T_7)/2;
        // Varr = {V_0, V_1, V_2, V_3, V_4, V_5, V_6, V_7};
        Varr[0] = V_0;Varr[1] = V_1;Varr[2] = V_2;Varr[3] = V_3;
        Varr[4] = V_4;Varr[5] = V_5;Varr[6] = V_6;Varr[7] = V_7;

        // S_0 = S_0
        double S_1 = V_0*T_1+A_0*SQUARE(T_1)/2+J*CUBE(T_1)/6;
        double S_2 = V_1*T_2+am*SQUARE(T_2)/2;
        double S_3 = V_2*T_3+am*SQUARE(T_3)/2-J*CUBE(T_3)/6;
        double S_4 = V_3*T_4;
        double S_5 = V_4*T_5-J*CUBE(T_5)/6;
        double S_6 = V_5*T_6-an*SQUARE(T_6)/2;
        double S_7 = V_6*T_7-an*SQUARE(T_7)/2+J*CUBE(T_7)/6;
        // Sarr = {S_0, S_1, S_2, S_3, S_4, S_5, S_6, S_7};
        Sarr[0] = S_0;Sarr[1] = S_1;Sarr[2] = S_2;Sarr[3] = S_3;
        Sarr[4] = S_4;Sarr[5] = S_5;Sarr[6] = S_6;Sarr[7] = S_7;
    };

    void VelForwardSj::_buildPosVecSegment_s40(void){
        auto& pms = *pPar;
        double J = pms.J;
        double S_0 = pms.S_0, V_0=pms.V_0, A_0=pms.A_0;

        Varr[0] = V_0;
        Varr[1] = Varr[0] +A_0 * Tarr[1] + J*SQUARE(Tarr[1])/2;
        Varr[2] = Varr[1];
        Varr[3] = Varr[2] +am * Tarr[3] - J*SQUARE(Tarr[3])/2;
        Varr[4] = Varr[3]; //  vm
        Varr[5] = Varr[4] -J * SQUARE(Tarr[5])/2;
        Varr[6] = Varr[5];
        Varr[7] = Varr[6] -an * Tarr[7] +J*SQUARE(Tarr[7])/2; // V_d

        Sarr[0] = S_0;
        Sarr[1] = Varr[0] * Tarr[1] + A_0*SQUARE(Tarr[1])/2 + J*CUBE(Tarr[1])/6;
        Sarr[2] = 0;
        Sarr[3] = Varr[2] * Tarr[3] + am*SQUARE(Tarr[3])/2 - J*CUBE(Tarr[3])/6;
        Sarr[4] = 0;
        Sarr[5] = Varr[4] * Tarr[5] - J*CUBE(Tarr[5])/6;
        Sarr[6] = 0;
        Sarr[7] = Varr[6] * Tarr[7] - an*SQUARE(Tarr[7])/2 + J*CUBE(Tarr[7])/6;
    };

    void VelForwardSj::_buildPosVecSegment_s31(void){
        auto& pms = *pPar;
        double V_0 = pms.V_0;
        double A_0 = pms.A_0;
        // double V_d = pms.V_d;
        double J = pms.J;
        double S_0 = pms.S_0;
        // double Sm = pms.S_d;
        double J2 = SQUARE(J);

        double T_1 = Tarr[1];
        double T_2 = Tarr[2];
        double T_3 = Tarr[3];
        double T_4 = Tarr[4];
        double T_5 = Tarr[5];
        double T_6 = Tarr[6];
        double T_7 = Tarr[7];

        // double V_0 = V_0;
        double V_1 = V_0+A_0*T_1+J*SQUARE(T_1)/2;
        double V_2 = V_1+am*T_2;
        double V_3 = V_2+am*T_3-J*SQUARE(T_3)/2;
        double V_4 = V_3;
        double V_5 = V_4;
        double V_6 = V_5;
        double V_7 = V_6;
        // Varr = {V_0, V_1, V_2, V_3, V_4, V_5, V_6, V_7};
        Varr[0] = V_0;Varr[1] = V_1;Varr[2] = V_2;Varr[3] = V_3;
        Varr[4] = V_4;Varr[5] = V_5;Varr[6] = V_6;Varr[7] = V_7;

        // S_0 = S_0
        double S_1 = V_0*T_1+A_0*SQUARE(T_1)/2+J*CUBE(T_1)/6;
        double S_2 = V_1*T_2+am*SQUARE(T_2)/2;
        double S_3 = V_2*T_3+am*SQUARE(T_3)/2-J*CUBE(T_3)/6;
        double S_4 = V_3*T_4;
        double S_5 = V_4*T_5;
        double S_6 = V_5*T_6;
        double S_7 = V_6*T_7;
        // Sarr = {S_0, S_1, S_2, S_3, S_4, S_5, S_6, S_7};
        Sarr[0] = S_0;Sarr[1] = S_1;Sarr[2] = S_2;Sarr[3] = S_3;
        Sarr[4] = S_4;Sarr[5] = S_5;Sarr[6] = S_6;Sarr[7] = S_7;
    };


    void VelForwardSj::_buildPosVecSegment_s32(void){
        auto& pms = *pPar;
        double V_0 = pms.V_0;
        double A_0 = pms.A_0;
        // double V_d = pms.V_d;
        double J = pms.J;
        double S_0 = pms.S_0;
        // double Sm = pms.S_d;
        double J2 = SQUARE(J);

        double T_1 = Tarr[1];
        double T_2 = Tarr[2];
        double T_3 = Tarr[3];
        double T_4 = Tarr[4];
        double T_5 = Tarr[5];
        double T_6 = Tarr[6];
        double T_7 = Tarr[7];

        // double V_0 = V_0;
        double V_1 = V_0;
        double V_2 = V_1;
        double V_3 = V_2;
        double V_4 = V_3;
        double V_5 = V_4+A_0*T_5-J*SQUARE(T_5)/2;
        double V_6 = V_5-an*T_6;
        double V_7 = V_6-an*T_7+J*SQUARE(T_7)/2;
        // Varr = {V_0, V_1, V_2, V_3, V_4, V_5, V_6, V_7};
        Varr[0] = V_0;Varr[1] = V_1;Varr[2] = V_2;Varr[3] = V_3;
        Varr[4] = V_4;Varr[5] = V_5;Varr[6] = V_6;Varr[7] = V_7;

        // S_0 = S_0
        double S_1 = V_0*T_1;
        double S_2 = V_1*T_2;
        double S_3 = V_2*T_3;
        double S_4 = V_3*T_4;
        double S_5 = V_4*T_5+A_0*SQUARE(T_5)/2-J*CUBE(T_5)/6;
        double S_6 = V_5*T_6-an*SQUARE(T_6)/2;
        double S_7 = V_6*T_7-an*SQUARE(T_7)/2+J*CUBE(T_7)/6;
        // Sarr = {S_0, S_1, S_2, S_3, S_4, S_5, S_6, S_7};
        Sarr[0] = S_0;Sarr[1] = S_1;Sarr[2] = S_2;Sarr[3] = S_3;
        Sarr[4] = S_4;Sarr[5] = S_5;Sarr[6] = S_6;Sarr[7] = S_7;
    };

    void VelForwardSj::_buildPosVecSegment_s21(void){
        auto& pms = *pPar;
        double J = pms.J;
        double S_0 = pms.S_0, V_0=pms.V_0, A_0=pms.A_0;

        Varr[0] = V_0;
        Varr[1] = Varr[0] +A_0 * Tarr[1] + J*SQUARE(Tarr[1])/2;
        Varr[2] = Varr[1];
        Varr[3] = Varr[2] +am * Tarr[3] - J*SQUARE(Tarr[3])/2;
        Varr[4] = Varr[3]; //  vm
        Varr[5] = Varr[4];
        Varr[6] = Varr[5];
        Varr[7] = Varr[6]; // V_d

        Sarr[0] = S_0;
        Sarr[1] = Varr[0] * Tarr[1] + A_0*SQUARE(Tarr[1])/2 + J*CUBE(Tarr[1])/6;
        Sarr[2] = 0;
        Sarr[3] = Varr[2] * Tarr[3] + am*SQUARE(Tarr[3])/2 - J*CUBE(Tarr[3])/6;
        Sarr[4] = 0;
        Sarr[5] = 0;
        Sarr[6] = 0;
        Sarr[7] = 0;
    };

static int _check_array_positive(const double* arr){
    for (int i=0;i<8;i++){
        if (arr[i]<-EPS)
            return -1;
    }
    return 0;
}
    void VelForwardSj::_buildPosVecSegment_s22(void){
        auto& pms = *pPar;
        double J = pms.J;
        double S_0 = pms.S_0, V_0=pms.V_0, A_0=pms.A_0;

        Varr[0] = V_0;
        Varr[1] = Varr[0];
        Varr[2] = Varr[1];
        Varr[3] = Varr[2];
        Varr[4] = Varr[3]; //  vm
        Varr[5] = Varr[4] + A_0 * Tarr[5] - J*SQUARE(Tarr[5])/2;
        Varr[6] = Varr[5];
        Varr[7] = Varr[6] - an * Tarr[7] + J*SQUARE(Tarr[7])/2; // V_d

        Sarr[0] = S_0;
        Sarr[1] = 0;
        Sarr[2] = 0;
        Sarr[3] = 0;
        Sarr[4] = 0;
        Sarr[5] = Varr[4] * Tarr[5] + A_0*SQUARE(Tarr[5])/2 - J*CUBE(Tarr[5])/6;
        Sarr[6] = 0;
        Sarr[7] = Varr[6] * Tarr[7] - an*SQUARE(Tarr[7])/2 + J*CUBE(Tarr[7])/6;
    };

    int VelForwardSj::check() const{
        const auto& pms = *pPar;
        // assert abs(S_d - slp.s_sum) < 1e-5
        // assert abs(slp.Varr[4] - slp.vm) < 1e-5
        // assert abs(slp.Varr[7] - V_d) < 1e-5
        int err = 0;
        if ((pms.V_m >= vm) && (pms.A_m >= am) && (pms.A_n >= an)){
            err = 0;
        }else{
            err = 1;
        };
        err |= (_check_array_positive(Tarr)<<2);
        err |= (_check_array_positive(Sarr)<<3);
        err |= (_check_array_positive(Varr)<<4);
        err |= ((Varr[3] > pms.V_m + TOL)<<5);
        return err;
    }

double VelForwardSj::get_prefer_insertime(double tm)const{
    if (tm <= cTarr[3]){
        return cTarr[3];
    }else if (tm <= cTarr[4]){
        return tm;
    }else{
        return tm; // cTarr[7];
    }
}

    double VelForwardSj::get_distance(double tm)const{
        switch(nseg){
            case SLADDER_70_SEGMENT:
            case SLADDER_61_SEGMENT:
            case SLADDER_62_SEGMENT:
                return _get_distance_s70(tm);
            case SLADDER_60_SEGMENT:
                return _get_distance_s60(tm);
            case SLADDER_50_SEGMENT:
                return _get_distance_s50(tm);
            case SLADDER_40_SEGMENT:
                return _get_distance_s40(tm);
            case SLADDER_51_SEGMENT:
                return _get_distance_s51(tm);
            case SLADDER_52_SEGMENT:
                return _get_distance_s52(tm); 
            case SLADDER_31_SEGMENT:
                return _get_distance_s31(tm);
            case SLADDER_21_SEGMENT:
                return _get_distance_s21(tm);                                                                                   
            default:
                return _get_distance_s70(tm);
        }
    }

double VelForwardSj::_get_distance_s70(double tm)const{
    auto& pms = *pPar;
    double J = pms.J;
    double A_0 = pms.A_0;
    double sc,dt;
    // Varr = self.Varr
    if (tm <= cTarr[0]) {
        sc = cSarr[0]; }
    else if (tm <= cTarr[1]){
        dt = tm - cTarr[0];
        sc = cSarr[0] + Varr[0] * dt + A_0 * SQUARE(dt) /2 + J*CUBE(dt)/6;
    }else if (tm <= cTarr[2]){
        dt = tm - cTarr[1] ;
        sc = cSarr[1] + Varr[1] * dt+ am * SQUARE(dt) /2;
    }else if (tm <= cTarr[3]){
        dt = tm - cTarr[2];
        sc = cSarr[2] + Varr[2] * dt+ am * SQUARE(dt) /2 - J*CUBE(dt)/6;
    }else if (tm <= cTarr[4]){
        dt = tm - cTarr[3];
        sc = cSarr[3] + Varr[3] * dt;
    }else if (tm <= cTarr[5]){
        dt = tm - cTarr[4];
        sc = cSarr[4] + Varr[4] * dt - J*CUBE(dt)/6;
    }else if (tm <= cTarr[6]){
        dt = tm - cTarr[5];
        sc = cSarr[5] + Varr[5] * dt - an * SQUARE(dt) /2;
    }else if (tm <= cTarr[7]){
        dt = tm - cTarr[6];
        sc = cSarr[6] + Varr[6] * dt - an * SQUARE(dt) /2 + J*CUBE(dt)/6;
    }else {
        sc = cSarr[7];
    }
    return sc;
}

double VelForwardSj::_get_distance_s60(double tm)const{
    return _get_distance_s70(tm);
}

double VelForwardSj::_get_distance_s50(double tm)const{
    return _get_distance_s70(tm);
}

double VelForwardSj::_get_distance_s51(double tm)const{
    return _get_distance_s70(tm);
}

double VelForwardSj::_get_distance_s52(double tm)const{
    return _get_distance_s70(tm);
}

double VelForwardSj::_get_distance_s40(double tm)const{
    return _get_distance_s70(tm);
}

double VelForwardSj::_get_distance_s31(double tm)const{
    return _get_distance_s70(tm);
}

double VelForwardSj::_get_distance_s21(double tm)const{
    return _get_distance_s70(tm);
}

double VelForwardSj::get_rate(double tm)const{
    switch(nseg){
        case SLADDER_70_SEGMENT:
        case SLADDER_61_SEGMENT:
        case SLADDER_62_SEGMENT:
            return _get_rate_s70(tm);
        case SLADDER_60_SEGMENT:
            return _get_rate_s60(tm);
        case SLADDER_50_SEGMENT:
            return _get_rate_s50(tm);
        case SLADDER_40_SEGMENT:
            return _get_rate_s40(tm);
        case SLADDER_51_SEGMENT:
            return _get_rate_s51(tm);
        case SLADDER_52_SEGMENT:
            return _get_rate_s52(tm); 
        case SLADDER_31_SEGMENT:
            return _get_rate_s31(tm);
        case SLADDER_21_SEGMENT:
            return _get_rate_s21(tm);                                                                                  
        default:
            return _get_rate_s70(tm);
    }
}

double VelForwardSj::_get_rate_s70(double tm)const{
    auto& pms = *pPar;
    double J = pms.J;
    double A_0 = pms.A_0;
    double sc, dt;
        if (tm <= cTarr[0]) {
            sc = Varr[0];
        }else if (tm <= cTarr[1]) {
            dt = tm - cTarr[0];
            sc = Varr[0] + A_0 * dt + J*SQUARE(dt)/2;
        }else if (tm <= cTarr[2]) {
            dt = tm - cTarr[1] ;
            sc = Varr[1] + am * dt;
        }else if (tm <= cTarr[3]) {
            dt = tm - cTarr[2];
            sc = Varr[2] + am * dt - J*SQUARE(dt)/2;
        }else if (tm <= cTarr[4]) {
            dt = tm - cTarr[3];
            sc = Varr[3];
        }else if (tm <= cTarr[5]) {
            dt = tm - cTarr[4];
            sc = Varr[4] - J*SQUARE(dt)/2;
        }else if (tm <= cTarr[6]) {
            dt = tm - cTarr[5];
            sc = Varr[5] - an * dt;
        }else if (tm <= cTarr[7]) {
            dt = tm - cTarr[6];
            sc = Varr[6] - an * dt + J*SQUARE(dt)/2;
        }else {
            sc = 0;
        }
        return sc;
}

double VelForwardSj::_get_rate_s60(double tm)const{
    return _get_rate_s70(tm);
}

double VelForwardSj::_get_rate_s50(double tm)const{
    return _get_rate_s70(tm);
}

double VelForwardSj::_get_rate_s51(double tm)const {
    return _get_rate_s70(tm);
}

double VelForwardSj::_get_rate_s52(double tm)const {
    return _get_rate_s70(tm);
}

double VelForwardSj::_get_rate_s40(double tm)const {
    return _get_rate_s70(tm);
}

double VelForwardSj::_get_rate_s31(double tm)const {
    return _get_rate_s70(tm);
}

double VelForwardSj::_get_rate_s21(double tm)const {
    return _get_rate_s70(tm);
}

double VelForwardSj::_get_acc_rate_s70(double tm)const {
    auto& pms = *pPar;
    double J = pms.J;
    // double am = pms.am;
    // double an = pms.an;
    double A_0 = pms.A_0;
    double sc,dt;
    // double pms.Varr;
    if (tm <= cTarr[0]) {
        sc = 0;
    }else if (tm <= cTarr[1]) {
        dt = tm - cTarr[0];
        sc = A_0  + J*dt;
    }else if (tm <= cTarr[2]) {
        sc = am;
    }else if (tm <= cTarr[3]) {
        dt = tm - cTarr[2];
        sc = am  - J*dt;
    }else if (tm <= cTarr[4]) {
        sc = 0;
    }else if (tm <= cTarr[5]) {
        dt = tm - cTarr[4];
        sc = - J*dt;
    }else if (tm <= cTarr[6]) {
        sc = - an;
    }else if (tm <= cTarr[7]) {
        dt = tm - cTarr[6];
        sc = - an  + J*dt;
    }else {
        sc = 0;
    }
    return sc;
}

double VelForwardSj::_get_acc_rate_s60(double tm)const {
    return _get_acc_rate_s70(tm);
}

double VelForwardSj::_get_acc_rate_s50(double tm)const {
    return _get_acc_rate_s70(tm);
}

double VelForwardSj::_get_acc_rate_s51(double tm)const {
    return _get_acc_rate_s70(tm);
}

double VelForwardSj::_get_acc_rate_s52(double tm)const {
    return _get_acc_rate_s70(tm);
}

double VelForwardSj::_get_acc_rate_s40(double tm)const {
    return _get_acc_rate_s70(tm);
}

double VelForwardSj::_get_acc_rate_s31(double tm)const {
    return _get_acc_rate_s70(tm);
}

double VelForwardSj::_get_acc_rate_s21(double tm)const {
    return _get_acc_rate_s70(tm);
}

double VelForwardSj::get_acc(double tm)const {
    switch(nseg){
        case SLADDER_70_SEGMENT:
        case SLADDER_61_SEGMENT:
        case SLADDER_62_SEGMENT:
            return _get_acc_rate_s70(tm);
        case SLADDER_60_SEGMENT:
            return _get_acc_rate_s60(tm);
        case SLADDER_50_SEGMENT:
            return _get_acc_rate_s50(tm);
        case SLADDER_40_SEGMENT:
            return _get_acc_rate_s40(tm);
        case SLADDER_51_SEGMENT:
            return _get_acc_rate_s51(tm);
        case SLADDER_52_SEGMENT:
            return _get_acc_rate_s52(tm);    
        case SLADDER_31_SEGMENT:
            return _get_acc_rate_s31(tm);
        case SLADDER_21_SEGMENT:
            return _get_acc_rate_s21(tm);                                                                           
        default:
            return _get_acc_rate_s70(tm);
    }
}
    double VelForwardSj::get_jerk(double tm)const{
        return _get_jerk_s70(tm);
    }

    double VelForwardSj::_get_jerk_s70(double tm)const
    {
        auto& pms = *pPar;
        double J = pms.J;
        double jc;
        if (tm <= cTarr[0]) {
            jc = 0;
        }else if (tm <= cTarr[1]) {
            jc = J;
        }else if (tm <= cTarr[2]) {
            jc = 0;
        }else if (tm <= cTarr[3]) {
            jc = -J;
        }else if (tm <= cTarr[4]) {
            jc = 0;
        }else if (tm <= cTarr[5]) {
            jc = -J;
        }else if (tm <= cTarr[6]) {
            jc = 0;
        }else if (tm <= cTarr[7]) {
            jc = J;
        }else {
            jc = 0;
        }
        return jc;
    }
}