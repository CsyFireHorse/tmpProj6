

#include "fb_cpp_def.h"
#include "fbt_axis_virtual.h"
#include "fbt_axis_base.h"
#include "AxisRunTmpl.hpp"
#include <stdio.h>
#include <utility>  

using namespace zmc;



template<typename T, typename T2>
struct SgAxFbDbTmpl2:public AxDbMotTmpl<T, T2, 1>{
    SgAxFbDbTmpl2(){
        this->get_config_ref(0).cycle_time = 50000;
        this->reload_config(0);
        this->count_to_exit = 140;
        this->bindAxis(0);
        set_fb_vel_config(this->mov, 5, 3, 3, 0);
        set_fb_vel_config(this->mov2, 3, 4, 2, 0);
    }
    // void postRunCycle()override{
    //     return this->printAxisFb();
    // }
};

void motnodeSgrp_abs_blend_rel_demo(){
    SgAxFbDbTmpl2<FbtMoveAbsolute, FbtMoveRelative> axFb;
    axFb.mov.Position = 5;
    axFb.mov.BufferMode = mcBlendingLow;
    // setFbVelEndLimit(axFb.mov, 2.0);
    axFb.mov2.Distance = 5;
    axFb.mov2.BufferMode = mcBlendingLow;
    axFb.verbose_flag = 1;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1){
            axFb.mov.Execute = true;
            axFb.mov2.Execute = true;
        }
        int ret = axFb.check_set_pva(0);
        if (ret)
            axFb.printAxis();
    });
}


void motnodeSgrp_abs_blend_rel_demo2(){
    SgAxFbDbTmpl2<FbtMoveAbsolute, FbtMoveRelative> axFb;
    axFb.mov.Position = 5;
    axFb.mov.BufferMode = mcBlendingLow;
    axFb.mov2.Distance = 5;
    axFb.mov2.BufferMode = mcBlendingLow;
    axFb.verbose_flag = 1;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 30)
            axFb.mov2.Execute = true;
        int ret = axFb.check_set_pva(0);
        if (ret)
            axFb.printAxis();
    });
}


void motnodeSgrp_abs_blend_corner_demo(){
    SgAxFbDbTmpl2<FbtMoveAbsolute, FbtMoveRelative> axFb;
    axFb.mov.Position = 5;
    axFb.mov.BufferMode = mcBlendingLow;
    setFbVelEndLimit(axFb.mov, 2.0);
    axFb.mov2.Distance = -7;
    axFb.mov2.BufferMode = mcBlendingLow;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1){
            axFb.mov.Execute = true;
            axFb.mov2.Execute = true;
        }
        int ret = axFb.check_set_pva(0);
        if (ret)
            axFb.printAxis();
    });
}


void motnodeSgrp_blend_3_demo(){
    AxThrMotTmpl<FbtMoveDiscrete, FbtMoveDiscrete, FbtMoveDiscrete> axFb;
    axFb.bindAxis(0);
    axFb.get_config_ref(0).cycle_time = 50000;
    axFb.reload_config(0);
    axFb.count_to_exit = 140;
    axFb.mov.PosOrDist = 5;
    set_fb_vel_config(axFb.mov, 3, 3, 3, 0);
    axFb.mov.BufferMode = mcBlendingHigh;
    axFb.mov.MotionType = 0;
    axFb.mov2.BufferMode = mcBlendingHigh;
    axFb.mov2.MotionType = 1;
    axFb.mov2.PosOrDist = 6;
    set_fb_vel_config(axFb.mov2, 5, 4, 2, 0);
    axFb.mov3.PosOrDist = 7;
    set_fb_vel_config(axFb.mov3, 6, 5, 4, 0);
    axFb.mov3.BufferMode = mcBlendingHigh;
    axFb.mov3.MotionType = 1; 

    setFbVelEndLimit(axFb.mov, 2.0);
    axFb.verbose_flag = 1;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1){
            axFb.mov.Execute = true;
            axFb.mov2.Execute = true;
        }else if (i == 30){
            axFb.mov3.Execute = true;
        }
        int ret = axFb.check_set_pva(0);
        if (ret)
            axFb.printAxis();
    });
}

void motnodeSgrp_blend_4_demo(){
    AxMotArrTmpl<FbtMoveDiscrete, 4> axFb;
    axFb.bindAxis(0);
    axFb.get_config_ref(0).cycle_time = 50000;
    axFb.reload_config(0);
    axFb.count_to_exit = 200;

    axFb.movArr[0].PosOrDist = 5;
    axFb.movArr[0].MotionType = 0;
    set_fb_vel_config(axFb.movArr[0], 3, 4, 5, 0);
    axFb.movArr[0].BufferMode = mcBlendingHigh;

    axFb.movArr[1].PosOrDist = 6;
    axFb.movArr[1].MotionType = 1;
    set_fb_vel_config(axFb.movArr[1], 2, 5, 6, 0);
    axFb.movArr[1].BufferMode = mcBlendingHigh;

    axFb.movArr[2].PosOrDist = 8;
    axFb.movArr[2].MotionType = 1;
    set_fb_vel_config(axFb.movArr[2], 5, 4, 2, 0);
    axFb.movArr[2].BufferMode = mcBlendingHigh;

    axFb.movArr[3].PosOrDist = 8;
    axFb.movArr[3].BufferMode = mcBlendingHigh;
    axFb.movArr[3].MotionType = 1;
    set_fb_vel_config(axFb.movArr[3], 6, 3, 6, 0);

    axFb.verbose_flag = 1;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1){
            axFb.movArr[0].Execute = true;
            axFb.movArr[1].Execute = true;
        }else if(i==30){
            axFb.movArr[2].Execute = true;
        }else if(i==40){
            axFb.movArr[3].Execute = true;
        }
        int ret = axFb.check_set_pva(0);
        if (ret)
            axFb.printAxis();
    });

}
struct SgAxFbDb:public AxDbMotTmpl<FbtMoveAbsolute, FbtMoveAbsolute>{
    SgAxFbDb(){
        this->get_config_ref(0).cycle_time = 100000;
        this->reload_config(0);

        this->bindAxis(0);
        this->mov.Position = -3;
        set_fb_vel_config(this->mov, 3, 4, 3, 0);

        this->mov2.Position = -7;
        set_fb_vel_config(this->mov2, 2, 3, 4, 0);
    }
        // printf("%d,%f,%f,%f,%f,0x%x,0x%x\n", ax.m_cycleCounter, ax.get_current_time(), ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration, 0, 0); // mov.get_fb_stat()
    
};


void motNodeSgaxGrp_db_demo(){
    SgAxFbDb axFb;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 6)
            axFb.mov2.Execute = true;
    });
}


void motNodeSgaxGrp_db_buf_demo() {
    SgAxFbDb axFb;
    axFb.mov2.BufferMode = mcBuffered;
    axFb.mov2.Position = -7;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 6)
            axFb.mov2.Execute = true;
    });
}

void motNodeSgaxGrp_db_blend_demo() {
    SgAxFbDb axFb;
    axFb.mov2.BufferMode = mcBlendingLow;
    axFb.mov2.Position = -7;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 6)
            axFb.mov2.Execute = true;
    });
}

void motNodeSgaxGrp_db_corner_demo() {
    SgAxFbDb axFb;
    axFb.mov2.BufferMode = mcBlendingLow;
    axFb.mov2.Position = 3;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 6)
            axFb.mov2.Execute = true;
    });
}


void motNodeSgaxGrp_dyn_db_demo() {
    SgAxFbDb axFb;
    axFb.mov2.BufferMode = mcBlendingNext;
    axFb.mov2.Position = 3;
    axFb.mov.Position = -3;
    set_fb_vel_config(axFb.mov, 3, 4, 3, 0);
    axFb.mov2.Position = -7;
    set_fb_vel_config(axFb.mov2, 2, 3, 4, 0);

    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 20) {
            axFb.mov2.Execute = true;
        }else if (i == 35) {
            set_fb_vel_config(axFb.mov2, 3, 5, 6, 0);
        }
    });
}


void motNodeSgaxGrp_abort_db_demo() {
    SgAxFbDb axFb;
    axFb.mov.Position = -3;
    set_fb_vel_config(axFb.mov, 3, 4, 3, 0);

    axFb.mov2.Position = -7;
    set_fb_vel_config(axFb.mov2, 2, 3, 4, 0);
    axFb.mov2.BufferMode = mcAborting;

    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 20) {
            axFb.mov2.Execute = true;
        }else if (i == 35) {
            set_fb_vel_config(axFb.mov2, 3, 5, 6, 0);
        }
    });
}


void motNodeSgaxGrp_abort_add_db_demo() {
    AxThrMotTmpl<FbtMoveAbsolute, FbtMoveAbsolute, FbtMoveAbsolute> axFb;
    axFb.bindAxis(0);
    axFb.mov.Position = -3;
    set_fb_vel_config(axFb.mov, 3, 4, 3, 0);
    axFb.mov2.Position = -7;
    set_fb_vel_config(axFb.mov2, 2, 3, 4, 0);
    axFb.mov2.BufferMode = mcAborting;
    axFb.mov3.Position = -2;
    set_fb_vel_config(axFb.mov3, 3, 5, 4, 0);
    axFb.mov3.BufferMode = mcAborting;
    axFb.get_config_ref(0).cycle_time = 100000;
    axFb.reload_config(0);

    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i==1)
            axFb.mov.Execute = true;
        else if (i == 20) {
            axFb.mov2.Execute = true;
        }else if (i == 35) {
            axFb.mov3.Execute = true;
        }
    });
}

void motNodeSgaxGrp_dyn_cont_demo() {
    AxThrMotTmpl<FbtMoveAbsolute, FbtMoveContinuousAbsolute, FbtMoveAbsolute> axFb;
    axFb.bindAxis(0);
    axFb.mov.Position = -3;
    set_fb_vel_config(axFb.mov, 3, 4, 3, 0);

    axFb.mov2.Position = -7;
    set_fb_vel_config(axFb.mov2, 2, 3, 4, 0);
    axFb.mov2.EndVelocity = -1;

    axFb.mov3.Position = -10;
    set_fb_vel_config(axFb.mov3, 3, 2, 4, 0);
    axFb.mov3.BufferMode = mcBlendingNext;

    axFb.get_config_ref(0).cycle_time = 100000;
    axFb.reload_config(0);

    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 20) {
            axFb.mov2.Execute = true;
        }else if (i == 35) {
            set_fb_vel_config(axFb.mov2, 3, 5, 6, 0);
        }else if (i == 40) {
            axFb.mov3.Execute = true;
            // mov3.set_vel_config(3, 5, 6, 0);
        }
    });
}
