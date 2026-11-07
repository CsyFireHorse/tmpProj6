#if 1
#include <gtest/gtest.h>
// #include "VelToyTrapz.h"
#include "VelUnCppWrap.h"
#include "stdio.h"
#include "math.h"
#include "velocity_utils.hpp"
#include "vel_test_data.hpp"
#include "scurve_base.h"

using namespace zmc;

#define LEN(x) sizeof(x)/sizeof(*(x))

template <typename P>
void VelSgParams_to_vel_check(const P& par) {
    VelocityWrap vt;
    VelSgParams_to_vel_check(vt, par);
}


TEST(VelToyTrapz, base)
{
    VelocityWrap vt; 
    // VelToyTrapz vt;
    double sd = 9;
    vt.set_config(10, 5, 5, 0);
    vt.set_begin_pva(0, 1);
    vt.set_end_pva(sd, 0);
    int ret = vt.build();
    EXPECT_EQ(ret, 0);
    double planTime, vMax;
    vt.get_plan(&planTime, &vMax, nullptr, nullptr);
    // printf("%f,%f\n", planTime, vMax);
    double pos, vel;
    for (int i=0; i<10; i++){
        double tm = i*0.5;
        vt.get_slice(tm, &pos, &vel, nullptr);
        // printf("%d,%f,%f,%f\n", i, tm, pos, vel);
    }
    EXPECT_NEAR(pos, sd, 1e-6);
    EXPECT_EQ(pos_vel_check(vt, 0.05), 0);
}

TEST(velWrap, base)
{
    VelocityWrap vt;
    double sd = 9;
    vt.set_config(10, 5, 5, 0);
    vt.set_begin_pva(0, 1);
    vt.set_end_pva(sd, 0);
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
    EXPECT_NEAR(pos, sd, 1e-6);
}

TEST(velWrap, DISABLED_fault)
{
    VelocityWrap vt;
    double sd = 9;
    vt.set_config(10, 5, 5, 0);
    vt.set_begin_pva(0, 1);
    vt.set_end_pva(sd, 100);
    int ret = vt.build();
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(vt.check(), 1);
}


TEST(velWrap, TrapzBatch)
{
    double T=0;
    auto& par_arr = test_vel::trapz_param;
    for (int i=0; i< LEN(par_arr); i++){
        auto par = par_arr[i];
        VelSgParams_to_vel_check(par);
    }
}


TEST(velWrap, TrapzVelRevBatch)
{
    double T=0;
    VelSgParams2 par_arr[] = {
        {0, 10, -0.5, 0, 3, 5, 5, 0, T, LADDER_30_SEGMENT}, // 30
        {0, 10, -1, 2, 3, 5, 5, 0, T, LADDER_30_SEGMENT}, // 30
        {0, 10, 1, -2, 3, 5, 4, 0, T, LADDER_30_SEGMENT}, // 30
    };
    for (int i=0; i< LEN(par_arr); i++){
        VelSgParams2& par = par_arr[i];
        VelSgParams_to_vel_check(par);
    }
}

TEST(velWrap, TrapzRevBatch)
{
    auto& par_arr = test_vel::trapz_param;
    for (int i=0; i< LEN(par_arr); i++){
        auto par = par_arr[i];
        par.invert();
        par.swap_am_an();
        VelSgParams_to_vel_check(par);
    }
}

TEST(velWrap, DISABLED_TrapzTime)
{
    VelocityWrap vt;
    vt.set_config(30, 5, 5, 0);
    double tc = 6;
    vt.set_begin_pva(0, 1);
    vt.set_end_pva(12, 0);
    vt.set_total_time(tc);
    int ret = vt.build();
    EXPECT_EQ(ret, 0);
    double planTime;
    planTime = vt.get_end_time();
    double pos, vel;
    for (int i=0; i<14; i++){
        double tm = i*0.5;
        vt.get_slice(tm, &pos, &vel, nullptr);
        // printf("%d,%f,%f,%f\n", i, tm, pos, vel);
    }
    EXPECT_NEAR(pos, 12, 1e-6);
    EXPECT_NEAR(planTime, tc, 1e-6);
    EXPECT_EQ(pos_vel_check(vt, 0.05), 0);
}

TEST(velWrap, TrapzTimeBatch)
{
    double T=0;
    VelSgParams2 par_arr[] = {
        {0, 10, 1, 1.5, 3, 5, 3, 0, 20, LADDER_34_SEGMENT},
        {0, 10, 1, 1.5, 3, 5, 3, 0, 8, LADDER_32_SEGMENT},
        {0, 10, 1, 1.5, 3, 5, 3, 0, 6, LADDER_30_SEGMENT}, 
        {0, 10, 1, 1.5, 3, 5, 3, 0, 2.65, LADDER_30_SEGMENT},
        {0, 10, 1.5, 1, 3, 3, 5, 0, 8, LADDER_31_SEGMENT},
    };
    for (int i=0; i< LEN(par_arr); i++){
        VelSgParams_to_vel_check(par_arr[i]);
    }
}



TEST(velWrap, TrapzRevTimeBatch)
{
    double T=0;
    VelSgParams2 par_arr[] = {
        {0, -10, 0, 0, 3, 5, 3, 0, 10, LADDER_30_SEGMENT},  
        {0, -10, 0.1, 0.1, 3, 5, 3, 0, 10, LADDER_30_SEGMENT},  
        {0, -10, -0.1, -0.1, 3, 5, 3, 0, 10, LADDER_30_SEGMENT},  
        // {0, -10, 0.1, 0, 3, 5, 3, 0, 10, LADDER_30_SEGMENT},  
        // {0, -10, 0, 0.1, 3, 5, 3, 0, 10, LADDER_30_SEGMENT},  

        // {0, -10, -0.1, 0, 3, 5, 3, 0, 10, LADDER_30_SEGMENT},  
        // {0, -10, 0, 0.1, 3, 5, 3, 0, 10, LADDER_30_SEGMENT},  
        // {0, -10, 0.1, -0.1, 3, 5, 3, 0, 10, LADDER_30_SEGMENT},  
        // {0, -10, -0.1, 0.1, 3, 5, 3, 0, 10, LADDER_30_SEGMENT}, 
     
        // {0, -10, 0.1, 0.2, 3, 5, 3, 0, 10, LADDER_30_SEGMENT},  
        // {0, -10, 0.2, 0.1, 3, 5, 3, 0, 10, LADDER_30_SEGMENT},  

    };
    for (int i=0; i< LEN(par_arr); i++){
        VelSgParams_to_vel_check(par_arr[i]);
    }
}
#endif