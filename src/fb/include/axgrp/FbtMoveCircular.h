#ifndef ZMC_FBT_MOVE_CIRCULAR_H
#define ZMC_FBT_MOVE_CIRCULAR_H

#include "fbt_common.h"
#include "mc_enum_def.h"
#include "mc_robot_def.h"
#include "axgrp/AxisGrpZmotRef.h"

namespace zmc {

struct FbtMoveCircularStruct {
    void* pImpl;
    AxisGrpZmotRef AxisGroup;
    ZMC_BOOL Execute;
    uint16_t MotionType;  // 0: absolute, 1: relative
    ZMC_POS_REF AuxPoint;
    ZMC_POS_REF EndPosition;
    uint16_t PathChoice;
    uint16_t CircMode;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    MC_BUFFER_MODE BufferMode;

    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Active;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL CommandAccepted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
};

class FbtMoveCircular : public FbtMoveCircularStruct {
   public:
    FbtMoveCircular();
    ~FbtMoveCircular();
    void runCycle();
    void reset();
    uint8_t getStat() const;
    uint16_t getErrorId() const;
};

struct FbtMoveMixStruct {
    void* pImpl;
    AxisGrpZmotRef AxisGroup;
    ZMC_BOOL Execute;
    uint16_t MotionType;  // 0: absolute, 1: relative
    uint16_t MMode;       // 0: linear, 1: circular
    ZMC_POS_REF EndPosition;   // Shared target: linear target or circular end point
    ZMC_POS_REF AuxPoint;      // Circular aux point
    uint16_t PathChoice;
    uint16_t CircMode;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    MC_BUFFER_MODE BufferMode;

    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Active;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL CommandAccepted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
};

class FbtMoveMix : public FbtMoveMixStruct {
   public:
    FbtMoveMix();
    ~FbtMoveMix();
    void runCycle();
    void reset();
    uint8_t getStat() const;
    uint16_t getErrorId() const;
};

}  // namespace zmc

#endif
