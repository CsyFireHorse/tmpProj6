#if 1
#include <gtest/gtest.h>

#include "VelUnCppWrap.h"
#include "scurve_base.h"
// #include "VelToyTrapz.h"
#include "math.h"
#include "stdio.h"
#include "velocity_utils.hpp"
#include "vel_test_helper.hpp"
#include "zal_common.h"
using namespace zmc;


TEST(velS, VelocityWrap) {
    VelocityWrap vt;
    double sd = 9;
    vt.set_mode(2);
    vt.set_config(10, 5, 5, 100);
    vt.set_begin_pva(0, 1, 0);
    vt.set_end_pva(sd, 0, 0);

    int ret = vt.build();
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(vt.check(), 0);
    double planTime = vt.get_end_time();
    // printf("%f,%f\n", planTime, vMax);
    double pos, vel, acc;
    for (int i = 0; i < 10; i++) {
        double tm = i * 0.5;
        vt.get_slice(tm, &pos, &vel, &acc);
        // printf("%d,%f,%f,%f\n", i, tm, pos, vel);
    }
    EXPECT_NEAR(pos, sd, 1e-6);
    EXPECT_EQ(pos_vel_check(vt, 0.01), 0);
}


namespace zmc{
    void VelSgParams_to_vel_check(const VelSgParams3& par, double Ts = 0.01){
        VelocityWrap vt;
        vt.set_mode(2);
        vt.set_config(par.V_m, par.A_m, par.A_n, par.Sparam);
        vt.set_begin_pva(0, par.V_0, par.A_0);
        vt.set_end_pva(par.S_d, par.V_d, 0);
        vt.set_total_time(par.Tc);
        int errCode = vt.build();
        EXPECT_EQ(errCode, 0);
        // ZAL_DEBUG_DISP("%d,%d,%d\n", i, errCode, vt.nseg);
        EXPECT_EQ(vt.get_nseg(), int(par.nseg));
        EXPECT_EQ(vt.check(), 0);

        double  plan_time = vt.get_end_time();

        // const double Ts = 0.01;
        int CNT = ceil(plan_time / Ts);
        double sc, vc, ac;
        for (int i = 0; i <= CNT; i++) {
            double tm = Ts * i;
            vt.get_slice(tm, &sc, &vc, &ac);
            if (i == 0) {
                EXPECT_NEAR(sc, 0, 1e-6);
                EXPECT_NEAR(vc, par.V_0, 1e-6);
            }
            // printf("%d,%f,%f,%f,%f\n", i, tm, sc, vc, ac);
        }
        EXPECT_NEAR(sc, par.S_d, 1e-6);
        EXPECT_NEAR(vc, par.V_d, 1e-6);
        EXPECT_EQ(pos_vel_check(vt, Ts*0.1), 0);
    }
}

TEST(velS, AsymVmaxThresholdBoundaries) {
    const double eps = 1e-9;
    const VelSgParams3 par_arr[] = {
        // V_d equals V_m + V_EPS and must build through the single-overlimit branch.
        {100, 9, 10 + eps, 0, 10, 2, 3, 100, 0, SLADDER_62_SEGMENT},
        // V_0 equals V_m + V_EPS and must build through the opposite branch.
        {100, 10 + eps, 9, 0, 10, 2, 3, 100, 0, SLADDER_61_SEGMENT},
    };

    for (int i = 0; i < LEN(par_arr); i++) {
        VelSgParams_to_vel_check(par_arr[i]);
    }
}

TEST(velS, AsymOverlimitRejectsUnreachableDistance) {
    // The minimum jerk-limited distance from 9 to 11 is greater than 2.5.
    VelocityWrap vt;
    vt.set_mode(2);
    vt.set_config(10, 15, 15, 100);
    vt.set_begin_pva(0, 9, 0);
    vt.set_end_pva(2.5, 11, 0);
    EXPECT_EQ(vt.build(), 0);
}


static double Tt = 0;
static const VelSgParams3 par_arr[] = {
        {55, 1, 0, 0.5, 5, 3, 3, 10, Tt, SLADDER_70_SEGMENT},
        {5, 1, 0, 0.5, 5, 3, 3, 10, Tt, SLADDER_60_SEGMENT},
        {15, 1, 0, 0.5, 4, 6, 3, 10, Tt, SLADDER_61_SEGMENT},
        {15, 1, 0, 0.5, 4, 3, 8, 10, Tt, SLADDER_62_SEGMENT},
        {15, 1, 0, 0.5, 3, 6, 6, 10, Tt, SLADDER_50_SEGMENT},
        {5, 1, 0, 0.5, 4, 6, 6, 10, Tt, SLADDER_51_SEGMENT},
        {5, 1, 0, 0.5, 4, 3, 8, 10, Tt, SLADDER_52_SEGMENT},
        {5, 1, 0, 0.5, 15, 30, 30, 10, Tt, SLADDER_40_SEGMENT},
        {5, 1, 0, 0.5, 4, 6, 3, 10, Tt, SLADDER_51_SEGMENT},
        {5, 1, 0, 0.5, 115, 13, 3, 10, Tt, SLADDER_51_SEGMENT},
        {5, 0, 2, 0.5, 5, 3, 13, 10, Tt, SLADDER_52_SEGMENT},
        {5, 0, 2, 0.5, 115, 3, 13, 10, Tt, SLADDER_52_SEGMENT},
    };

TEST(velS, Batch) {
    double Tt = 0;
    for (int i = 0; i < LEN(par_arr); i++) {
        auto& par = par_arr[i];
        printf("idx=%d\n", i);
        VelSgParams_to_vel_check(par);
    }
}

TEST(velS, RevBatch) {
    for (int i = 0; i < LEN(par_arr); i++) {
        auto par = par_arr[i];
        par.invert();
        par.swap_am_an();
        // printf("idx=%d\n", i);
        VelSgParams_to_vel_check(par);
    }
}

TEST(velS, SymmetryBatch) {
    double Tt = 0;
    const VelSgParams3 par_arr[] = {
        {55, 1, 1, 0.0, 5, 3, 3, 10, Tt, SLADDER_70_SEGMENT},
        {5, 1, 1, 0.0, 5, 3, 3, 10, Tt, SLADDER_60_SEGMENT},
        {15, 1, 1, 0.0, 3, 6, 6, 10, Tt, SLADDER_50_SEGMENT},
        {5, 1, 1, 0.0, 15, 30, 30, 10, Tt, SLADDER_40_SEGMENT},
        {10, 3, 3, 0.0, 3, 3, 3, 10, Tt, SLADDER_10_SEGMENT},
        // z symmetry
        {55, 0, 0, 0.0, 5, 3, 3, 10, Tt, SLADDER_70_SEGMENT},
        {5, 0, 0, 0.0, 5, 3, 3, 10, Tt, SLADDER_60_SEGMENT},
        {15, 0, 0, 0.0, 3, 6, 6, 10, Tt, SLADDER_50_SEGMENT},
        {5, 0, 0, 0.0, 15, 30, 30, 10, Tt, SLADDER_40_SEGMENT},
        // inv
        {55, -1, -1, 0.0, 5, 3, 3, 10, Tt, SLADDER_70_SEGMENT},
        // overlimit
        {10, 3, 3, 0.0, 1, 3, 3, 10, Tt, SLADDER_70_SEGMENT},
        {2, 3, 3, 0.0, 1, 3, 3, 10, Tt, SLADDER_40_SEGMENT},
        {4, 3, 3, 0.0, 1, 6, 6, 10, Tt, SLADDER_50_SEGMENT},
        {2.5, 3, 3, 0.0, 2.4, 1.5, 1.5, 10, Tt, SLADDER_60_SEGMENT},
    };
    for (int i = 0; i < LEN(par_arr); i++) {
        auto& par = par_arr[i];
        // printf("idx=%d\n", i);
        VelSgParams_to_vel_check(par);
    }

    for (int i = 0; i < LEN(par_arr); i++) {
        auto par = par_arr[i];
        par.invert();
        // printf("idx=%d\n", i);
        VelSgParams_to_vel_check(par);
    }
}


#endif
