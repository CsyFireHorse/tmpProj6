#include "fb_cpp_def.h"
#include "fbt_axis_virtual.h"
#include "fbt_axis_base.h"
#include "fbt_motion_traits.hpp"
#include <stdio.h>
#include "AxisRunTmpl.hpp"

using namespace zmc;

template<typename T, typename T2>
struct SgAxFbHaltTmp:public AxDbMotTmpl<T, T2, 1>{
    SgAxFbHaltTmp(){
        this->get_config_ref(0).cycle_time = 100000;
        this->reload_config(0);
        set_fb_vel_config(this->mov, 5, 3, 3, 0);
        this->bindAxis(0);
        this->mov2.Deceleration = 2;
        this->mov2.Jerk = 0;
        this->count_to_exit = 100;
    }
    // void postRunCycle()override{
    //     return this->printAxisFb();
    // }
};


void motnodeSgrp_vel_halt_demo(){
    SgAxFbHaltTmp<FbtMoveVelocity, FbtHalt> axFb;
    axFb.mov2.Deceleration = 200;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 0)
            axFb.mov.Execute = true;
        else if (i == 20)
            axFb.mov2.Execute = true;
        else if (i == 45)
            axFb.mov2.Execute = false;
    });
}

void motnodeSgrp_vel_stop_demo(){
    SgAxFbHaltTmp<FbtMoveVelocity, FbtStop> axFb;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 0)
            axFb.mov.Execute = true;
        else if (i == 20)
            axFb.mov2.Execute = true;
        else if (i == 45)
            axFb.mov2.Execute = false;
    });
}