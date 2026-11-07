#if 0
#include "VelWrapImpl.h"
#include "stdio.h"
#include "vel_combiner.hpp"
#include "vel_combiner2.hpp"

using namespace zmc;

void velCombiner_demo() {
    VelCombiner<VelocityWrapImpl> vt;
    vt.set_config(10, 5, 5, 0);
    vt.append_tail(5);
    vt[0].set_begin_end_vel(1, 0);
    int ret = vt.build();
    double planTime;
    planTime = vt.get_end_time();
    printf("idx,time,pos,vel,acc\n");
    double pos, vel, acc;
    for (int i = 0; i < 40; i++) {
        double tm = i * 0.1;
        vt.get_slice(tm, &pos, &vel, &acc);
        printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    }
}

void velCombiner_db_demo() {
    VelCombiner<VelocityWrapImpl> vt;
    vt.set_config(10, 5, 5, 0);
    vt.append_tail(5);
    vt[0].set_begin_end_vel(0.5, 0);

    vt.set_config(10, 5, 5, 0);
    vt.append_tail(-3);
    vt[1].set_begin_end_vel(0, 0);
    int ret = vt.build();
    double planTime;
    planTime = vt.get_end_time();
    printf("idx,time,pos,vel,acc\n");
    double pos, vel, acc;
    for (int i = 0; i < 40; i++) {
        double tm = i * 0.1;
        vt.get_slice(tm, &pos, &vel, &acc);
        printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    }
}

void velCombiner_dyn_db_demo0() {
    VelCombiner<VelocityWrapImpl> vt;
    vt.set_config(10, 5, 5, 0);
    vt.append_tail(5);
    vt[0].set_begin_end_vel(0.5, 0);

    int ret = vt.build();
    double planTime;
    planTime = vt.get_end_time();
    printf("idx,time,pos,vel,acc\n");
    double pos, vel, acc;
    for (int i = 0; i < 55; i++) {
        double tm = i * 0.1;
        if (i == 12) {
            vt.set_config(10, 5, 5, 0);
            vt.append_tail(-3);
            vt[1].set_begin_end_vel(1, 0);
            vt.timeTo(tm);
            vt.build();
        }
        if (i == 25) {
            vt.set_config(10, 5, 5, 0);
            vt.append_tail(4);
            vt[2].set_begin_end_vel(0, 1);
            vt.timeTo(tm);
            vt.build();
        }
        vt.get_slice(tm, &pos, &vel, &acc);
        printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    }
}

void velCombiner_dyn_db_demo() {
    VelCombinerOnline<VelocityWrapImpl> vt;
    vt.set_config(10, 5, 5, 0);
    vt.append_tail(5);
    vt[0].set_begin_end_vel(0.5, 0);

    int ret = vt.build();
    double planTime;
    printf("idx,time,pos,vel,acc\n");
    double pos, vel, acc;
    int i=0;
    while (1) {
        double tm = i * 0.1;
        if (i == 12) {
            vt.set_config(10, 5, 5, 0);
            vt.append_tail(-3);
            vt.back().set_begin_end_vel(1, 0);
            vt.timeToAuto();
            vt.build();
        }
        if (i == 25) {
            vt.set_config(10, 5, 5, 0);
            vt.append_tail(4);
            vt.back().set_begin_end_vel(0, 1);
            vt.timeToAuto();
            vt.build();
        }
        vt.runCycle();
        PosVelAcc pva;
        int ret = vt.get_commands(pva);
        printf("%d,%f,%f,%f,%f\n", i, tm, pva.pos, pva.vel, pva.acc);
        i++;
        if (ret == 0) break;
    }
}
#endif