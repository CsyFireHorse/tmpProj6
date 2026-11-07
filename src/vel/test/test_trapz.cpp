#include <gtest/gtest.h>

#include "TrapzVel.h"
#include "math.h"
#include "stdio.h"
#include "vel_test_helper.hpp"
#include "vel_test_data.hpp"
// #include "velocity_utils.hpp"
#include "vel_sin_square.hpp"
#include "scurve_base.h"

using namespace zmc;
#define LEN(x) sizeof(x)/sizeof(*(x))


TEST(vel, velToyTrapz) {
    TrapzVel vt;
    double sd = 99;
    vt.set_config(10, 5, 5, 0);
    vt.set_begin_pva(0.1 , 1);
    vt.set_end_pva(sd,  0);
    int ret = vt.build();
    EXPECT_EQ(ret, 0);
    double planTime;
    planTime = vt.get_end_time();
    // printf("%f,%f\n", planTime, vMax);
    // double pos, vel;
    // for (int i = 0; i < 10; i++) {
        // double tm = i * 0.5;
        // vt.get_slice(tm, &pos, &vel, nullptr);
        // printf("%d,%f,%f,%f\n", i, tm, pos, vel);
    // }
    // EXPECT_NEAR(pos, sd, 1e-6);
    // pos_vel_check(vt, 0.05);
}


// TEST(vel, fault) {
//     VelocityWrap vt;
//     double sd = 9;
//     vt.set_config(10, 5, 5, 0);
//     vt.set_begin_end_pos_vel(0, sd, 1, 100);
//     int ret = vt.build();
//     EXPECT_EQ(ret, 1);
//     EXPECT_EQ(vt.check(), 1);
// }


TEST(vel, TrapzBatch) {
    double T = 0;
    auto& par_arr = test_vel::trapz_param;
    for (int i = 0; i < LEN(par_arr); i++) {
        auto& par = par_arr[i];
        TrapzVel vt;
        printf("index=%d\n", i);
        VelSgParams_to_vel_check<TrapzVel, VelSgParams2, 40>(vt, par);
    }
}

TEST(vel, TrapzVmaxBoundaryRegression) {
    const double eps = 1e-12;
    VelSgParams2 par_arr[] = {
        // V_m + V_EPS rounds to V_m; the V_d overlimit branch must still build.
        {0, 20000, 9999, 19999, 19999, 9999, 9999, 0, 0, 0},
        // An endpoint just below V_m remains on the in-limit path.
        {0, 20000, 9999, 19998, 19999, 9999, 9999, 0, 0, 0},
        // Equality at V_m also exercises the opposite V_0 overlimit branch.
        {0, 20000, 19999, 9999, 19999, 9999, 9999, 0, 0, 0},
    };

    for (int i = 0; i < LEN(par_arr); i++) {
        TrapzVel vt;
        const VelSgParams2& par = par_arr[i];
        VelSgParams_to_vel_check<TrapzVel, VelSgParams2>(vt, par);
    }

    // A small V_m keeps V_m + V_EPS distinct and checks the inclusive comparison itself.
    VelSgParams2 exact_threshold = {0, 10, 0.5, 1 + eps, 1, 1, 1, 0, 0, 0};
    TrapzVel vt;
    VelSgParams_to_vel_check<TrapzVel, VelSgParams2>(vt, exact_threshold);

    // Both endpoints at or above the threshold still use the double-overlimit path.
    VelSgParams2 both_overlimit = {0, 1, 1 + eps, 1 + 3 * eps, 1, 1, 1, 0, 0, 0};
    TrapzVel overlimit_vt;
    VelSgParams_to_vel_check<TrapzVel, VelSgParams2>(overlimit_vt, both_overlimit);
}

TEST(velSin, SinSquareVmaxBoundaryRegression) {
    const double eps = 1e-12;
    VelSgParams2 par_arr[] = {
        // Reproduce the large V_m case where adding V_EPS rounds back to V_m.
        {0, 20000, 9999, 19999, 19999, 9999, 9999, 0, 0, 0},
        // Exercise the inclusive V_d comparison where V_m + V_EPS is representable.
        {0, 10, 0.5, 1 + eps, 1, 1, 1, 0, 0, 0},
        // Exercise the corresponding V_0 boundary.
        {0, 20000, 19999, 9999, 19999, 9999, 9999, 0, 0, 0},
    };

    for (int i = 0; i < LEN(par_arr); i++) {
        VelSinSquarePlanner vt;
        const VelSgParams2& par = par_arr[i];
        VelSgParams_to_vel_check<VelSinSquarePlanner, VelSgParams2>(vt, par, 0.001);
    }
}

TEST(vel, TrapzSgRevBatch) {
    double T = 0;
    VelSgParams2 par_arr[] = {
        {0, 10, -0.5, 0, 3, 5, 5, 0, T, LADDER_30_SEGMENT},  // 30
        {0, 10, -1, 2, 3, 5, 5, 0, T, LADDER_30_SEGMENT},    // 30
        {0, 10, 1, -2, 3, 5, 4, 0, T, LADDER_30_SEGMENT},    // 30

        {0, 10, -10, 2, 1, 5, 10, 0, T, LADDER_31_SEGMENT},    // 31
        {0, 10, 2, -10, 1, 5, 10, 0, T, LADDER_32_SEGMENT},    // 32
        {0, 10, 10, -2, 1, 5, 10, 0, T, LADDER_31_SEGMENT},    // 31
        {0, 10, -2, 10, 1, 5, 10, 0, T, LADDER_32_SEGMENT},    // 32
    };

    for (int i = 0; i < LEN(par_arr); i++) {
        VelSgParams2& par = par_arr[i];
        TrapzVel vt;
        VelSgParams_to_vel_check<TrapzVel, VelSgParams2, 40>(vt, par);
    }
}

TEST(vel, TrapzRevBatch) {
    double T = 0;
    auto& par_arr = test_vel::trapz_param;
    for (int i = 0; i < LEN(par_arr); i++) {
        auto par = par_arr[i];
        par.invert();
        TrapzVel vt;
        VelSgParams_to_vel_check<TrapzVel, VelSgParams2, 40>(vt, par);
    }
}


TEST(vel, TrapzFromDoc) {
    double T = 0;
    VelSgParams2 par_arr[] = {
        // S30 sym + head + tail
        // cli -s 5 -v0 -2 -vd -2
        {0, 5, -2, -2, 1, 1, 1, 0, T, LADDER_30_SEGMENT},
        
        // S30 sym + head + tail
        // cli -s 5 -v0 -0.5 -vd -0.5 --dec 2
        {0, 5, -0.5, -0.5, 1, 1, 2, 0, T, LADDER_30_SEGMENT},
        
        // S10
        // cli -s 5 -v0 1 -vd 1 -s0 -1
        {0, 5, 1, 1, 1, 1, 1, 0, T, LADDER_10_SEGMENT},
        
        // S30
        // cli -s 1 -v0 0.2 -vd 0.8 --dec 2
        {0, 1, 0.2, 0.8, 1, 1, 2, 0, T, LADDER_30_SEGMENT},
        
        // S20
        // cli -s 0.1 -v0 0.2 -vd 0.8 --dec 2 -vm 2
        {0, 0.5, 0.2, 0.8, 2, 1, 2, 0, T, LADDER_20_SEGMENT},
        
        // S31
        // cli -s 10 -v0 0.5 -vd 3 --dec 2
        {0, 10, 0.5, 3, 1, 1, 2, 0, T, LADDER_31_SEGMENT},
        
        // S32
        // cli -s 10 -v0 3 -vd 0.5 --dec 2
        {0, 10, 3, 0.5, 1, 1, 2, 0, T, LADDER_32_SEGMENT},
        
        // S34
        // cli -s 10 -v0 2 -vd 3 --dec 2
        {0, 10, 2, 3, 1, 1, 2, 0, T, LADDER_34_SEGMENT},
        
        // S23
        // cli -s 4 -v0 2 -vd 3 --dec 2
        {0, 4, 2, 3, 1, 1, 2, 0, T, LADDER_23_SEGMENT},
        
        // S30 + head
        // cli -s 1 -v0 -5 -vd 0.8 --dec 2
        {0, 1, -5, 0.8, 1, 1, 2, 0, T, LADDER_30_SEGMENT},
        
        // S20 + head
        // cli -s 0.5 -v0 -3 -vd 0.8 --dec 2 -vm 4
        {0, 0.5, -3, 0.8, 4, 1, 2, 0, T, LADDER_20_SEGMENT},
        
        // S34 + head
        // cli -s 1 -v0 5 -vd -0.8 --dec 2
        {0, 1, 5, -0.8, 1, 1, 2, 0, T, LADDER_34_SEGMENT},
        
        // S23 + head
        // cli -s 0.5 -v0 3 -vd -0.8 --dec 2 -vm 4
        {0, 0.5, 3, -0.8, 4, 1, 2, 0, T, LADDER_23_SEGMENT},
        
        // S34 + tail
        // cli -s 1 -v0 -0.8 -vd 5 --dec 2
        {0, 1, -0.8, 5, 1, 1, 2, 0, T, LADDER_34_SEGMENT},
        
        // S23 + tail
        // cli -s 0.5 -v0 -0.8 -vd 3 --dec 2 -vm 4
        {0, 0.5, -0.8, 3, 4, 1, 2, 0, T, LADDER_23_SEGMENT},
        
        // S30 + tail
        // cli -s 1 -v0 0.8 -vd -5 --dec 2
        {0, 1, 0.8, -5, 1, 1, 2, 0, T, LADDER_30_SEGMENT},
        
        // S20 + tail
        // cli -s 0.5 -v0 0.8 -vd -3 --dec 2 -vm 4
        {0, 0.5, 0.8, -3, 4, 1, 2, 0, T, LADDER_20_SEGMENT},
        
        // T20 + tail
        {0, 0.4, 0.2, -0.8, 1, 1, 10, 0, T, LADDER_20_SEGMENT},

        // S21 + head
        // cli -s 0.5 -v0 -3 -vd 1 --dec 2 -vm 1
        {0, 0.5, -3, 1, 1, 1, 2, 0, T, LADDER_21_SEGMENT},
        
        // S22 + tail
        // cli -s 0.5 -v0 1 -vd -3 --dec 2
        {0, 0.5, 1, -3, 1, 1, 2, 0, T, LADDER_22_SEGMENT},
        
        // -S22 + tail
        // cli -s 0.5 -v0 -1 -vd 3 --dec 2
        {0, 0.5, -1, 3, 1, 1, 2, 0, T, LADDER_22_SEGMENT},
        
        // -S22 + head
        // cli -s 0.5 -v0 3 -vd -1 --dec 2
        {0, 0.5, 3, -1, 1, 1, 2, 0, T, LADDER_22_SEGMENT},
        
        // S30 + head + tail
        // cli -s 1 -v0 -0.8 -vd -5 --dec 2
        {0, 1, -0.8, -5, 1, 1, 2, 0, T, LADDER_30_SEGMENT},

        // T20 + head + tail
        {0, 0.1, -0.2, -0.5, 1, 1, 10, 0, T, LADDER_20_SEGMENT},

        // S34 + head + tail
        // cli -s 1 -v0 0.8 -vd 5 --dec 2
        {0, 1, 0.8, 5, 1, 1, 2, 0, T, LADDER_34_SEGMENT},
        
        // S23 + head + tail
        // cli -s 1 -v0 0.8 -vd 2 --dec 2 -vm 4
        {0, 1, 0.8, 2, 4, 1, 2, 0, T, LADDER_23_SEGMENT},
    };
    for (int i = 0; i < LEN(par_arr); i++) {
        VelSgParams2& par = par_arr[i];
        TrapzVel vt;
        printf("index=%d\n", i);
        VelSgParams_to_vel_check<TrapzVel, VelSgParams2, 40>(vt, par);
    }
}

#define T (0)

    VelSgParams2 par_arr[] = {
        // S30 sym + head + tail
        // cli -s 5 -v0 -2 -vd -2
        {0, 5, -2, -2, 1, 1, 1, 0, T, LADDER_30_SEGMENT},
        
        // S30 sym + head + tail
        // cli -s 5 -v0 -0.5 -vd -0.5 --dec 2
        {0, 5, -0.5, -0.5, 1, 1, 2, 0, T, LADDER_30_SEGMENT},
        
        // S10
        // cli -s 5 -v0 1 -vd 1 -s0 -1
        {0, 5, 1, 1, 1, 1, 1, 0, T, LADDER_10_SEGMENT},
        
        // S30
        // cli -s 1 -v0 0.2 -vd 0.8 --dec 2
        {0, 1, 0.2, 0.8, 1, 1, 2, 0, T, LADDER_30_SEGMENT},
        
        // S20
        // cli -s 0.1 -v0 0.2 -vd 0.8 --dec 2 -vm 2
        {0, 0.5, 0.2, 0.8, 2, 1, 2, 0, T, LADDER_20_SEGMENT},
        
        // S31
        // cli -s 10 -v0 0.5 -vd 3 --dec 2
        {0, 10, 0.5, 3, 1, 1, 2, 0, T, LADDER_31_SEGMENT},
        
        // S32
        // cli -s 10 -v0 3 -vd 0.5 --dec 2
        {0, 10, 3, 0.5, 1, 1, 2, 0, T, LADDER_32_SEGMENT},
        
        // S34
        // cli -s 10 -v0 2 -vd 3 --dec 2
        {0, 10, 2, 3, 1, 1, 2, 0, T, LADDER_34_SEGMENT},
        
        // S23
        // cli -s 4 -v0 2 -vd 3 --dec 2
        {0, 4, 2, 3, 1, 1, 2, 0, T, LADDER_23_SEGMENT},
        
        // S30 + head
        // cli -s 1 -v0 -5 -vd 0.8 --dec 2
        {0, 1, -5, 0.8, 1, 1, 2, 0, T, LADDER_30_SEGMENT},
        
        // S20 + head
        // cli -s 0.5 -v0 -3 -vd 0.8 --dec 2 -vm 4
        {0, 0.5, -3, 0.8, 4, 1, 2, 0, T, LADDER_20_SEGMENT},
        
        // S34 + head
        // cli -s 1 -v0 5 -vd -0.8 --dec 2
        {0, 1, 5, -0.8, 1, 1, 2, 0, T, LADDER_34_SEGMENT},
        
        // S23 + head
        // cli -s 0.5 -v0 3 -vd -0.8 --dec 2 -vm 4
        {0, 0.5, 3, -0.8, 4, 1, 2, 0, T, LADDER_23_SEGMENT},
        
        // S34 + tail
        // cli -s 1 -v0 -0.8 -vd 5 --dec 2
        {0, 1, -0.8, 5, 1, 1, 2, 0, T, LADDER_34_SEGMENT},
        
        // S23 + tail
        // cli -s 0.5 -v0 -0.8 -vd 3 --dec 2 -vm 4
        {0, 0.5, -0.8, 3, 4, 1, 2, 0, T, LADDER_23_SEGMENT},
        
        // S30 + tail
        // cli -s 1 -v0 0.8 -vd -5 --dec 2
        {0, 1, 0.8, -5, 1, 1, 2, 0, T, LADDER_30_SEGMENT},
        
        // S20 + tail
        // cli -s 0.5 -v0 0.8 -vd -3 --dec 2 -vm 4
        {0, 0.5, 0.8, -3, 4, 1, 2, 0, T, LADDER_20_SEGMENT},

        // T20 + tail
        {0, 0.7, 0.2, -0.8, 1, 1, 10, 0, T, LADDER_20_SEGMENT},
        
        // S21 + head
        // cli -s 0.5 -v0 -3 -vd 1 --dec 2 -vm 1
        {0, 0.5, -3, 1, 1, 1, 2, 0, T, LADDER_21_SEGMENT},
        
        // S22 + tail
        // cli -s 0.5 -v0 1 -vd -3 --dec 2
        {0, 0.5, 1, -3, 1, 1, 2, 0, T, LADDER_22_SEGMENT},
        
        // -S22 + tail
        // cli -s 0.5 -v0 -1 -vd 3 --dec 2
        {0, 0.5, -1, 3, 1, 1, 2, 0, T, LADDER_22_SEGMENT},
        
        // -S22 + head
        // cli -s 0.5 -v0 3 -vd -1 --dec 2
        {0, 0.5, 3, -1, 1, 1, 2, 0, T, LADDER_22_SEGMENT},
        
        // S30 + head + tail
        // cli -s 1 -v0 -0.8 -vd -5 --dec 2
        {0, 1, -0.8, -5, 1, 1, 2, 0, T, LADDER_30_SEGMENT},
        
        // T20 + head + tail
        {0, 0.1, -0.2, -0.5, 1, 1, 10, 0, T, LADDER_20_SEGMENT},

        // S34 + head + tail
        // cli -s 1 -v0 0.8 -vd 5 --dec 2
        {0, 1, 0.8, 5, 1, 1, 2, 0, T, LADDER_34_SEGMENT},
        
        // S23 + head + tail
        // cli -s 1 -v0 0.8 -vd 2 --dec 2 -vm 4
        {0, 1, 0.8, 2, 4, 1, 2, 0, T, LADDER_23_SEGMENT},
    };

TEST(velSin, TrapzFromDoc) {
    for (int i = 0; i < LEN(par_arr); i++) {
        VelSgParams2& par = par_arr[i];
        VelSinSquarePlanner vt;
        // printf("index=%d\n", i);
        VelSgParams_to_vel_check<VelSinSquarePlanner, VelSgParams2, 40>(vt, par);
    }
}

TEST(velSin, TrapzFromDocInv) {
    for (int i = 0; i < LEN(par_arr); i++) {
        VelSgParams2 par = par_arr[i];
        par.shift_pos(3);
        par.invert();
        VelSinSquarePlanner vt;
        // printf("index=%d\n", i);
        VelSgParams_to_vel_check<VelSinSquarePlanner, VelSgParams2, 40>(vt, par);
    }
}
