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
#include "velocity_utils.hpp"

using namespace zmc;


TEST(fbStop, empty) {
    AxisMotEmu axis;
    auto& cfg = axis.get_config_ref();
    cfg.debug.servo_fak_pos = 3;
    cfg.cycle_time = 200000; // 0.2s
    axis.reload_config();
    auto& ax = get_axis_impl_ref((AXIS_REF)&axis);
    // ax.diActVelocity = 6;
    // ax.fActVelocity = 6;
    
    FbtStop mov;
    mov.Axis = (AXIS_REF)&axis;
    mov.Deceleration = 3;
    mov.Jerk = 0;


    for (int i = 0; i < 15; i++) {
        if (i == 1) {
            mov.Execute = true;
        }

        mov.runCycle();
        axis.runCycle();
        if (i == 2) {
            EXPECT_NEAR(ax.fSetPosition, 3, 1e-3);
            // EXPECT_NEAR(ax.fSetVelocity, 6, 1e-3);
            EXPECT_EQ(ax.axisStat, mcStopping);
        } else if (i == 14) {
            EXPECT_NEAR(ax.fSetVelocity, 0, 1e-3);
            EXPECT_TRUE(mov.Done);
            mov.Execute = false;
        } else if (i == 15) {
            EXPECT_FALSE(mov.Done);
        }
        if (i>2)
        EXPECT_EQ(0, axis.check_set_pva());
        // printf("%d,%f,%f,%f\n", i, ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration);
    }
}


template<typename T, typename T2>
struct SgAxFbStpTmp:public AxDbMotTmpl<T, T2, 1>{
    int low_idx = 39;
    int done_idx = 35;
    SgAxFbStpTmp(){
        // this->verbose_flag = true;
        this->get_config_ref(0).cycle_time = 100000;
        this->get_config_ref(0).debug.servo_fak_pos = -300;
        this->reload_config(0);
        set_fb_vel_config(this->mov, 5, 20, 20, 0);
        this->bindAxis(0);
        this->mov2.Deceleration = 10;
        this->mov2.Jerk = 0;
        this->count_to_exit = 70;
    }
    void preRunCycle()override{
        auto _idx = this->getIndex();
        if (this->getIndex() > 5){
            EXPECT_EQ(this->check_set_pva(0), 0);
        }
        int fbStat = this->mov2.Done * 4 + this->mov2.Busy * 3;
        auto& ax = this->axRef(0);
        if (_idx == 2){
            this->mov.Execute = true;
        }else if (_idx == 30) {
            this->mov2.Execute = true;
        } else if (_idx == this->low_idx) {
            this->mov2.Execute = false;
        } else if (_idx >= 31 && _idx < done_idx) {
            // EXPECT_FALSE(this->mov2.Done);
            // EXPECT_TRUE(this->mov2.Busy);
            EXPECT_EQ(fbStat, FB_STAT_BUSY_ACTIVE);
            EXPECT_EQ(ax.axisStat, mcStopping);
        } else if (_idx == done_idx) {
            EXPECT_EQ(fbStat, FB_STAT_DONE);
            EXPECT_EQ(ax.axisStat, mcStopping);
        }else if (_idx > done_idx && _idx > this->low_idx) {
            EXPECT_EQ(ax.axisStat, mcStandstill);
            EXPECT_EQ(fbStat, 0);
        }
        this->fbRunCycle();
    }
    // void postRunCycle()override{
    //     return this->printAxisFb();
    // }
};


TEST(fbStop, base) {
    SgAxFbStpTmp<FbtMoveVelocity, FbtStop> axFb;
    axFb.low_idx = 39;
    // axFb.done_idx = 36;
    axFb.run();
}

TEST(fbStop, base2) {
    SgAxFbStpTmp<FbtMoveVelocity, FbtStop> axFb;
    axFb.low_idx = 32;
    // axFb.done_idx = 36;
    axFb.run();
}

TEST(fbStop, quick) {
    SgAxFbStpTmp<FbtMoveVelocity, FbtStop> axFb;
    axFb.done_idx = 32;
    axFb.mov2.Deceleration = 100;
    axFb.run();
}

template<typename T>
struct SgAxFbHaltTmp:public SgAxFbStpTmp<T, FbtHalt>{
    void preRunCycle()override{
        auto _idx = this->getIndex();
        auto& ax = this->axRef(0);
        int fbStat = this->mov2.Done * 4 + this->mov2.Busy * 3;
        if (_idx == 2) {
            this->mov.Execute = true;
        }else if (_idx == 30) {
            this->mov2.Execute = true;
        } else if (_idx == this->low_idx) {
            this->mov2.Execute = false;
        } else if (_idx >= 31 && _idx < this->done_idx) {
            EXPECT_EQ(fbStat, FB_STAT_BUSY_ACTIVE);
            EXPECT_EQ(ax.axisStat, mcDiscreteMotion);
        } else if (_idx == this->done_idx) {
            EXPECT_EQ(fbStat, FB_STAT_DONE);
            EXPECT_EQ(ax.axisStat, mcStandstill);
        }else if (_idx > this->done_idx && _idx > this->low_idx) {
            EXPECT_EQ(ax.axisStat, mcStandstill);
            EXPECT_EQ(fbStat, 0);
        }
        this->fbRunCycle();
    }
};

TEST(fbHalt, base) {
    SgAxFbHaltTmp<FbtMoveVelocity> axFb;
    axFb.run();
};
TEST(fbHalt, base2) {
    SgAxFbHaltTmp<FbtMoveVelocity> axFb;
    axFb.low_idx = 32;
    // axFb.done_idx = 36;
    axFb.run();
};
TEST(fbHalt, quick) {
    SgAxFbHaltTmp<FbtMoveVelocity> axFb;
    axFb.done_idx = 32;
    // axFb.done_idx = 36;
    axFb.mov2.Deceleration = 100;
    axFb.run();
}