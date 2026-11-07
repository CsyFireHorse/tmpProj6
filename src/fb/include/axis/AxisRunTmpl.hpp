#ifndef _ZMC_AXIS_RUN_TMPL_HPP_
#define _ZMC_AXIS_RUN_TMPL_HPP_

// #include "AxisRef.h"
#include "fbt_axis.h"
#include "fbt_axis_base.h"
#include "stdio.h"
#include "fbt_axis_virtual.h"
#include <vector>
#include <utility>  
#include <tuple>
#include <type_traits>
#include "impl/fbt_fb_impl.h"
#include "AxVerboseFlag.h"
#include "fbt_axis_impl.h"

namespace zmc {



template <class T>
class AxisRunableTmpl {
   public:
    virtual void preRun(){
        // printVerboseHeader((AxVerboseFlag)verbose_flag);
    }
    void fbRunCycle(){}
    virtual void preRunCycle() {
        fbRunCycle();
    }
    virtual void postRunCycle() {
        // printAxis();
    }
    void axisRunCycle(){
        // auto sz = axArr.size();
        // for (int i = 0; i < sz; i++) {
        //     axArr[i].runCycle();
        // }
    }
    void runCycle() {
        preRunCycle();
        static_cast<T*>(this)->axisRunCycle();
        postRunCycle();
        idx++;
        _try_exit();
    }
    void run() {
        preRun();
        static_cast<T*>(this)->axisRunCycle();
        postRunCycle();
        while (!_exitFlag) {
            runCycle();
        }
    }
    template<typename F>
    void run(F&& f){
        preRun();
        static_cast<T*>(this)->axisRunCycle();
        while (!_exitFlag) {
            std::forward<F>(f)();            
            runCycle();
        }
    }
    // template<typename F, typename F2>
    // void run(F&& f, F2&& f2){
    //     preRun();
    //     static_cast<T*>(this)->axisRunCycle();
    //     while (!_exitFlag) {
    //         std::forward<F>(f)();            
    //         runCycle();
    //         std::forward<F2>(f2)();
    //     }
    // }
    void _try_exit() {
        if (count_to_exit && idx > count_to_exit) _exitFlag = true;
    }
    inline int getIndex()const{
        return idx;
    }
   public:
    bool _exitFlag = false;
    uint8_t verbose_flag = 0;
    uint16_t verbose_modulo = 1;
    int count_to_exit = 100;
    uint32_t idx = 0;
};


// reloadConfig
template <class T=AxisCia402Fak, int SZ=1>
class AxisRunTmpl:public AxisRunableTmpl<AxisRunTmpl<T,SZ>> {
   public:
    AxisRunTmpl(int num=SZ):axArr(num){
        axArr.resize(num);
    }
    void reload_config(int i) {
        axArr[i].reload_config();
    }
    inline uint32_t axis_num() const { return axArr.size(); }
    AXIS_REF axisRef(int i) { return  (AXIS_REF) &axArr[i]; }
    AxisImplBase& axRef(int i) { return (AxisImplBase&)axArr[i]; }
    const AxisImplBase& axRef(int i) const { return (const AxisImplBase&)axArr[i]; }
    AxisCiaConfig& get_config_ref(int i) { return axArr[i].get_config_ref(); }
    void installLogger(int idx, AxLoggerType type, const std::string& filename = "") {
        reinterpret_cast<AxisImpl*>(&axArr[idx])->installLogger(type, filename);
    }
    virtual void preRun(){
        printVerboseHeader((AxVerboseFlag) this->verbose_flag);
    }
    virtual void postRunCycle() {
        printAxis();
    }
    void printAxis()const{
        if (this->verbose_flag != 0 && (this->idx % this->verbose_modulo == 0)) {
            auto sz = axArr.size();
            for (int i = 0; i < sz; i++) {
                AxisImplBase& ax = (AxisImplBase&)axArr[i];
                printAxisVerbose((AxVerboseFlag)this->verbose_flag, ax);
            }
        }
    }

    void axisRunCycle(){
        auto sz = axArr.size();
        for (int i = 0; i < sz; i++) {
            axArr[i].runCycle();
        }
    }
    void powerOn(int idx) {
        auto& ax = (AxisImplBase&)axArr[idx];
        ax.bDriveStart = true;
        ax.bRegulatorOn = true;
        if (ax.axisStat == mcDisabled)
        ax.axisStat = mcStandstill;
    }
   public:
    std::vector<T> axArr;
};


template<typename T, typename T2, int SZ=1>
struct AxDbFbTmpl:public AxisRunTmpl<AxisCia402Fak, SZ>{
    T mov;
    T2 mov2;
    void bindAxis(int axIdx){
        mov.Axis = this->axisRef(axIdx);
        mov2.Axis = this->axisRef(axIdx);
    }
    void fbRunCycle(){
        mov.runCycle();
        mov2.runCycle();
    }
    virtual void preRunCycle()override{
        fbRunCycle();
    }
    void printAxisFb()const{
        auto& ax = this->axRef(0);
        printf("%d,%f,%f,%f,%f,0x%x,0x%x\n", ax.m_cycleCounter, ax.get_current_time(), ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration, get_fb_impl_stat(this->mov), get_fb_impl_stat(this->mov2)); // mov.get_fb_stat()
    }
};


// T=AxisCia402Fak
// AxisMotEmu
template<int SZ=1>
struct AxMotRunTmpl:public AxisRunTmpl<AxisMotEmu, SZ> {
    // int count_to_exit = 100;
    int check(int axIdx, double pos, double vel, double acc){
        return this->axArr[axIdx].check(pos, vel, acc);
    }
    int check_set_pva(int axIdx){
        return this->axArr[axIdx].check_set_pva();
    }
};

template<typename T, int SZ=1>
struct AxSgMotTmpl:public AxMotRunTmpl<SZ>{
    T mov;
    void bindAxis(int axIdx){
        mov.Axis = this->axisRef(axIdx);
    }
    void fbRunCycle(){
        mov.runCycle();
    }
    virtual void preRunCycle()override{
        fbRunCycle();
    }
};

template<typename T, typename T2, int SZ=1>
struct AxDbMotTmpl:public AxMotRunTmpl<SZ>{
    T mov;
    T2 mov2;
    void bindAxis(int axIdx){
        mov.Axis = this->axisRef(axIdx);
        mov2.Axis = this->axisRef(axIdx);
    }
    void fbRunCycle(){
        mov.runCycle();
        mov2.runCycle();
    }
    virtual void preRunCycle()override{
        fbRunCycle();
    }
    void printAxisFb()const{
        auto& ax = this->axRef(0);
        printf("%d,%f,%f,%f,%f,0x%x,0x%x\n", ax.m_cycleCounter, ax.get_current_time(), ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration, get_fb_impl_stat(this->mov), get_fb_impl_stat(this->mov2)); // mov.get_fb_stat()
    }
};

template<typename T, typename T2, typename T3, int SZ=1>
struct AxThrMotTmpl:public AxMotRunTmpl<SZ>{
    T mov;
    T2 mov2;
    T3 mov3;
    void bindAxis(int axIdx){
        mov.Axis = this->axisRef(axIdx);
        mov2.Axis = this->axisRef(axIdx);
        mov3.Axis = this->axisRef(axIdx);
    }
    void fbRunCycle(){
        mov.runCycle();
        mov2.runCycle();
        mov3.runCycle();
    }
    virtual void preRunCycle()override{
        fbRunCycle();
    }
    void printAxisFb()const{
        auto& ax = this->axRef(0);
        printf("%d,%f,%f,%f,%f,0x%x,0x%x\n", ax.m_cycleCounter, ax.get_current_time(), ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration, get_fb_impl_stat(this->mov), get_fb_impl_stat(this->mov2)); // mov.get_fb_stat()
    }
};


template<typename T, typename T2, typename T3, typename T4, int SZ=1>
struct AxFourMotTmpl:public AxMotRunTmpl<SZ>{
    T mov;
    T2 mov2;
    T3 mov3;
    T4 mov4;
    void bindAxis(int axIdx){
        mov.Axis = this->axisRef(axIdx);
        mov2.Axis = this->axisRef(axIdx);
        mov3.Axis = this->axisRef(axIdx);
        mov4.Axis = this->axisRef(axIdx);
    }
    void fbRunCycle(){
        mov.runCycle();
        mov2.runCycle();
        mov3.runCycle();
        mov4.runCycle();
    }
    virtual void preRunCycle()override{
        fbRunCycle();
    }
    void printAxisFb()const{
        auto& ax = this->axRef(0);
        printf("%d,%f,%f,%f,%f,0x%x,0x%x,0x%x,0x%x\n", ax.m_cycleCounter, ax.get_current_time(), ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration, get_fb_impl_stat(this->mov), get_fb_impl_stat(this->mov2), get_fb_impl_stat(this->mov3), get_fb_impl_stat(this->mov4)); // mov.get_fb_stat()
    }
};


template<typename T, int N, int SZ=1>
struct AxMotArrTmpl:public AxMotRunTmpl<SZ>{
    T movArr[N];
    void bindAxis(int axIdx){
        for (int i = 0; i < N; i++) {
            movArr[i].Axis = this->axisRef(axIdx);
        }
    }
    void fbRunCycle(){
        for (int i = 0; i < N; i++) {
            movArr[i].runCycle();
        }
    }
    virtual void preRunCycle()override{
        fbRunCycle();
    }
};

}  // namespace zmc
#endif