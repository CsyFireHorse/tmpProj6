#ifndef _VEL_FORWARD_TRAPZ_H_
#define _VEL_FORWARD_TRAPZ_H_

#include "VelBase.h"
// #include "zal_common.h"
#include "math.h"
#include "scurve_base.h"
#include "base/include/solver.h"
#include "vel_forward_common.h"
#include "common/misc/polyval.h"
#include "vel_par_trapz.h"

namespace zmc{

using VelForwarderTTmpl = VelForwarderCommon<3>;

template<bool IsNext=false>
inline double next_vel(const VelForwarderTTmpl& vel){
    return vel.pPar->V_0;
};
template<>
inline double next_vel<true>(const VelForwarderTTmpl& vel){
    return vel.vm;
};

template<bool IsNext=false>
inline double next_vel_diff(const VelForwarderTTmpl& vel){
    return vel.vm - vel.pPar->V_0;
};
template<>
inline double next_vel_diff<true>(const VelForwarderTTmpl& vel){
    return vel.pPar->V_d - vel.vm;
};

struct TimeVelPos{
    double time;
    double vel;
    double pos;
};

template<bool IsNext=false>
struct Noper: public VelForwarderTTmpl{
    enum { IsValid = false};
    inline double get_distance(double dt)const {
        return 0;
    }
    inline double get_rate(double dt)const {
        return next_vel<IsNext>(*this);
    }
    inline double get_acc(double dt)const {
        return 0;
    }
    inline TimeVelPos build_TimeSegment(){
        return TimeVelPos{0, next_vel<IsNext>(*this), 0};
    }
};

/*   
  vel
         _
        / 
       /  
    0_/                                            
*/
template<bool IsNext=false>
struct Accer: public VelForwarderTTmpl{
    enum { IsValid = true};
    // enum { IsNext = true};
    inline double get_distance(double dt)const {
        return polyval2(dt, am/2, next_vel<IsNext>(*this));
    }
    inline double get_rate(double dt)const {
        return next_vel<IsNext>(*this) + am * dt;
    }
    inline double get_acc(double dt)const {
        return am;
    }
    inline TimeVelPos build_TimeSegment(){
        double dt = next_vel_diff<IsNext>(*this) / am;
        return TimeVelPos{dt, next_vel<IsNext>(*this) + am * dt, polyval2(dt, am/2, next_vel<IsNext>(*this))};
    }
};

/*   
  vel
    _
     \
      \
       \_ 0                                         
*/
template<bool IsNext=true>
struct Decer: public VelForwarderTTmpl{
    enum { IsValid = true};
    inline double get_distance(double dt)const {
        return polyval2(dt, -an/2, next_vel<IsNext>(*this));
    }
    inline double get_rate(double dt)const {
        return next_vel<IsNext>(*this) - an * dt;
    }
    inline double get_acc(double dt)const {
        return -an;
    }
    inline TimeVelPos build_TimeSegment()const{
        double dt = - next_vel_diff<IsNext>(*this) / an;
        double v0 = next_vel<IsNext>(*this);
        // double dt = (vm -  pPar->V_d) / an;
        // double dt = (pPar->V_0 - vm) / an;
        return TimeVelPos{dt, v0 - an * dt, polyval2(dt, -an/2, v0)};
    }
};

// VelForwarderTrapzTmpl

/*   
  vel
         ___________
        /           \
       /             \
     _/               \_                                           
*/
template<typename R, typename T, bool HasMean>
class VelForwarderTrapzMixer:public VelForwarderTTmpl{
public:
    template<typename TT>
    inline TT& forwarder(){
        return *((TT*) this);
    }
    template<typename TT>
    inline const TT& forwarder()const{
        return *((const TT*) this);
    }
    int get_slice(double slice, double *position, double *velocity, double *acc) const{
        int ret = E_VEL_UNCOVERED;
        if (slice < get_begin_time()) slice = 0;
        else if (slice >= get_end_time()) {
            // slice = get_end_time() + 1e-6;
            ret = E_VEL_COVERED;
        }
        if (position) *position = get_distance(slice);
        if (velocity) *velocity = get_rate(slice);
        if (acc) *acc = get_acc(slice);
        return ret;
    };
    double get_distance(double tm) const{
        double sc, dt;
        if (tm <= cTarr[0]){
            sc = cSarr[0];
        }else if (tm <= cTarr[1]){
            dt = tm - cTarr[0];
            sc = cSarr[0] + forwarder<R>().get_distance(dt);
        }else if (HasMean && tm <= cTarr[2]){
            dt = tm - cTarr[1];
            sc = cSarr[1] + this->vm * dt;
        }else if (tm <= cTarr[3]){
            dt = tm - cTarr[2];
            sc = cSarr[2] + forwarder<T>().get_distance(dt);
        }else{
            if (pPar->is_continuous_vel())
                sc = cSarr[3] + (tm - cTarr[3]) * this->Varr[3];
            else
                sc = cSarr[3];
        }
        return sc;
    }
    double get_rate(double tm) const{
        double vc;
        if (tm < cTarr[0])
            vc = Varr[0];
        else if  (tm <= cTarr[1]){
            vc = forwarder<R>().get_rate(tm- cTarr[0]);
        }else if (HasMean && tm <= cTarr[2])
            vc = this->vm;
        else if (tm <= cTarr[3])
            vc = forwarder<T>().get_rate(tm- cTarr[2]);
        else
            vc = this->Varr[3];
        return vc;
    }

    double get_acc(double tm) const{
        double ac;
        if (tm < cTarr[0]){
            ac = 0;
        }else if (tm <= cTarr[1])
            ac = forwarder<R>().get_acc(tm- cTarr[0]);
        else if (HasMean && tm <= cTarr[2])
            ac = 0;
        else if (tm <= cTarr[3])
            ac = forwarder<T>().get_acc(tm- cTarr[2]);
        else
            ac = 0;
        return ac;
    }

    void _buildTimeSegment(){ 
        Tarr[0] = 0;
        TimeVelPos st = forwarder<R>().build_TimeSegment();  // (vm - pPar->V_0) / am;
        Tarr[1] = st.time;
        TimeVelPos st3 = forwarder<T>().build_TimeSegment();  // (vm - pPar->V_d) / an;
        Tarr[3] = st3.time;

        double& V_0 = this->pPar->V_0;
        Varr[0] = this->pPar->V_0;
        Varr[1] = st.vel;
        Varr[2] = vm;
        Varr[3] = st3.vel;

        Sarr[0] = this->pPar->S_0; 
        Sarr[1] = st.pos;
        Sarr[3] = st3.pos;
        if (HasMean)
            Tarr[2] = (this->pPar->get_S() - Sarr[1] - Sarr[3]) / vm;
        else 
            Tarr[2] = 0;
        Sarr[2] = vm * Tarr[2];
    }
    void _buildPosVecSegment(){}
};

using VelForwarderTrapzT30=VelForwarderTrapzMixer<Accer<false>, Decer<true>, true>;
using VelForwarderTrapzT20=VelForwarderTrapzMixer<Accer<false>, Decer<true>, false>;
using VelForwarderTrapzT31=VelForwarderTrapzMixer<Decer<false>, Decer<true>, true>;
using VelForwarderTrapzT32=VelForwarderTrapzMixer<Accer<false>, Accer<true>, true>;
using VelForwarderTrapzT23=VelForwarderTrapzMixer<Decer<false>, Accer<true>, false>;
using VelForwarderTrapzT10=VelForwarderTrapzMixer<Noper<false>, Noper<true>, true>;
using VelForwarderTrapzT11=VelForwarderTrapzMixer<Accer<false>, Noper<true>, false>;
using VelForwarderTrapzT12=VelForwarderTrapzMixer<Noper<false>, Decer<true>, false>;
using VelForwarderTrapzT34=VelForwarderTrapzMixer<Decer<false>, Accer<true>, true>;


struct VelForwarderTrapzUn:public VelForwarderTrapzT30{
    // int get_slice(double slice, double *position, double *velocity, double *acc=nullptr) const;
    template<typename T>
    T& forwarder(){ return *(T*)(this);}
    template<typename T>
    const T& forwarder()const{ return *(const T*)(this);}
    template<typename F, typename R, typename... T>
    inline R get_val(T&&... args)const{
        switch (get_nseg())
        {
        case LADDER_30_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT30>(), args...);
        case LADDER_20_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT20>(), args...);
        case LADDER_31_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT31>(), args...);
        case LADDER_32_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT32>(), args...);
        case LADDER_34_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT34>(), args...);
        case LADDER_23_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT23>(), args...);
        case LADDER_24_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT31>(), args...);
        case LADDER_25_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT32>(), args...);
        case LADDER_10_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT10>(), args...);
        case LADDER_11_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT11>(), args...);
        case LADDER_12_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT12>(), args...);
        default:
            return F()(forwarder<VelForwarderTrapzT30>(), args...);
        }
    }
    template<typename F, typename R, typename... T>
    inline R get_val(T&&... args){
        switch (get_nseg())
        {
        case LADDER_30_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT30>(), args...);
        case LADDER_20_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT20>(), args...);
        case LADDER_31_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT31>(), args...);
        case LADDER_32_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT32>(), args...);
        case LADDER_34_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT34>(), args...);
        case LADDER_23_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT23>(), args...);
        case LADDER_24_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT31>(), args...);
        case LADDER_25_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT32>(), args...);
        case LADDER_10_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT10>(), args...);
        case LADDER_11_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT11>(), args...);
        case LADDER_12_SEGMENT:
            return F()(forwarder<VelForwarderTrapzT12>(), args...);
        default:
            return F()(forwarder<VelForwarderTrapzT30>(), args...);
        }
    }
public:
    void build(TrapzRelaxParam& rParam, const VelParam& param){
        nseg = rParam.seg_num;
        vm = rParam.vMax;
        am  = rParam.aMax;
        an  = rParam.aMin;
        pPar = (VelParam*) &param;
        // if (nseg == LADDER_23_SEGMENT)
        //     _vn = vm;
        _buildTimeSegment();
        _buildPosVecSegment();
        _cumsum();
    }

    void _buildTimeSegment(){
        return get_val<Vel_buildTimeSegment, void>();
    }

    void _buildPosVecSegment(){
        return get_val<Vel_buildPosVecSegment, void>();
    }

    int get_slice(double time, double* pos, double* vel, double* acc)const{
        return get_val<Vel_get_slice, int>(time, pos, vel, acc);
    }
    double get_distance(double tm)const{
        return get_val<Vel_get_distance, double>(tm);
    }
    double get_rate(double tm)const{
        return get_val<Vel_get_rate, double>(tm);
    }
    double get_acc(double tm)const{
        return get_val<Vel_get_acc, double>(tm);
    }
    inline int check()const{
        return get_val<Vel_check, int>();
    }
};

class FuncVelTrapz:public VelFunctorBase{
public:
    int operator()(const VelParam& param, VelForwarderTrapzUn& fw);
};


inline 
int FuncVelTrapz::operator()(const VelParam& param, VelForwarderTrapzUn& fwUn){
    TrapzParams& par = * (TrapzParams*) &param;
    return velBuild<TrapzRelaxParam>(par, fwUn);
}


class VelTrapz:public VelTmplBase<FuncVelTrapz, VelForwarderTrapzUn>{
};

} /* namespace zmc */
#endif
