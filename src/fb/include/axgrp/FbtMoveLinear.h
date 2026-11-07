#ifndef ZMC_FBT_MOVE_LINEAR_H
#define ZMC_FBT_MOVE_LINEAR_H

// #include <array>
#include "fbt_common.h"
#include "mc_enum_def.h"
#include "mc_robot_def.h"
#include "axgrp/AxisGrpZmotRef.h"

namespace zmc {


// C-style POD-like layout: no default member initializers in the struct; zero-initialize in ctor.
struct FbtMoveLinearStruct {
    void* pImpl;
    AxisGrpZmotRef AxisGroup;
    ZMC_BOOL Execute;
    uint16_t MotionType;  // 0: absolute, 1: relative
    ZMC_POS_REF Position;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    MC_BUFFER_MODE BufferMode;

    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Active;
    ZMC_BOOL CommandAborted;

    // The command has been accepted by the axis group. If the output becomes TRUE, the function block instance can be used to command the next movement with a new rising edge on Execute. While the output is FALSE, keep calling the function block without a new rising edge of Execute. The output is reset to FALSE on a falling edge of Execute.
    ZMC_BOOL CommandAccepted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
};

class FbtMoveLinear : public FbtMoveLinearStruct {
   public:
    FbtMoveLinear();
    ~FbtMoveLinear();
    void runCycle();
    void reset();
    uint8_t getStat() const;
    uint16_t getErrorId() const;
};

}  // namespace zmc

#endif
