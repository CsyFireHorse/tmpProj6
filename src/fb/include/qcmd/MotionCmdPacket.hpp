#ifndef _ZMC_MOTION_CMD_PACKET_HPP_
#define _ZMC_MOTION_CMD_PACKET_HPP_
// Typed pack on MotionPacket subclass; hides FBM_* from callers.
// No extra fields: sizeof must match MotionPacket wire layout.
#include "common/container/FbCmdQueue.hpp"
#include "motbuf/MotionCmdTypes.hpp"

namespace zmc {

// MotionCmdPacket: append-only typed helpers; does not enqueue or send.
struct MotionCmdPacket : public MotionPacket {
    // Hard full or nearly full (payload_len >= MAX_PAYLOAD_SIZE * 9 / 10).
    bool canAppend(uint16_t param_len) const;

    // SingleAxis-facing pack APIs (same signatures as MotionCmdInterface.hpp).
    int axisEnable(bool enable);
    // Integer num/den ratio setter; mirrors the SingleAxis signature.
    int setUnitsNumDen(int num, int den);
    int setSpeed(double speed);
    int setAccel(double accel);
    int setDecel(double decel);
    int setJerk(double jerk);
    int setFastDecel(double fastDecel);
    int setSpeedRatio(double ratio);
    int setDirection(int val);
    int setBufferMode(int bufferMode);
    int setFlowConfig(const FlowCtrlConfig& cfg);
    int powerOn();
    int powerOff();
    int stop();
    int halt();
    int reset();
    int setBaseAxis(int axis);
    int setAxis(int axis);
    int setMasterAxis(int axis);
    int moveAbs(int pos_len, double* posArr);
    int moveRel(int pos_len, double* posArr);
    int moveAdd(int pos_len, double* posArr);
    int home(double pos = 0.0);
    // Appends a drive-home SDO configuration command without Position data.
    int homeSdo(const HomeSdoConfig& config);
    int homing();
    int gearIn();
    int gearOut();
    int setPosition(double pos);
    int datum(int mode);
    int moveVelocity(int dir);
    int forward();
    int reverse();

    // AxisGroup-facing pack APIs.
    int multiMoveAbs(int pos_len, const double* posArr);
    int multiMoveRel(int pos_len, const double* posArr);
    int groupSelect(int idx);
    int groupSetAxes(int n, const int* axisIdxArr);
    int groupEnable();
    int groupDisable();
    int groupReset();
};

}  // namespace zmc
#endif
