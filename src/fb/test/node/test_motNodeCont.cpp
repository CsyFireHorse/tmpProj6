// #include "MotNode.h"
// #include "PfbMoveCont.h"
// #include "gtest/gtest.h"
// #include "include/velocity_utils.hpp"


#include <gtest/gtest.h>
#include "AxisRunTmpl.hpp"
// #include "AxisFbCompose.hpp"
#include "common/misc/AxisChecker.hpp"
#include "fbt_axis.h"
#include "fbt_axis_impl.h"
#include "fb_cpp_def.h"
#include "fbt_fb_base.hpp"
#include "math.h"
#include "stdio.h"
// #include "velocity_utils.hpp"

using namespace zmc;

template<typename T>
struct AxContTestTmpl : public AxSgMotTmpl<T, 1> {
    AxContTestTmpl() {
        this->get_config_ref(0).debug.servo_fak_pos = 4;
        this->get_config_ref(0).cycle_time = 200000;
        this->reload_config(0);
        this->mov.Axis = this->axisRef(0);
        set_fb_vel_config(this->mov, 5, 3, 3, 0);
        this->count_to_exit = 100;
    }
};

TEST(motNode, vel) {
    AxContTestTmpl<FbtMoveVelocity> axFb;
    // axFb.mov.Direction = mcShortestWay;
    
    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);
        
        if (i == 1) {
            EXPECT_NEAR(ax.fSetPosition, 4, 1e-3);
            axFb.mov.Execute = true;
        } else if (i == 2) {
            EXPECT_EQ(ax.axisStat, mcContinuousMotion);
            EXPECT_TRUE(axFb.mov.Busy);
        } else if (i == 19) {
            EXPECT_NEAR(ax.fSetVelocity, 5, 1e-3);
            EXPECT_TRUE(axFb.mov.InVelocity);
        }
        EXPECT_EQ(0, axFb.check_set_pva(0));
    });
}

TEST(motNode, vel_reload) {
    AxContTestTmpl<FbtMoveVelocity> axFb;
    axFb.get_config_ref(0).debug.servo_fak_pos = 5;
    axFb.reload_config(0);
    // axFb.mov.Direction = mcShortestWay;
    axFb.count_to_exit = 30;
    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);
        
        if (i == 1) {
            axFb.mov.Execute = true;
        } else if (i == 2) {
            // EXPECT_NEAR(ax.fSetVelocity, 1, 1e-3);
            EXPECT_FALSE(axFb.mov.InVelocity);
        } else if (i == 15) {
            EXPECT_NEAR(ax.fSetVelocity, 5, 1e-3);
            EXPECT_TRUE(axFb.mov.InVelocity);
            axFb.mov.Execute = false;
        } else if (i == 16) {
            axFb.mov.Velocity = 3;
            axFb.mov.Execute = true;
        } else if (i == 30) {
            EXPECT_NEAR(ax.fSetVelocity, 3, 1e-3);
            EXPECT_TRUE(axFb.mov.InVelocity);
        }
        EXPECT_EQ(0, axFb.check_set_pva(0));
        // printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    });
}

TEST(motNode, move_cont_abs) {
    AxContTestTmpl<FbtMoveContinuousAbsolute> axFb;
    axFb.get_config_ref(0).debug.servo_fak_pos = 2;
    axFb.reload_config(0);
    
    axFb.mov.Position = 10;
    set_fb_vel_config(axFb.mov, 5, 4, 5, 0);
    axFb.mov.EndVelocity = 3;
    // axFb.mov.Direction = mcShortestWay;
    axFb.mov.EndVelocityDirection = mcNegativeDirection;
    axFb.count_to_exit = 30;

    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);
        
        if (i == 1) {
            EXPECT_NEAR(ax.fSetPosition, 2, 1e-3);
            EXPECT_FALSE(axFb.mov.InEndVelocity);
            EXPECT_FALSE(axFb.mov.PositionReached);
            axFb.mov.Execute = true;
        } else if (i == 2) {
            EXPECT_EQ(ax.axisStat, mcContinuousMotion);
            EXPECT_TRUE(axFb.mov.Busy);
        } else if (i == 28) {
            EXPECT_NEAR(ax.fSetVelocity, -3, 1e-3);
            EXPECT_TRUE(axFb.mov.InEndVelocity);
            EXPECT_TRUE(axFb.mov.PositionReached);
        }
        EXPECT_EQ(0, axFb.check_set_pva(0));
        // printf("%d,%f,%f,%f\n", i, ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration);
    });
    
    auto& ax = axFb.axRef(0);
    EXPECT_EQ(ax.axisStat, mcContinuousMotion);
    EXPECT_NEAR(ax.fSetVelocity, -3, 1e-3);
}

TEST(motNode, move_cont_rel) {
    AxContTestTmpl<FbtMoveContinuousRelative> axFb;
    axFb.get_config_ref(0).debug.servo_fak_pos = 5;
    axFb.reload_config(0);

    axFb.mov.Distance = 8;
    axFb.mov.EndVelocity = 3;
    set_fb_vel_config(axFb.mov, 5, 4, 5, 0);
    axFb.mov.EndVelocityDirection = mcNegativeDirection;
    axFb.count_to_exit = 35;

    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);
        
        if (i == 1) {
            axFb.mov.Execute = true;
            EXPECT_NEAR(ax.fSetPosition, 5, 1e-3);
            EXPECT_FALSE(axFb.mov.InEndVelocity);
            EXPECT_FALSE(axFb.mov.DistanceTravelled);
        } else if (i == 33) {
            EXPECT_NEAR(ax.fSetVelocity, -3, 1e-3);
            EXPECT_TRUE(axFb.mov.InEndVelocity);
            EXPECT_TRUE(axFb.mov.DistanceTravelled);
        }
        EXPECT_EQ(0, axFb.check_set_pva(0));
        // printf("%d,%f,%f,%f\n", i, ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration);
    });
    
    auto& ax = axFb.axRef(0);
    EXPECT_EQ(ax.axisStat, mcContinuousMotion);
    EXPECT_NEAR(ax.fSetVelocity, -3, 1e-3);
}

