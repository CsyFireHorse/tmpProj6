#include "VelGearFollower.h"
#include "common/misc/TrapzVelToyForwarder.hpp"
#include "stdio.h"

using namespace zmc;

// track zero velocity

void _VelFollower_v0_demo(double v0) {
    ToyTrapzVelPlanner vw;
    vw.set_config(5, 3, 3, 0);
    vw.set_begin_pva(10, v0);
    vw.set_end_pva(20, 0);
    vw.build();

    const double Ts = 0.1;
    VelGearFollower egt;
    egt.set_config(6, 3, 3, 0);
    egt.set_begin_pva(0, 0, 0);
    egt.set_num_den(1, 1);
    egt.set_Ts(Ts);

    PosVelAcc mPos;
    PosVelAcc pva;
    // printf("idx,tm,pos,vel,acc,egSt,spos,svel,sacc\n");
    printf("idx,axId,tm,St,pos,vel,acc\n");
    for (int i = 0; i < 42; i++) {
        double tm = i * Ts;
        vw.get_slice(tm, mPos);
        egt.follow(mPos);
        egt.runCycle();
        int ret = egt.get_cur_slice(pva);
        printf("%d,%d,%f,%d,%f,%f,%f\n", i, 0, tm, 0, mPos.pos, mPos.vel, mPos.acc);
        printf("%d,%d,%f,%d,%f,%f,%f\n", i, 1, tm, ret, pva.pos, pva.vel, pva.acc);
    }
}

void VelFollower_demo() { _VelFollower_v0_demo(0); }
void VelFollower_nonzero_vel_demo() { _VelFollower_v0_demo(1); }

// track with low acc
void VelFollower_low_acc_demo() {
    ToyTrapzVelPlanner vw;
    vw.set_config(5, 4, 3, 0);
    vw.set_begin_pva(10, 0);
    vw.set_end_pva(22, 0);
    vw.build();

    const double Ts = 0.1;
    VelGearFollower egt;
    egt.set_config(2, 3, 3, 0);
    egt.set_begin_pva(0, 0, 0);
    egt.set_num_den(1, 1);
    egt.set_Ts(Ts);

    PosVelAcc mPos;
    PosVelAcc pva;
    vw.get_slice(0, mPos);
    printf("idx,axId,tm,St,pos,vel,acc\n");
    for (int i = -2; i < 42; i++) {
        double tm = i * Ts;
        if (i >= 0) vw.get_slice(tm, mPos);
        egt.follow(mPos);
        egt.runCycle();
        int ret = egt.get_cur_slice(pva);
        printf("%d,%d,%f,%d,%f,%f,%f\n", i, 0, tm, 1, mPos.pos, mPos.vel, mPos.acc);
        printf("%d,%d,%f,%d,%f,%f,%f\n", i, 1, tm, ret, pva.pos, pva.vel, pva.acc);
    }
}

// TODO: track s curve