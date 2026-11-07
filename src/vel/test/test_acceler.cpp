#if 0
/*
 * @Author: ChenShengyong
 * @Date: 2024-07-04 18:14:50
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-12-20 15:17:54
 */
#include <gtest/gtest.h>

#include "VelStopper.h"
#include "math.h"
#include "stdio.h"
#include "velocity_utils.hpp"
#include "VelCppWrap.h"
#include "velStopperOnline.hpp"

using namespace zmc;

TEST(vel, stopperOnline){
    VelStopperOnline vt;
    double A_m = 1, V_0 = -3, Ts= 0.1;
    vt.set_config(10, A_m, A_m, 0);
    vt.Ts = Ts;
    vt.set_pos_vel_acc(0, V_0, 0);
    double sc, vc, ac;
    for (int i=0; i<32; i++){
        if (i) vt.build();
        vt.get_slice(0, &sc, &vc, &ac);
        if (i < 30){
            EXPECT_NEAR(vc, V_0 + i * A_m * Ts, 1e-6);
            EXPECT_NEAR(sc, (V_0*V_0 - vc*vc)/2 / -A_m, 1e-6);
        }
    }
    EXPECT_NEAR(vc, 0, 1e-6);
    EXPECT_NEAR(sc, -V_0*V_0 /2 / A_m, 1e-6);
}

TEST(vel, acceler) {
    VelDeceler vt;
    vt.set_config(10, 5, 5, 0);
    vt.set_begin_end_pos_vel(3, 0, 10, 0);
    vt.build();
    double planTime, vMax;
    vt.get_plan(&planTime, &vMax, nullptr, nullptr);
    // printf("%f,%f\n", planTime, vMax);
    double pos, vel, acc;
    for (int i = 0; i < 20; i++) {
        double tm = i * 0.2;
        vt.get_slice(tm, &pos, &vel, &acc);
        // printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    }
    // EXPECT_EQ(vt.check(), 0);
    EXPECT_EQ(pos_vel_check(vt, 0.01), 0);
}

// (V_0, V_d)
// (2, 5)
// (5, 2)
// (-2, -5)
// (-5, -2)
// (-5, 2)
// (-2, 5)
// (2, -5)
// (5, -2)
// (4, -4)

TEST(vel, VelocityWrapAccel)
{
    VelocityWrap vt;
    vt.set_no_dest(true);
    vt.set_config(10, 5, 4, 0);
    vt.set_begin_end_pos_vel(0, 0, 5, 0);
    int ret = vt.build();
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(vt.check(), 0);
    double planTime, vMax;
    vt.get_plan(&planTime, &vMax, nullptr, nullptr);
    // printf("%f,%f\n", planTime, vMax);
    double pos, vel;
    for (int i=0; i<10; i++){
        double tm = i*0.5;
        vt.get_slice(tm, &pos, &vel, nullptr);
        // printf("%d,%f,%f,%f\n", i, tm, pos, vel);
    }
    EXPECT_EQ(pos_vel_check(vt, 0.01), 0);
}

static const PosVelAcc arr[] = {
    {0.00,3, 0},
    {0.60,3, 0},
    {1.20,3, 0},
    {1.80,3, 0}
};

TEST(velChecker, base){
    VelOnlineChecker chk;
    chk.Ts = 0.2;
    for (int i=0; i<4; i++){
        PosVelAcc pva = arr[i];
        if (i == 3)
            pva.vel= 4;
        int ret = chk.check(pva.pos, pva.vel, pva.acc);
        if (i == 3)
            EXPECT_EQ(ret, 2);
        else
            EXPECT_EQ(ret, 0);
    }
    
    chk.reset();
}

TEST(velChecker, base2){
    VelOnlineChecker chk;
    chk.Ts = 0.2;
    for (int i=0; i<4; i++){
        PosVelAcc pva = arr[i];
        if (i == 3)
            pva.pos= 1.7;
        int ret = chk.check(pva.pos, pva.vel, pva.acc);
        if (i == 3)
            EXPECT_EQ(ret, 1);
        else
            EXPECT_EQ(ret, 0);
    }
}

TEST(velAcceler, update_vel){
    VelAcceler vt;
    vt.set_config(10, 5, 4, 0);
    vt.set_begin_end_pos_vel(3, 0, 2, -4);
    vt.build();
    double planTime, vMax;
    vt.get_plan(&planTime, &vMax, nullptr, nullptr);

    double pos, vel, acc;
    double tmOfs = 0;
    VelOnlineChecker chk;
    chk.Ts = 0.2;
    for (int i = 0; i < 35; i++) {
        double tm = i * 0.2;
        if (i == 10) {
            vt.chop_left_time(tm - tmOfs);
            vt.set_param(VEL_INNER_END_VEL, 3);
            vt.build();
            tmOfs += tm - tmOfs;
        } else if (i == 23) {
            vt.chop_left_time(tm - tmOfs);
            vt.set_param(VEL_INNER_END_VEL, 1);
            vt.set_param(VEL_INNER_MAX_DEC, 1);
            vt.build();
            tmOfs += tm - tmOfs;
        }

        vt.get_slice(tm - tmOfs, &pos, &vel, &acc);
        EXPECT_EQ(0, chk.check(pos, vel, acc));
        // printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    }
}
#endif