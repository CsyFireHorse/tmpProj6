#include "fbt_axis.h"
#include "fbt_axis_impl.h"
#include "fb_cpp_def.h"
#include "fbt_fb_base.hpp"
#include "math.h"
#include "stdio.h"
#include "AxisRunTmpl.hpp"

using namespace zmc;

void motNode_vel_demo() {
    AxSgMotTmpl<FbtMoveVelocity> axFb;
    auto& cfg = axFb.get_config_ref(0);
    cfg.debug.servo_fak_pos = 4;
    cfg.cycle_time = 200000;
    axFb.reload_config(0);
    axFb.bindAxis(0);

    set_fb_vel_config(axFb.mov, 5, 3, 3, 0);
    axFb.mov.Direction = mcShortestWay;
    auto& ax = axFb.axRef(0);
   
    int verboseFlag = 1;
    printAxisHeader(verboseFlag);
    for (int i = 0; i < 20; i++) {
        axFb.runCycle();
        if (i == 1){
            axFb.mov.Execute = true;
        }
        axFb.axisRef(0)->print(verboseFlag);
    }
}

void motNode_vel_reload_demo() {
    AxSgMotTmpl<FbtMoveVelocity> axFb;
    auto& cfg = axFb.get_config_ref(0);
    cfg.debug.servo_fak_pos = 5;
    cfg.cycle_time = 200000;
    axFb.reload_config(0);
    axFb.bindAxis(0);

    axFb.mov.Velocity = 5;
    set_fb_vel_config(axFb.mov, 5, 3, 3, 0);
    axFb.mov.Direction = mcShortestWay;
    auto& ax = axFb.axRef(0);
    
    int verboseFlag = 1;
    printAxisHeader(verboseFlag);
    for (int i = 0; i < 30; i++) {
        axFb.runCycle();
        if (i== 0)
            axFb.mov.Execute = true;
        else if (i == 14) {
            axFb.mov.Execute = false;
        }else if (i == 15) {
            axFb.mov.Velocity = 3;
            axFb.mov.Execute = true;
        }
        axFb.axisRef(0)->print(verboseFlag);
    }
}

void motNode_move_cont_abs_demo() {
    AxSgMotTmpl<FbtMoveContinuousAbsolute> axFb;
    auto& cfg = axFb.get_config_ref(0);
    cfg.debug.servo_fak_pos = 2;
    cfg.cycle_time = 100000;
    axFb.reload_config(0);
    axFb.bindAxis(0);

    axFb.mov.Position = 10;
    set_fb_vel_config(axFb.mov, 5, 4, 5, 0);
    axFb.mov.EndVelocity = 3;
    axFb.mov.Direction = mcShortestWay;
    axFb.mov.EndVelocityDirection = mcNegativeDirection;
    auto& ax = axFb.axRef(0);

    int verboseFlag = 8;
    int32_t dwExtraData = 0;
    ax.pDwExtraData = &dwExtraData;
    printAxisHeader(verboseFlag);
    for (int i = 0; i < 50; i++) {
        axFb.runCycle();
        if (i == 1) {
            axFb.mov.Execute = true;
        }
        dwExtraData = axFb.mov.Busy + axFb.mov.CommandAborted*5 + axFb.mov.Error*4 + axFb.mov.InEndVelocity + axFb.mov.PositionReached;
        axFb.axisRef(0)->print(verboseFlag);
        if (i > 5) {
            axFb.check_set_pva(0);
        }
    }
}

void motNode_move_cont_rel_demo() {
    AxSgMotTmpl<FbtMoveContinuousRelative> axFb;
    auto& cfg = axFb.get_config_ref(0);
    cfg.debug.servo_fak_pos = 5;
    cfg.cycle_time = 100000;
    axFb.reload_config(0);
    axFb.bindAxis(0);

    axFb.mov.Distance = 8;
    set_fb_vel_config(axFb.mov, 5, 4, 5, 0);
    axFb.mov.EndVelocity = 3;
    axFb.mov.EndVelocityDirection = mcNegativeDirection;
    auto& ax = axFb.axRef(0);

    int verboseFlag = 8;
    int32_t dwExtraData = 0;
    ax.pDwExtraData = &dwExtraData;
    printAxisHeader(verboseFlag);
    for (int i = 0; i < 50; i++) {
        axFb.runCycle();
        if (i == 0) {
            axFb.mov.Execute = true;
        }
        dwExtraData = axFb.mov.Busy + axFb.mov.CommandAborted*5 + axFb.mov.Error*4 + axFb.mov.InEndVelocity + axFb.mov.DistanceTravelled;
        axFb.axisRef(0)->print(verboseFlag);
        if (i > 5) {
            axFb.check_set_pva(0);
        }
    }
}

template<typename T, typename T2>
struct SgAxContFbTmpl:public AxDbMotTmpl<T, T2, 1>{
    SgAxContFbTmpl(){
        this->get_config_ref(0).cycle_time = 100000;
        this->reload_config(0);
        this->bindAxis(0);
        set_fb_vel_config(this->mov, 5, 3, 3, 0);
        set_fb_vel_config(this->mov2, 3, 4, 2, 0);
    }
    // void postRunCycle()override{
    //     return this->printAxisFb();
    // }
};

// continuous motion
void motnodeSgrp_vel_abs_demo(){
    SgAxContFbTmpl<FbtMoveVelocity, FbtMoveAbsolute> axFb;
    // axFb.mov.Position = 10;
    axFb.mov2.Position = -5;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 16)
            axFb.mov2.Execute = true;
    });
}

void motnodeSgrp_vel_buf_abs_demo(){
    SgAxContFbTmpl<FbtMoveVelocity, FbtMoveAbsolute> axFb;
    axFb.get_config_ref(0).cycle_time = 50000;
    axFb.reload_config(0);
    axFb.count_to_exit = 140;
    axFb.mov2.Position = -5;
    axFb.mov2.BufferMode = mcBuffered;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 16)
            axFb.mov2.Execute = true;
    });
}

void motnodeSgrp_abs_buf_vel_demo(){
    SgAxContFbTmpl<FbtMoveAbsolute, FbtMoveVelocity> axFb;
    axFb.get_config_ref(0).cycle_time = 50000;
    axFb.reload_config(0);
    axFb.count_to_exit = 140;
    axFb.mov.Position = -5;
    axFb.mov.BufferMode = mcBuffered;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 0)
            axFb.mov.Execute = true;
        else if (i == 15)
            axFb.mov2.Execute = true;
    });
}


void motnodeSgrp_movecontabs_abs_demo(){
    SgAxContFbTmpl<FbtMoveContinuousAbsolute, FbtMoveAbsolute> axFb;
    axFb.mov.Position = -5;
    axFb.mov.EndVelocity = 2;
    axFb.mov2.Position = 10;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 16)
            axFb.mov2.Execute = true;
    });
}

void motnodeSgrp_movecontabs_buf_abs_demo(){
    SgAxContFbTmpl<FbtMoveContinuousAbsolute, FbtMoveAbsolute> axFb;
    axFb.get_config_ref(0).cycle_time = 50000;
    axFb.reload_config(0);
    axFb.count_to_exit = 140;
    axFb.mov.Position = -5;
    axFb.mov.EndVelocity = 2;
    axFb.mov2.Position = 10;
    axFb.mov2.BufferMode = mcBuffered;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 16)
            axFb.mov2.Execute = true;
    });
}


void motnodeSgrp_abs_movecontabs_demo(){
    SgAxContFbTmpl<FbtMoveAbsolute, FbtMoveContinuousAbsolute> axFb;
    axFb.get_config_ref(0).cycle_time = 50000;
    axFb.reload_config(0);
    axFb.count_to_exit = 140;
    axFb.mov.Position = 10;
    axFb.mov2.Position = -5;
    axFb.mov2.EndVelocity = 2;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 16)
            axFb.mov2.Execute = true;
    });
}
