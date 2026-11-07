/*
 * @Author: ChenShengyong 
 * @Date: 2024-07-18 18:51:28 
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-06-11 17:11:43
 */
#include "fb_cpp_def.h"
#include "fbt_axis_virtual.h"
#include "fbt_axis_base.h"
#include <stdio.h>
#include "AxisRunTmpl.hpp"

using namespace zmc;
void motNode_gearIn_demo() {
    AxisMotEmu axis;
    auto& cfg = axis.get_config_ref();
    cfg.cycle_time = 10000;
    cfg.debug.servo_fak_pos = 5;
    axis.reload_config();

    AxisMotEmu axis2;
    auto& cfg2 = axis2.get_config_ref();
    cfg2.cycle_time = 10000;
    axis2.reload_config();
    auto& ax = get_axis_impl_ref((AXIS_REF) &axis);
    auto& ax2 = get_axis_impl_ref((AXIS_REF) &axis2);
    assign_axis_id(&axis2, 1);

    FbtMoveAbsolute mov;
    mov.Position = 10;
    set_fb_vel_config(mov, 5, 3, 3, 0);
    mov.Direction = mcShortestWay;
    mov.Axis = &axis;

    FbtGearIn egr;
    set_fb_acc_config(egr, 5, 5, 0);
    egr.RatioNumerator = 1;
    egr.RatioDenominator = 1;
    egr.Master = &axis;
    egr.Slave = &axis2;

    printAxisHeader(1);
    for (int i = 0; i < 280; i++) {
        double tm = ax.get_current_time();
        if (i==2)
            egr.Execute = true;
        if (i==2)
            mov.Execute = true;
        mov.runCycle();
        egr.runCycle();
        axis.runCycle();
        axis2.runCycle();
        axis.print(1);
        axis2.print(1);
    }
}

void motNode_follow_demo() {
    AxSgMotTmpl<FbtFollowPosition> axFb;
    axFb.get_config_ref(0).debug.servo_fak_pos = 10;
    axFb.get_config_ref(0).cycle_time = 200000;
    axFb.reload_config(0);
    axFb.mov.Axis = axFb.axisRef(0); 
    axFb.verbose_flag = 1;
    double pos = 10;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i >= 2  && i<20){
            axFb.mov.bExecute = true;
            axFb.mov.fSetPosition = pos;
            pos -= 2.5;
        }
    });
}

void motNode_follow_out_demo() {
    AxDbMotTmpl<FbtFollowPosition, FbtGearOut> axFb;
    axFb.get_config_ref(0).debug.servo_fak_pos = 10;
    axFb.get_config_ref(0).cycle_time = 200000;
    axFb.reload_config(0);
    axFb.mov.Axis = axFb.axisRef(0); 
    axFb.mov2.Slave = axFb.axisRef(0);
    axFb.verbose_flag = 1;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 2)
            axFb.mov.bExecute = true;
        else if(i==18){
            axFb.mov2.Execute = true;
        }else if(i>4 && i<20){
            axFb.mov.fSetPosition = 2.5 * (8 - i);
        }
    });
}
