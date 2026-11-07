// FbMoveWrap2 implementation.
// AI modification: GPT-5 Codex
#include "motbuf/FbMoveWrapV3.hpp"

#include "common/utils/double2frac.hpp"
#include "mc_parameter_enum_def.h"

namespace zmc {

AxisWithFb& FbMoveWrap2::selAx() {
    // Return a reference to the currently selected axis based on axSelect.
    return axisMotbuf.axis(axSelect);
}

void FbMoveWrap2::sendUnits(double val) {
    CommandStructDefault cmd;
    cmd.cmdType = FBM_SET_NUM_DEN;

    NumDenUnion numDen = {{1, 1}};
    doubleToFraction(val, numDen.num, numDen.den);
    cmd.setIntByIndex(0, numDen.num);
    cmd.setIntByIndex(1, numDen.den);
    selAx().pushCommand(cmd);
}

void FbMoveWrap2::_initCycleTime() {
    rdPara.Enable = true;
    rdPara.Axis = axisMotbuf.axisRef(axSelect);
    rdPara.ParameterNumber = PARA_fTaskCycle;
    rdPara.runCycle();
    cycleTime = rdPara.Value;
}

void FbMoveWrap2::show_axis(int i) {
    // auto& ax = axRef(i);

    const AxisWithFb& axisFb = axisMotbuf.axis(i);
    auto axStat = axisFb.getAxisState();
    double posAct = axisFb.fbRdActualValues.Position;
    LREAL pos = axisFb.fbRdSetValues.Position;
    LREAL vel = axisFb.fbRdSetValues.Velocity;
    LREAL acc = axisFb.fbRdSetValues.Acceleration;

    printf("%d,%d,%d,%f,%f,%f,%f\n", cnt, i, axStat, posAct, pos, vel, acc);
    // printf("0x%x, 0x%x, %d, %f\n", ctrl, stat, posAct);
}

FbMoveWrap2::FbMoveWrap2(int axNum) : FbMoveWrapWithAxis(axNum) {
    if (axNum > 0) {
        _initCycleTime();
    }
}

int FbMoveWrap2::resize(int axNum) {
    if (axisMotbuf.resize(axNum) != 0) {
        return -1;
    }
    if (axNum > 0) {
        _initCycleTime();
    }
    return 0;
}

void FbMoveWrap2::show_state() {
    for (std::size_t i = 0; i < axisMotbuf.size(); ++i) {
        show_state(static_cast<int>(i));
    }
}

void FbMoveWrap2::show_state(int i) {
    const AxisWithFb& axisFb = axisMotbuf.axis(i);
    auto pos = axisFb.fbRdPowerParam.SetPosition;
    auto mpul = axisFb.fbRdPowerParam.ActPosition;
    auto axStat = axisFb.getAxisState();
    auto errorId = axisFb.fbRdError.AxisErrorID;
    printf("%d,%d,%d,%d,%d,%d\n", cnt, i, axStat, errorId, pos, mpul);
}

void FbMoveWrap2::show_power() {
    for (std::size_t i = 0; i < axisMotbuf.size(); ++i) {
        show_power_sg(static_cast<int>(i));
    }
}

void FbMoveWrap2::show_motbuf() {
    for (std::size_t i = 0; i < axisMotbuf.size(); ++i) {
        show_motbuf_sg(static_cast<int>(i));
    }
}

void FbMoveWrap2::show_motnode() {
    for (std::size_t i = 0; i < axisMotbuf.size(); ++i) {
        show_motnode_sg(static_cast<int>(i));
    }
}

void FbMoveWrap2::show_cmdresult() { printCmdResultInfo(); }

void FbMoveWrap2::show_motbuf_sg(int i) { axisMotbuf.axis(i).printFbListInfo(); }

void FbMoveWrap2::show_motnode_sg(int i) {}

void FbMoveWrap2::show_power_sg(int i) {
    const AxisWithFb& axisFb = axisMotbuf.axis(i);
    auto controlWord = axisFb.fbRdPowerParam.ControlWord;
    auto statusWord = axisFb.fbRdPowerParam.StatusWord;
    auto setPosition = axisFb.fbRdPowerParam.SetPosition;
    auto actPosition = axisFb.fbRdPowerParam.ActPosition;
    printf("%d,%d,0x%x,0x%x,%d,%d\n", cnt, i, controlWord, statusWord, setPosition, actPosition);
}

void FbMoveWrap2::show_mparam() {
    const AxisWithFb& axisFb = axisMotbuf.axis(axSelect);
    printf("%d,%d,%f,%f,%f,%f\n", cnt, axSelect, axisFb.cmdVel, axisFb.cmdAcc, axisFb.cmdDec,
           axisFb.cmdJerk);
}

void FbMoveWrap2::show_home_fb_sg(int i) {}

void FbMoveWrap2::show() {
    for (std::size_t i = 0; i < axisMotbuf.size(); ++i) {
        show_axis(static_cast<int>(i));
    }
}

void FbMoveWrap2::setAxisSelect(int f) {
    if (f < axisMotbuf.size()) axSelect = f;
}

void FbMoveWrap2::setMasterAxis(int f) {
    if (f < axisMotbuf.size()) selAx().axMasterIdx = f;
}

void FbMoveWrap2::setPowerOn() { selAx().setAxisEnable(true); }

void FbMoveWrap2::setPowerOff() { selAx().setAxisEnable(false); }

void FbMoveWrap2::setAxisEnable(bool enable) { selAx().setAxisEnable(enable); }

void FbMoveWrap2::sendReset() {
    CommandStructDefault cmd;
    cmd.cmdType = FBM_RESET;
    selAx().pushCommand(cmd);
}

void FbMoveWrap2::sendResetLow() {}

void FbMoveWrap2::sendFault() {}

void FbMoveWrap2::setBufferMode(int v) { selAx().bufferMode = v; }

void FbMoveWrap2::setDirection(int v) { selAx().direction = v; }

void FbMoveWrap2::setCmdPos(double v) { selAx().cmdPos = v; }

void FbMoveWrap2::setCmdVel(double v) { selAx().cmdVel = v; }

void FbMoveWrap2::setCmdAcc(double v) { selAx().cmdAcc = v; }

void FbMoveWrap2::setCmdDec(double v) { selAx().cmdDec = v; }

void FbMoveWrap2::setCmdJerk(double v) { selAx().cmdJerk = v; }

void FbMoveWrap2::setFastDec(double v) { selAx().fastDec = v; }

void FbMoveWrap2::setCreep(double v) { selAx().creep = v; }

void FbMoveWrap2::sendExecute(int v) {}

void FbMoveWrap2::sendExecuteMoveAbs(bool v) {
    CommandStructDefault cmd;
    cmd.cmdType = FBM_MOVE_ABSOLUTE;
    cmd.setDouble(selAx().cmdPos);
    selAx().pushCommand(cmd);
}

void FbMoveWrap2::sendExecuteMoveRel(bool v) {
    CommandStructDefault cmd;
    cmd.cmdType = FBM_MOVE_RELATIVE;
    cmd.setDouble(selAx().cmdPos);
    selAx().pushCommand(cmd);
}

void FbMoveWrap2::sendExecuteMoveAdd(bool v) {
    CommandStructDefault cmd;
    cmd.cmdType = FBM_MOVE_ADDITIVE;
    cmd.setDouble(selAx().cmdPos);
    selAx().pushCommand(cmd);
}

void FbMoveWrap2::sendExecuteMoveVel(bool v) {
    CommandStructDefault cmd;
    cmd.cmdType = FBM_MOVE_VELOCITY;
    cmd.setInt(selAx().direction);
    selAx().pushCommand(cmd);
}

void FbMoveWrap2::sendAxisLow() {}

void FbMoveWrap2::sendAxisMotLow() {}

void FbMoveWrap2::sendStop() {
    CommandStructDefault cmd;
    cmd.cmdType = FBM_STOP;
    selAx().pushCommand(cmd);
}

void FbMoveWrap2::sendStopLow() {}

void FbMoveWrap2::sendHalt() {
    CommandStructDefault cmd;
    cmd.cmdType = FBM_HALT;
    selAx().pushCommand(cmd);
}

void FbMoveWrap2::sendHaltLow() {}

void FbMoveWrap2::sendHome() {
    CommandStructDefault cmd;
    cmd.cmdType = FBM_HOME;
    selAx().pushCommand(cmd);
}

void FbMoveWrap2::sendHomeLow() {}

void FbMoveWrap2::sendHoming() {
    CommandStructDefault cmd;
    cmd.cmdType = FBM_HOMING;
    selAx().pushCommand(cmd);
}

void FbMoveWrap2::sendSetPos() {
    CommandStructDefault cmd;
    cmd.cmdType = FBM_SET_POSITION;
    cmd.setDouble(selAx().cmdPos);
    selAx().pushCommand(cmd);
}

void FbMoveWrap2::sendGearIn() {
    CommandStructDefault cmd;
    cmd.cmdType = FBM_GEAR_IN;
    selAx().pushCommand(cmd);
}

void FbMoveWrap2::sendGearInLow() {}

void FbMoveWrap2::sendGearOut() {
    CommandStructDefault cmd;
    cmd.cmdType = FBM_GEAR_OUT;
    selAx().pushCommand(cmd);
}

void FbMoveWrap2::sendGearOutLow() {}

double FbMoveWrap2::getCycleTime() const { return cycleTime; }

void FbMoveWrap2::setManualMode(bool v) { isManaulMode = v; }

void FbMoveWrap2::setPauseFlag(bool v) {
    skipFlag = v;
    if (!v) ticksToPause = -1;
}

void FbMoveWrap2::sendSteps(int n) {
    ticksToPause = n;
    skipFlag = false;
}

void FbMoveWrap2::sendTickTo(int n) {
    if (n <= cnt) return;
    ticksToPause = n - cnt;
    skipFlag = false;
}

void FbMoveWrap2::sendTimeTo(double time) {
    double Ts = getCycleTime();
    if (time <= cnt * Ts) return;
    ticksToPause = time / Ts - cnt;
    skipFlag = false;
}

void FbMoveWrap2::sendDelay(double time) {
    double Ts = getCycleTime();
    ticksToPause = time / Ts;
    skipFlag = false;
}

void FbMoveWrap2::sendExitFlag() { exitFlag = true; }

void FbMoveWrap2::preRunCycle() {
    if (isManaulMode) {
        if (ticksToPause >= 0) {
            skipFlag = !(ticksToPause--);
        }
    }
}

void FbMoveWrap2::reset_runtime() {
    FbMoveWrapWithAxis::reset_runtime();
    rdPara.Axis = nullptr;
    rdPara.Enable = false;
    rdPara.ParameterNumber = 0;
    rdPara.Valid = false;
    rdPara.Busy = false;
    rdPara.Error = false;
    rdPara.ErrorID = 0;
    rdPara.Value = 0.0;
    isManaulMode = 0;
    skipFlag = false;
    ticksToPause = -1;
    cycleTime = 0;
}
}  // namespace zmc
