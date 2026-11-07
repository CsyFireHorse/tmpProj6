#ifndef _POU_BASE_HPP_
#define _POU_BASE_HPP_
#include "stdint.h"
#include <vector>

namespace zmc {

class PouTaskBase {
   public:
    virtual void runCycle() {}
    virtual void postRun() {}
};

class RunnerBase {
   public:
    void run(int count = 0) {
        preRun();
        if (count) count_to_exit = count;
        while (!_exitFlag) {
            run_once();
        }
        postRun();
    }
    void run_once() {
        if (idx > 0) {
            preRunCycle();
            if (skipFlag) return;
            runCycle();
        }
        postRunCycle();
        idx++;
        if (count_to_exit && idx > count_to_exit) _exitFlag = true;
    }
    inline int getIndex() const { return idx; }
    // use for print verbose header
    virtual void preRun() {}
    virtual void preRunCycle() {};
    // use for task.runCycle
    virtual void runCycle() {}
    // use for axisArr.runCycle
    virtual void postRunCycle() {};
    // use for task.postRun
    virtual void postRun() {};

   public:
    void* pImpl;
    unsigned int idx = 0;
    bool _exitFlag = false;
    uint8_t verbose_flag = 0;
    uint16_t verbose_modulo = 1;
    int count_to_exit = 0;
    bool skipFlag = false;
};
}  // namespace zmc
#endif