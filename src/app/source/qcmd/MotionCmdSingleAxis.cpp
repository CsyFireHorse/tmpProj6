#include "qcmd/MotionCmdSingleAxis.hpp"

#include "common/utils/double2frac.hpp"
#include "fbmEnum.h"
#include "fbt_errorcode_def.h"

namespace zmc {

namespace {

inline int motcmdRet(MC_ERROR_CODE code) { return static_cast<int>(code); }

}  // namespace

// Select the command target immediately before appending its single-axis operation.
int MotionCmdSingleAxis::appendAxisItem(int axis, uint16_t type, const void* param,
                                       uint16_t paramLen) {
    const int ret = setAxis(axis);
    return ret != 0 ? ret : appendOrDeferItem(type, param, paramLen);
}

// Axis enable (priority API: same class as powerOn / powerOff)
int MotionCmdSingleAxis::axisEnable(int axis, bool enable) {
    double val = enable ? 1.0 : 0.0;
    const int ret = setAxis(axis);
    return ret != 0 ? ret : priorityAppendItem(FBM_POWER, &val, sizeof(double));
}

// Configuration commands
int MotionCmdSingleAxis::setUnitsNumDen(int axis, int num, int den) {
    NumDenUnion numDen = {{num, den}};
    return appendAxisItem(axis, FBM_SET_NUM_DEN, &numDen, sizeof(NumDenUnion));
}

int MotionCmdSingleAxis::setSpeed(int axis, double speed) {
    return appendAxisItem(axis, FBM_SET_SPEED, &speed, sizeof(double));
}

int MotionCmdSingleAxis::setAccel(int axis, double accel) {
    return appendAxisItem(axis, FBM_SET_ACCEL, &accel, sizeof(double));
}

int MotionCmdSingleAxis::setDecel(int axis, double decel) {
    return appendAxisItem(axis, FBM_SET_DECEL, &decel, sizeof(double));
}

int MotionCmdSingleAxis::setJerk(int axis, double jerk) {
    return appendAxisItem(axis, FBM_SET_JERK, &jerk, sizeof(double));
}

int MotionCmdSingleAxis::setFastDecel(int axis, double fastDecel) {
    return appendAxisItem(axis, FBM_SET_FAST_DECEL, &fastDecel, sizeof(double));
}

int MotionCmdSingleAxis::setSpeedRatio(int axis, double ratio) {
    if (!(ratio > 0.0 && ratio <= 1.0)) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    return appendAxisItem(axis, FBM_SET_SPEED_RATIO, &ratio, sizeof(double));
}

int MotionCmdSingleAxis::setDirection(int axis, int direction) {
    return appendAxisItem(axis, FBM_SET_DIRECTION, &direction, sizeof(int));
}

int MotionCmdSingleAxis::setBufferMode(int axis, int bufferMode) {
    return appendAxisItem(axis, FBM_SET_BUFFER_MODE, &bufferMode, sizeof(int));
}

// Control commands (priority APIs share priorityAppendItem)
int MotionCmdSingleAxis::powerOn(int axis) { return axisEnable(axis, true); }

int MotionCmdSingleAxis::powerOff(int axis) { return axisEnable(axis, false); }

int MotionCmdSingleAxis::stop(int axis) {
    const int ret = setAxis(axis);
    return ret != 0 ? ret : priorityAppendItem(FBM_STOP, nullptr, 0);
}

int MotionCmdSingleAxis::halt(int axis) {
    const int ret = setAxis(axis);
    return ret != 0 ? ret : priorityAppendItem(FBM_HALT, nullptr, 0);
}

int MotionCmdSingleAxis::reset(int axis) { return appendAxisItem(axis, FBM_RESET, nullptr, 0); }

int MotionCmdSingleAxis::setBaseAxis(int axis) {
    int ax = axis;
    return appendOrDeferItem(FBM_BASE, &ax, sizeof(int));
}

int MotionCmdSingleAxis::setAxis(int axis) {
    int ax = axis;
    return appendOrDeferItem(FBM_AXIS, &ax, sizeof(int));
}

int MotionCmdSingleAxis::setMasterAxis(int axis, int masterAxis) {
    return appendAxisItem(axis, FBM_SET_MASTER, &masterAxis, sizeof(int));
}

// Motion commands
int MotionCmdSingleAxis::moveAbs(int axis, double pos) {
    return appendAxisItem(axis, FBM_MOVE_ABSOLUTE, &pos, sizeof(double));
}

int MotionCmdSingleAxis::moveRel(int axis, double distance) {
    return appendAxisItem(axis, FBM_MOVE_RELATIVE, &distance, sizeof(double));
}

int MotionCmdSingleAxis::moveAdd(int axis, double pos) {
    return appendAxisItem(axis, FBM_MOVE_ADDITIVE, &pos, sizeof(double));
}

int MotionCmdSingleAxis::home(int axis, double pos) {
    return appendAxisItem(axis, FBM_HOME, &pos, sizeof(double));
}

int MotionCmdSingleAxis::homeSdo(int axis, const HomeSdoConfig& config) {
    return appendAxisItem(axis, FBM_HOME_SDO, &config, sizeof(config));
}

int MotionCmdSingleAxis::homing(int axis) { return appendAxisItem(axis, FBM_HOMING, nullptr, 0); }

int MotionCmdSingleAxis::gearIn(int axis) { return appendAxisItem(axis, FBM_GEAR_IN, nullptr, 0); }

int MotionCmdSingleAxis::gearOut(int axis) { return appendAxisItem(axis, FBM_GEAR_OUT, nullptr, 0); }

int MotionCmdSingleAxis::setPosition(int axis, double pos) {
    return appendAxisItem(axis, FBM_SET_POSITION, &pos, sizeof(double));
}

int MotionCmdSingleAxis::datum(int axis, int mode) {
    return appendAxisItem(axis, FBM_DATUM, &mode, sizeof(int));
}

int MotionCmdSingleAxis::moveVelocity(int axis, int dir) {
    int direction = (dir > 0) ? 1 : -1;
    return appendAxisItem(axis, FBM_MOVE_VELOCITY, &direction, sizeof(int));
}

int MotionCmdSingleAxis::forward(int axis) { return moveVelocity(axis, 1); }

int MotionCmdSingleAxis::reverse(int axis) { return moveVelocity(axis, -1); }

}  // namespace zmc
