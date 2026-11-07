#include "fb_cpp_def.h"
#include "fbt_axis_virtual.h"
#include "fbt_axis_base.h"
#include <stdio.h>
#include "fbt_motion_traits.hpp"

using namespace zmc;

void fbMoveAbs_loop_demo(){
    AxisMotEmu axFak;
    auto& ax = get_axis_impl_ref((AXIS_REF) &axFak);
    FbtMoveAbsolute fbMov;
    fbMov.Axis = (AXIS_REF) &axFak;
    int direction = 1;
    fbMov.Position = 5 * direction;
    fbMov.Velocity = 100000;
    fbMov.Acceleration = 400000;
    fbMov.Deceleration = 400000;
    fbMov.Jerk = 0;
    int i=0;
    axFak.get_config_ref().cycle_time = 100000;
    axFak.reload_config();

    printf("idx,stat,fb,pos,vel,acc\n");
    bool nextToRun = false;
    while(i<13555){ 
        if (i==3){
            fbMov.Execute = true;
        }
        if (fbMov.Done && !nextToRun) {
            fbMov.Execute = false;
            direction *= -1;
            fbMov.Position = 5 * direction;
            nextToRun = true;
        }else if(nextToRun){
            fbMov.Execute = true;
            nextToRun = false; 
        }
        fbMov.runCycle();
        axFak.runCycle();
        LREAL pos = ax.fSetPosition;
        LREAL vel = ax.fSetVelocity;
        LREAL acc = ax.fSetAcceleration;
        auto axStat = ax.axisStat;
        printf("%d,%d,0x%d,%f,%f,%f\n", i++, axStat, get_move_stat(fbMov), pos, vel, acc);
    }
}