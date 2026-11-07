// #include "fb_cnc_axis_def.h"

#include "fb_cnc_axis_def.h"
#include "fb_cpp_def.h"
// #include "fbt_axis_virtual.h"
// #include "fbt_axis_base.h"
#include <stdio.h>
#include "AxisRunTmpl.hpp"
#include "common/misc/CmdArgsToy.hpp"
#include "fb/fb_cnc_toy_def.h"

using namespace zmc;
// <>
// Template for ControlAxisByPos demo with single axis

template<typename T>
struct AxSgControlAxisByPosTmpl:public AxDbMotTmpl<T, FbtControlAxisByPos>{
    AxSgControlAxisByPosTmpl(CmdArgsToy& argsToy):args(argsToy){
        this->get_config_ref(0).cycle_time = 10000; // 0.01s cycle time
        this->get_config_ref(0).linear.increments_units_in_application_num = 10000;
        this->get_config_ref(0).dynamic_limits.velocity = 10;
        this->get_config_ref(0).dynamic_limits.acceleration = 50;
        this->get_config_ref(0).dynamic_limits.deceleration = 50;
        // this->get_config_ref(0).dynamic_limits.jerk = 0;
        this->reload_config(0);
        this->mov2.Axis = this->axisRef(0);
        
        // Set default gap parameters for ControlAxisByPos
        // this->mov.fGapVelocity = argsToy["gap_velocity"].get<double>(5.0);
        this->mov2.fGapVelocity = 5.0;        // Gap velocity [u/s]
        this->mov2.fGapAcceleration = 20.0;   // Gap acceleration [u/s²]
        this->mov2.fGapDeceleration = 20.0;   // Gap deceleration [u/s²]
        this->mov2.fGapJerk = 0.0;            // Gap jerk [u/s³]
        // Enable gap avoidance by default
        this->mov2.bAvoidGaps = true;
        
        this->verbose_flag = 1;
    }
    CmdArgsToy& args;
    static AxSgControlAxisByPosTmpl<T>* create(int argc, char** argv){
        CmdArgsToy* pArgsToy = new CmdArgsToy;
        parse_args(argc, argv, *pArgsToy);
        return new AxSgControlAxisByPosTmpl<T>(*pArgsToy); 
    }
    int extra_data = 0;
};

// Template for ControlAxisByPos demo with command line arguments

// Basic ControlAxisByPos demo function
int motNode_controlAxisByPos_demo0(int argc, char** argv) {
    if (argc && (argv[0] == "--help" || argv[0] == "-h")){
        printf("Usage: --help \n [--help] [--cycle_time <int>] [--set_position <double>] [--gap_velocity <double>] [--gap_acceleration <double>] [--gap_deceleration <double>] [--gap_jerk <double>] [--avoid_gaps <bool>] [--verbose <int>] [--cycles <int>]" );
        return 0;
    }
    auto& axFb = *AxSgControlAxisByPosTmpl<FbtPower>::create(argc, argv);
    CmdArgsToy& argsToy = axFb.args;
    
    axFb.count_to_exit = argsToy["cycles"].get<int>(300);

    const double pos_list[] = {5, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 75, 80.0};
    axFb.run([&](){
        auto i = axFb.getIndex();
        // Enable the ControlAxisByPos function block at cycle 1
        if (i >= 2 ) {
            axFb.mov2.iStatus = 3;
            axFb.mov2.bEnable = true;
            axFb.mov2.fSetPosition = pos_list[(i-2)%10];
        }else{
            axFb.mov2.bEnable = false;
        }        
    });
    return 0;
}



int motNode_controlAxisByPos_demo(int argc, char** argv) {
    if (argc && (argv[0] == "--help" || argv[0] == "-h")){
        printf("Usage: --help \n [--help] [--cycle_time <int>] [--set_position <double>] [--gap_velocity <double>] [--gap_acceleration <double>] [--gap_deceleration <double>] [--gap_jerk <double>] [--avoid_gaps <bool>] [--verbose <int>] [--cycles <int>]" );
        return 0;
    }
    auto& axFb = *AxSgControlAxisByPosTmpl<FbtInterpolatorToy>::create(argc, argv);
    CmdArgsToy& argsToy = axFb.args;
    
    axFb.count_to_exit = argsToy["cycles"].get<int>(100);

    // const double pos_list[] = {5, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 75, 80.0};
    const double pos_list[] = {1, 1.01, 1.015, 1.0155, 1.02,1.05, 1.1, 1.15, -2, 1, 30.0, 40.0, 50.0, 60.0, 70.0, 75, 80.0};
    FbtOutqueueToy oq;
    oq.pData = (double*)pos_list;
    oq.nCapacity = 6;
    // oq.ratio = 0.0002;
    axFb.mov.poqDataIn = &oq;
    axFb.verbose_flag = 9;
    axFb.axRef(0).bind_extra_data(&axFb.extra_data);
    axFb.run([&](){
        auto i = axFb.getIndex();
        // Enable the ControlAxisByPos function block at cycle 1
        if (i>=2){
            axFb.mov.bExecute = true;
            axFb.mov.bEmergency_Stop = axFb.mov2.bStopIpo;
        }
        if (i >= 4 ) {
            axFb.mov2.bEnable = axFb.mov.bWorking;
            axFb.mov2.iStatus = axFb.mov.iStatus;
            axFb.mov2.fSetPosition = axFb.mov.piSetPosition;
        }else{
            axFb.mov2.bEnable = axFb.mov.bWorking;;
        }       
        if (i>=230 && i< 232){
            axFb.mov.bExecute = false;
            oq.curIndex = 0;
        }else if (i>=232){
            axFb.mov.bExecute = true;
            oq.nCapacity = 9;
            
        }
        axFb.extra_data = axFb.mov2.bBusy + axFb.mov2.bStopIpo*2 + axFb.mov2.bCommandAborted*5 + axFb.mov2.bError*4 + 
        (axFb.mov.bBusy + axFb.mov.bDone*3 + axFb.mov.bError*4)*16 + 
        axFb.mov.iStatus*4096 + axFb.mov.bWorking*256;
    });
    return 0;
}
