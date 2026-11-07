#ifndef _VEL_FORWARD_COMMON_H_
#define _VEL_FORWARD_COMMON_H_

#include "VelBase.h"
// #include "zal_common.h"
#include "math.h"
#include "scurve_base.h"
#include "base/include/solver.h"

namespace zmc{

enum VelCheckErr{
    VEL_CHECK_ERR_NONE = 0,
    VEL_CHECK_ERR_NSEG =1,
    VEL_CHECK_ERR_BEGIN = 2,
    VEL_CHECK_ERR_END = (1<<2),
    VEL_CHECK_ERR_LIMIT =  (1<<3),
    VEL_CHECK_ERR_TIME =  (1<<4),
    VEL_CHECK_ERR_POS_SEG =  (1<<5),
    VEL_CHECK_ERR_VEL_SEG =  (1<<6),     
    VEL_CHECK_ERR_ACC_SEG =  (1<<7),
};

template<unsigned int SZ=3>
class VelForwarderCommon:public VelForwarderBase{
public:
    enum { NUM_LADDER_SEGMENT = SZ };
    int get_plan(double* total_time, double* rVmax, double* rAcc=nullptr, double* rDec=nullptr)const{
        if (total_time) *total_time = get_end_time();
        if (rVmax) *rVmax = vm;
        return 0;
    };
    inline double get_begin_time()const { return cTarr[0];}
    inline double get_end_time()const { return cTarr[NUM_LADDER_SEGMENT];}
    inline double get_begin_distance()const { return cSarr[0];}
    inline double get_rmax_vel() const { return vm; }
    void _cumsum(){
        cTarr[0] = Tarr[0];
        for (int i=0; i<SZ; i++)
            cTarr[i+1] = cTarr[i] + Tarr[i+1];
        cSarr[0] = Sarr[0];
        for (int i=0; i<SZ; i++)
            cSarr[i+1] = cSarr[i] + Sarr[i+1];
    }
    void shift_pos(double ofs){
        for (unsigned int i=0; i<NUM_LADDER_SEGMENT + 1; i++)
            cSarr[i] += ofs;
    }
    void shift_time(double ofs){
        for (unsigned int i=0; i<NUM_LADDER_SEGMENT + 1; i++)
            cTarr[i] += ofs;
    }
    // double         _S;
    double         vm;
    double         am;
    double         an;
    // double         _vn;
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

    friend class FuncVelTrapz;

    // template<bool IsNext=false>
    // inline double next_vel()const{
    //     return pPar->V_0;
    // }
    // template<bool IsNext=false>
    // inline double next_vel_diff()const{
    //     return vm - pPar->V_0;
    // }
};

// template<unsigned int SZ>
// class VelForwarderCommon{
//     template<bool IsNext>
//     inline double VelForwarderCommon<SZ>::next_vel<true>()const{
//         return vm;
//     };
//     template<bool IsNext>
//     inline double VelForwarderCommon<SZ>::next_vel_diff<true>()const{
//         return vm -  pPar->V_d;
//     };
// };

struct Vel_buildTimeSegment{
    template<typename T>
    inline void operator ()(T& obj){
        obj._buildTimeSegment();
    }
};

struct Vel_buildPosVecSegment{
    template<typename T>
    inline void operator ()(T& obj){
        obj._buildPosVecSegment();
    }
};
struct Vel_get_slice{
    template<typename T>
    inline int operator ()(const T& obj, double time, double* pos, double* vel, double* acc){
        return obj.get_slice(time,  pos, vel, acc);
    }
};
struct Vel_get_distance{
    template<typename T>
    inline double operator ()(const T& obj, double time){
        return obj.get_distance(time);
    }
};
struct Vel_get_rate{
    template<typename T>
    inline double operator ()(const T& obj, double time){
        return obj.get_rate(time);
    }
};
struct Vel_get_acc{
    template<typename T>
    inline double operator ()(const T& obj, double time){
        return obj.get_acc(time);
    }
};


template<typename T>
int vel_check(const T& vp){
    // static_assert(std::is_base_of<VelForwarderBase, T>::value, "Type T should be Herit of VelForwarderBase");
    enum { NUM_LADDER_SEGMENT = T::NUM_LADDER_SEGMENT};
    int errCode = 0;
    const double DT = 1e-7;
    const double TOL = 1e-6;
    const auto& cSarr = vp.cSarr;
    const auto& cTarr = vp.cTarr;
    const auto& Varr = vp.Varr;
    if (vp.nseg <= LADDER_0_SEGMENT)
        return VEL_CHECK_ERR_NSEG;
    const double am = vp.pPar->A_m;
    const double an = vp.pPar->A_n;
    if (fabs(vp.cSarr[0] - vp.pPar->S_0) > TOL){
        errCode |= VEL_CHECK_ERR_BEGIN;
    }
    if (fabs(Varr[0] - vp.pPar->V_0) > TOL){
        errCode |= VEL_CHECK_ERR_BEGIN; 
    }
    if (fabs(vp.cSarr[NUM_LADDER_SEGMENT] - vp.pPar->S_d) > TOL){
        errCode |= VEL_CHECK_ERR_END;
    }
    if (0 && fabs(Varr[NUM_LADDER_SEGMENT] - vp.pPar->V_d) > TOL){
        errCode |= VEL_CHECK_ERR_END; 
    }
    for (int i=0;i<NUM_LADDER_SEGMENT;i++){
        if (cTarr[i] > cTarr[i+1] + DT)
            errCode |= VEL_CHECK_ERR_TIME; 
    }
    for (int i=0; i<NUM_LADDER_SEGMENT + 1; i++){
        const double tt[] = {
            cTarr[i] - DT,
            cTarr[i],
            cTarr[i] + DT};
        const double ss[] = {
            vp.get_distance(tt[0]),
            vp.get_distance(tt[1]),
            vp.get_distance(tt[2])};
        const double vv[] = {
            vp.get_rate(tt[0]),
            vp.get_rate(tt[1]),
            vp.get_rate(tt[2])};
        const double aa[] = {
            vp.get_acc(tt[0]),
            vp.get_acc(tt[1]),
            vp.get_acc(tt[2])};

        if (fabs(ss[1] - cSarr[i]) > TOL)
            errCode |= VEL_CHECK_ERR_POS_SEG;
        if (fabs(vv[1] - Varr[i]) > TOL)
            errCode |= VEL_CHECK_ERR_VEL_SEG;
        if (i>0 && tt[1] != cTarr[0]){
            // judge difference  in decel direction
            if (fabs(ss[0] - ss[1]) > TOL)
                errCode |= VEL_CHECK_ERR_POS_SEG;
            if (fabs(vv[0] - vv[1]) > TOL)
                errCode |= VEL_CHECK_ERR_VEL_SEG;
            if (aa[0] > am + TOL || aa[0] < -an - TOL)
                errCode |= VEL_CHECK_ERR_ACC_SEG;
            if (aa[1] > am + TOL || aa[1] < -an - TOL)
                errCode |= VEL_CHECK_ERR_ACC_SEG;
        }
        if (i<NUM_LADDER_SEGMENT && tt[1] != cTarr[NUM_LADDER_SEGMENT]){
            // judge difference  in acceleration direction
            if (abs(ss[2] - ss[1]) > TOL)
                errCode |= VEL_CHECK_ERR_POS_SEG;
            if (abs(vv[2] - vv[1]) > TOL)
                errCode |= VEL_CHECK_ERR_VEL_SEG;
            if (aa[2] > am + TOL || aa[2] < -an - TOL)
                errCode |= VEL_CHECK_ERR_ACC_SEG;
            if (aa[1] > am + TOL || aa[1] < -an - TOL)
                errCode |= VEL_CHECK_ERR_ACC_SEG;
        }
        // FIXME
        // if (abs(get_rate(tt[2]) - get_rate(tt[0]) - 2* DT * get_acc(tt[1])) > TOL)
        //     errCode |= (1<<6);
        // if (abs(get_distance(tt[2]) - get_distance(tt[0]) - 2* DT * get_rate(tt[1]) - 2*DT*DT*get_acc(tt[1])) > TOL)
        //     errCode |= (1<<4);
    }
    return errCode;
}

struct Vel_check{
    template<typename T>
    inline int operator ()(const T& obj){
        return vel_check(obj);
    }
};
} /* namespace zmc */
#endif
