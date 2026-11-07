#if 0
#include "VelGearFollower.h"
#include "VelUnCppWrap.h"
#include "stdio.h"

using namespace zmc;

// track zero velocity
void VelFollower_demo0() {

    VelocityWrap vw;
    vw.set_begin_end_acc();
    vw.set_config(5, 3, 3, 0);
    vw.set_begin_end_pos_vel(10, 20, 0, 0);
    vw.build();

    const double Ts = 0.1;
    VelGearFollower egt;
    egt.set_config(2, 3, 3, 0);
    egt.set_begin_end_pos_vel(0, 0, 0, 0);
    egt.set_begin_pos(0, 0, 0);
    egt.num = 1;
    egt.den = 1;
    egt.Ts = Ts;

    PosInfo mPos = {5, -4, 0};
    double pos, vel, acc;
    for (int i = -2; i < 42; i++) {
        double tm = i * Ts;
        vw.get_slice(tm, &mPos.pos, &mPos.vel, &mPos.acc);
        egt.follow(mPos);
        egt.build();
        pos = egt.get_distance(tm);
        vel = egt.get_rate(tm);
        acc = egt.get_acc(tm);
        printf("%d,%f,%f,%f,%f,%f,%f,%f\n", i, tm, mPos.pos, mPos.vel, mPos.acc, pos, vel ,acc);
    }
}
// track with non-zero velocity
void VelFollower_demo() {

    VelocityWrap vw;
    vw.set_begin_end_acc();
    vw.set_config(5, 3, 3, 0);
    vw.set_begin_end_pos_vel(10, 20, 1, 0);
    vw.build();

    const double Ts = 0.1;
    VelGearFollower egt;
    egt.set_config(2, 3, 3, 0);
    egt.set_begin_end_pos_vel(0, 0, 0, 0);
    egt.set_begin_pos(0, 0, 0);
    egt.num = 1;
    egt.den = 1;
    egt.Ts = Ts;

    PosInfo mPos = {5, -4, 0};
    double pos, vel, acc;
    printf("idx,axisId,time,pos,vel,acc\n");
    for (int i = 0; i < 42; i++) {
        double tm = i * Ts;
        vw.get_slice(tm, &mPos.pos, &mPos.vel, &mPos.acc);
        egt.follow(mPos);
        egt.build();
        pos = egt.get_distance(tm);
        vel = egt.get_rate(tm);
        acc = egt.get_acc(tm);
        printf("%d,%d,%f,%f,%f,%f\n", i, 0, tm, mPos.pos, mPos.vel, mPos.acc);
        printf("%d,%d,%f,%f,%f,%f\n", i, 1, tm, pos, vel ,acc);
    }
}
// track with low acc
void VelFollower_demo3() {
    VelocityWrap vw;
    vw.set_begin_end_acc();
    vw.set_config(5, 4, 3, 0);
    vw.set_begin_end_pos_vel(10, 22, 0, 0);
    vw.build();

    const double Ts = 0.1;
    VelGearFollower egt;
    egt.set_config(2, 3, 3, 0);
    egt.set_begin_end_pos_vel(0, 0, 0, 0);
    egt.set_begin_pos(0, 0, 0);
    egt.num = 1;
    egt.den = 1;
    egt.Ts = Ts;

    PosInfo mPos = {5, -4, 0};
    double pos, vel, acc;
    printf("idx,axisId,time,pos,vel,acc\n");
    for (int i = -2; i < 42; i++) {
        double tm = i * Ts;
        vw.get_slice(tm, &mPos.pos, &mPos.vel, &mPos.acc);
        egt.follow(mPos);
        egt.build();
        pos = egt.get_distance(tm);
        vel = egt.get_rate(tm);
        acc = egt.get_acc(tm);
        printf("%d,%d,%f,%f,%f,%f\n", i, 0, tm, mPos.pos, mPos.vel, mPos.acc);
        printf("%d,%d,%f,%f,%f,%f\n", i, 1, tm, pos, vel ,acc);
    }
}


void VelFollower_demo9() {
    const double Ts = 0.1;
    VelGearFollower egt;
    egt.set_config(2, 3, 3, 0);
    egt.set_begin_end_pos_vel(0, 0, 0, 0);
    egt.set_begin_pos(0, 0, 0);
    egt.num = 1;
    egt.den = 1;
    egt.Ts = Ts;

    PosInfo mPos = {5, -4, 0};
    double pos, vel, acc;
    for (int i = 0; i < 18; i++) {
        double tm = i * Ts;
        mPos.pos += mPos.vel * Ts;
        if (i > 8) mPos.acc = 0;
        egt.follow(mPos);
        egt.build();
        pos = egt.get_distance(tm);
        vel = egt.get_rate(tm);
        acc = egt.get_acc(tm);
        printf("%d,%f,%f,%f,%f,%f,%f\n", i, tm, mPos.pos, mPos.vel, pos, vel ,acc);
    }
}

// track s curve 
void VelFollower_demo2() {
    const double Ts = 0.1;
    VelocityWrap vw;
    vw.set_begin_end_acc();
    vw.set_config(5, 3, 3, 10);
    vw.set_begin_end_pos_vel(0, 55, 1, 0);
    vw.build();

    VelocityWrap egt; 
    egt.set_slave(true);
    egt.init();
    egt.set_begin_end_acc();
    egt.set_config(5, 4, 3, 10);
    egt.set_begin_end_pos_vel(0, 0, 0, 0);
    egt.set_num_den(2, 1);
    egt.set_time_sample(Ts);
    egt.build();
    double pos, vel, acc;
    double sc, vc, ac;
    printf("idx,axisId,time,pos,vel,acc\n");
    for (int i = 0; i < 180; i++) {
        double tm = i * Ts;
        vw.get_slice(tm, &sc, &vc, &ac);
        if (i>10){
            egt.follow(sc, vc, ac);
            egt.build();
        }
        int inGear = !egt.get_slice(tm, &pos, &vel, &acc);
        // printf("%d,%f,%f,%d,%f,%f,%f\n", i, tm, sc, inGear, pos, vel ,acc);
        printf("%d,%d,%f,%f,%f,%f\n", i, 0, tm, sc, vc, ac);
        printf("%d,%d,%f,%f,%f,%f\n", i, 1, tm, pos, vel ,acc);
    }
}
#endif