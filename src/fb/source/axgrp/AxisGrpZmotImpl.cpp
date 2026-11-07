#include "AxisGrpZmotImpl.h"

#include "impl/fbt_impl_enum_def.h"

#include "axis/fbt_axis_opaque.h"

namespace zmc {
void AxisEnhanced::onGroupError() {
    if (!axisRef) return;
    axisRequestSwError(axisRef, ZMC_ERR_AXISGRP_ERRORSTOP);
}

void AxisEnhanced::bindAxis(AXIS_REF ax) {
    axisRef = ax;
    fbFollowPosVel.dwValueMask = 0x7;
    fbFollowPosVel.Axis = ax;
    fbHalt.Axis = ax;
    fbRdStatus.Axis = ax;
    fbRdSetValues.Axis = ax;
    fbRdActValues.Axis = ax;
    fbRdStatus.Enable = true;
    fbRdSetValues.Source = ZMC_VALUESELECT_SAVE;
    fbRdSetValues.Enable = true;
    fbRdActValues.Enable = true;
}

void AxisEnhanced::unbindAxis() {
    axisRef = nullptr;
    fbFollowPosVel.Axis = nullptr;
    fbFollowPosVel.bExecute = false;
    fbHalt.Axis = nullptr;
    fbHalt.Execute = false;
    fbRdStatus.Axis = nullptr;
    fbRdStatus.Enable = false;
    fbRdSetValues.Axis = nullptr;
    fbRdSetValues.Enable = false;
    fbRdActValues.Axis = nullptr;
    fbRdActValues.Enable = false;
}

void AxisEnhanced::takeBindingFromNextSlot(const AxisEnhanced& nextSlot) {
    if (!nextSlot.axisRef) {
        unbindAxis();
        return;
    }
    bindAxis(nextSlot.axisRef);
    fbFollowPosVel.fSetPosition = nextSlot.fbFollowPosVel.fSetPosition;
    fbFollowPosVel.fSetVelocity = nextSlot.fbFollowPosVel.fSetVelocity;
    fbFollowPosVel.fSetAcceleration = nextSlot.fbFollowPosVel.fSetAcceleration;
    fbFollowPosVel.bExecute = false;
}

void AxisEnhanced::writePosVel(double pos, double vel, double acc) {
    fbFollowPosVel.fSetPosition = pos;
    fbFollowPosVel.fSetVelocity = vel;
    fbFollowPosVel.fSetAcceleration = acc;
}

void AxisEnhanced::followRunCycle() {
    if (!axisRef) return;

    fbFollowPosVel.runCycle();
}

void AxisEnhanced::triggerHalt(double dec) {
    if (!axisRef) return;
    fbHalt.Deceleration = dec;
    fbHalt.Execute = true;
}

void AxisEnhanced::haltRunCycle() {
    if (!axisRef) return;
    fbHalt.runCycle();
    if (fbHalt.Done || fbHalt.Error) {
        fbHalt.Execute = false;
    }
}

int AxisGrpZmotInner::addAxis(AXIS_REF ax) {
    if (axNum >= kZmotMaxGroupAxes) {
        return ZMC_ERR_AXISGRP_FULL;
    }
    if (groupState != Disabled) {
        return ZMC_ERR_AXISGRP_INVALID_STATE;
    }
    axes[axNum].bindAxis(ax);
    axes[axNum].readCycle();
    fSetPos[axNum] = axes[axNum].fbRdSetValues.Position;
    targetVel[axNum] = axes[axNum].fbRdSetValues.Velocity;
    targetAcc[axNum] = axes[axNum].fbRdSetValues.Acceleration;
    ++axNum;

    if (axNum == 1) {
        cycleTime = axisGetCycleTime(axes[0].axisRef);
    } else {
        // check cycleTime equal?
    }
    return 0;
}
int AxisGrpZmotInner::ungroupAll() {
    if (groupState != Disabled) return ZMC_ERR_AXISGRP_INVALID_STATE;
    axNum = 0;
    for (auto& axis : axes) {
        axis.unbindAxis();
    }
    groupState = Disabled;
    errorId = 0;
    return 0;
}

int AxisGrpZmotInner::removeAxis(AXIS_REF ax) {
    if (!ax) return ZMC_AXIS_IS_NO_AXIS_REF;
    if (groupState != Disabled) return ZMC_ERR_AXISGRP_INVALID_STATE;

    for (int i = 0; i < axNum; ++i) {
        if (axes[i].axisRef == ax) {
            for (int j = i; j < axNum - 1; ++j) {
                fSetPos[j] = fSetPos[j + 1];
                targetVel[j] = targetVel[j + 1];
                targetAcc[j] = targetAcc[j + 1];
                axes[j].takeBindingFromNextSlot(axes[j + 1]);
            }
            --axNum;
            axes[axNum].unbindAxis();
            fSetPos[axNum] = 0.0;
            targetVel[axNum] = 0.0;
            targetAcc[axNum] = 0.0;
            return 0;
        }
    }
    return ZMC_ERR_AXISGRP_AXIS_NOT_FOUND;
}

// The errors of individual axes always direct the axis group to the GroupErrorStop state.
// If the axis group switches to GroupMoving, then all axes are switched to SynchronizedMotion.
// If the axis group switches from GroupMoving to GroupStandby, then all axes are switched to
// standstill. If the axis group switches from GroupMoving to GroupErrorStop, then all axes are
// switched to GroupErrorStop. If the axis group is in GroupStandby, then the individual axes are
// not necessarily all in standstill because they can be controlled by means of single-axis motion
// function blocks such as MC_Jog. If motion is terminated with an error, then all buffered
// subsequent movements are aborted with CommandAborted. As long as the axis group follows a
// dynamic coordinate system, it will stay in GroupMoving. The axis group is in GroupMoving if and
// only if the group is moved in a coordinated manner (by one of the motion blocks from Part 4).
// Switching from GroupMoving to GroupStandby is done one cycle after the last position change.

void AxisGrpZmotInner::runCycle() {
    for (int i = 0; i < axNum; ++i) {
        axes[i].readCycle();
    }

    if (groupState != ErrorStop) {
        for (int i = 0; i < axNum; ++i) {
            if (axes[i].isAxisErrorStop()) {
                groupState = ErrorStop;
                break;
            }
        }
    }

    switch (groupState) {
        case Homing:
        case Stopping:
        case Moving:
            for (int i = 0; i < axNum; ++i) {
                axes[i].writePosVel(fSetPos[i], targetVel[i], targetAcc[i]);
                axes[i].fbFollowPosVel.bExecute = true;
                axes[i].fbHalt.Execute = false;
            }
            break;
        case Standby:
            if (groupStateBak == Moving || groupStateBak == Homing || groupStateBak == Stopping) {
                for (int i = 0; i < axNum; ++i) {
                    axes[i].writePosVel(fSetPos[i], targetVel[i], targetAcc[i]);
                    axes[i].fbFollowPosVel.bExecute = false;
                }
            } else {
                for (int i = 0; i < axNum; ++i) {
                    axes[i].fbFollowPosVel.bExecute = false;
                    axes[i].triggerHalt(fastDecel);
                }
            }
            break;
        case Disabled: {
            if (groupStateBak != Disabled) {
                for (int i = 0; i < axNum; ++i) {
                    axes[i].fbHalt.Execute = false;
                    axes[i].fbFollowPosVel.bExecute = false;
                }
            }
            break;
        }
        case ErrorStop:
            if (groupStateBak != ErrorStop) {
                for (int i = 0; i < axNum; ++i) {
                    axes[i].fbFollowPosVel.bExecute = false;
                    axes[i].fbHalt.Execute = false;
                    axes[i].onGroupError();
                }
            }
            break;
        default:
            break;
    }
    for (int i = 0; i < axNum; ++i) {
        axes[i].haltRunCycle();
        axes[i].followRunCycle();
    }

    for (int i = 0; i < axNum; ++i) {
        fSavePos[i] = fSetPos[i];
    }
    groupStateBak = groupState;
    tick++;
}
}  // namespace zmc