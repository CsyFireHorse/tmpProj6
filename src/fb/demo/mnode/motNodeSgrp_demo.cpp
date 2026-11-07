// #include "mot_node.h"
#include "fb_cpp_def.h"
#include "fbt_axis_virtual.h"
#include "fbt_axis_base.h"
#include <stdio.h>
// #include "impl/fbt_motion_traits.hpp"
#include "AxisRunTmpl.hpp"

using namespace zmc;

void motnodeSgrp_demo(){
    FbtMoveAbsolute mov;
    AxisMotEmu axis;
    auto& ax = get_axis_impl_ref((AXIS_REF) &axis);

    ax.cycleTime = 0.1;

    mov.Axis = &axis;
    mov.Execute = true;
    mov.Position = 10;
    mov.Velocity = 5;
    mov.Acceleration = 3;
    mov.Deceleration = 3;
    mov.Jerk = 0;

    printf("idx,time,pos,vel,acc,fb\n");
    for (int i = 0; i < 45; i++) {
        ax.m_cycleCounter = i + 1;
        double tm = ax.get_current_time();
        mov.runCycle();
        // mov.get_fb_stat()
        printf("%d,%f,%f,%f,%f,0x%x\n", i, tm, ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration, 0);
    }
}

template<typename T, typename T2>
struct SgAxFbDbTmp:public AxDbMotTmpl<T, T2, 1>{
    SgAxFbDbTmp(){
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


void motnodeSgrp_db_demo(){
    SgAxFbDbTmp<FbtMoveAbsolute, FbtMoveAbsolute> axFb;
    axFb.mov.Position = 10;
    axFb.mov2.Position = 5;
    axFb.run([&](){
        auto& mov2 = axFb.mov2;
        auto i = axFb.getIndex();
        if (i == 0)
            axFb.mov.Execute = true;
        else if (i == 15)
            mov2.Execute = true;
        else if (i == 29)
            mov2.Execute = false;
        else if (i == 30){
            mov2.Execute = true;
            mov2.Velocity = 5;
            mov2.Acceleration = 5;
            mov2.Deceleration = 6;
        }
    });
}

void motnodeSgrp_abs_rel_demo(){
    SgAxFbDbTmp<FbtMoveAbsolute, FbtMoveRelative> axFb;
    axFb.mov.Position = 5;
    axFb.mov2.Distance = -10;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 16)
            axFb.mov2.Execute = true;
    });
    // endPos in [-5, 0]
}

void motnodeSgrp_abs_buf_rel_demo(){
    SgAxFbDbTmp<FbtMoveAbsolute, FbtMoveRelative> axFb;
    axFb.mov.Position = 5;
    axFb.mov2.Distance = -10;
    axFb.mov2.BufferMode = mcBuffered;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 16)
            axFb.mov2.Execute = true;
    });
    // endPos = -5
}

void motnodeSgrp_abs_add_demo(){
    SgAxFbDbTmp<FbtMoveAbsolute, FbtMoveAdditive> axFb;
    axFb.mov.Position = 5;
    axFb.mov2.Distance = -10;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 16)
            axFb.mov2.Execute = true;
    });
    // endPos = -5
}

// blend



void motnodeSgrp_abs_blend_demo(){
    AxSgMotTmpl<FbtMoveAbsolute> axFb;
    axFb.get_config_ref(0).cycle_time = 50000;
    axFb.reload_config(0);
    axFb.bindAxis(0);
    axFb.mov.Position = 5;
    set_fb_vel_config(axFb.mov, 5, 3, 3, 0);
    axFb.mov.BufferMode = mcBlendingLow;
    axFb.verbose_flag = 1;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        int ret = axFb.check_set_pva(0);
        if (ret)
            axFb.printAxis();
    });
}

