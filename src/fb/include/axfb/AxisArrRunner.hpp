#ifndef _AXIS_ARR_RUNNER_HPP_
#define _AXIS_ARR_RUNNER_HPP_

#include "AxisRef.h"
#include <vector>
#include "AxVerboseFlag.h"
#include "common/misc/pou_base.hpp"

namespace zmc {

class AxisImplBase;
class AxisCia402Fak;
class AxisImplBase;

template <class T=AxisCia402Fak, int SZ=1>
class AxisArrRunner:public RunnerBase{
public:
    AxisArrRunner(int axisNum=SZ):axArr(axisNum){}
    void reload_config(int i) {
        axArr[i].reload_config();
    }
    inline uint32_t axis_num() const { return axArr.size(); }
    AxisImplBase& axRef(int i) { return (AxisImplBase&)axArr[i]; }
    const AxisImplBase& axRef(int i) const { return (const AxisImplBase&)axArr[i]; }
    AXIS_REF axisRef(int i) { return  (AXIS_REF) &axArr[i]; }
    AXIS_REF axisRef(int i) const { return  (const AXIS_REF) &axArr[i]; }
    void runCycle() override {
        for (auto& task : taskArr) task->runCycle();
    }
    void postRunCycle () override{
        axRunCycle();
        printAxis();
    }
    void postRun() override{
        for (auto& task : taskArr) task->postRun();
    }
    void printAxis()const{
        if (this->verbose_flag != 0 && (this->idx % this->verbose_modulo == 0)) {
            for (auto& ax: this->axArr) {
                printAxisVerbose((AxVerboseFlag)this->verbose_flag, (AxisImplBase&) ax);
            }
        }
    }
    void axRunCycle(){
        for (auto& axis : axArr) {
            axis.runCycle();
        }
    };
    void bind(int i, PouTaskBase* pObj) { taskArr.push_back(pObj); }
    template<typename TP>
    void bind2(int i, TP& pou){
        pou.axis = &axArr[i];
        bind(i, &pou);
    }

    std::vector<T> axArr;
    std::vector<PouTaskBase*> taskArr;
};

template<typename T>
using SgAxisRunner=AxisArrRunner<T, 1>;

// template<typename T, typename R, typename P>
// void runBindTask(R& runner, const P& param){
//     T pou(param);
//     runner.bind2(0, pou);
//     runner.run();
// }

// template<typename T, typename R, typename P>
// void runBindTaskParam(R& runner,const P& param, int count=0){
//     T pou(param);
//     runner.bind2(0, pou);
//     runner.run(count);
// }

// template<typename T, typename R, typename P>
// void AxSgRunTmpl(const P& param){
//     R runner(param);
//     // runBindTaskParam<T>(runner, param);
//     T pou(param);
//     runner.bind2(0, pou);
//     runner.run();
// }

}

#endif