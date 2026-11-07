#include "fb_cpp_def.h"
#include "fbt_axis_virtual.h"
#include "fbt_axis_base.h"
#include <stdio.h>
#include "fbt_motion_traits.hpp"
#include "AxisRunTmpl.hpp"
#include "common/misc/CmdArgsToy.hpp"


using namespace zmc;

template<typename T>
struct AxSgMoveTmpl:public AxSgMotTmpl<T, 1>{
    AxSgMoveTmpl(){
        this->get_config_ref(0).cycle_time = 100000;
        this->reload_config(0);
        this->mov.Axis = this->axisRef(0);
        set_fb_vel_config(this->mov, 5, 3, 3, 0);
        this->verbose_flag = 1;
    }
};


template<typename T>
struct AxSgMoveTmplArgs:public AxSgMotTmpl<T, 1>{
    AxSgMoveTmplArgs(CmdArgsToy& argsToy):args(argsToy){
        this->get_config_ref(0).cycle_time = argsToy["cycle_time"].get<int>(100000);
        this->get_config_ref(0).debug.level = argsToy["debug_level"].get<int>(0);
        this->get_config_ref(0).debug.modulo = argsToy["debug_modulo"].get<int>(2048);
        this->installLogger(0, AxLoggerType::CONSOLE);
        this->reload_config(0);
        this->mov.Axis = this->axisRef(0);
        set_fb_vel_config(this->mov, argsToy["velocity"].get<double>(5), argsToy["acceleration"].get<double>(3), argsToy["deceleration"].get<double>(3), argsToy["jerk"].get<double>(0));
        this->verbose_flag = argsToy["verbose"].get<int>(1);
    }
    CmdArgsToy& args;
    static AxSgMoveTmplArgs<T>* create(int argc, char** argv){
        CmdArgsToy* pArgsToy = new CmdArgsToy;
        parse_args(argc, argv, *pArgsToy);
        return new AxSgMoveTmplArgs<T>(*pArgsToy); 
    }
};

template<typename T>
int motNode_mov_demo_tmpl(int argc, char** argv) {
    if (argc && (argv[0] == "--help" || argv[0] == "-h")){
        printf("Usage: --help \n [--help] [--cycle_time <int>] [--distance <double>] [--velocity <double>] [--acceleration <double>] [--deceleration <double>] [--jerk <double>] [--acceleration2 <double>] [--deceleration2 <double>] [--verbose <int>] [--cycles <int>]" );
        return 0;
    }
    auto& axFb = *AxSgMoveTmplArgs<T>::create(argc, argv);
    CmdArgsToy& argsToy = axFb.args;
    axFb.get_config_ref(0).debug.servo_fak_pos = 3;
    axFb.get_config_ref(0).cycle_time = argsToy["cycle_time"].get<int>(200000);
    axFb.reload_config(0);
    setFbDistance(&axFb.mov, argsToy["distance"].get<double>(8));
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 0)
            axFb.mov.Execute = true;
    });
    return 0;
}

int motNode_mov_abs_demo(int argc, char** argv) {
    return motNode_mov_demo_tmpl<FbtMoveAbsolute>(argc, argv);
}
int motNode_mov_rel_demo(int argc, char** argv) {
    return motNode_mov_demo_tmpl<FbtMoveRelative>(argc, argv);
}
int motNode_mov_add_demo(int argc, char** argv) {
    return motNode_mov_demo_tmpl<FbtMoveAdditive>(argc, argv);
}

int motNode_demo(int argc, char** argv) { 
    // find --move_type moveabs moverel moveadd
    // then call the corresponding function
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--move_type") == 0) {
            if (strcmp(argv[i+1], "moveabs") == 0) {
                return motNode_mov_demo_tmpl<FbtMoveAbsolute>(argc, argv);
            } else if (strcmp(argv[i+1], "moverel") == 0) {
                return motNode_mov_demo_tmpl<FbtMoveRelative>(argc, argv);
            } else if (strcmp(argv[i+1], "moveadd") == 0) {
                return motNode_mov_demo_tmpl<FbtMoveAdditive>(argc, argv);
            } else if (strcmp(argv[i+1], "movecontabs") == 0) {
                return motNode_mov_demo_tmpl<FbtMoveContinuousAbsolute>(argc, argv);
            } else if (strcmp(argv[i+1], "movecontrel") == 0) {
                return motNode_mov_demo_tmpl<FbtMoveContinuousRelative>(argc, argv);
            }else {
                printf("move_type must be moveabs, moverel, moveadd, movecontabs or movecontrel\n");
                return -1;
            }
        }
    }
    return motNode_mov_abs_demo(argc, argv); 
}


void motNode_abs_cont_update_demo() {
    AxSgMoveTmpl<FbtMoveAbsolute> axFb;
    axFb.get_config_ref(0).debug.servo_fak_pos = 3;
    axFb.get_config_ref(0).cycle_time = 100000;
    axFb.reload_config(0);
    axFb.mov.Position = 15;
    set_fb_vel_config(axFb.mov, 3, 2, 2, 0);
    axFb.mov.Direction = mcShortestWay;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1) {
            axFb.mov.Execute = true;
        } else if (i == 15) {
            setFbContinousUpdate(axFb.mov, true);
            axFb.mov.Velocity = 2;
        } else if (i == 25) {
            set_fb_vel_config(axFb.mov, 4, 5, 5, 0);
        }
    });
}

void motNode_add_cont_update_demo() {
    AxSgMoveTmpl<FbtMoveAdditive> axFb;
    axFb.get_config_ref(0).debug.servo_fak_pos = 3;
    axFb.get_config_ref(0).cycle_time = 100000;
    axFb.reload_config(0);
    axFb.mov.Distance = 12;
    set_fb_vel_config(axFb.mov, 3, 2, 2, 0);
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1) {
            axFb.mov.Execute = true;
        } else if (i == 15) {
            setFbContinousUpdate(axFb.mov, true);
            axFb.mov.Velocity = 2;
        } else if (i == 25) {
            set_fb_vel_config(axFb.mov, 4, 5, 5, 0);
        }
    });
}

void motNode_invert_end_demo() {
    AxSgMoveTmpl<FbtMoveAbsolute> axFb;
    axFb.get_config_ref(0).debug.servo_fak_pos = 3;
    axFb.get_config_ref(0).cycle_time = 100000;
    axFb.reload_config(0);
    axFb.mov.Position = 15;
    set_fb_vel_config(axFb.mov, 3, 2, 2, 0);
    axFb.mov.Direction = mcShortestWay;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 0){
            axFb.mov.Execute = true;
        } else if (i == 14) {
            axFb.mov.Execute = false;
        } else if (i == 15) {
            axFb.mov.Velocity = 2;
            axFb.mov.Position = 5;
            axFb.mov.Execute = true;
        }
    });
}

void motNode_invert_vel_demo() {
    AxSgMoveTmpl<FbtMoveAbsolute> axFb;
    axFb.get_config_ref(0).debug.servo_fak_pos = 3;
    axFb.get_config_ref(0).cycle_time = 100000;
    axFb.reload_config(0);
    axFb.mov.Position = 15;
    set_fb_vel_config(axFb.mov, 3, 2, 2, 0);
    axFb.mov.Direction = mcShortestWay; 
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 0){
            axFb.mov.Execute = true;
        }else if (i == 14) {
            axFb.mov.Execute = false;
        }else if (i == 15) {
            axFb.mov.Velocity = 2;
            axFb.mov.Position = -3;
            axFb.mov.Execute = true;
        }
    });
}


template<typename T>
struct SgAxFbMoveOverrideTmpl:public AxDbMotTmpl<T, FbtSetOverride, 1>{
    SgAxFbMoveOverrideTmpl(){
        this->get_config_ref(0).cycle_time = 100000;
        this->reload_config(0);
        set_fb_vel_config(this->mov, 5, 3, 3, 0);
        this->bindAxis(0);
        this->mov2.Enable = true;
        this->count_to_exit = 100;
        this->verbose_flag = 1;
    }
};


void motnodeSgrp_move_override_demo(){
    SgAxFbMoveOverrideTmpl<FbtMoveAbsolute> axFb;
    axFb.mov.Position = 15;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 20)
            axFb.mov2.VelFactor = 0.6;  
        else if (i == 30){
            axFb.mov2.VelFactor = 0.8;  
            axFb.mov2.AccFactor = 0.5;
        }
    });
}
