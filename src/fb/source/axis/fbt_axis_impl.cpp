#include <stdio.h>

#include <cmath>
#include <new>

#include "common/misc/un_int64.hpp"
#include "fbt_axis_impl.h"
#include "fbt_axis_impl_cfg.hpp"

#include "fbt_fb_base.hpp"
#include "limits.h"
#include "vel_stop_online.hpp"
#include "AxisMixChecker.h"
#include "motion_node_lookahead_list.h"
#include "ServoAdaptorBase.hpp"
#include "AxVerboseFlag.h"

#ifndef __INT32_MAX__
#define __INT32_MAX__ LONG_MAX  // _I32_MAX
#endif
namespace zmc {

inline void axis_use_new_pva(AxisImpl& ax) {
    ax.diSetPosition = ax.fPos2diPos(ax.fSetPosition);
    ax.diSetVelocity = ax.fVel2diVel(ax.fSetVelocity);
    ax.diSetAcceleration = ax.fVel2diVel(ax.fSetAcceleration);
}

template <typename T = ServoAdapterBase>
void AxisInnerContent_init(AxisInnerContent* pContent) {
    pContent->pServoAdapter = new T;
}

template <typename T = ServoAdapterBase>
void AxisInnerContent_deinit(AxisInnerContent* pContent) {
    delete pContent->pServoAdapter;
    pContent->pServoAdapter = nullptr;
}

void AxisImplBaseWithLog::setLogger(std::unique_ptr<AxLogger> logger) {
    ((AxisInnerContent*)native)->logger = std::move(logger);
}
AxLogger* AxisImplBaseWithLog::getLogger() { return ((AxisInnerContent*)native)->logger.get(); }

uint8_t AxisImplBaseWithLog::is_ready_to_print() const {
    auto& _cfg = ((AxisInnerContent*)native)->axisCfg;
    if (_cfg.debug.level && (((m_cycleCounter + 1) % _cfg.debug.modulo) == 0))
        return _cfg.debug.level;
    else
        return 0;
}

AxisImpl::AxisImpl() {
    auto pContent = new AxisInnerContent();
    AxisInnerContent_init<ServoAdapterBase>(pContent);
    native = pContent;
}

AxisImpl::~AxisImpl() {
    AxisInnerContent_deinit<ServoAdapterBase>((AxisInnerContent*)native);
    delete (AxisInnerContent*)native;
    native = nullptr;
    // pNode.clear();
}

FbStatEnum AxisImpl::cur_fb_stat() const {
    if (pNode.isEmpty())
        return FB_STAT_IDLE;
    else
        return pNode.current()->fbStat;
}

FbStatEnum AxisImpl::prev_fb_stat() const { return fbBak ? fbBak->fbStat : FB_STAT_IDLE; }

// double unwrap(double pos, int cnt){
//     return pos + cnt * __INT32_MAX__ * 2.0;
// }

// inline double wrap(double pos, int cnt){
//     return pos - cnt * __INT32_MAX__ * 2.0;
// }
// inline double fixOverFlow(double x, int& cnt)
// {
//     x -= cnt * __INT32_MAX__ * 2.0;
//     if (x >= __INT32_MAX__){
//         x -= __INT32_MAX__ * 2.0;
//         cnt += 1;
//     }else if (x < - __INT32_MAX__){
//         x += __INT32_MAX__ * 2.0;
//         cnt -= 1;
//     }
//     return x;
// }

int AxisImpl::fPos2diPos(double pos) {
    // return fSetPos2diSetPos(pos, iTurn);
    auto ret = fSetPos2diSetPos(fSetPosition, iTurn, fPosRes);
    // printf("%d,%f\n", m_cycleCounter, fPosRes);
    return ret;
}

double AxisImpl::diPos2fPos(int pos) {
    return diActPos2fActPos(pos, iTurn) + scale_di2set(getPosRes());
}

// diActVelocity -> fActVelocity
// diActAcceleration -> fActAcceleration
double AxisImpl::diVel2fVel(int vel) const {
    return ((double)vel) / iRatioTechUnitsNum * dwRatioTechUnitsDenom;
}

// fSetVelocity -> diSetVelocity
// fSetAcceleration -> diSetAcceleration
int AxisImpl::fVel2diVel(double vel) const {
    return (int)round((vel * iRatioTechUnitsNum / dwRatioTechUnitsDenom));
}

// fSetPosition -> liSetPosition -> diSetPosition
// drop high 32 bits
int32_t AxisImpl::fSetPos2diSetPos(double x, int& cnt) const {
    double pos2 = (x - fOffsetPosition) * iRatioTechUnitsNum / dwRatioTechUnitsDenom;
    un_I64 pos3 = {(int64_t)round(pos2)};
    return pos3.iLow;
}

int32_t AxisImpl::fSetPos2diSetPos(double x, int& cnt, float& res) const {
    double pos2 = (x - fOffsetPosition) * iRatioTechUnitsNum / dwRatioTechUnitsDenom;
    res = pos2 - round(pos2);
    un_I64 pos3 = {(int64_t)round(pos2)};
    return pos3.iLow;
}

// diActPosition -> liActPosition -> fActPosition
// guess & reconstruct high 32 bits
double AxisImpl::diActPos2fActPos(int32_t pos, int& cnt) const {
    un_I64 unp;
    unp.iHigh = cnt;
    unp.iLow = dwLastPosition;
    unp.update(pos);
    cnt = unp.iHigh;
    return ((double)(unp.val)) / iRatioTechUnitsNum * dwRatioTechUnitsDenom + fOffsetPosition;
}

void AxisImpl::bind_servo_adapter(ServoAdapterBase* pServoAdapter) {
    ((AxisInnerContent*)native)->pServoAdapter = pServoAdapter;
}

void AxisImpl::reset_runtime() {
    static_cast<AxisImplCore*>(this)->reset();

    const unsigned kPvaResetFieldCount = 15u;
    std::memset(&fSetPosition, 0, sizeof(fSetPosition) * kPvaResetFieldCount);
    fFactorVel = 1;
    fFactorAcc = 1;
    fFactorJerk = 1;

    fbBak = nullptr;
    pNode.clear();
    pNode.pSuperimposedNode = nullptr;
}

void AxisImpl::init_position(int pos, int vel) {
    int _ = 0;
    diActPosition = pos;
    diSetPosition = pos;
    fActPosition = diActPos2fActPos(pos, _);
    fSetPosition = fActPosition;
    // fSavePosition = fActPosition;
    // fAimPosition = fActPosition;

    diSetVelocity = vel;
    diActVelocity = vel;
    fActVelocity = diVel2fVel(vel);
    fSetVelocity = fActVelocity;
    // fSaveVelocity = fActVelocity;
}

void AxisImpl::print(int verboseFlag) const {
    printAxisVerbose((AxVerboseFlag)verboseFlag, *this);
}

bool AxisImpl::checkLag() {
    auto errCode = axisCheckLag(*this);
    if (errCode) {
        _onLagError();
        return false;
    }
    return true;
}

bool AxisImpl::checkNewPva() {
    auto errCode = axisCheckPosVelAccLimit(*this);
    if (errCode) {
        _onSwError(errCode);
        return false;
    }
    return true;
}

void AxisImpl::runCycle() {
    _runCycle();
    runCycleLog();
    _updateState();
}

void AxisImpl::_calcCmd() {
    if (bDriveStart && !bRegulatorOn) {
        if (cmd != AXIS_CMD_POWEROFF && cmd != AXIS_CMD_RESET) cmd = AXIS_CMD_QUICKSTOP;
    } else if (!bDriveStart) {
        if (cmd != AXIS_CMD_RESET) cmd = AXIS_CMD_POWEROFF;
    } else if (bRegulatorOn && bDriveStart) {
        if (cmd == AXIS_CMD_NOP) cmd = AXIS_CMD_POWERON;
    }
    // hotfix for byOpMode==0
    if (cmd >= AXIS_CMD_POWERON && byRealControllerMode == E_OP_MODE_TO_CTRL_MODE)
        cmd = AXIS_CMD_POWEROFF;
}

// Identifies the temporary power transition owned by the active Stop node.
static bool isStopPowerTransition(const AxisImpl& ax) {
    if (ax.axisStat != mcStopping || ax.pNode.isEmpty()) return false;
    const auto& node = ax.pNode.currentNode();
    if (node.fbType != FB_TYPE_STOP || node.hasVelPlan) return false;
    return ax.cmd == AXIS_CMD_POWEROFF || ax.cmd == AXIS_CMD_POWERON;
}

// Function name changed to reflect backup and update of axis state
void AxisImpl::_updateState() {
    switch (byPosCmdCur) {
        case E_AXIS_USE_CONST_POS:
            if (byPosCmdCur == byPosCmdBak && m_cycleCounter > 2)
                _use_save_pos();
            else
                _use_last_position();
            break;
        case E_AXIS_USE_SET_PVA:
            axis_use_new_pva(*this);
            break;
        case E_AXIS_USE_ACT_PVA:
            _use_last_pva();
        default:
            break;
    };

    _updateMotionstat();
    updateAxisStateDetail();

    cmdBak = cmd;
    // Backup current set values and status
    fSavePosition = fSetPosition;
    fSaveVelocity = fSetVelocity;
    fSaveAcceleration = fSetAcceleration;
    axisStatBak = axisStat;
    dwLastPosition = diActPosition;

    byPosCmdBak = byPosCmdCur;

    if (isPrevPowerOn() && !isPowerOn()) {
        // count consequent powerOff status
        byCount = 1;
    } else if (!isPowerOn()) {
        if (byCount > 0 && byCount != 0xff && (m_cycleCounter % 8 == 0)) byCount += 1;
    } else {
        byCount = 0;
    }
    // Update moving history based on previous axis status
    byMovingHistory <<= 1;  // Shift history to make space for the new status
    if (axisStatBak == mcDiscreteMotion || axisStatBak == mcContinuousMotion ||
        axisStatBak == mcSynchronizedMotion || axisStatBak == mcStopping) {
        byMovingHistory |= 1;  // Set LSB if axis was in a user-commanded motion state
    }
    bDriveErrorBak = driveError;
    bAxisSwErrorBak = axisSwError;
    bCommunicationErrorBak = communicationError;
    bRegulatorRealStateBak = regulatorRealState;
    bDriveStartRealStateBak = driveStartRealState;

    // Update error and control flags
    bError = driveError | axisSwError | communicationError | notOperationalError;
    bSetValuesModifiedByMoveSuperimposed = false;
    bControlAxis = false;
    bClearAxError = false;
    byResTimeNumer = 0;
    byMotbufEvent = 0;
    // Increment cycle counter
    m_cycleCounter++;
}

void AxisImpl::_runCycle() {
    fbBak = pNode.current();

    auto pdoEnFlag = ((AxisInnerContent*)native)->axisCfg.pdo_flag;
    uint16_t mask = (pdoEnFlag.enable_actual_postion) | (pdoEnFlag.enable_actual_velocity << 1) |
                    (pdoEnFlag.enable_actual_torque << 3);
    if (!bDeaf) {
        if (m_cycleCounter <= 1) {
            pushActPosVelAccAtFirst(diActPosition, diActVelocity, diActAcceleration, mask);
        } else {
            pushActPosVelAcc(diActPosition, diActVelocity, diActAcceleration, mask);
        }
    }

    usiSWEndSwitchState = axisCheckActSwLimit(*this);
    // Communication, drive, software, and lag errors have distinct reactions.
    if (axisStat != mcErrorStop) {
        if (communicationError || notOperationalError) {
            axisStat = mcErrorStop;
            if (!dwErrorID) dwErrorID = getCommunicationErrorId();
        } else if (driveError) {
            axisStat = mcErrorStop;
            dwErrorID = ZMC_ERR_P_WHEN_DRIVE_ERROR;
        } else if (axisSwError) {
            axisStat = mcErrorStop;
            if (!dwErrorID) dwErrorID = ZMC_ERR_UNKNOWN_SW_ERROR;
        } else if (!checkLag()) {
            axisStat = mcErrorStop;
            dwErrorID = ZMC_DI_POSITIONLAGERROR;
        }
    }
    _calcCmd();

    // lookahead method
    MotionNodeLookaheadList& lookaheadList = (MotionNodeLookaheadList&)pNode;
    MotionNodeLookaheadParam param = {byMotbufEvent, fSavePosition, fSaveVelocity};
    lookaheadList.runCycle(param);

    // main switch
    // case: motion to errorStop
    // case: motion begin
    // case; motion end
    switch (axisStat) {
        case mcStandstill: {
            if (axisStatBak == mcDisabled || axisStatBak == mcHoming ||
                (axisStatBak == mcErrorStop && !isPrevPowerOn())) {
                // _use_last_position();
                byPosCmdCur = E_AXIS_USE_CONST_POS;
            } else {
                if (byMovingHistory & 0xff) {
                    if (!checkNewPva()) {
                        return _onError();
                    }
                }
                byPosCmdCur = E_AXIS_USE_SET_PVA;
                // _use_save_pos();
            }

        } break;
        case mcHoming: {
            if (!pNode.isEmpty() && pNode.currentNode().fbType == FB_TYPE_HOMING) {
                byPosCmdCur = E_AXIS_USE_SET_PVA;
            } else {
                byPosCmdCur = E_AXIS_USE_ACT_PVA;
            }
        } break;
        case mcDiscreteMotion:
        case mcContinuousMotion:
        case mcSynchronizedMotion: {
            if (!regulatorRealState) {
                if (!pNode.isEmpty()) pNode.clearFbByAxisStat(mcDisabled);
                // pNode.current()->sendError(ZMC_ERR_POWEROFF_WHEN_MOVING);
                return _onSwError(ZMC_ERR_POWEROFF_WHEN_MOVING);
            }
            if (pNode.isEmpty()) {
                if (axisStat == mcContinuousMotion) {
                    _onConstantVelocityCoasting();
                    return axis_use_new_pva(*this);
                } else {
                    return _onSwError(ZMC_FB_WASNT_CALLED_DURING_MOTION);
                }
            }

            auto curFB = pNode.current();
            if (curFB->isError()) {
                return _onSwError(curFB->error_code);
            } else if (!curFB->isActive() || !curFB->isInCycle(m_cycleCounter)) {
                curFB->sendError(ZMC_FB_WASNT_CALLED_DURING_MOTION);
                return _onSwError(ZMC_FB_WASNT_CALLED_DURING_MOTION);
            } else if (curFB->isActive() && byRealControllerMode != ZMC_position) {
                curFB->sendError(ZMC_WRONG_CONTROLLER_MODE);
                return _onSwError(ZMC_WRONG_CONTROLLER_MODE);
            }
            if (!checkNewPva()) {
                return _onError();
            }
            byPosCmdCur = E_AXIS_USE_SET_PVA;
        }; break;
        case mcStopping:
            if (isAxisQuickstopping(*this)) {
                cmd = AXIS_CMD_QUICKSTOP;
                byPosCmdCur = E_AXIS_USE_ACT_PVA;
            } else if (isStopPowerTransition(*this)) {
                // Preserve the Stop-owned power command and track feedback before CSP takes over.
                byPosCmdCur = E_AXIS_USE_ACT_PVA;
            } else if (!regulatorRealState) {
                cmd = AXIS_CMD_QUICKSTOP;
                byPosCmdCur = E_AXIS_USE_ACT_PVA;
            } else {
                // axis_use_new_pva(*this);
                byPosCmdCur = E_AXIS_USE_SET_PVA;
            }
            break;
        case mcErrorStop: {
            _onError();
            onResetError();
        } break;
        case mcDisabled:
        default: {
            if (cmd >= AXIS_CMD_POWERON || isPowerOn()) {
                byPosCmdCur = E_AXIS_USE_CONST_POS;
            } else {
                // if (mcStandstill== axisStatBak)
                // _use_last_position();
                if (byCount > 0 && byCount < ZMC_AXIS_POWEROFF_COUNT) {
                    // axis_use_new_pva(*this);
                    byPosCmdCur = E_AXIS_USE_CONST_POS;
                } else {
                    byPosCmdCur = E_AXIS_USE_ACT_PVA;
                }
            }
        } break;
    }
}

void AxisImpl::_use_last_pva() {
    fSetPosition = fActPosition;
    fSetVelocity = fActVelocity;
    fSetAcceleration = fActAcceleration;
    fSetTorque = fActTorque;

    diSetPosition = diActPosition;
    diSetVelocity = diActVelocity;
    diSetAcceleration = diActAcceleration;
    diSetTorque = diActTorque;
}

void AxisImpl::_use_save_pos() {
    fSetPosition = fSavePosition;
    fSetVelocity = 0;
    fSetAcceleration = 0;
    fSetTorque = fActTorque;

    // diSetPosition = diActPosition;
    diSetVelocity = 0;
    diSetAcceleration = 0;
    diSetTorque = diActTorque;
}

void AxisImpl::_use_last_position() {
    fSetPosition = fActPosition;
    fSetVelocity = 0;
    fSetAcceleration = 0;
    fSetTorque = fActTorque;

    diSetPosition = diActPosition;
    diSetVelocity = 0;
    diSetAcceleration = 0;
    diSetTorque = diActTorque;
}

void AxisImpl::_updateMotionstat() {
#define EPS (1e-6)
    nDirection = fSetVelocity > EPS ? 1 : (fSetVelocity < -EPS ? -1 : nDirection);
    Accelerating = fSetAcceleration > EPS;
    Decelerating = fSetAcceleration < -EPS;
    ConstantVelocity = !Accelerating && !Decelerating;  // mcPositiveDirection
}

void AxisImpl::pushActPosVelAcc(int diPosNew, int diVelNew, int diAccNew, int flag) {
    switch (flag) {
        case 1: {
            double posNew = diPos2fPos(diPosNew);
            double velNew = (posNew - fActPosition) / cycleTime;
            fActAcceleration = (velNew - fActVelocity) / cycleTime;
            fActVelocity = velNew;
            fActPosition = posNew;
        } break;
        case 2: {
            double velNew = diVel2fVel(diActVelocity);
            fActAcceleration = (velNew - fActVelocity) / cycleTime;
            fActPosition += (fActVelocity + velNew) * cycleTime / 2;
            fActVelocity = velNew;
        } break;
        case 3: {
            double posNew = diPos2fPos(diPosNew);
            double velNew = diVel2fVel(diActVelocity);
            fActAcceleration = (velNew - fActVelocity) / cycleTime;
            fActVelocity = velNew;
            fActPosition = posNew;
        } break;
        case 4: {
            double accNew = diVel2fVel(diAccNew);
            double velNew = fActVelocity + fActAcceleration * cycleTime;
            fActPosition +=
                (fActVelocity + velNew) * cycleTime / 2 + accNew * cycleTime * cycleTime / 2;
            fActAcceleration = accNew;
            fActVelocity = velNew;
        } break;
        case 7: {
            fActAcceleration = diVel2fVel(diAccNew);
            fActVelocity = diVel2fVel(diVelNew);
            fActPosition = diPos2fPos(diPosNew);
        } break;
        default:
            break;
    }
}

void AxisImpl::pushActPosVelAccAtFirst(int diPosNew, int diVelNew, int diAccNew, int flag) {
    switch (flag) {
        case 1: {
            double posNew = diPos2fPos(diPosNew);
            double velNew = 0;
            fActAcceleration = 0;
            fActVelocity = 0;
            fActPosition = posNew;
        } break;
        case 2: {
            double velNew = diVel2fVel(diActVelocity);
            fActAcceleration = 0;
            fActPosition += (fActVelocity + velNew) * cycleTime / 2;
            fActVelocity = velNew;
        } break;
        case 3: {
            double posNew = diPos2fPos(diPosNew);
            double velNew = diVel2fVel(diActVelocity);
            fActAcceleration = 0;
            fActVelocity = velNew;
            fActPosition = posNew;
        } break;
        case 4: {
            double accNew = diVel2fVel(diAccNew);
            double velNew = fActVelocity + fActAcceleration * cycleTime;
            fActPosition +=
                (fActVelocity + velNew) * cycleTime / 2 + accNew * cycleTime * cycleTime / 2;
            fActAcceleration = accNew;
            fActVelocity = velNew;
        } break;
        case 7: {
            fActAcceleration = diVel2fVel(diAccNew);
            fActVelocity = diVel2fVel(diVelNew);
            fActPosition = diPos2fPos(diPosNew);
        } break;
        default:
            break;
    }
}

void AxisImpl::onResetError() {
    if ((cmd == AXIS_CMD_RESET || bClearAxError) && fabs(fSetVelocity) < 1e-6 &&
        fabs(fSetAcceleration) < 1e-6) {
        pNode.clearFbByAxisStat(mcErrorStop);
        if (bClearAxError) {
            if (axisSwError) axisSwError = false;
            if (dwErrorID) dwErrorID = ZMC_NO_ERROR;
        }
    }
}
void AxisImpl::_onError() {
    if (getCommunicationErrorId() != ZMC_NO_ERROR) {
        _onCommunicationError();
    } else if (driveError) {
        _onDriveError();
    } else
        _onSwErrorstop();
}

void AxisImpl::_onCommunicationError() {
    // Do not consume actual PVA while EtherCAT feedback is not trustworthy.
    fSetPosition = fSavePosition;
    fSetVelocity = 0.0;
    fSetAcceleration = 0.0;
    byPosCmdCur = E_AXIS_USE_SET_PVA;
    cmd = AXIS_CMD_POWEROFF;
}

void AxisImpl::_onDriveError() {
    byPosCmdCur = E_AXIS_USE_ACT_PVA;
    // _use_last_position();
}

#define MAX(a, b) ((a) >= (b) ? (a) : (b))
void AxisImpl::_onSwErrorstop() {
    auto pva = PosVelAcc{fSavePosition, fSaveVelocity, fSaveAcceleration};
    double Deceleration = fSWLimitDeceleration > 0 ? fSWLimitDeceleration : fSWMaxDeceleration;
    if (fSaveAcceleration > 0 && fSaveVelocity < 0)
        Deceleration = MAX(Deceleration, fSaveAcceleration);
    else if (fSaveAcceleration < 0 && fSaveVelocity > 0)
        Deceleration = MAX(Deceleration, -fSaveAcceleration);
    if (fSWErrorMaxDistance > 0) {
        double dec0 = fSaveVelocity * fSaveVelocity / 2 / fSWErrorMaxDistance;
        Deceleration = MAX(Deceleration, dec0);
    }
    auto pva2 = calc_next_decel_pva(pva, Deceleration, cycleTime);
    fSetPosition = pva2.pos;
    fSetVelocity = pva2.vel;
    fSetAcceleration = pva2.acc;

    // axis_use_new_pva(*this);
    byPosCmdCur = E_AXIS_USE_SET_PVA;
}

void AxisImpl::_onSwError(uint16_t error_id) {
    axisSwError = true;
    dwErrorID = error_id;
    axisStat = mcErrorStop;
    _onSwErrorstop();
}
void AxisImpl::_onLagError() {
    axisSwError = true;
    dwErrorID = ZMC_DI_POSITIONLAGERROR;
    axisStat = mcErrorStop;
    if (eCheckPositionLag == POS_LAG_SUPER_STAY_ENABLED)
        _onSwErrorstop();
    else if (eCheckPositionLag == POS_LAG_SUPER_DISABLE_DRIVE) {
        bDriveStart = false;
        bRegulatorOn = false;
        cmd = AXIS_CMD_POWEROFF;
        // byPosCmdCur = E_AXIS_USE_CONST_POS;
    } else if (eCheckPositionLag == POS_LAG_SUPER_DO_QUICK_STOP) {
        bDriveStart = false;
        cmd = AXIS_CMD_QUICKSTOP;
        // byPosCmdCur = E_AXIS_USE_ACT_PVA;
    }
}
void AxisImpl::_onConstantVelocityCoasting() {
    auto pva2 = PosVelAcc{fSavePosition + fSaveVelocity * cycleTime, fSaveVelocity, 0};
    fSetPosition = pva2.pos;
    fSetVelocity = pva2.vel;
    fSetAcceleration = pva2.acc;
}
// uint16_t get_fb_stat(FbBase* fb) { return (uint16_t)((implPtr*)fb)->ptr->_fbOStat.ostat; }
}  // namespace zmc
