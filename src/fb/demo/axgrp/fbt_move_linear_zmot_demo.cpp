// Demo: FbtMoveLinear + AxisGrpZmot + virtual axes only (no AxisMotbufZmot / command layer).
#include "axgrp/FbtGroupAdmin.h"
#include "AxisCiaConfig.hpp"
#include "AxisGrpZmot.h"
// #include "AxisManager.h"
#include "axgrp/FbtMoveLinear.h"
#include "fbt_axis_virtual.h"
// #include "impl/fbt_axis_base.h"
// #include "impl/fbt_impl_enum_def.h"
#include "mc_enum_def.h"

#include <cmath>
#include <cstdio>
#include <cstring>

using namespace zmc;


void fbt_move_linear_zmot_demo() {
    AxisMotEmu axArr[2];

    AxisGrpZmot group;
    const int axisIdx[2] = {0, 1};

    FbtAddAxisToGroup addGrp;
    FbtAddAxisToGroup addGrp2;
    FbtGroupEnable enGrp;
    FbtMoveLinear move;
    assign_axis_id(&axArr[1], 1);
    move.AxisGroup = &group;
    move.MotionType = 0;  // absolute
    move.Position.v[0] = 3.0;
    move.Position.v[1] = 4.0;
    move.Velocity = 80.0;
    move.Acceleration = 400.0;
    move.Deceleration = 400.0;
    move.Jerk = 0.0;
    constexpr int kMaxCycles = 500;
    
    addGrp.Axis = &axArr[0];
    addGrp2.Axis = &axArr[1];
    addGrp.AxisGroup = &group;
    addGrp2.AxisGroup = &group;
    enGrp.AxisGroup = &group;

    printAxisHeader(2);
    for (int i = 0; i < kMaxCycles; ++i) {
        if (i==1){
            addGrp.Execute = true;
            addGrp2.Execute = true;
        }
        if (i == 2){
            enGrp.Execute = true;
        }
        if (i == 10)
            move.Execute = true;
        addGrp.runCycle();
        addGrp2.runCycle();
        enGrp.runCycle();
        move.runCycle();
        group.runCycle();

        for (auto& ax : axArr) {
            ax.runCycle();
            ax.print(2);
        }
    }
}
