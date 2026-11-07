#ifndef _ZMC_AXIS_FB_COMPOSE_HPP_
#define _ZMC_AXIS_FB_COMPOSE_HPP_

// #include "AxisRef.h"
#include "fbt_axis.h"
#include "fbt_axis_base.h"
#include "stdio.h"
#include "AxVerboseFlag.h"

namespace zmc {

template <class T, int SZ = 1>
class AxFbComposeTmpl {
   public:
    void reloadConfig(int i) {
        AxisCiaConfig& cfg = axArr[i].get_config_ref();
        axArr[i].set_config(&cfg);
    }
    void preRun(){
        printVerboseHeader((AxVerboseFlag)verbose_flag);
    }
    void preRunCycle() {}
    void postRunCycle() {}
    void runCycle() {
        static_cast<T*>(this)->preRunCycle();
        for (int i = 0; i < SZ; i++) {
            axArr[i].runCycle();
        }
        static_cast<T*>(this)->postRunCycle();
        if (verbose_flag && (idx % verbose_modulo == 0)) {
            for (int i = 0; i < SZ; i++) {
                AxisImplBase& ax = (AxisImplBase&)axArr[i];
                printAxisVerbose((AxVerboseFlag)verbose_flag, ax);
            }
        }
        idx++;
        _try_exit();
    }
    void run() {
        static_cast<T*>(this)->preRun();
        while (!_exitFlag) {
            runCycle();
        }
    }
    void _try_exit() {
        if (count_to_exit && idx > count_to_exit) _exitFlag = true;
    }
    AxisImplBase& axRef(int i) { return  (AxisImplBase&) axArr[i]; }
   public:
    bool _exitFlag = false;
    uint8_t verbose_flag = 0;
    uint16_t verbose_modulo = 1;
    int count_to_exit = 0;
    uint32_t idx = 0;
    AxisCia402Fak axArr[SZ];
};

}  // namespace zmc
#endif