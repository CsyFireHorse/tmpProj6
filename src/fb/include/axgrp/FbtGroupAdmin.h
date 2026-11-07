#ifndef ZMC_FBT_GROUP_ADMIN_H
#define ZMC_FBT_GROUP_ADMIN_H

#include "AxisRef.h"
#include "fbt_common.h"
#include "mc_robot_def.h"
#include "axgrp/AxisGrpZmotRef.h"

namespace zmc {

class AxisGrpZmot;

// Struct-only IO containers (PLCopen-like) + hidden implementation via pImpl.
// pImpl is the first member, and must never expose executeBak_ in the header.
struct FbtAddAxisToGroupStruct {
    void* pImpl;

    AxisGrpZmotRef AxisGroup;
    AXIS_REF Axis;

    ZMC_BOOL Execute;
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
};

class FbtAddAxisToGroup : public FbtAddAxisToGroupStruct {
   public:
    FbtAddAxisToGroup();
    ~FbtAddAxisToGroup();
    void runCycle();
};

struct FbtGroupEnableStruct {
    void* pImpl;
    AxisGrpZmotRef AxisGroup;

    ZMC_BOOL Execute;
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
};

class FbtGroupEnable : public FbtGroupEnableStruct {
   public:
    FbtGroupEnable();
    ~FbtGroupEnable();
    void runCycle();
};

struct FbtGroupDisableStruct {
    void* pImpl;
    AxisGrpZmotRef AxisGroup;

    ZMC_BOOL Execute;
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
};

class FbtGroupDisable : public FbtGroupDisableStruct {
   public:
    FbtGroupDisable();
    ~FbtGroupDisable();
    void runCycle();
};

struct FbtGroupResetStruct {
    void* pImpl;
    AxisGrpZmotRef AxisGroup;

    ZMC_BOOL Execute;
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
};

class FbtGroupReset : public FbtGroupResetStruct {
   public:
    FbtGroupReset();
    ~FbtGroupReset();
    void runCycle();
};

struct FbtUngroupAllAxesStruct {
    void* pImpl;
    AxisGrpZmotRef AxisGroup;

    ZMC_BOOL Execute;
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
};

class FbtUngroupAllAxes : public FbtUngroupAllAxesStruct {
   public:
    FbtUngroupAllAxes();
    ~FbtUngroupAllAxes();
    void runCycle();
};

struct FbtRemoveAxisFromGroupStruct {
    void* pImpl;

    AxisGrpZmotRef AxisGroup;
    AXIS_REF Axis;

    ZMC_BOOL Execute;
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
};

class FbtRemoveAxisFromGroup : public FbtRemoveAxisFromGroupStruct {
   public:
    FbtRemoveAxisFromGroup();
    ~FbtRemoveAxisFromGroup();
    void runCycle();
};

struct FbtGroupSetUnitsStruct {
    void* pImpl;

    AxisGrpZmotRef AxisGroup;

    ZMC_BOOL Execute;
    ZMC_GroupUnits Units;
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
};

class FbtGroupSetUnits : public FbtGroupSetUnitsStruct {
   public:
    FbtGroupSetUnits();
    ~FbtGroupSetUnits();
    void runCycle();
};

struct FbtGroupPowerStruct {
    void* pImpl;

    AxisGrpZmotRef AxisGroup;

    ZMC_BOOL Enable;
    ZMC_BOOL bRegulatorOn;
    ZMC_BOOL bDriveStart;
    ZMC_BOOL Status;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
};

class FbtGroupPower : public FbtGroupPowerStruct {
   public:
    FbtGroupPower();
    ~FbtGroupPower();
    void runCycle();
};

struct FbtGroupSetPositionStruct {
    void* pImpl;

    AxisGrpZmotRef AxisGroup;

    ZMC_BOOL Execute;
    ZMC_POS_REF Position;
    ZMC_BOOL Relative;
    ZMC_COORD_SYSTEM CoordSystem;
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
};

class FbtGroupSetPosition : public FbtGroupSetPositionStruct {
   public:
    FbtGroupSetPosition();
    ~FbtGroupSetPosition();
    void runCycle();
};

}  // namespace zmc

#endif
