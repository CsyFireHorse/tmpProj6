#include "qcmd/MotionCmdPacket.hpp"

#include "common/utils/double2frac.hpp"
#include "fbmEnum.h"
#include "fbt_errorcode_def.h"

namespace zmc {

namespace {

inline int motcmdRet(MC_ERROR_CODE code) { return static_cast<int>(code); }

// Pack helper: append one FBM item; hide type enum from typed API callers.
inline int appendOrFail(MotionCmdPacket& pkt, uint16_t type, const void* param, uint16_t len) {
    if (!pkt.appendItem(type, param, len)) {
        return motcmdRet(ZMC_ERR_MOTCMD_PACKET_APPEND_FAILED);
    }
    return motcmdRet(ZMC_NO_ERROR);
}

}  // namespace

static_assert(sizeof(MotionCmdPacket) == sizeof(MotionPacket),
              "MotionCmdPacket must not add fields beyond MotionPacket");

bool MotionCmdPacket::canAppend(uint16_t param_len) const {
    // Nearly full: stop merging (same 90% rule as legacy autoFlushPending).
    if (payload_len >= MAX_PAYLOAD_SIZE * 9 / 10) {
        return false;
    }
    const size_t need = sizeof(uint16_t) + sizeof(uint16_t) + param_len;
    return payload_len + need <= MAX_PAYLOAD_SIZE;
}

int MotionCmdPacket::axisEnable(bool enable) {
    double val = enable ? 1.0 : 0.0;
    return appendOrFail(*this, FBM_POWER, &val, sizeof(double));
}

int MotionCmdPacket::setUnitsNumDen(int num, int den) {
    NumDenUnion numDen = {{num, den}};
    return appendOrFail(*this, FBM_SET_NUM_DEN, &numDen, sizeof(NumDenUnion));
}

int MotionCmdPacket::setSpeed(double speed) {
    return appendOrFail(*this, FBM_SET_SPEED, &speed, sizeof(double));
}

int MotionCmdPacket::setAccel(double accel) {
    return appendOrFail(*this, FBM_SET_ACCEL, &accel, sizeof(double));
}

int MotionCmdPacket::setDecel(double decel) {
    return appendOrFail(*this, FBM_SET_DECEL, &decel, sizeof(double));
}

int MotionCmdPacket::setJerk(double jerk) {
    return appendOrFail(*this, FBM_SET_JERK, &jerk, sizeof(double));
}

int MotionCmdPacket::setFastDecel(double fastDecel) {
    return appendOrFail(*this, FBM_SET_FAST_DECEL, &fastDecel, sizeof(double));
}

int MotionCmdPacket::setSpeedRatio(double ratio) {
    if (!(ratio > 0.0 && ratio <= 1.0)) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    return appendOrFail(*this, FBM_SET_SPEED_RATIO, &ratio, sizeof(double));
}

int MotionCmdPacket::setDirection(int val) {
    int mode = val;
    return appendOrFail(*this, FBM_SET_DIRECTION, &mode, sizeof(int));
}

int MotionCmdPacket::setBufferMode(int bufferMode) {
    return appendOrFail(*this, FBM_SET_BUFFER_MODE, &bufferMode, sizeof(int));
}

int MotionCmdPacket::setFlowConfig(const FlowCtrlConfig& cfg) {
    FlowCtrlConfig tmp = cfg;
    return appendOrFail(*this, FBM_FLOWCTRL_CFG, &tmp, sizeof(FlowCtrlConfig));
}

int MotionCmdPacket::powerOn() { return axisEnable(true); }

int MotionCmdPacket::powerOff() { return axisEnable(false); }

int MotionCmdPacket::stop() { return appendOrFail(*this, FBM_STOP, nullptr, 0); }

int MotionCmdPacket::halt() { return appendOrFail(*this, FBM_HALT, nullptr, 0); }

int MotionCmdPacket::reset() { return appendOrFail(*this, FBM_RESET, nullptr, 0); }

int MotionCmdPacket::setBaseAxis(int axis) {
    int ax = axis;
    return appendOrFail(*this, FBM_BASE, &ax, sizeof(int));
}

int MotionCmdPacket::setAxis(int axis) {
    int ax = axis;
    return appendOrFail(*this, FBM_AXIS, &ax, sizeof(int));
}

int MotionCmdPacket::setMasterAxis(int axis) {
    int val = axis;
    return appendOrFail(*this, FBM_SET_MASTER, &val, sizeof(int));
}

int MotionCmdPacket::moveAbs(int pos_len, double* posArr) {
    const uint16_t param_size = static_cast<uint16_t>(pos_len * sizeof(double));
    return appendOrFail(*this, FBM_MOVE_ABSOLUTE, posArr, param_size);
}

int MotionCmdPacket::moveRel(int pos_len, double* posArr) {
    const uint16_t param_size = static_cast<uint16_t>(pos_len * sizeof(double));
    return appendOrFail(*this, FBM_MOVE_RELATIVE, posArr, param_size);
}

int MotionCmdPacket::moveAdd(int pos_len, double* posArr) {
    const uint16_t param_size = static_cast<uint16_t>(pos_len * sizeof(double));
    return appendOrFail(*this, FBM_MOVE_ADDITIVE, posArr, param_size);
}

int MotionCmdPacket::home(double pos) {
    return appendOrFail(*this, FBM_HOME, &pos, sizeof(double));
}

int MotionCmdPacket::homeSdo(const HomeSdoConfig& config) {
    return appendOrFail(*this, FBM_HOME_SDO, &config, sizeof(config));
}

int MotionCmdPacket::homing() { return appendOrFail(*this, FBM_HOMING, nullptr, 0); }

int MotionCmdPacket::gearIn() { return appendOrFail(*this, FBM_GEAR_IN, nullptr, 0); }

int MotionCmdPacket::gearOut() { return appendOrFail(*this, FBM_GEAR_OUT, nullptr, 0); }

int MotionCmdPacket::setPosition(double pos) {
    return appendOrFail(*this, FBM_SET_POSITION, &pos, sizeof(double));
}

int MotionCmdPacket::datum(int mode) {
    int m = mode;
    return appendOrFail(*this, FBM_DATUM, &m, sizeof(int));
}

int MotionCmdPacket::moveVelocity(int dir) {
    int direction = (dir > 0) ? 1 : -1;
    return appendOrFail(*this, FBM_MOVE_VELOCITY, &direction, sizeof(int));
}

int MotionCmdPacket::forward() { return moveVelocity(1); }

int MotionCmdPacket::reverse() { return moveVelocity(-1); }

int MotionCmdPacket::multiMoveAbs(int pos_len, const double* posArr) {
    if (pos_len <= 0 || !posArr) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    const uint16_t param_size = static_cast<uint16_t>(static_cast<size_t>(pos_len) * sizeof(double));
    return appendOrFail(*this, FBM_MULTI_MOVE, posArr, param_size);
}

int MotionCmdPacket::multiMoveRel(int pos_len, const double* posArr) {
    if (pos_len <= 0 || !posArr) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    const uint16_t param_size = static_cast<uint16_t>(static_cast<size_t>(pos_len) * sizeof(double));
    return appendOrFail(*this, FBM_MULTI_MOVE_REL, posArr, param_size);
}

int MotionCmdPacket::groupSelect(int idx) {
    return appendOrFail(*this, FBM_GROUP_SELECT, &idx, sizeof(int));
}

int MotionCmdPacket::groupSetAxes(int n, const int* axisIdxArr) {
    if (n <= 0 || !axisIdxArr) {
        return motcmdRet(ZMC_ERR_MOTCMD_INVALID_ARG);
    }
    const uint16_t param_size = static_cast<uint16_t>(static_cast<size_t>(n) * sizeof(int));
    return appendOrFail(*this, FBM_AXIS_GROUP, axisIdxArr, param_size);
}

int MotionCmdPacket::groupEnable() { return appendOrFail(*this, FBM_GROUP_ENABLE, nullptr, 0); }

int MotionCmdPacket::groupDisable() { return appendOrFail(*this, FBM_GROUP_DISABLE, nullptr, 0); }

int MotionCmdPacket::groupReset() { return appendOrFail(*this, FBM_GROUP_RESET, nullptr, 0); }

}  // namespace zmc
