#include <gtest/gtest.h>
#include "fb_cpp_def.h"
#include "AxisRunTmpl.hpp"
#include "math.h"
#include "stdio.h"

using namespace zmc;

template<typename R, typename S>
class FbDbMoveTmpl:public AxDbMotTmpl<R, S>{
public:
    FbDbMoveTmpl(int axisNum = 1) {
        this->get_config_ref(0).linear.increments_units_in_application_num = 10;
        this->get_config_ref(0).linear.increments_units_in_application_den = 1;
        this->get_config_ref(0).debug.servo_fak_pos = -1000;
        this->get_config_ref(0).cycle_time = 10000;
        this->reload_config(0);

        set_fb_vel_config(this->mov, 200, 400, 400, 0);
        this->mov.Axis = this->axisRef(0);
        this->mov2.Axis = this->axisRef(0);
        set_fb_vel_config(this->mov2, 250, 300, 300, 0);

        this->mov.Distance = Pos;
        this->mov2.Distance = Pos;
    }
    void preRunCycle() override{
        auto _idx = this->getIndex();
        if (_idx == 5) {
            this->mov.Execute = true;
        } else if (_idx == _trig_idx) {
            this->mov2.Execute = true;
        }
        this->fbRunCycle();
        // auto _idx = this->getIndex();
        auto& ax = this->axRef(0);
        if (_idx == 4) {
            EXPECT_EQ(ax.axisStat, mcStandstill);
        } else if (_idx == 5) {
            EXPECT_EQ(this->mov.Busy, true);
            EXPECT_EQ(ax.axisStat, mcDiscreteMotion);
        } else if (_idx == _trig_idx) {
            EXPECT_EQ(this->mov2.Busy, true);
            // EXPECT_EQ(this->mov2.Active, false);
            EXPECT_EQ(ax.axisStat, mcDiscreteMotion);
        } else if (_idx == done_idx) {
            EXPECT_EQ(this->mov.Done, true);
        } else if (_idx == done_idx * 2 + 1) {
            EXPECT_EQ(this->mov2.Done, true);
            EXPECT_EQ(ax.axisStat, mcStandstill);
            EXPECT_NEAR(ax.fSetPosition, 100, 1e-6);
        }
        if (_idx > 5) {
            EXPECT_EQ(this->check_set_pva(0), 0);
        }
    }
   public:
    int _trig_idx = 116;
    int done_idx = 117;
    const int scale = 10;
    const int fakPos = -1000;
    const double Pos = 100;
};



TEST(axis, moveadd_idle_add) {
    FbDbMoveTmpl<FbtMoveAdditive, FbtMoveAdditive> asw;
    asw.count_to_exit = 240;
    asw.run();
}


TEST(axis, moveadd_idle_rel) {
    FbDbMoveTmpl<FbtMoveAdditive, FbtMoveRelative>  asw;
    asw.count_to_exit = 240;
    asw.run();
}


template<typename R, typename S>
class FbDbMoveTmpl2: public FbDbMoveTmpl<R, S>{
    void preRunCycle() override{
        auto _idx = this->getIndex();
        if (_idx == 5) {
            this->mov.Execute = true;
        } else if (_idx == this->_trig_idx) {
            this->mov2.Execute = true;
        }
        this->fbRunCycle();
        auto& ax = this->axRef(0);
        if (_idx == 4) {
            EXPECT_EQ(ax.axisStat, mcStandstill);
        } else if (_idx == 5) {
            EXPECT_EQ(this->mov.Busy, true);
            EXPECT_EQ(ax.axisStat, mcDiscreteMotion);
        } else if (_idx == this->_trig_idx) {
            EXPECT_EQ(this->mov2.Busy, true);
            EXPECT_EQ(ax.axisStat, mcDiscreteMotion);
        } else if (_idx == this->_trig_idx + 1) {
            EXPECT_EQ(this->mov.CommandAborted, true);
        } else if (_idx == this->done_idx) {
            // EXPECT_EQ(this->mov.CommandAborted, true);
        } else if (_idx == this->done_idx * 2 + 1) {
            EXPECT_EQ(this->mov2.Done, true);
            EXPECT_EQ(ax.axisStat, mcStandstill);
            EXPECT_NEAR(ax.fSetPosition, 100, 1e-6);
        }
        if (_idx > 5) {
            EXPECT_EQ(this->check_set_pva(0), 0);
        }
    }
};

TEST(axis, moveadd_add) {
    FbDbMoveTmpl2<FbtMoveAdditive, FbtMoveAdditive> asw;
    asw._trig_idx = 40;
    asw.count_to_exit = 240;
    // asw.verbose_flag = true;
    asw.run();
}


TEST(axis, moveabs_buf_rel){
    AxDbMotTmpl<FbtMoveAbsolute, FbtMoveRelative> axFb;
    axFb.get_config_ref(0).cycle_time = 100000;
    axFb.reload_config(0);
    axFb.bindAxis(0);
    axFb.mov.Position = 5;
    axFb.mov2.Distance = -10;
    axFb.mov2.BufferMode = mcBuffered;
    set_fb_vel_config(axFb.mov, 5, 3, 3, 0);
    set_fb_vel_config(axFb.mov2, 3, 4, 2, 0);
    axFb.count_to_exit = 75;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 16)
            axFb.mov2.Execute = true;
        EXPECT_EQ(0, axFb.check_set_pva(0));
    });
    EXPECT_NEAR(axFb.axRef(0).fSetPosition, -5, 1e-6);
    // endPos = -5
}


TEST(axis, moveabs_buf_add){
    AxDbMotTmpl<FbtMoveAbsolute, FbtMoveAdditive> axFb;
    axFb.get_config_ref(0).cycle_time = 100000;
    axFb.reload_config(0);
    axFb.bindAxis(0);
    axFb.mov.Position = 5;
    axFb.mov2.Distance = -10;
    set_fb_vel_config(axFb.mov, 5, 3, 3, 0);
    set_fb_vel_config(axFb.mov2, 3, 4, 2.5, 0);
    axFb.count_to_exit = 75;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 16)
            axFb.mov2.Execute = true;
        EXPECT_EQ(0, axFb.check_set_pva(0));
    });
    EXPECT_NEAR(axFb.axRef(0).fSetPosition, -5, 1e-6);
    // endPos = -5
}

TEST(axis, moveabs_buf_rel_inv_order){
    AxDbMotTmpl<FbtMoveRelative, FbtMoveAbsolute> axFb;
    axFb.get_config_ref(0).cycle_time = 100000;
    axFb.reload_config(0);
    axFb.bindAxis(0);
    axFb.mov2.Position = 5;
    axFb.mov.Distance = -10;
    axFb.mov.BufferMode = mcBuffered;
    set_fb_vel_config(axFb.mov2, 5, 3, 3, 0);
    set_fb_vel_config(axFb.mov, 3, 4, 2, 0);
    // axFb.count_to_exit = 75;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov2.Execute = true;
        else if (i == 16)
            axFb.mov.Execute = true;
        EXPECT_EQ(0, axFb.check_set_pva(0));
    });
    EXPECT_NEAR(axFb.axRef(0).fSetPosition, -5, 1e-6);
}

template<typename T, typename T2>
struct SgAxFbDbTmpl:public AxDbMotTmpl<T, T2, 1>{
    SgAxFbDbTmpl(){
        this->get_config_ref(0).cycle_time = 100000;
        this->reload_config(0);
        this->bindAxis(0);
        set_fb_vel_config(this->mov, 5, 3, 3, 0);
        set_fb_vel_config(this->mov2, 3, 4, 3.2, 0);
    }
    // void postRunCycle()override{
        // return this->printAxisFb();
    // }
};


TEST(motNode, vel_abs){
    SgAxFbDbTmpl<FbtMoveVelocity, FbtMoveAbsolute> axFb;
    // axFb.mov.Position = 10;
    axFb.mov2.Position = -5;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 16)
            axFb.mov2.Execute = true;
        else if (i == 18){
            EXPECT_TRUE(axFb.mov.CommandAborted);
            EXPECT_TRUE(axFb.mov2.Active);
        }
        EXPECT_EQ(0, axFb.check_set_pva(0));
    });
    EXPECT_NEAR(axFb.axRef(0).fSetPosition, -5, 1e-3);
}

TEST(motNode, vel_buf_abs){
    SgAxFbDbTmpl<FbtMoveVelocity, FbtMoveAbsolute> axFb;
    axFb.get_config_ref(0).cycle_time = 50000;
    axFb.reload_config(0);
    axFb.count_to_exit = 160;
    axFb.mov2.Position = -5;
    axFb.mov2.BufferMode = mcBuffered;
    axFb.mov2.Velocity = 4;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 16)
            axFb.mov2.Execute = true;
        else if (i == 18){
            EXPECT_FALSE(axFb.mov.CommandAborted);
            EXPECT_FALSE(axFb.mov2.Active);
            EXPECT_TRUE(axFb.mov2.Busy);
        }
        EXPECT_EQ(0, axFb.check_set_pva(0));
    });
    EXPECT_NEAR(axFb.axRef(0).fSetPosition, -5, 1e-3);
}

TEST(motNode, abs_buf_vel){
    SgAxFbDbTmpl<FbtMoveAbsolute , FbtMoveVelocity> axFb;
    axFb.get_config_ref(0).cycle_time = 50000;
    axFb.reload_config(0);
    axFb.count_to_exit = 140;
    axFb.mov.Position = -5;
    axFb.mov.BufferMode = mcBuffered;
    axFb.mov2.BufferMode = mcBuffered;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 16)
            axFb.mov2.Execute = true;
        else if (i == 20){
            EXPECT_FALSE(axFb.mov2.Active);
            EXPECT_TRUE(axFb.mov2.Busy);
            EXPECT_EQ(axFb.axRef(0).axisStat, mcDiscreteMotion);
        }else if (i == 55){
            EXPECT_TRUE(axFb.mov.Done);
        }else if (i == 56){
            // EXPECT_EQ(axFb.axRef(0).axisStat, mcContinuousMotion);
        }
        EXPECT_EQ(0, axFb.check_set_pva(0));
    });
}

TEST(motNode, vel_buf_vel){
    SgAxFbDbTmpl<FbtMoveVelocity , FbtMoveVelocity> axFb;
    axFb.get_config_ref(0).cycle_time = 50000;
    axFb.reload_config(0);
    axFb.count_to_exit = 140;
    axFb.mov.BufferMode = mcBuffered;
    axFb.mov2.BufferMode = mcBuffered;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 1)
            axFb.mov.Execute = true;
        else if (i == 16)
            axFb.mov2.Execute = true;
        EXPECT_EQ(0, axFb.check_set_pva(0));
    });
}
