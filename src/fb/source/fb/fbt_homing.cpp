/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-01
@ArtifactStatusAuthority GPT-5 Codex
*/

#include <stdio.h>
#include "fbt_fb_base.hpp"
#include "fb_cpp_def.h"
#include "fbt_axis_impl.h"
#include "vel/include/Vel4Wrap.h"
#include "fbt_move_impl.hpp"
#include "misc_helper.hpp"
#include "common/ecat/ecat_pdo_key.h"
#include "fb/smc_parameter_utils.h"
#include "../axis/Cia402Ex.h"
#include "math.h"

namespace zmc {

// typedef TrapzVel Vel4Wrap;

// #define USE_POWEROFF_BEFORE_HOME

class FbtImplHome : public FbtExeTmpl<FbtImplHome, FbtImplParamMotTmpl<FbtHomeStruct>> {
   public:
    FbtImplHome() { this->fbType = FB_TYPE_HOME; }

    enum SubState : uint8_t { POWEROFF, POWERON, HOMING };
    enum SdoPhase : uint8_t { SDO_NONE, SDO_MODE, SDO_OFFSET, SDO_FAST, SDO_SLOW, SDO_ACCEL };

    void configureSdo(ZMC_DINT mode, ZMC_LREAL offset, ZMC_LREAL velocityFast,
                      ZMC_LREAL velocitySlow, ZMC_LREAL acceleration) {
        sdo_enabled_ = true;
        sdo_mode_ = mode;
        sdo_offset_ = offset;
        sdo_velocity_fast_ = velocityFast;
        sdo_velocity_slow_ = velocitySlow;
        sdo_acceleration_ = acceleration;
    }

    void disableSdo() { sdo_enabled_ = false; }

    void setSkipPosition(bool skipPosition) { skip_position_ = skipPosition; }

    void setTimeoutMs(ZMC_DINT timeoutMs) { timeout_ms_ = timeoutMs; }

    bool checkAxisStatBeforeActivate() {
        auto& ax = get_axis_impl_ref(this->pParam->Axis);
        if (ax.axisStat == mcDisabled) {
            this->onError(ZMC_H_AXIS_WASNT_STANDSTILL);
        } else if (ax.axisStat == mcErrorStop) {
            this->onError(ZMC_H_AXIS_IN_ERRORSTOP);
            return false;
        } else if (ax.axisStat == mcStopping) {
            this->onError(ZMC_ERR_FB_ACTIVATE_AXIS_STOPPING);
            return false;
            // mcDisabled
        } else if (ax.axisStat == mcHoming) {
            // Reject a new drive Home without changing the running command's operation mode.
            FbtBase::onError(ZMC_ERR_H_AXIS_IN_MOVING);
            return false;
        } else if (ax.axisStat != mcStandstill) {
            this->onError(ZMC_ERR_H_AXIS_IN_MOVING);
        } else {
            return true;
        }
        return false;
    }

    void onStarted() {
        auto& param = *this->pParam;
        if (this->fbStat != FB_STAT_IDLE) return;
        if (!param.Axis) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }
        auto& ax = get_axis_impl_ref(param.Axis);

        if (!checkAxisStatBeforeActivate()) {
            return;
        }
        ax.setAxisState(mcHoming, AXIS_STATE_DETAIL_HOME);
        active_skip_position_ = skip_position_;
        active_position_ = param.Position;
        active_sdo_mode_ = sdo_mode_;
        active_sdo_offset_ = sdo_offset_;
        active_sdo_velocity_fast_ = sdo_velocity_fast_;
        active_sdo_velocity_slow_ = sdo_velocity_slow_;
        active_sdo_acceleration_ = sdo_acceleration_;
        active_timeout_cycles_ = getTimeoutCycles(ax);
        sdo_phase_ = sdo_enabled_ ? SDO_MODE : SDO_NONE;
        if (sdo_enabled_) prepareSdoWrite();
#ifdef USE_POWEROFF_BEFORE_HOME
        _subState = POWEROFF;
#else
        _subState = POWERON;
#endif
        _cnt_home = _cnt_op = _cnt_done_error = homing_timeout_age_ = 0;
        this->fbStat = FB_STAT_UNACTIVATED;
        ax.pNode.push_back(this);
    }
    inline uint8_t toCtrlMode(OP_MODE mode) const { return E_OP_MODE_TO_CTRL_MODE - mode; }
    bool checkAxisStatWhenActivated() {
        auto& ax = get_axis_impl_ref(this->pParam->Axis);
        if (ax.axisStat == mcErrorStop) {
            this->onError(ZMC_ERR_ERRORSTOP_WHEN_MOVING);
            return false;
        } else if (ax.axisStat == mcDisabled) {
            this->onError(ZMC_ERR_POWEROFF_WHEN_MOVING);
            return false;
        } else if (ax.axisStat == mcStopping) {
            this->onError(ZMC_ERR_FB_ACTIVATE_AXIS_STOPPING);
            return false;
        }
        return true;
    }
    void onPlan() {}
    void onExecuting() {
        if (!this->checkReceiveAbortError()) return;
        if (!this->checkAxisStatWhenActivated()) return onClearOpMode();
        // if (this->fbStat != FB_STAT_BUSY_ACTIVE) return onClearOpMode();
        if (this->isRisingEdge()) {
            onPlan();
        }
        onForward();
    }
    void onForward() {
        auto& param = *this->pParam;
        auto& ax = get_axis_impl_ref(param.Axis);

        switch (this->fbStat) {
            case FB_STAT_UNACTIVATED:
                this->fbStat = FB_STAT_BUSY_ACTIVE;
                if (sdo_phase_ != SDO_NONE) break;
                ax.byControllerMode = toCtrlMode(OP_MODE_HOME);
#ifdef USE_POWEROFF_BEFORE_HOME
                ax.cmd = AXIS_CMD_POWEROFF;
#endif
                break;
            case FB_STAT_BUSY_ACTIVE:
                if (ax.axisStat == mcErrorStop) {
                    return this->onError(ZMC_H_AXIS_IN_ERRORSTOP);
                } else if (ax.axisStat != mcHoming) {
                    this->fbStat = FB_STAT_ABORTED;
                    return onClearOpMode();
                } else if (_cnt_op > 1000) {
                    this->fbStat = FB_STAT_ERROR;
                    if (ax.byRealControllerMode != toCtrlMode(OP_MODE_HOME))
                        this->onError(ZMC_H_SET_OP_MODE_HOMING_FAILED);
                    if (ax.axisStat == mcHoming) ax.setAxisState(mcStandstill);
                    return;
                }
                if (sdo_phase_ != SDO_NONE) {
                    if (runSdoConfig(ax)) ax.byControllerMode = toCtrlMode(OP_MODE_HOME);
                    return;
                }
                ax.byControllerMode = toCtrlMode(OP_MODE_HOME);
                handleSubState(ax);
                break;
            case FB_STAT_DONE:
                if (ax.pNode.isCurrentNode(this)) {
                    ax.pNode.pop_front();
                }
            default:
                break;
        }
    }

    void onCapture() { this->execute = this->pParam->Execute; }
    void onUpdate() {
        this->pParam->Busy = this->isBusy();
        this->pParam->Done = this->isDone();
        this->pParam->Error = this->isError();
        this->pParam->CommandAborted = this->isAborted();
        this->pParam->ErrorID = this->getErrorID();
    }

   private:
    inline void onClearOpMode() {
        auto& ax = get_axis_impl_ref(this->pParam->Axis);
        ax.byControllerMode = toCtrlMode(OP_MODE_CSP);
    }
    inline void onError(MC_ERROR_CODE err) {
        fbStat = FB_STAT_ERROR;
        error_code = err;
        onClearOpMode();
    }

    ZMC_DINT getTimeoutCycles(const AxisImplBase& ax) const {
        if (timeout_ms_ <= 0) return 0;
        return static_cast<ZMC_DINT>(ceil(timeout_ms_ * 1e-3 / ax.cycleTime));
    }

    void prepareSdoWrite() {
        sdo_write_.Axis = this->pParam->Axis;
        sdo_write_.TimeoutCycles = active_timeout_cycles_;
        switch (sdo_phase_) {
            case SDO_MODE:
                sdo_write_.ParameterNumber =
                    zmc_parameter_number_coe(PDO_PO_HOMING_METHOD, 0, 1);
                sdo_write_.Value = active_sdo_mode_;
                break;
            case SDO_OFFSET:
                sdo_write_.ParameterNumber =
                    zmc_parameter_number_coe(PDO_PO_HOMING_OFFSET, 0, 4);
                sdo_write_.Value = active_sdo_offset_;
                break;
            case SDO_FAST:
                sdo_write_.ParameterNumber =
                    zmc_parameter_number_coe(PDO_PO_HOMING_SPEED, 1, 4);
                sdo_write_.Value = active_sdo_velocity_fast_;
                break;
            case SDO_SLOW:
                sdo_write_.ParameterNumber =
                    zmc_parameter_number_coe(PDO_PO_HOMING_SPEED, 2, 4);
                sdo_write_.Value = active_sdo_velocity_slow_;
                break;
            case SDO_ACCEL:
                sdo_write_.ParameterNumber =
                    zmc_parameter_number_coe(PDO_PO_HOMING_ACCEL, 0, 4);
                sdo_write_.Value = active_sdo_acceleration_;
                break;
            default:
                return;
        }
        // Each SDO request is edge-triggered and must finish before the next object starts.
        sdo_write_.Execute = true;
    }

    bool runSdoConfig(AxisImplBase& ax) {
        sdo_write_.runCycle();
        if (sdo_write_.Busy) return false;

        if (sdo_write_.Error) {
            const MC_ERROR_CODE error = static_cast<MC_ERROR_CODE>(sdo_write_.ErrorID);
            sdo_write_.Execute = false;
            sdo_write_.runCycle();
            sdo_phase_ = SDO_NONE;
            this->onError(error);
            ax.setAxisState(mcStandstill);
            if (ax.pNode.isCurrentNode(this)) ax.pNode.pop_front();
            return false;
        }
        if (!sdo_write_.Done) return false;

        sdo_write_.Execute = false;
        sdo_write_.runCycle();
        sdo_phase_ = static_cast<SdoPhase>(sdo_phase_ + 1);
        if (sdo_phase_ > SDO_ACCEL) {
            sdo_phase_ = SDO_NONE;
            return true;
        }
        prepareSdoWrite();
        return false;
    }

    void handleSubState(AxisImplBase& ax) {
        auto smEx = (Cia402SmEx&)ax.swSmEx;
        switch (_subState) {
            case SubState::POWEROFF:
                _cnt_op++;
                if (!ax.regulatorRealState) {  // DRIVE_STAT_POWERON
                    _subState = SubState::POWERON;
                    ax.cmd = AXIS_CMD_POWERON;
                    break;
                }
                ax.cmd = AXIS_CMD_POWEROFF;
                break;
            case SubState::POWERON:
                _cnt_op++;
                if (ax.regulatorRealState && ax.driveStartRealState &&
                    ax.byRealControllerMode == toCtrlMode(OP_MODE_HOME)) {
                    _subState = SubState::HOMING;
                    homing_timeout_age_ = 0;
                    ax.cmd = AXIS_CMD_HOME;
                    break;
                }
                ax.cmd = AXIS_CMD_POWERON;
                break;
            case SubState::HOMING:
                _cnt_home++;
                if (_cnt_home > 2 && smEx.is_home_error() && _cnt_done_error++ > 2) {
                    ax.setAxisState(mcErrorStop);
                    this->onError(ZMC_DI_HOMING_ERROR);
                    ax.byControllerMode = toCtrlMode(OP_MODE_CSP);
                } else if (_cnt_home > 2 && smEx.is_home_done() && _cnt_done_error++ > 2) {
                    ax.setAxisState(mcStandstill);
                    if (!active_skip_position_)
                        axisInfoShift(ax, active_position_ - ax.fActPosition);
                    ax.byControllerMode = toCtrlMode(OP_MODE_CSP);
                    this->fbStat = FB_STAT_DONE;
                } else if (active_timeout_cycles_ > 0 &&
                           ++homing_timeout_age_ > static_cast<uint32_t>(active_timeout_cycles_)) {
                    ax.setAxisState(mcStandstill);
                    this->onError(ZMC_ERR_H_TIMEOUT);
                } else if (smEx.is_halt_done() && 0) {
                    this->fbStat = FB_STAT_ABORTED;
                    ax.setAxisState(mcStandstill);
                } else {
                    ax.cmd = AXIS_CMD_HOME;
                }
            default:
                break;
        }
    }
    SubState _subState = POWEROFF;
    SdoPhase sdo_phase_ = SDO_NONE;
    FbtWriteCoeParameter sdo_write_;
    bool sdo_enabled_ = false;
    bool skip_position_ = false;
    bool active_skip_position_ = false;
    ZMC_DINT timeout_ms_ = 0;
    ZMC_DINT active_timeout_cycles_ = 0;
    ZMC_DINT sdo_mode_ = 0;
    ZMC_LREAL sdo_offset_ = 0;
    ZMC_LREAL sdo_velocity_fast_ = 0;
    ZMC_LREAL sdo_velocity_slow_ = 0;
    ZMC_LREAL sdo_acceleration_ = 0;
    ZMC_LREAL active_position_ = 0;
    ZMC_DINT active_sdo_mode_ = 0;
    ZMC_LREAL active_sdo_offset_ = 0;
    ZMC_LREAL active_sdo_velocity_fast_ = 0;
    ZMC_LREAL active_sdo_velocity_slow_ = 0;
    ZMC_LREAL active_sdo_acceleration_ = 0;
    uint32_t _cnt_home = 0;
    uint8_t _cnt_done_error = 0;
    uint16_t _cnt_op = 0;
    uint32_t homing_timeout_age_ = 0;
};

FbtHome::FbtHome() {
    all_zero_init<FbtHomeStruct>(*this);
    pImpl = new FbtImplHome();
    static_cast<FbtImplHome*>(pImpl)->pParam = this;
}

FbtHome::~FbtHome() {
    delete static_cast<FbtImplHome*>(pImpl);
    pImpl = nullptr;
}

void FbtHome::configureSdo(ZMC_DINT mode, ZMC_LREAL offset, ZMC_LREAL velocityFast,
                           ZMC_LREAL velocitySlow, ZMC_LREAL acceleration) {
    static_cast<FbtImplHome*>(pImpl)->configureSdo(mode, offset, velocityFast, velocitySlow,
                                                   acceleration);
}

void FbtHome::disableSdo() {
    static_cast<FbtImplHome*>(pImpl)->disableSdo();
}

void FbtHome::setSkipPosition(bool skipPosition) {
    static_cast<FbtImplHome*>(pImpl)->setSkipPosition(skipPosition);
}

void FbtHome::setTimeoutMs(ZMC_DINT timeoutMs) {
    static_cast<FbtImplHome*>(pImpl)->setTimeoutMs(timeoutMs);
}

void FbtHome::runCycle() {
    auto impl = static_cast<FbtImplHome*>(pImpl);
    impl->runCycle();
}

class FbtImplHoming : public FbExeAbortingTmpl<FbtImplHoming, FbtHomingStruct> {
   public:
    enum SubState : uint8_t {
        IDLE,
        BUSY,
        FAST,
        SLOW,
        BSLOW,
        STOP,
        SETPOS_STOP,
        SETPOS2,
        MOVE_TO_ZERO,
        DONE
    };

    FbtImplHoming() { this->fbType = FB_TYPE_HOMING; }

    bool checkAxisStatBeforeActivate() {
        auto& ax = get_axis_impl_ref(this->pParam->Axis);
        if (ax.axisStat == mcErrorStop) {
            this->onError(ZMC_H_AXIS_IN_ERRORSTOP);
            return false;
        } else if (ax.axisStat == mcDisabled) {
            this->onError(ZMC_H_AXIS_WASNT_STANDSTILL);
            return false;
        } else if (ax.axisStat == mcStopping) {
            this->onError(ZMC_ERR_FB_ACTIVATE_AXIS_STOPPING);
            return false;
        } else if (ax.axisStat == mcHoming) {
            // Only software Homing nodes may be replaced by another software Homing command.
            if (ax.getAxisStateEx() == mcHomeEx) {
                this->onError(ZMC_ERR_H_AXIS_IN_MOVING);
                return false;
            }
        } else if (ax.axisStat != mcStandstill) {
            this->onError(ZMC_ERR_H_AXIS_IN_MOVING);
            return false;
        }
        return true;
    }

    bool checkAxisStatWhenActivated() {
        auto& ax = get_axis_impl_ref(this->pParam->Axis);
        if (ax.axisStat == mcErrorStop) {
            this->onError(ZMC_ERR_ERRORSTOP_WHEN_MOVING);
            return false;
        } else if (ax.axisStat == mcDisabled) {
            this->onError(ZMC_ERR_POWEROFF_WHEN_MOVING);
            return false;
        } else if (ax.axisStat == mcStopping) {
            this->onError(ZMC_ERR_FB_ACTIVATE_AXIS_STOPPING);
            return false;
        }
        return true;
    }

    void onStarted() {
        auto& param = *this->pParam;
        if (this->fbStat != FB_STAT_IDLE) return;
        if (!param.Axis) {
            this->onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }
        auto& ax = get_axis_impl_ref(param.Axis);

        if (!checkAxisStatBeforeActivate()) {
            return;
        }
        if (ax.axisStat == mcHoming) {
            if (ax.getAxisStateEx() == mcHomeEx) {
                this->onError(ZMC_ERR_H_AXIS_IN_MOVING);
                return;
            }
            ax.pNode.clearFbByAborting();
        }
        // Initialize state machine
        ax.setAxisState(mcHoming, AXIS_STATE_DETAIL_DEFAULT);
        this->fbStat = FB_STAT_BUSY_ACTIVE;
        ax.pNode.push_back(this);
        trapzVel.set_mode(ax.eRampType);
    }

    void onErrorWhenBuild(MC_ERROR_CODE err) {
        auto& ax = this->getAxImpl();
        this->onError(err);
        if (ax.axisStatBak == mcStandstill || ax.axisStatBak == mcDisabled) {
            // A build failure before motion only rolls back the activation made in this cycle.
            ax.setAxisState(mcStandstill);
            ax.cmd = ax.cmdBak;
        } else {
            // A build failure after motion starts must use the axis software ErrorStop path.
            ax.requestSwError(err);
        }
        if (ax.pNode.isCurrentNode(this)) {
            ax.pNode.pop_front();
        }
    }

    void onPlan() {
        auto& param = *this->pParam;
        _subState = BUSY;
        this->nHomingMode = param.nHomingMode;
        this->bReturnToZero = param.bReturnToZero;
        // _prevSwitchState = param.bReferenceSwitch;
        // Validate parameters
        if (param.fVelocitySlow <= 0 || param.fVelocityFast <= 0 || param.fAcceleration <= 0 ||
            param.fDeceleration <= 0) {
            this->onErrorWhenBuild(ZMC_INVALID_VELACCJERK_VALUES);
            return;
        }
        if (nHomingMode == FAST_BSLOW_I_S_STOP || nHomingMode >= FAST_SLOW_I_S_STOP) {
            this->onErrorWhenBuild(
                ZMC_3SH_INVALID_VELACC_VALUES);  // TODO: add specific error code
            return;
        }
    }
    void onOnlinePlan() {
        auto& param = *this->pParam;
        auto& ax = getAxImpl();

        bool isStopDone = (_lastRet == VEL_END_PT_REACHED);
        _subState = fsm_simple(_subState, param.bReferenceSwitch, isStopDone, bReturnToZero,
                               (MC_HOMING_MODE)nHomingMode);

        if (_subState == DONE) {
            return;
        }
        // this->startTick = ax.m_cycleCounter;
        if (_subState != _subStateBak) {
            switch (_subState) {
                case FAST:
                    planFastStage(ax, param);
                    break;
                case SLOW:
                    planSlowStage(ax, param);
                    break;
                case BSLOW:
                    planBslowStage(ax, param);
                    break;
                case SETPOS_STOP:
                    planSetPos2Stage(ax, param);
                    planStopStage(ax, param);
                    break;
                case STOP:
                    planStopStage(ax, param);
                    break;
                case SETPOS2:
                    planSetPos2Stage(ax, param);
                    break;
                case MOVE_TO_ZERO:
                    planMoveToZero(ax, param);
                    break;
                case DONE:
                default:
                    break;
            }
        }
    }

    void onForward() {
        onOnlinePlan();
        _onForward();
    }

    void _onForward() {
        if (this->isError()) return;
        auto& param = *this->pParam;
        auto& ax = getAxImpl();

        PosVelAcc pva;
        int ret = trapzVel.get_slice(ax.get_offset_time(this->startTick), pva);
        _lastRet = ret;

        ax.cmd = AXIS_CMD_MOTION;
        ax.fSetPosition = pva.pos;
        ax.fSetVelocity = pva.vel;
        ax.fSetAcceleration = pva.acc;

        // Finalize when DONE
        if (_subState == DONE) {
            this->fbStat = FB_STAT_DONE;
            ax.setAxisState(mcStandstill);
            if (ax.pNode.isCurrentNode(this)) ax.pNode.pop_front();
        }
        _subStateBak = _subState;
    }

    void onCapture() { this->execute = this->pParam->bExecute; }

    void onUpdate() {
        this->pParam->bBusy = this->isBusy();
        this->pParam->bDone = this->isDone();
        this->pParam->bError = this->isError();
        this->pParam->bCommandAborted = this->isAborted();
        this->pParam->nErrorID = this->getErrorID();
    }

    void onClear() {
        this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
        this->startTick = 0;
        _subState = IDLE;
        _subStateBak = IDLE;
        // _prevSwitchState = false;
    }

   private:
    void planFastStage(AxisImplBase& ax, FbtHomingStruct& param) {
        bool bNegDirection = param.nDirection == mcNegativeDirection;
        double vEnd = bNegDirection ? -param.fVelocityFast : param.fVelocityFast;
        planVelStage(ax, param, vEnd, param.fVelocityFast);
    }

    void planBslowStage(AxisImplBase& ax, FbtHomingStruct& param) {
        bool bNegDirection = param.nDirection != mcNegativeDirection;
        double vEnd = bNegDirection ? -param.fVelocitySlow : param.fVelocitySlow;
        planVelStage(ax, param, vEnd, param.fVelocitySlow);
    }

    void planSlowStage(AxisImplBase& ax, FbtHomingStruct& param) {
        bool bNegDirection = param.nDirection == mcNegativeDirection;
        double vEnd = bNegDirection ? -param.fVelocitySlow : param.fVelocitySlow;
        planVelStage(ax, param, vEnd, param.fVelocitySlow);
    }

    void planVelStage(AxisImplBase& ax, FbtHomingStruct& param, double vEnd, double vMax) {
        double endPos = vEnd < 0 ? -1e19 : 1e19;
        trapzVel.set_begin_pva(ax.fSavePosition, ax.fSaveVelocity, ax.fSaveAcceleration);
        trapzVel.set_end_pva(endPos, vEnd, 0);
        setVelConfig(param, ax, vMax);
        if (trapzVel.build() != 0) {
            this->onErrorWhenBuild(ZMC_ERR_VEL_BUILD_FAILED);
            return;
        }
        this->startTick = ax.m_cycleCounter - ax.is_prev_moving();
    }

    void planStopStage(AxisImplBase& ax, FbtHomingStruct& param) {
        trapzVel.set_begin_pva(ax.fSavePosition, ax.fSaveVelocity, ax.fSaveAcceleration);
        trapzVel.set_config(ax.fSWMaxVelocity, param.fDeceleration, param.fDeceleration,
                            param.fJerk * ax.fFactorJerk);
        double endPos = trapzVel.cal_stop_pos();
        trapzVel.set_end_pva(endPos, 0, 0);
        if (trapzVel.build() != 0) {
            this->onErrorWhenBuild(ZMC_ERR_VEL_BUILD_FAILED);
            return;
        }
        this->startTick = ax.m_cycleCounter - ax.is_prev_moving();
    }

    void planMoveToZero(AxisImplBase& ax, FbtHomingStruct& param) {
        trapzVel.set_inversable(true);
        trapzVel.set_begin_pva(ax.fSavePosition, ax.fSaveVelocity, ax.fSaveAcceleration);
        trapzVel.set_end_pva(0, 0, 0);
        setVelConfig(param, ax, param.fVelocityFast);
        if (trapzVel.build() != 0) {
            this->onErrorWhenBuild(ZMC_ERR_VEL_BUILD_FAILED);
            return;
        }
        this->startTick = ax.m_cycleCounter - ax.is_prev_moving();
    }

    // Set position after stopping (SETPOS2 equivalent): apply offset only, no motion plan
    void planSetPos2Stage(AxisImplBase& ax, FbtHomingStruct& param) {
        double offset = 0;
        if (param.nHomingMode == FAST_BSLOW_S_STOP || param.nHomingMode == FAST_SLOW_S_STOP ||
            param.nHomingMode == FAST_BSLOW_I_S_STOP) {
            offset = ax.fSetPosition - param.fHomePosition - ax.fSaveVelocity * param.fSignalDelay;
        } else {
            offset = ax.fSetPosition - param.fHomePosition;
        }
        axisInfoShift(ax, -offset);
        trapzVel.shift_pos(-offset);
    }

    void setVelConfig(FbtHomingStruct& param, AxisImplBase& ax, double velocity) {
        trapzVel.set_config(velocity * ax.fFactorVel, param.fAcceleration * ax.fFactorAcc,
                            param.fDeceleration * ax.fFactorAcc, param.fJerk * ax.fFactorJerk);
    }

    SubState fsm_simple(SubState st, bool sw, bool isStopDone, bool returnToZero,
                        MC_HOMING_MODE mode) const {
        switch (st) {
            case BUSY:
                if (sw) {
                    return FAST;
                } else {
                    return BSLOW;
                }
                break;
            case FAST:
                if (!sw) {
                    if (mode == FAST_BSLOW_S_STOP || mode == FAST_BSLOW_STOP_S)
                        return BSLOW;
                    else
                        return SLOW;
                }
                return FAST;
            case BSLOW:
                if (sw) {
                    if (mode == FAST_BSLOW_S_STOP || mode == FAST_SLOW_S_STOP)
                        return SETPOS_STOP;
                    else
                        return STOP;
                }
                return BSLOW;
            case SLOW:
                if (sw) {
                    if (mode == FAST_SLOW_S_STOP)
                        return SETPOS_STOP;
                    else
                        return STOP;
                }
                return SLOW;
            case SETPOS_STOP:
                if (isStopDone) {
                    if (returnToZero) {
                        return MOVE_TO_ZERO;
                    } else {
                        return DONE;
                    }
                } else
                    return SETPOS_STOP;
            case STOP:
                if (isStopDone) {
                    return SETPOS2;
                } else
                    return STOP;
            case SETPOS2:
                if (returnToZero) {
                    return MOVE_TO_ZERO;
                } else {
                    return DONE;
                }
            case MOVE_TO_ZERO:
                if (isStopDone) {
                    return DONE;
                }
                return MOVE_TO_ZERO;
            case DONE:
                return DONE;
            case IDLE:
            default:
                return IDLE;
        }
    }

    Vel4Wrap trapzVel;
    SubState _subState = IDLE;
    SubState _subStateBak = IDLE;
    // bool hasVelPlan = false;
    // bool bNegDirection = false;
    // bool _prevSwitchState = false;
    uint8_t bReturnToZero : 1;
    uint8_t nHomingMode : 3;
    int8_t _lastRet = 0;
};

FbtHoming::FbtHoming() {
    make_impl<FbtImplHoming, FbtHomingStruct>(*this);
    this->bReferenceSwitch = true;
    this->nDirection = mcNegativeDirection;
}

FbtHoming::~FbtHoming() {
    if (pImpl) {
        delete static_cast<FbtImplHoming*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtHoming::runCycle() {
    if (pImpl) {
        static_cast<FbtImplHoming*>(pImpl)->runCycle();
    }
}

enum JogImplStat : uint8_t { JOG_IDLE = 0, JOG_POSITIVE, JOG_NEGATIVE, JOG_HALT };
inline JogImplStat getTargetStat(bool jogForward, bool jogBackward) {
    if (jogForward == jogBackward) {
        return JOG_IDLE;
    } else if (jogForward && !jogBackward) {
        return JOG_POSITIVE;
    } else {
        return JOG_NEGATIVE;
    }
}

typedef struct FbtJogImpl : public FbtEnBase<FB_TYPE_JOG> {
    FbtMoveVelocity moveVel;
    FbtHalt halt;
    JogImplStat stat = JOG_IDLE;
    JogImplStat statBak = JOG_IDLE;

    bool isError() const { return moveVel.Error || halt.Error; }
    bool isCommandAborted() const {
        return (stat == JOG_HALT) ? halt.CommandAborted : moveVel.CommandAborted;
    }
    void fsm(JogImplStat targetStat) {
        if (isError()) {
            if (JOG_IDLE == targetStat) onAction(JOG_IDLE);
            return;
        } else if (isCommandAborted()) {
            if (JOG_IDLE == targetStat) onAction(JOG_IDLE);
            return;
        }
        switch (stat) {
            case JOG_IDLE:
                onAction(targetStat);
                break;
            case JOG_POSITIVE:
                if (stat != targetStat) onAction(JOG_HALT);
                break;
            case JOG_NEGATIVE:
                if (stat != targetStat) onAction(JOG_HALT);
                break;
            case JOG_HALT:
                if (halt.Done) {
                    onAction(targetStat);
                }
                break;
            default:
                break;
        }
    }
    void onAction(JogImplStat targetStat) {
        switch (targetStat) {
            case JOG_POSITIVE:
                moveVel.Direction = MC_DIRECTION::mcPositiveDirection;
                moveVel.Execute = true;
                stat = JOG_POSITIVE;
                break;
            case JOG_NEGATIVE:
                moveVel.Direction = MC_DIRECTION::mcNegativeDirection;
                moveVel.Execute = true;
                stat = JOG_NEGATIVE;
                break;
            case JOG_HALT:
                moveVel.Execute = false;
                halt.Execute = true;
                stat = JOG_HALT;
                break;
            default:
                moveVel.Execute = false;
                halt.Execute = false;
                stat = JOG_IDLE;
                break;
        }
    }

    void updateOutputs(FbtJogStruct* jog) {
        jog->Busy = (stat != JOG_IDLE) ? true : false;

        jog->Error = isError();
        if (moveVel.Error) {
            jog->ErrorID = moveVel.ErrorID;
        } else if (halt.Error) {
            jog->ErrorID = halt.ErrorID;
        } else {
            jog->ErrorID = ZMC_NO_ERROR;
        }
        jog->CommandAborted = isCommandAborted();
    }
    void runCycle(FbtJogStruct* jog) {
        // FbtJogImpl* impl = static_cast<FbtJogImpl*>(pImpl);
        moveVel.Axis = jog->Axis;
        moveVel.Velocity = jog->Velocity;
        moveVel.Acceleration = jog->Acceleration;
        moveVel.Deceleration = jog->Deceleration;
        moveVel.Jerk = jog->Jerk;

        halt.Axis = jog->Axis;
        halt.Deceleration = jog->Deceleration;
        halt.Jerk = jog->Jerk;

        JogImplStat targetStat = getTargetStat(jog->JogForward, jog->JogBackward);
        fsm(targetStat);

        moveVel.runCycle();
        halt.runCycle();
        updateOutputs(jog);
        statBak = stat;
    }
} FbtJogImpl;

#if 0
FbtJog::FbtJog() {
    // make_impl<FbtJogImpl, FbtJogStruct>(*this);
    // pImpl = new FbtJogImpl();
    all_zero_init<FbtJogStruct>(*this);
    this->pImpl = new FbtJogImpl();
}

FbtJog::~FbtJog() {
    if (pImpl != nullptr) {
        delete static_cast<FbtJogImpl*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtJog::runCycle() {
    FbtJogImpl* impl = static_cast<FbtJogImpl*>(pImpl);
    if (impl == nullptr) return;
    impl->runCycle(this);
}

#endif

}  // namespace zmc
