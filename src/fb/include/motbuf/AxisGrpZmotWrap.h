#ifndef ZMC_AXIS_GRP_ZMOT_WRAP_H
#define ZMC_AXIS_GRP_ZMOT_WRAP_H

#include <cstddef>

#include "FbMoveList.hpp"
#include "MotCmd.hpp"
#include "axgrp/AxisGrpZmot.h"
#include "axgrp/FbtGroupAdmin.h"
#include "common/container/ringbuffer_easy.hpp"
#include "mc_robot_def.h"

namespace zmc {

class AxisGrpZmotWrap : public AxisGrpZmot {
   public:
    AxisGrpZmotWrap() = default;

    int axNum() const;
    double cmdVel() const;
    double cmdAcc() const;
    double cmdDec() const;
    double cmdJerk() const;
    void setCmdVel(double v);
    void setCmdAcc(double v);
    void setCmdDec(double v);
    void setCmdJerk(double v);
    // Replace the group bindings with resolved axis references.
    int setAxisRefArr(const AXIS_REF* axes, size_t count);

    // cmdQueue wrappers
    void pushCmd(const void* cmd);
    bool cmdQueueEmpty() const;
    bool cmdQueueFrontAndPop(void* cmdOut);

    // fbList wrappers
    void* fbListSentinel();
    void fbListPushBack(void* node);
    void* fbListRemoveNext(void* prev);
    bool fbListEmpty() const;

    // embedded admin FB execution
    void runAdminFbs();
    void setAdminEnable(bool on);
    void setAdminDisable(bool on);
    void setAdminReset(bool on);

    void runCycle();
    void reset_runtime();

   private:
    RingBufferEasy<CommandStructDefault, 64> cmdQueue_;
    FbMoveList fbList_;
    FbtGroupEnable fbEnable_;
    FbtGroupDisable fbDisable_;
    FbtGroupReset fbReset_;
};

}  // namespace zmc

#endif
