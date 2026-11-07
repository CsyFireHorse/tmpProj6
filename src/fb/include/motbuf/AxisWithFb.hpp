/*
 * Axis function-block wrapper declarations.
 * AI modification: GPT-5 Codex
 */
#ifndef _ZMC_AXIS_WITH_FB_HPP
#define _ZMC_AXIS_WITH_FB_HPP

#include "fb_cpp_def.h"

#include "FbMoveList.hpp"
#include "MotCmd.hpp"
#include "MotionCmdTypes.hpp"
#include "common/container/ringbuffer_easy.hpp"

namespace zmc {

class FbMoveWrapWithAxis;

class AxisWithFb {
   public:
    AxisWithFb();
    ~AxisWithFb();
    AXIS_REF ax = nullptr;
    int axisId = 0;
    int axMasterIdx;
    int bufferMode = 1;
    int direction = 0;
    double cmdPos = 10.;
    double cmdVel = 5.;
    double cmdAcc = 20;
    double cmdDec = 20;
    double fastDec = 40;
    double cmdJerk = 0;
    double creep = 1;  // Home creep velocity.

    FbtReadSetValues fbRdSetValues;
    FbtReadActualValues fbRdActualValues;
    FbtReadStatus fbRdStatus;
    FbtReadAxisError fbRdError;
    FbtReadPowerParam fbRdPowerParam;
    FbtReadParameter fbRdParameter;
    FbtWriteParameter fbWrParameter;  // Numeric writes execute through the FB. GPT-6 Codex.
    FbtPower fbPower;
    // A command can run these FBs during command application in the top half.
    bool powerRanInTop_ = false;
    bool overrideRanInTop_ = false;

    FbMoveCmdTmpl<FbtGearOut> fbGearOut;
    FbMoveCmdTmpl<FbtGearIn> fbGearIn;
    FbMoveCmdTmpl<FbtReset> fbRest;
    FbMoveCmdTmpl<FbtStop> fbStp;
    FbMoveCmdTmpl<FbtHalt> fbHlt;
    FbMoveCmdTmpl<FbtHome> fbHome;
    FbMoveCmdTmpl<FbtSetPosition> fbSetPos;
    FbMoveCmdTmpl<FbtChangeRatio> fbChangeRatio;
    FbtSetOverride fbSetOverride;

    FbMoveList fbList;  // Intrusive linked list with sentinel node.
    RingBufferEasy<CommandStructDefault, 64> cmdQueue;

    // Non-owning parent pointer used to access the function-block object pools.
    FbMoveWrapWithAxis* parent_ = nullptr;

    int pushCommand(const CommandStructDefault& cmd) {
        cmdQueue.push(cmd);
        return 0;
    }
    void popCommand() { cmdQueue.pop_front(); }

    void setAxisEnable(bool enable) {
        fbPower.Enable = true;
        fbPower.bDriveStart = enable;
        fbPower.bRegulatorOn = enable;
    }
    // Returns the explicit extended state carried by AxisState::axis_state.
    int getAxisState() const;
    void bindAxis(AXIS_REF axRef);
    int readAxisInfo(AxisInfo& info);

    double getMaxDec() const { return cmdDec > fastDec ? cmdDec : fastDec; }
    FbMoveErasedBase* allocateFb(int cmdType);
    int processCommand(const CommandStructDefault& cmd);
    void printFbListInfo() const;
    void processCmdQue();
    void reset_runtime();

    void runCycleTop();
    void runCycleBottom();
    void runCycle();
    void bindCmd(FbMoveErasedBase& fbWrap, const CommandStructDefault& cmd);
    void bindCmd(CommandMidDefault& cmdMid, const CommandStructDefault& cmd);
};

}  // namespace zmc

#endif
