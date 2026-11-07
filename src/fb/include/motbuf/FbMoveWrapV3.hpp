/*
 * @Author: ChenShengyong
 * @Date: 2024-04-16 18:22:45
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2026-07-24 14:25:50
 */
#ifndef _ZMC_FB_MOVE_WRAP3_HPP
#define _ZMC_FB_MOVE_WRAP3_HPP

#include "fb_cpp_def.h"
// #include "fbt_axis_base.h"
#include <cstring>
#include <deque>
#include <queue>

#include "AxPou.hpp"
#include "AxisGroupMotbufRobot.h"
#include "AxisMotbufFb.h"
#include "AxisMotbufZmot.h"
#include "MotCmd.hpp"
#include "MotionCmdTypes.hpp"
#include "common/container/FbCmdQueue.hpp"
#include "common/container/ringbuffer_easy.hpp"
#include "common/misc/pou_base.hpp"
#include "fbmEnum.h"
#include "motbuf/MotionCmdQueueBase.hpp"
#include "stdio.h"
#include "string.h"

struct HeartbeatAck;
namespace zmc {

class FbMoveWrapWithAxis : public PouTaskBase {
   public:
    FbMoveWrapWithAxis(int axNum = 3);
    ~FbMoveWrapWithAxis();
    void setCtx(Ctx* ctx) {
        ctx_ = ctx;
        axisMotbuf.setCtx(ctx);
    }
    Ctx* ctx() { return ctx_; }
    const Ctx* ctx() const { return ctx_; }
    void runCycle() override;
    void _runCycle();
    void reset_runtime();
    int getCnt() const { return cnt; }

    void backupAndSwitchAxis(int val) {
        axSelectBak = axSelect;
        axSelect = val;
        axSelectFlag = 2;
    }

    void tryRestoreAxisSel() {
        if (axSelectFlag == 2) {
            axSelect = axSelectBak;
            axSelectFlag = 0;
        }
    }
    unsigned int cnt = 0;
    bool exitFlag = false;

    // Axis function-block motbuf and its shared object pool.
    AxisMotbufFb axisMotbuf;

    // Command result ring buffer (overwrites oldest when full)
    RingBufferEasy<CmdResult, 128> cmdResultArr;
    RingBufferEasy<CmdResult, 256> cmdConfirmArr;

    int axSelect = 0;
    // 0: use select, 1 will reset axSelectBak , 2 wait next motion command
    int axSelectFlag = 0;
    int axSelectBak = 0;

    void pushCmdResult(const CommandMidDefault& cmdMid, uint8_t fbStatus, void* pFb);
    bool pushCmdConfirm(const CmdResult& confirm);
    void printCmdResultInfo() const;

   private:
    Ctx* ctx_;
};

class FbMoveWrap2 : public FbMoveWrapWithAxis {
   public:
    FbMoveWrap2(int axNum = 3);
    int resize(int axNum);
    void _initCycleTime();
    void show_axis(int i);
    void show_state();
    void show_state(int i);
    void show_power();
    void show_motbuf();
    void show_motnode();
    void show_cmdresult();
    void show_motbuf_sg(int i);
    void show_motnode_sg(int i);
    void show_power_sg(int i);
    void show_mparam();
    void show_home_fb_sg(int i);
    void show();
    void setAxisSelect(int f);
    void setMasterAxis(int f);
    void setPowerOn();
    void setPowerOff();
    void setAxisEnable(bool enable);
    void sendReset();
    void sendResetLow();
    void sendFault();
    void setBufferMode(int v);
    void setDirection(int v);
    void setCmdPos(double v);
    void setCmdVel(double v);
    void setCmdAcc(double v);
    void setCmdDec(double v);
    void setCmdJerk(double v);
    void setFastDec(double v);
    void setCreep(double v);
    void sendExecute(int v);
    void sendExecuteMoveAbs(bool v);
    void sendExecuteMoveRel(bool v);
    void sendExecuteMoveAdd(bool v);
    void sendExecuteMoveVel(bool v);
    void sendAxisLow();
    void sendAxisMotLow();
    void sendStop();
    void sendStopLow();
    void sendHalt();
    void sendHaltLow();
    void sendHome();
    void sendHomeLow();
    void sendHoming();
    void sendUnits(double val);
    void sendSetPos();
    void sendGearIn();
    void sendGearInLow();
    void sendGearOut();
    void sendGearOutLow();
    double getCycleTime() const;
    void setManualMode(bool v);
    void setPauseFlag(bool v);
    void sendSteps(int n);
    void sendTickTo(int n);
    void sendTimeTo(double time);
    void sendDelay(double time);
    void sendExitFlag();
    void preRunCycle();
    void reset_runtime();
    int isManaulMode = 0;
    // bool exitFlag = false;
    bool skipFlag = false;
    int ticksToPause = -1;
    // int moveCmd = 0;
    // int sendStat = 0;
    double cycleTime = 0;
    FbtReadParameter rdPara;

   private:
    AxisWithFb& selAx();
};

class FbMoveWrapWithZmot : public FbMoveWrap2 {
   public:
    FbMoveWrapWithZmot(int axNum = 3) : FbMoveWrap2(axNum) {
        zmotMotbuf.setConfirmTarget(this);
    }
    void setCtx(Ctx* ctx) {
        FbMoveWrapWithAxis::setCtx(ctx);
        zmotMotbuf.setCtx(ctx);
    }
    void runCycle() override;
    void _runCycle();
    void _runCycleTop();
    void _runCycleBottom();
    void reset_runtime();
    AxisMotbufZmot zmotMotbuf;
};

// FbMoveWrap2 with command/status queue support
class FbMoveWrapWithRobot : public FbMoveWrapWithZmot {
   public:
    FbMoveWrapWithRobot(int axNum = 3) : FbMoveWrapWithZmot(axNum) {}
    void runCycle() override;
    void _runCycle();
    void _runCycleTop();
    void _runCycleBottom();
    void reset_runtime();
    AxisGroupMotbufRobot robotMotbuf;
};

// FbMoveWrap2 with command/status queue support
class FbMoveWrap2Queue : public FbMoveWrapWithRobot {
   public:
    FbMoveWrap2Queue(int axNum = 3) : FbMoveWrapWithRobot(axNum) {}

    void runCycle() override;
    void _runCycle();
    void reset_runtime();
    // Access to queues for external use
    CmdQueueType& getCmdQueue() { return cmdpackQueue_; }
    StatusQueueType& getStatusQueue() { return statusQueue_; }

    // Flow control setters
    void setStateSamplePeriod(unsigned int period) { stateSamplePeriod_ = period ? period : 1; }
    // void setMaxPeriod(unsigned int period) { maxPeriod_ = period ? period : 256; }
    // void setLowLatencyThreshold(unsigned int threshold) { lowLatencyThreshold_ = threshold; }
    // void setHighLatencyThreshold(unsigned int threshold) { highLatencyThreshold_ = threshold; }

    // Flow control getters (for monitoring)
    unsigned int getCurrentPeriod() const { return currentPeriod_; }
    unsigned int getLastLatency() const { return lastLatency_; }
    unsigned int getStatusDropCount() const { return statusDropCount_; }

   private:
    // Command and status queues
    CmdQueueType cmdpackQueue_;
    StatusQueueType statusQueue_;
    CmdTargetKind cmdTarget_ = CMD_TARGET_AXIS;
    uint64_t pendingAxisInfoMask_ = 0;

    // Flow control parameters
    unsigned int currentPeriod_ = 512;
    // unsigned int minPeriod_ = 1;
    // unsigned int maxPeriod_ = 1024;           // Max period: send every 256 cycles
    // unsigned int lowLatencyThreshold_ = 2;
    // unsigned int highLatencyThreshold_ = 16;

    // State sampling parameters
    unsigned int stateSamplePeriod_ = 1;  // State sample period (1=every cycle)
    unsigned int stateNextAxis_ = 0;      // Next axis index for batch sending

    static constexpr unsigned int STATE_BATCH_SIZE = 16;  // Keep base states and flow status in one packet.
    static constexpr unsigned int EXTENDED_STATE_ITEMS_PER_PACKET =
        MAX_PAYLOAD_SIZE / (sizeof(AxisState2) + 2 * sizeof(uint16_t));
    static_assert(STATE_BATCH_SIZE * (sizeof(AxisState) + 2 * sizeof(uint16_t)) +
                          sizeof(FlowCtrlStatus) + 2 * sizeof(uint16_t) <=
                      MAX_PAYLOAD_SIZE,
                  "Base axis states and flow status must fit in one status packet");
    static_assert(EXTENDED_STATE_ITEMS_PER_PACKET == 9,
                  "The extended axis packet must fit nine state items");

    // Flow control state
    unsigned int lastAckSeq_ = 0;
    unsigned int lastLatency_ = 0;

    // Flow control configuration (from FlowCtrlConfig command)
    uint8_t verboseFlag_ = 0;  // Control AxisState2 sending

    // Statistics
    unsigned int statusDropCount_ = 0;

    // >0: skip dispatchCmdPackets for remaining ticks
    int blockTicksRemain_ = 0;
    uint16_t processedCmdSeq_ = 0;  // Seq of the last fully accepted command packet.
    uint8_t commandEpoch_ = 0;
    uint16_t lastCommandSeq_ = 0;
    bool hasLastCommandSeq_ = false;

    // GPT-6 Codex, 2026-11-04. RT alone owns this bounded, single-command assembly.
    MotionCmdFragmentHeader fragment_ = {};
    uint32_t fragmentCommandId_ = 0;
    uint8_t fragmentData_[MOTION_CMD_FRAGMENT_PARAM_MAX];

    // Replace a final fragment with the original item for this cycle's dispatch.
    bool assembleFragment(uint16_t& type, uint16_t& paramLen, const uint8_t*& param,
                          uint32_t& id);
    void resetFragment(uint16_t errorCode = 0);
    // Assemble and dispatch supported fragmented motion commands.
    void processFragment(uint16_t paramLen, const uint8_t* param, uint32_t id);

    // Process one command packet fully in the dispatching cycle.
    void processCmdPacket(const CmdPacket& packet);
    // Execute one alias-based IO command into the caller-owned result.
    void processIoCommand(uint16_t type, const uint8_t* param, uint16_t paramLen,
                          CmdResult& result);
    void confirm(uint32_t id, uint16_t type, CmdTargetKind targetKind, uint16_t targetIndex,
                 uint8_t status, uint16_t errorCode = 0);
    void flushCmdConfirmations();
    void flushCmdResults();
    // Push the pending axis-info response after AxisWithFb applies this cycle's commands.
    void flushAxisInfoResult();

    void dispatchCmdPackets();
    // Generate status packets and push to status queue
    void generateStatusPackets();

    // Update flow control based on HeartbeatAck
    void updateFlowControl(const HeartbeatAck& ack);
};

class FbMoveWrap : public FbMoveWrap2Queue {
   public:
    explicit FbMoveWrap(int axNum = 3);
    FbMoveWrap(const FbMoveWrap&) = delete;
    FbMoveWrap& operator=(const FbMoveWrap&) = delete;

    MotionCmdInterfaceBase& getCmdInterface() { return cmdIf_; }
    void reset_runtime();

   private:
    MotionCmdInterfaceBase cmdIf_;
};

}  // namespace zmc

#endif
