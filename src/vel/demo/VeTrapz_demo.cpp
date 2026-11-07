#if 0
#include "vel_forward_trapz.h"
#include "vel_forward_trapz_rev.h"
#include "stdio.h"


using namespace zmc;
void velTrapz_demo(){
    VelTrapz vt;
    vt.set_config(10, 5, 5, 0);
    vt.set_begin_end_pos_vel(0, 10, 1, 0);
    vt.build();
    double planTime, vMax;
    vt.get_plan(&planTime, &vMax, nullptr, nullptr);
    printf("%f,%f\n", planTime, vMax);
    printf("idx,time,pos,vel,acc\n");
    double pos, vel, acc;
    for (int i=0; i<30; i++){
        double tm = i*0.2;
        vt.get_slice(tm, &pos, &vel, &acc);
        printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    }

}



void velTrapzRev_demo(){
    VelTrapzRev vt;
    vt.set_config(999, 5, 5, 0);
    vt.set_begin_end_pos_vel(0, 12, 2, 0.5);
    // lst = [12, 0.15, 2, 0, 999, 3, 3, 0, 15.468888888888889]
    
    vt.param.set_total_time(5);
    vt.build();
    if (vt.check())
        printf("error \n");
    double planTime, vMax;
    vt.get_plan(&planTime, &vMax, nullptr, nullptr);
    printf("%f,%f\n", planTime, vMax);
    double pos, vel, acc;
    for (int i=0; i<10; i++){
        double tm = i*0.5;
        vt.get_slice(tm, &pos, &vel, &acc);
        printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    }

}

void velTrapz_shift_pos_demo(){
    VelTrapz vt;
    vt.set_config(10, 5, 5, 0);
    vt.set_begin_end_pos_vel(0, 10, 1, 0);
    vt.build();
    double planTime, vMax;
    vt.shift_pos(-10);
    vt.get_plan(&planTime, &vMax, nullptr, nullptr);
    printf("%f,%f\n", planTime, vMax);
    printf("idx,time,pos,vel,acc\n");
    double pos, vel, acc;
    for (int i=0; i<30; i++){
        double tm = i*0.2;
        vt.get_slice(tm, &pos, &vel, &acc);
        printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    }

}

void velTrapz_shift_time_demo(){
    VelTrapz vt;
    vt.set_config(10, 5, 5, 0);
    vt.set_begin_end_pos_vel(0, 10, 1, 0);
    vt.build();
    double planTime, vMax;
    vt.shift_time(1);
    vt.get_plan(&planTime, &vMax, nullptr, nullptr);
    printf("%f,%f\n", planTime, vMax);
    printf("idx,time,pos,vel,acc\n");
    double pos, vel, acc;
    for (int i=0; i<30; i++){
        double tm = i*0.2;
        vt.get_slice(tm, &pos, &vel, &acc);
        printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    }
}

void velTrapz_time_demo(){
    VelTrapzRev vt;
    vt.set_config(10, 2, 2, 0);
    vt.set_begin_end_pos_vel(0, -10, 0, 0);
    vt.param.Tc = 9;
    vt.build();
    double planTime, vMax;
    vt.get_plan(&planTime, &vMax, nullptr, nullptr);
    printf("%f,%f\n", planTime, vMax);
    printf("idx,time,pos,vel,acc\n");
    double pos, vel, acc;
    for (int i=0; i<60; i++){
        double tm = i*0.2;
        vt.get_slice(tm, &pos, &vel, &acc);
        printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    }
}
#endif