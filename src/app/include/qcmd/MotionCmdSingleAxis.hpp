#ifndef _ZMC_MOTION_CMD_SINGLE_AXIS_HPP_
#define _ZMC_MOTION_CMD_SINGLE_AXIS_HPP_

#include "motbuf/MotionCmdTypes.hpp"
#include "qcmd/MotionCmdSync.hpp"

namespace zmc {

// Typed commands that target one explicit axis.
class MotionCmdSingleAxis : public MotionCmdSync {
   private:
    int appendAxisItem(int axis, uint16_t type, const void* param, uint16_t paramLen);

   public:
    MotionCmdSingleAxis() : MotionCmdSync() {}
    int axisEnable(int axis, bool enable);
    int setUnitsNumDen(int axis, int num, int den);
    int setSpeed(int axis, double speed);
    int setAccel(int axis, double accel);
    int setDecel(int axis, double decel);
    int setJerk(int axis, double jerk);
    int setFastDecel(int axis, double fastDecel);
    int setSpeedRatio(int axis, double ratio);
    int setDirection(int axis, int direction);
    int setBufferMode(int axis, int bufferMode);
    int powerOn(int axis);
    int powerOff(int axis);
    int stop(int axis);
    int halt(int axis);
    int reset(int axis);
    int setBaseAxis(int axis);
    int setAxis(int axis);
    int setMasterAxis(int axis, int masterAxis);
    int moveAbs(int axis, double pos);
    int moveRel(int axis, double distance);
    int moveAdd(int axis, double pos);
    int home(int axis, double pos);
    // Starts drive homing with SDO configuration and no Position correction.
    int homeSdo(int axis, const HomeSdoConfig& config);
    int homing(int axis);
    int gearIn(int axis);
    int gearOut(int axis);
    int setPosition(int axis, double pos);
    int datum(int axis, int mode);
    int moveVelocity(int axis, int dir);
    int forward(int axis);
    int reverse(int axis);
};

}  // namespace zmc

#endif  // _ZMC_MOTION_CMD_SINGLE_AXIS_HPP_
