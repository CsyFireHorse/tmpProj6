#include <gtest/gtest.h>

#include "AxisRunTmpl.hpp"
#include "fb_cpp_def.h"
#include "fbt_motion_traits.hpp"
#include "math.h"
#include "stdio.h"

using namespace zmc;

class AxUnitMoveTestCase : public AxSgMotTmpl<FbtMoveRelative, 1> {
public:
    double pos0 = -30.;
    double pos = 1.;
    int num = 542800;
    int den = 1;
    double scale = (double) num/den;
    AxUnitMoveTestCase() {
        auto& cfg = this->get_config_ref(0);
        cfg.linear.increments_units_in_application_num = num;
        cfg.linear.increments_units_in_application_den = den;
        cfg.cycle_time = 2000; // 0.002s
        cfg.debug.servo_fak_pos = pos0;
        cfg.debug.servo_fak_rcfilter_scale = 0.01;
        this->reload_config(0);

        // Setup FbtMoveRelative
        this->bindAxis(0);
        this->mov.Distance = pos;
        set_fb_vel_config(this->mov, 0.05, 10, 10, 0);
        
        this->count_to_exit = 12000;
    }
    
    void preRunCycle() override {
        auto _idx = this->getIndex();
        auto& ax = this->axRef(0);
        auto mstat = get_fb_impl_stat(this->mov);
        
        if (_idx == 8) {
            this->mov.Execute = true;
            EXPECT_EQ(ax.axisStat, mcStandstill);
            EXPECT_EQ(ax.diActPosition, pos0);
            EXPECT_NEAR(ax.fActPosition, pos0/scale, 1e-6);
        } else if (_idx == 10) {
            EXPECT_EQ(mstat, FB_STAT_BUSY_ACTIVE);
            EXPECT_EQ(ax.axisStat, mcDiscreteMotion);
        } else if (_idx == 11996) {
            EXPECT_EQ(ax.axisStat, mcStandstill);
            EXPECT_NEAR(ax.fSetPosition, pos0/scale + pos, 1e-6);
            EXPECT_EQ(ax.diActPosition, round(pos0 + scale * pos));
            EXPECT_EQ(mstat, FB_STAT_DONE);
        }
        this->fbRunCycle();
        // Check motion consistency if in motion
        // if (_idx > 10 && _idx < 11996) {
            EXPECT_EQ(this->check_set_pva(0), 0);
        // }
    }
};

TEST(fb, move_unit) {
    AxUnitMoveTestCase axm;
    axm.run();
}

TEST(fb, move_unit2) {
    AxUnitMoveTestCase axm;
    axm.num = 5428011;
    axm.den = 11;
    axm.scale = (double) axm.num/axm.den;
    auto& cfg = axm.get_config_ref(0);
    cfg.linear.increments_units_in_application_num = axm.num;
    cfg.linear.increments_units_in_application_den = axm.den;
    axm.reload_config(0);
    axm.run();
}

TEST(fbMove, pushFbWhenDisable){
    AxDbFbTmpl<FbtPower, FbtMoveVelocity> asw;
    asw.bindAxis(0);
    asw.count_to_exit = 25;
    set_fb_vel_config(asw.mov2, 50, 100, 100, 0);
    asw.run([&](){
        int cnt = asw.getIndex();
        auto& ax = asw.axRef(0);
        if (cnt == 2){
            EXPECT_EQ(ax.dwErrorID, 0);
            asw.mov.Enable = true;
            asw.mov.bDriveStart = false;
            asw.mov.bRegulatorOn = false;
        }else if (cnt == 15){
            EXPECT_EQ(ax.axisStat, mcDisabled);
            asw.mov2.Execute = true;
        }else if (cnt >= 16 && cnt < 20){
            EXPECT_TRUE(asw.mov2.Error);
            EXPECT_EQ(asw.mov2.ErrorID, ZMC_FB_ACTIVE_AXIS_DISABLED);
        }else if(cnt == 20){
            asw.mov2.Execute = false;
        }else if (cnt == 21){
            EXPECT_FALSE(asw.mov2.Error);
            EXPECT_FALSE(asw.mov2.Busy);
            EXPECT_FALSE(asw.mov2.CommandAborted);
        }
    });
}

TEST(fbMove, disableWhenMoving){
    AxDbFbTmpl<FbtPower, FbtMoveVelocity> asw;
    asw.bindAxis(0);
    asw.count_to_exit = 30;
    set_fb_vel_config(asw.mov2, 50, 100, 100, 0);
    asw.run([&](){
        int cnt = asw.getIndex();
        auto& ax = asw.axRef(0);
        if (cnt == 2){
            EXPECT_EQ(ax.dwErrorID, 0);
            EXPECT_EQ(ax.axisStat, mcDisabled);
            asw.mov.Enable = true;
            asw.mov.bDriveStart = true;
            asw.mov.bRegulatorOn = true;
        }else if (cnt == 10){
            EXPECT_EQ(ax.axisStat, mcStandstill);
        }else if (cnt == 20){
            asw.mov2.Execute = true;
        }else if(cnt == 25){
            EXPECT_TRUE(asw.mov2.Busy);
            EXPECT_EQ(ax.axisStat, mcContinuousMotion);
            asw.mov.bDriveStart = false;
            asw.mov.bRegulatorOn = false;
        }else if (cnt == 28){
            EXPECT_EQ(ax.axisStat, mcErrorStop);
            EXPECT_EQ(ax.dwErrorID, ZMC_ERR_POWEROFF_WHEN_MOVING);
            EXPECT_EQ(asw.mov2.ErrorID, ZMC_ERR_POWEROFF_WHEN_MOVING);
            EXPECT_TRUE(asw.mov2.Error);
        }
    });
}


TEST(moveabs, reloadError) {
    AxSgMotTmpl<FbtMoveAbsolute> axm;
    auto& cfg = axm.get_config_ref(0);
    cfg.debug.servo_fak_pos = 1;
    cfg.cycle_time = 50000;
    axm.reload_config(0);
    axm.bindAxis(0);
    axm.count_to_exit = 25;
    set_fb_vel_config(axm.mov, 20, 40, 40, 0);
    axm.mov.Position = 10;
    // axm.verbose_flag = 3;
    axm.run([&](){
        auto mstat = get_fb_impl_stat(axm.mov);
        auto _idx = axm.getIndex();
        auto& ax = axm.axRef(0);
        
        if (_idx == 8) {
            EXPECT_EQ(ax.axisStat, mcStandstill);
            axm.mov.Execute = true;
        } else if (_idx == 9) {
            EXPECT_EQ(mstat, FB_STAT_BUSY_ACTIVE);
            EXPECT_EQ(ax.axisStat, mcDiscreteMotion);
        } else if (_idx == 15) {
            axm.mov.Execute = false;
        } else if (_idx == 16) {
            axm.mov.Execute = true;
            set_fb_vel_config(axm.mov, 0, 0, 0, 0);
        } else if (_idx == 17) {
            EXPECT_EQ(mstat, FB_STAT_ERROR);
            EXPECT_EQ(axm.mov.ErrorID, ZMC_INVALID_VELACCJERK_VALUES);
            axm.mov.Execute = false;
        } else if (_idx == 20) {
            EXPECT_EQ(mstat, FB_STAT_IDLE);
        }
        if (_idx >= 5) {
            EXPECT_EQ(axm.check_set_pva(0), 0);
        }
    });
}
