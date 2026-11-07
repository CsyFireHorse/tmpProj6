#ifndef SCURVE_BASE_H_
#define SCURVE_BASE_H_

namespace zmc
{

enum LadderSegmentExceptionType:unsigned char
{
    NONE_EXCEPTION = 0,
    CAN_REVERSE = 1,
    FLEXIBLE_VELOCITY = 2, 
    FLEXIBLE_REVERSE = 3,
    LADDER_EXCEPTION = 4,
    FLEXIBLE_DECEL = 8,
    FLEXIBLE_DECEL_JERK = 16,
};

enum LadderSegmentType:signed char
{
    LADDER_ERR_SEGMENT = -1, //
    LADDER_0_SEGMENT = 0, // 
    LADDER_10_SEGMENT = 10, // T2 __
    LADDER_11_SEGMENT = 11, // T1 /
    LADDER_12_SEGMENT = 12, // T3 \ ,
    LADDER_20_SEGMENT = 20, // T1,T3  /\ , 
    LADDER_21_SEGMENT = 21, // T1,T2 /--
    LADDER_22_SEGMENT = 22, // T2,T3  --\ ,
    LADDER_23_SEGMENT = 23, //T3,T6  inverse   \/
    LADDER_24_SEGMENT = 24, //  T4, T2   V0>Vm  \__
    LADDER_25_SEGMENT = 25, //  Vd>Vm  __/
    LADDER_30_SEGMENT = 30, // T1,T2,T3, V0<Vm, V_d<Vm
    LADDER_31_SEGMENT = 31, // T4,T2,T3, V0>Vm, V_d<Vm S-> +inf
    LADDER_32_SEGMENT = 32, // T1,T2,T1, Vd>Vm
    LADDER_33_SEGMENT = 33, // T3,T5,T6, V0>Vm, V_d>Vm
    LADDER_34_SEGMENT = 34, // Vd>Vm V0>Vm  \__/
    LADDER_36_SEGMENT = 36 // T3,T4,T6  inverse
};

enum SLadderSegmentType:signed char
{
    SLADDER_ERR_SEGMENT = -1, //
    SLADDER_0_SEGMENT = 0,
    SLADDER_10_SEGMENT = 10, // T4
    SLADDER_20_SEGMENT = 20,
    SLADDER_21_SEGMENT = 21, // T1,T3 , {A_0, A_m, 0}
    SLADDER_22_SEGMENT = 22, // T5,T7   {0, -A_n, 0}
    SLADDER_23_SEGMENT = 23, // T1,T3 ,      {A_0, -A_n, 0}
    SLADDER_24_SEGMENT = 24, // T5,T7  {0, A_m, 0}

    SLADDER_30_SEGMENT = 30,
    SLADDER_31_SEGMENT = 31, // T1,T2,T3,  {A_0, A_m, A_m, 0}, A_0 < A_m
    SLADDER_32_SEGMENT = 32, // T5,T6,T7   {0, -A_n, -A_n, 0}
    SLADDER_33_SEGMENT = 33, // T1,T3,T4
    SLADDER_34_SEGMENT = 34, // T4,T5,T7

    SLADDER_35_SEGMENT = 35, // T1,T2,T3, \__/  {A_0, -A_n, -A_n, 0}
    SLADDER_36_SEGMENT = 36, // T5,T6,T7,   {0, A_m, A_m, 0}

    SLADDER_37_SEGMENT = 37, // T5,T7,T4 V_0

    SLADDER_38_SEGMENT = 38, // T1,T2,T3,  {A_0, A_m, A_m, 0}, A_0 > A_m
    SLADDER_39_SEGMENT = 39, // T1,T2,T3,   {A_0, -A_m, -A_m, 0}, A_0 < -A_m

    SLADDER_40_SEGMENT = 40, // T1,T3,T5,T7
    SLADDER_41_SEGMENT = 41, // T1,T2,T3,T4
    SLADDER_42_SEGMENT = 42, // T4,T5,T6,T7
    SLADDER_43_SEGMENT = 43, // T5,T6,T7,T4 V_0
    SLADDER_44_SEGMENT = 44, // T5,T7,T1,T3    V_m < V_0, V_d  
    SLADDER_50_SEGMENT = 50, // T1,T3,T4,T5,T7
    SLADDER_51_SEGMENT = 51, // T1,T3,T5,T6,T7
    SLADDER_52_SEGMENT = 52, // T1,T2,T3,T5,T7
    SLADDER_53_SEGMENT = 53, // T5,T7,T4,T5,T7    V_0
    SLADDER_54_SEGMENT = 54, // T5,T7,T4,T1,T3   V_m < V_0, V_d
    SLADDER_60_SEGMENT = 60, // T1,T2,T3,T5,T6,T7
    SLADDER_61_SEGMENT = 61, // T1,T3,T4,T5,T6,T7
    SLADDER_62_SEGMENT = 62, // T1,T2,T3,T4,T5,T7
    SLADDER_64_SEGMENT = 64, // T5,T7,T4,T5,T6,T7 V_0
    SLADDER_65_SEGMENT = 65, // T5,T6,T7,T4,T5,T7 V_0
    SLADDER_66_SEGMENT = 66, // T5,T6,T7,T1,T2,T3    V_m < V_0, V_d
    SLADDER_70_SEGMENT = 70, // T1,T2,T3,T4,T5,T6,T7
    SLADDER_71_SEGMENT = 71, // T5,T6,T7,T4,T5,T6,T7  V_0 > V_m > V_d
    SLADDER_72_SEGMENT = 72, // T5,T6,T7,T4,T1,T2,T3  V_m < V_0, V_d
    SLADDER_73_SEGMENT = 73, // T1,T2,T3,T4,T1,T2,T3  V_0 < V_m < V_d


    SLADDER_76_SEGMENT = 76,
    SLADDER_77_SEGMENT = 77, // T_2
    SLADDER_78_SEGMENT = 78, // T_6
    SLADDER_79_SEGMENT = 79, // T_2, T_6
};

struct SLadderRelaxParam{
    SLadderSegmentType nseg;
    double _vm;
    double _am;
    double _an;
};


inline SLadderRelaxParam gen_SLadderRelaxParam(SLadderSegmentType nseg, double vm, double am, double an)
{
    SLadderRelaxParam ret = {nseg, vm, am, an};
    return ret;
}

enum SLadderSymmetry:unsigned char
{
    SLADDER_AYMMETRY = 0, // not symmetry
    SLADDER_AZ_SYMMETRY, // A0=Ad=0
    SLADDER_V_S_SYMMETRY, // A0=Ad=0, V0=Vd, AM!=AN
    SLADDER_V_SYMMETRY, // A0=Ad=0, V0=Vd, AM=AN
    SLADDER_Z_SYMMETRY, // A0=Ad=0, V0=Vd=0, AM=AN

    // SLADDER_ND_SYMMETRY = 4, // AM!=AN
    // SLADDER_AD_SYMMETRY = 5, // A0!=0,V0=Vd，AM=AN
    // SLADDER_VD_SYMMETRY = 6, // A0!=0,V0=Vd，AM=AN
    // SLADDER_ZD_SYMMETRY = 7, // A0!=0， V0=Vd=0，AM=AN
};

enum SLadderSortType
{
    SLADDER_Vm_Am_An = 3, // Vm > Am2Vm > An2Vm
    SLADDER_Vm_Am_An_V0d = 4, // Vm > Am2Vm > An2Vm > max(V0,Vd)
    SLADDER_Vm_Am_V0d_An = 5, // Vm > Am2Vm > max(V0,Vd) > An2Vm

    SLADDER_Vm_An_Am = 6,
    SLADDER_Vm_An_Am_V0d = 7,
    SLADDER_Vm_An_V0d_Am = 8,

    SLADDER_Am_Vm_An = 9,
    SLADDER_Am_Vm_An_V0d = 10,
    SLADDER_Am_Vm_V0d_An = 11,

    SLADDER_An_Vm_Am = 12,
    SLADDER_An_Vm_Am_V0d = 13,
    SLADDER_An_Vm_V0d_Am = 14,

    SLADDER_Am_An_Vm = 15,
    SLADDER_An_Am_Vm = 18,
};


struct VelSgParams{
    double S;
    double V_0;
    double V_d;
    double V_m;
    double A_m;
    double A_n;
    double Sparam;
    int vtype;
    int nseg;
};


enum VelocityMode:unsigned char
{
    VelocityMode_constant = 0,                             //!< 恒速模式
    VelocityMode_t,                                        //!< t速度模式
    VelocityMode_s,                                        //!< s速度模式
    VelocityMode_s_j,                                      //!< s_j速度模式
    VelocityMode_s_ttt,                                    //!< s_ttt速度模式
    VelocityMode_s_tr,                                      //!< s_tr速度模式
    VelocityMode_trp,                                       //!< t速度模式
    VelocityMode_misc,                                      //!< 混合速度模式
    VelocityMode_slice,                                     //!< 切片速度模式
    VelocityMode_constime,                                  //!< 定时恒速模式
    VelocityMode_num,
};



inline SLadderSegmentType TrapzNseg2Scurve(LadderSegmentType seg, double tr, double tr2)
{
    bool has_plain_acc = tr<1.0;
    bool has_plain_dec = tr<1.0;
    switch (seg)
    {
    case LADDER_0_SEGMENT:
        return SLADDER_0_SEGMENT;
    case LADDER_10_SEGMENT:
        return SLADDER_10_SEGMENT;
    case LADDER_11_SEGMENT:
        if (has_plain_acc) return SLADDER_31_SEGMENT;
        else return SLADDER_21_SEGMENT;
    case LADDER_12_SEGMENT:
        if (has_plain_dec) return SLADDER_32_SEGMENT;
        else return SLADDER_22_SEGMENT;
    case LADDER_20_SEGMENT:
        if (has_plain_acc && has_plain_dec) return SLADDER_60_SEGMENT;
        else if (!has_plain_acc && has_plain_dec) return SLADDER_51_SEGMENT;
        else if (has_plain_acc && !has_plain_dec) return SLADDER_52_SEGMENT;
        else return SLADDER_40_SEGMENT;
    case LADDER_21_SEGMENT:
        return SLADDER_41_SEGMENT;
    case LADDER_22_SEGMENT:
        return SLADDER_42_SEGMENT;
    case LADDER_24_SEGMENT:
        return SLADDER_43_SEGMENT;
        if (has_plain_acc ) return SLADDER_43_SEGMENT;
        else return SLADDER_37_SEGMENT;
    case LADDER_30_SEGMENT:
        if (has_plain_acc && has_plain_dec) return SLADDER_70_SEGMENT;
        else if (!has_plain_acc && has_plain_dec) return SLADDER_61_SEGMENT;
        else if (has_plain_acc && !has_plain_dec) return SLADDER_62_SEGMENT;
        else return SLADDER_50_SEGMENT;
    case LADDER_31_SEGMENT:
        return SLADDER_71_SEGMENT;
        if (has_plain_acc && has_plain_dec) return SLADDER_71_SEGMENT;
        else if (!has_plain_acc && has_plain_dec) return SLADDER_64_SEGMENT;
        else if (has_plain_acc && !has_plain_dec) return SLADDER_65_SEGMENT;
        else return SLADDER_53_SEGMENT;
    case LADDER_ERR_SEGMENT:
        return SLADDER_ERR_SEGMENT;
    default:
        return SLADDER_70_SEGMENT;
    }
};

};

#endif