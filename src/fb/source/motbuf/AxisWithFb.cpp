/*
 * Axis function-block wrapper implementation.
 * AI modification: GPT-5 Codex
 */
#include "motbuf/AxisWithFb.hpp"

#include <cstdio>
#include <cstring>

#include "impl/fbt_axis_base.h"
#include "fbt_errorcode_def.h"
#include "mc_parameter_enum_def.h"
#include "motbuf/FbMoveWrapV3.hpp"

namespace zmc {

class FbMoveErasedWrapper : public FbMoveErasedTmpl<FbtMoveRelative> {
   public:
    void setExecute(bool val) { setFbExecute(get(), val); }
    int getStat() { return getFbImplStat(get()); }
};

template <typename T, typename P>
inline void tryLowerExecuteAndRunCycle2(T& fbWrap, P parent) {
    auto& fb = *fbWrap.get();
    auto fbStat = getFbImplStat(&fb);
    if (fbStat == FB_STAT_DONE || fbStat == FB_STAT_ERROR || fbStat == FB_STAT_ABORTED) {
        fb.Execute = false;
        // printf("tryLowerExecuteAndRunCycle %d %d\n", fbStat, fb.ErrorID);

        const CommandMidDefault& cmdMid = fbWrap.getCmdMid();
        parent->pushCmdResult(cmdMid, fbStat, (void*)fbWrap.get());
    }
    fb.runCycle();
}

template <typename T>
void reset_command_fb(T& fbWrap) {
    auto* fb = fbWrap.get();
    if (!fb) return;
    fb->Execute = false;
}

AxisWithFb::AxisWithFb() {}

AxisWithFb::~AxisWithFb() {
    // Release all active intrusive-list nodes.
    fbList.clear();
}

int AxisWithFb::getAxisState() const {
    if (!ax) return static_cast<int>(mcDisabledEx);
    return static_cast<int>(get_axis_impl_ref(ax).getAxisStateEx());
}

void AxisWithFb::bindAxis(AXIS_REF axRef) {
    ax = axRef;

    fbRdSetValues.Axis = axRef;
    fbRdActualValues.Axis = axRef;
    fbRdStatus.Axis = axRef;
    fbRdError.Axis = axRef;
    fbRdPowerParam.Axis = axRef;
    fbRdParameter.Axis = axRef;
    fbWrParameter.Axis = axRef;

    fbRdSetValues.Enable = true;
    fbRdActualValues.Enable = true;
    fbRdStatus.Enable = true;
    fbRdError.Enable = true;
    fbRdPowerParam.Enable = true;
    fbRdParameter.Enable = true;

    fbPower.Axis = axRef;
    fbRest.get()->Axis = axRef;
    fbStp.get()->Axis = axRef;
    fbHome.get()->Axis = axRef;
    fbSetPos.get()->Axis = axRef;
    fbChangeRatio.get()->Axis = axRef;
    fbSetOverride.Axis = axRef;
    fbHlt.get()->Axis = axRef;

    fbGearIn.get()->Slave = axRef;
    fbGearOut.get()->Slave = axRef;
}

int AxisWithFb::readAxisInfo(AxisInfo& info) {
    info = {};
    info.axis_id = static_cast<uint32_t>(axisId);
    info.velocity = cmdVel;
    info.acceleration = cmdAcc;
    info.deceleration = cmdDec;
    info.jerk = cmdJerk;

    fbRdParameter.ParameterNumber = PARA_iRatioTechUnitsNum;
    fbRdParameter.runCycle();
    if (fbRdParameter.Error || !fbRdParameter.Valid) {
        return static_cast<int>(fbRdParameter.ErrorID);
    }
    info.pulses_per_unit_num = static_cast<int32_t>(fbRdParameter.Value);

    fbRdParameter.ParameterNumber = PARA_dwRatioTechUnitsDenom;
    fbRdParameter.runCycle();
    if (fbRdParameter.Error || !fbRdParameter.Valid) {
        return static_cast<int>(fbRdParameter.ErrorID);
    }
    info.pulses_per_unit_den = static_cast<uint32_t>(fbRdParameter.Value);
    return 0;
}

void AxisWithFb::bindCmd(FbMoveErasedBase& fbWrap, const CommandStructDefault& cmd) {
    CommandMidDefault& cmdMid = fbWrap.getCmdMid();
    cmdMid.cmdType = cmd.cmdType;
    cmdMid.id = cmd.id;
    cmdMid.axisId = axisId;
    cmdMid.tickStarted = parent_->cnt;
    cmdMid.val = cmd.getDouble();
}

void AxisWithFb::bindCmd(CommandMidDefault& cmdMid, const CommandStructDefault& cmd) {
    cmdMid.cmdType = cmd.cmdType;
    cmdMid.id = cmd.id;
    cmdMid.axisId = axisId;
    cmdMid.tickStarted = parent_->cnt;
    cmdMid.val = cmd.getDouble();
}

FbMoveErasedBase* AxisWithFb::allocateFb(int cmdType) {
    if (!parent_) {
        printf("FbMoveWrapV3: allocateFb: parent_ is null\n");
        return nullptr;
    }
    return parent_->axisMotbuf.allocateFb(cmdType, ax);
}

int AxisWithFb::processCommand(const CommandStructDefault& cmd) {
    int error = 0;
    switch (cmd.cmdType) {
        // Read through the already bound FB. Model signature: GPT-6 Codex.
        case FBM_READ_PARAMETER:
            fbRdParameter.ParameterNumber = cmd.getInt();
            fbRdParameter.runCycle();
            if (fbRdParameter.Error) error = fbRdParameter.ErrorID;
            break;
        // Execute and reset the FB before the ordinary command ACK. GPT-6 Codex.
        case FBM_SET_PARAMETER: {
            WriteParameterParam param;
            if (!cmd.readParamAs(param)) return ZMC_ERR_MOTCMD_INVALID_ARG;
            fbWrParameter.ParameterNumber = param.parameter_number;
            fbWrParameter.Value = param.value;
            fbWrParameter.Execute = true;
            fbWrParameter.runCycle();
            if (fbWrParameter.Error) error = fbWrParameter.ErrorID;
            fbWrParameter.Execute = false;
            fbWrParameter.runCycle();
            break;
        }
        case FBM_SET_NUM_DEN: {
            auto& fbChangeRatio = *this->fbChangeRatio.get();
            if (fbChangeRatio.Execute) return ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED;
            fbChangeRatio.Execute = true;
            fbChangeRatio.dwRatioTechUnitsDenom = cmd.getIntByIndex(1);
            fbChangeRatio.iRatioTechUnitsNum = cmd.getIntByIndex(0);
            bindCmd(this->fbChangeRatio.getCmdMid(), cmd);
            break;
        }
        case FBM_SET_SPEED:
            cmdVel = cmd.getDouble();
            break;
        case FBM_SET_ACCEL:
            cmdAcc = cmd.getDouble();
            break;
        case FBM_SET_DECEL:
            cmdDec = cmd.getDouble();
            break;
        case FBM_SET_JERK:
            cmdJerk = cmd.getDouble();
            break;
        case FBM_SET_FAST_DECEL:
            fastDec = cmd.getDouble();
            break;
        case FBM_SET_SPEED_RATIO: {
            const double ratio = cmd.getDouble();
            fbSetOverride.Enable = true;
            fbSetOverride.VelFactor = ratio;
            fbSetOverride.runCycle();
            overrideRanInTop_ = true;
            if (fbSetOverride.Error) error = fbSetOverride.ErrorID;
            break;
        }
        case FBM_SET_CREEP:
            creep = cmd.getDouble();
            break;
        case FBM_SET_BUFFER_MODE:
            bufferMode = cmd.getInt();
            break;
        case FBM_SET_DIRECTION:
            direction = cmd.getInt();
            break;
        case FBM_SET_MASTER:
            axMasterIdx = cmd.getInt();
            break;
        case FBM_POWER:
            setAxisEnable(cmd.getDouble());
            fbPower.runCycle();
            powerRanInTop_ = true;
            if (fbPower.Error) error = fbPower.ErrorID;
            break;
        case FBM_RESET:
            if (fbRest.get()->Execute) return ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED;
            fbRest.get()->Execute = true;
            bindCmd(fbRest.getCmdMid(), cmd);
            break;
        case FBM_STOP: {
            auto& fbStp = *this->fbStp.get();
            if (fbStp.Execute) return ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED;
            fbStp.Execute = true;
            fbStp.Deceleration = getMaxDec();
            fbStp.Jerk = cmdJerk;
            bindCmd(this->fbStp.getCmdMid(), cmd);
            break;
        }
        case FBM_HALT: {
            auto pFb = fbHlt.get();
            if (pFb->Execute) return ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED;
            pFb->Deceleration = getMaxDec();
            pFb->Jerk = cmdJerk;
            pFb->Execute = true;
            bindCmd(fbHlt.getCmdMid(), cmd);
            break;
        }
        case FBM_GEAR_IN: {
            auto& fbGearIn = *this->fbGearIn.get();
            if (fbGearIn.Execute) return ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED;
            fbGearIn.Execute = true;
            fbGearIn.Master = parent_->axisMotbuf.axisRef(axMasterIdx);
            set_fb_acc_config(fbGearIn, cmdAcc, cmdDec, cmdJerk);
            bindCmd(this->fbGearIn.getCmdMid(), cmd);
            break;
        }
        case FBM_GEAR_OUT: {
            if (fbGearOut.get()->Execute) return ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED;
            fbGearOut.get()->Execute = true;
            bindCmd(fbGearOut.getCmdMid(), cmd);
            break;
        }
        case FBM_HOME: {
            auto& fbHome = *this->fbHome.get();
            if (fbHome.Execute) return ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED;
            // The legacy Position-based command must not reuse SDO-home settings.
            fbHome.disableSdo();
            fbHome.setSkipPosition(false);
            fbHome.setTimeoutMs(0);
            fbHome.Execute = true;
            fbHome.Position = cmd.getDouble();
            bindCmd(this->fbHome.getCmdMid(), cmd);
            break;
        }
        case FBM_HOME_SDO: {
            if (fbHome.get()->Execute) return ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED;
            HomeSdoConfig config{};
            std::memcpy(&config, cmd.getParamData(), sizeof(config));
            auto& fbHome = *this->fbHome.get();
            fbHome.configureSdo(config.mode, config.offset, config.velocity_fast,
                                config.velocity_slow, config.acceleration);
            fbHome.setTimeoutMs(config.timeout_ms);
            fbHome.setSkipPosition(true);
            fbHome.Execute = true;
            bindCmd(this->fbHome.getCmdMid(), cmd);
            break;
        }
        case FBM_SET_POSITION: {
            auto& fbSetPos = *this->fbSetPos.get();
            if (fbSetPos.Execute) return ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED;
            fbSetPos.Execute = true;
            fbSetPos.Position = cmd.getDouble();
            bindCmd(this->fbSetPos.getCmdMid(), cmd);
            break;
        }
        case FBM_MOVE_RELATIVE: {
            auto pFb =
                static_cast<FbMoveErasedTmpl<FbtMoveRelative>*>(allocateFb(FBM_MOVE_RELATIVE));
            if (pFb) {
                bindCmd(*pFb, cmd);
                auto* fb = pFb->get();
                setFbDistance(fb, cmd.getDouble());
                fb->Execute = true;
                fb->BufferMode = (MC_BUFFER_MODE)bufferMode;
                set_fb_vel_config(*fb, cmdVel, cmdAcc, cmdDec, cmdJerk);
                fbList.pushBack(pFb);
            } else
                error = ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED;
            break;
        }
        case FBM_MOVE_ABSOLUTE: {
            auto pFb =
                static_cast<FbMoveErasedTmpl<FbtMoveAbsolute>*>(allocateFb(FBM_MOVE_ABSOLUTE));
            if (pFb) {
                bindCmd(*pFb, cmd);
                auto* fb = pFb->get();
                setFbDistance(fb, cmd.getDouble());
                fb->Execute = true;
                fb->BufferMode = (MC_BUFFER_MODE)bufferMode;
                set_fb_vel_config(*fb, cmdVel, cmdAcc, cmdDec, cmdJerk);
                fb->Direction = (MC_DIRECTION)direction;
                fbList.pushBack(pFb);
            } else
                error = ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED;
            break;
        }
        case FBM_MOVE_ADDITIVE: {
            auto pFb =
                static_cast<FbMoveErasedTmpl<FbtMoveAdditive>*>(allocateFb(FBM_MOVE_ADDITIVE));
            if (pFb) {
                bindCmd(*pFb, cmd);
                auto* fb = pFb->get();
                fb->Distance = cmd.getDouble();
                fb->Execute = true;
                set_fb_vel_config(*fb, cmdVel, cmdAcc, cmdDec, cmdJerk);
                fbList.pushBack(pFb);
            } else
                error = ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED;
            break;
        }
        case FBM_MOVE_VELOCITY: {
            auto* pFb =
                static_cast<FbMoveErasedTmpl<FbtMoveVelocity>*>(allocateFb(FBM_MOVE_VELOCITY));
            if (pFb) {
                bindCmd(*pFb, cmd);
                auto* fb = pFb->get();
                fb->Execute = true;
                fb->BufferMode = (MC_BUFFER_MODE)bufferMode;
                fb->Direction = cmd.getInt() >= 0 ? mcPositiveDirection : mcNegativeDirection;
                set_fb_vel_config(*fb, cmdVel, cmdAcc, cmdDec, cmdJerk);
                fbList.pushBack(pFb);
            } else
                error = ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED;
            break;
        }
        default:
            error = ZMC_ERR_MOTCMD_UNKNOWN_COMMAND;
            break;
    }
    return error;
}

void AxisWithFb::printFbListInfo() const {
    printf("fbList.size() = %zu\n", fbList.size());
    int idx = 0;
    const FbMoveErasedBase* pFb = fbList.begin();
    while (pFb) {
        int type = getFbImplType(const_cast<void*>(pFb->getPtr()));
        int stat = getFbImplStat(const_cast<void*>(pFb->getPtr()));
        printf("  [%d] Fb ptr: %p  Type: %d  Stat: %d  Pool: %p\n", idx++, (const void*)pFb, type,
               stat, (const void*)pFb->getPool());
        pFb = pFb->getNext();
    }
}

void AxisWithFb::processCmdQue() {
    while (!cmdQueue.empty()) {
        const CommandStructDefault& cmd = cmdQueue.front();
        const int error = processCommand(cmd);
        if (cmd.id != 0 && parent_) {
            CmdResult confirm = {};
            confirm.id = cmd.id;
            confirm.command_type = static_cast<uint16_t>(cmd.cmdType);
            confirm.target_index = static_cast<uint8_t>(axisId);
            confirm.target_kind = static_cast<uint8_t>(CMD_TARGET_AXIS);
            confirm.status = error ? FB_STAT_ERROR
                             : (isAsyncAxisCommand(cmd.cmdType) || cmd.cmdType == FBM_POWER)
                                 ? FB_STAT_BUSY_ACTIVE
                                 : FB_STAT_DONE;
            confirm.error_code = static_cast<uint16_t>(error);
            confirm.tick = parent_->cnt;
            // Preserve all double bits in the existing two result words. GPT-6 Codex.
            if (!error && cmd.cmdType == FBM_READ_PARAMETER) {
                int32_t words[2];
                std::memcpy(words, &fbRdParameter.Value, sizeof(words));
                confirm.data = words[0];
                confirm.data_ext = words[1];
            }
            parent_->pushCmdConfirm(confirm);
        }
        cmdQueue.pop_front();
    }
}

void AxisWithFb::reset_runtime() {
    powerRanInTop_ = false;
    overrideRanInTop_ = false;
    fbPower.Enable = false;
    fbPower.bDriveStart = false;
    fbPower.bRegulatorOn = false;
    if (fbPower.Axis) {
        fbPower.runCycle();
    }
    fbSetOverride.Enable = false;
    if (fbSetOverride.Axis) {
        fbSetOverride.runCycle();
    }
    reset_command_fb(fbGearOut);
    reset_command_fb(fbGearIn);
    reset_command_fb(fbRest);
    reset_command_fb(fbStp);
    reset_command_fb(fbHlt);
    reset_command_fb(fbHome);
    reset_command_fb(fbSetPos);
    reset_command_fb(fbChangeRatio);

    fbRdSetValues.Enable = false;
    fbRdActualValues.Enable = false;
    fbRdStatus.Enable = false;
    fbRdError.Enable = false;
    fbRdPowerParam.Enable = false;
    fbRdParameter.Enable = false;
    axMasterIdx = 0;
    bufferMode = 1;
    direction = 0;
    cmdPos = 10.0;
    cmdVel = 5.0;
    cmdAcc = 20.0;
    cmdDec = 20.0;
    fastDec = 40.0;
    cmdJerk = 0.0;
    creep = 1.0;

    FbMoveErasedBase* current = fbList.begin();
    while (current) {
        FbMoveErasedBase* next = current->getNext();
        current->reset_runtime();
        current = next;
    }
    fbList.clear();
    cmdQueue.clear();
}

void AxisWithFb::runCycleTop() { processCmdQue(); }

void AxisWithFb::runCycle() {
    runCycleTop();
    runCycleBottom();
}

void AxisWithFb::runCycleBottom() {
    fbRdSetValues.runCycle();
    fbRdActualValues.runCycle();
    fbRdStatus.runCycle();
    fbRdError.runCycle();
    fbRdPowerParam.runCycle();
    if (!powerRanInTop_) fbPower.runCycle();
    if (!overrideRanInTop_) fbSetOverride.runCycle();
    powerRanInTop_ = false;
    overrideRanInTop_ = false;

    tryLowerExecuteAndRunCycle2(fbStp, parent_);
    tryLowerExecuteAndRunCycle2(fbHlt, parent_);
    tryLowerExecuteAndRunCycle2(fbGearOut, parent_);
    tryLowerExecuteAndRunCycle2(fbGearIn, parent_);
    tryLowerExecuteAndRunCycle2(fbHome, parent_);
    tryLowerExecuteAndRunCycle2(fbRest, parent_);
    tryLowerExecuteAndRunCycle2(fbSetPos, parent_);
    tryLowerExecuteAndRunCycle2(fbChangeRatio, parent_);

    // Iterate with a predecessor pointer so removals preserve list traversal.
    FbMoveErasedBase* prev = fbList.getSentinel();
    FbMoveErasedBase* current = prev->getNext();
    while (current) {
        auto pFb2 = static_cast<FbMoveErasedWrapper*>(current);
        int fbStat = pFb2->getStat();
        if (fbStat == FB_STAT_DONE || fbStat == FB_STAT_ERROR || fbStat == FB_STAT_ABORTED) {
            pFb2->setExecute(false);
            if (parent_) {
                auto pFb = static_cast<FbMoveErasedWrapper*>(current);
                const CommandMidDefault& cmdMid = pFb->getCmdMid();
                parent_->pushCmdResult(cmdMid, fbStat, pFb->getPtr<void>());
            }
        }
        current->runCycle();
        fbStat = pFb2->getStat();
        if (fbStat == FB_STAT_IDLE) {
            if (current->getPool()) {
                void* pool = current->getPool();
                FbMoveErasedBase* removed = fbList.removeNext(prev);
                parent_->axisMotbuf.releaseFb(removed, pool);
                current = prev->getNext();
            } else {
                fbList.removeNext(prev);
                current = prev->getNext();
            }
        } else {
            prev = current;
            current = current->getNext();
        }
    }
}

}  // namespace zmc
