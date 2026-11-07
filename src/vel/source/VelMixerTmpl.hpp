#pragma once

#include "vel/VelWrapCommon.h"
#include "vel_enum_def.h"

namespace zmc
{
  
template<typename P, typename I, typename F>
class VelMixerTmpl:public P{
public:
    int build(){
        // static_assert(std::is_base_of<VelForwarderBase, T>::value, "Type T should be Herit of VelForwarderBase");
        // static_assert(std::is_base_of<VelFunctorBase, Func>::value, "Type T should be Herit of VelFunctorBase");
        return F()((P&) *this, fw);
    }
    int get_slice(double time, double* pos, double* vel, double* acc)const{
        return fw.get_slice(time, pos, vel, acc);
    };
    I fw;
};

template<typename P, typename I, typename F>
class VelMixer2Tmpl{
protected:
    void* pImpl;
public:
    VelMixer2Tmpl(){
        pImpl = new I();
    }
    ~VelMixer2Tmpl(){
        delete (I*)pImpl; 
        pImpl = nullptr;
    }
    VelMixer2Tmpl(const VelMixer2Tmpl& other){
        pImpl = new I();
        *((I*)pImpl) = *((I*)other.pImpl);
    }
    VelMixer2Tmpl& operator=(const VelMixer2Tmpl& other){
        if (this == &other) return *this;
        // delete (I*)pImpl;
        // pImpl = new I();
        *((I*)pImpl) = *((I*)other.pImpl);
        return *this;
    }
    inline void set_inversable(bool v) {
        param.set_inversable(v);
    }
    inline void set_policy_mask(bool val, uint32_t mask){
        param.set_policy_mask(val, mask);
    }
    inline void set_scurve(bool v) {
        param.set_scurve(v);
    }
    inline void set_continuous_vel(bool v) {
        param.set_continuous_vel(v);
    }
    void set_config(double vm, double acc, double dec, double jerk) {
       return param.set_config(vm, acc, dec, jerk);
    }
    void set_begin_pos_vel(double pos, double vel) {
        return param.set_begin_pos_vel(pos, vel);
    }
    void set_end_pos_vel(double pos, double vel) {
        return param.set_end_pos_vel(pos, vel);
    }

    void set_begin_pva(double s0, double v0, double a0 = 0) {
        return param.set_begin_pva(s0, v0, a0);
    }
    void set_end_pva(double sd, double vd, double ad = 0) {
        return param.set_end_pva(sd, vd, ad);
    }
    void set_begin_pos(double pos) {
        return param.set_begin_pos(pos);
    }
    void set_begin_vel(double vel) {
        return param.set_begin_vel(vel);
    }
    void set_end_pos(double pos) {
        return param.set_end_pos(pos);
    }
    void set_end_vel(double vel) {
        return param.set_end_vel(vel);
    }
    double get_end_pos() const {
        return param.get_end_pos();
    }
    double get_end_vel() const {
        return param.get_end_vel();
    }
    double get_delta_pos() const {
        return param.get_delta_pos();
    }
    double get_max_vel() const {
        return param.get_max_vel();
    }
    int build(){
        return F()(param, *(I*)pImpl);
    }
    double get_end_time() const {
        return static_cast<const I*>(pImpl)->get_end_time();
    }
    PosVelAcc get_slice(double tm) const{
        PosVelAcc pva;
        this->get_slice(tm, pva);
        return pva;
    }
    inline 
    int get_slice(double tm, PosVelAcc& pva) const {
        return static_cast<const I*>(pImpl)->get_slice(tm, pva);
    }
    int get_slice(double tm, double* pos, double* vel, double* acc) const {
        PosVelAcc pva;
        int ret =this->get_slice(tm, pva);
        *pos = pva.pos;
        *vel = pva.vel;
        *acc = pva.acc;
        return ret;
    }
    int get_slice_continuous(double tm, PosVelAcc& pva) const {
        return static_cast<const I*>(pImpl)->get_slice_continuous(tm, pva);
    }
    void shift_time(double dt){
        static_cast<I*>(pImpl)->shift_time(dt);
    }
    void shift_pos(double pos){
        static_cast<I*>(pImpl)->shift_pos(pos);
    }
    double cal_reachable_backward_vel(double target_vel) const {
        return param.cal_reachable_backward_vel(target_vel);
    }
    double cal_reachable_forward_vel(double current_vel) const {
        return param.cal_reachable_forward_vel(current_vel);
    }
    double cal_stop_pos() const {
        return param.cal_stop_pos();
    }

    void chop_left_time(double tm){
        PosVelAcc pva;
        static_cast<const I*>(pImpl)->get_slice(tm, pva);
        param.S_0 = pva.pos;
        param.V_0 = pva.vel;
        param.A_0 = pva.acc;
    } 
    P param;
};

template<typename P, typename I, typename F>
class VelMixer3Tmpl: public I{
public:
    int build(const P& par){
        return F()(par, *(I*)this);
    }
};
  
} // namespace zmc