#include "fb_cpp_def.h"
#include "fbt_axis.h"
#include "fbt_axis_impl.h"
#include "stdio.h"

using namespace zmc;
typedef AxisCia402 Axis;
void fbt_eager_demo() {
    Axis axis;
    AXIS_REF pAx = &axis;
    AxisImpl& ax =(AxisImpl&) axis;
    ax.bSimulation = true;
    ax.bDriveStart = true;
    ax.bRegulatorOn = true;
    ax.axisStat = mcStandstill;
    // FbtPower power;
    FbtMoveAbsolute move;
    FbtReset reset;

    // power.setAxis(pAx);
    move.Axis = pAx;
    reset.Axis = pAx;

    move.Position = 50.0;
    move.Velocity = 100.0;      
    move.Acceleration = 200.0;  
    move.Deceleration = 250.0;  
    move.BufferMode = mcBuffered;

    FbtMoveRelative move2;
    move2.Axis = pAx;
    move2.Distance = 80.0;      
    move2.Velocity = 120.0;     
    move2.Acceleration = 325.0; 
    move2.Deceleration = 400.0; 
    move2.Jerk = 0.0;           

    FbtStop move3;
    move3.Axis = pAx;
    move3.Deceleration = (500.0);
    move3.Jerk = 0.0;

    int verboseFlag = 9;
    int32_t dwExtraData = 0;
    ax.pDwExtraData = &dwExtraData;
    printAxisHeader(verboseFlag);
    for (int i = 0; i < 120; i++) {
        if (i == 3) {
            // power.Enable = true;
            // power.bRegulatorOn = true;
        } else if (i == 7) {
            move.Execute = true;
        } else if (i == 15) {
            move2.Execute = true;
            move.Execute = false;
        } else if (i == 20) {  // aborting
            move.Execute = true;
        } else if (i == 35) {
            move2.Execute = false;
        } else if (i == 36) {  // reload
            move2.Velocity = 160;
            move2.Execute = true;
        } else if (i == 85) {  // stop
            move3.Execute = true;
        } else if (i == 95) {
            move3.Execute = false;
        }

        // power.runCycle();
        move.runCycle();
        move2.runCycle();
        reset.runCycle();
        move3.runCycle();
        dwExtraData = move.Busy + move.Error*4 + move.CommandAborted*5 + ((move2.Busy + move2.Done*3 + move2.Error*4)<<8) 
         + (move3.Busy + move3.Done*3 + move3.Error*4)<<16;
        axis.runCycle();
        axis.print(verboseFlag);
    }
}

void fbt_movev_demo(){
    Axis axis;
    AXIS_REF pAx = &axis;
    AxisImpl& ax =(AxisImpl&) axis;
    ax.bSimulation = true;
    FbtPower power;
    FbtMoveVelocity moveV;
    FbtStop stop;


    power.Axis = pAx;
    moveV.Axis = pAx;
    stop.Axis = pAx;


    moveV.Velocity = 150.0;      
    moveV.Acceleration = 500.0;  
    moveV.Deceleration = 800.0;  
    moveV.Direction = mcPositiveDirection;
    moveV.BufferMode = mcBuffered;

    stop.Deceleration = (2000.0);

    int verboseFlag = 9;
    int32_t dwExtraData = 0;
    ax.pDwExtraData = &dwExtraData;
    printAxisHeader(verboseFlag);
    for (int i = 0; i < 200; i++) {
        if (i == 5) {
            power.Enable = true;
            power.bRegulatorOn = true;
            power.bDriveStart = true;
        } else if (i == 10) {
            moveV.Execute = true;
        } else if (i == 99) {
            moveV.Execute = false; 
        } else if (i == 100) {
            moveV.Execute = true; 
            moveV.Velocity = 200.0; 
        } else if (i == 150) {
            stop.Execute = true;
        }

        power.runCycle();
        moveV.runCycle();
        stop.runCycle();
        dwExtraData = moveV.Busy + moveV.Error*4 + moveV.CommandAborted*5 + ((stop.Busy + stop.Done*3 + stop.Error*4)<<8);
        axis.print(verboseFlag);
        axis.runCycle();
    }
}

void fbt_follow_demo() {
    Axis axis;
    axis.get_config_ref().cycle_time = 100000;
    axis.get_config_ref().is_axis_virtual = true;
    axis.reload_config();
    AXIS_REF pAx = &axis;
    AxisImpl& ax =(AxisImpl&) axis;
    // ax.bSimulation = true;
    FbtPower power;
    FbtFollowPosition follow;
    FbtHalt halt;

    power.Axis = pAx;
    follow.Axis = pAx;


    follow.fSetPosition = 0.0;
    halt.Axis = pAx;
    halt.Deceleration = 1000.0; 

    int verboseFlag = 9;
    int32_t dwExtraData = 0;
    ax.pDwExtraData = &dwExtraData;
    printAxisHeader(verboseFlag);
    for (int i = 0; i < 200; i++) {
        if (i == 5) {
            power.Enable = true;
            power.bRegulatorOn = true;
            power.bDriveStart = true; 
        } else if (i >= 10 && i<50) {
            follow.bExecute = true;
            follow.fSetPosition = 2.5 * (i-10);
        } else if (i >= 50 && i<80) {
            follow.fSetPosition = 100 + (i-50)*3.333;
        } else if (i >= 80 && i< 100) {
            follow.fSetPosition = 200- (i-80)*8;
        } else if (i == 100) {
            halt.Execute = true;
        } else if (i == 120) {
            follow.bExecute = false;
        } else if (i >= 130 && i<150) {
            follow.bExecute = true;
            follow.fSetPosition = 50.0 + (i-130)*5;
        }

        power.runCycle();
        follow.runCycle();
        halt.runCycle();
        int fbStat = follow.bBusy + follow.bCommandAborted * 5+ follow.bError * 4;
        dwExtraData = fbStat + ((halt.Busy + halt.Done*3 + halt.Error*4)<<8);
        axis.runCycle();
        axis.print(verboseFlag);
    }
}