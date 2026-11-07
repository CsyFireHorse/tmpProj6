#include <gtest/gtest.h>
#include "math.h"
#include "stdio.h"
#include "fb_cpp_def.h"
#include "fbt_axis.h"
#include "fbt_axis_base.h"
#include "fbt_fb_base.hpp"
#include "base/include/ServoFakerToy.h"
#include "source/axis/fbt_axis_impl_cfg.hpp"
using namespace zmc;

typedef AxisCia402Fak Axis;

namespace zmc{
ServoFakerToy* get_axis_cia402_servo_fak(AxisCia402Fak* pAxis);
}

TEST(fbReset, base){
    Axis axis;
    AxisImplBase& ax = (AxisImplBase&)axis;

    AxisCiaConfig cfg;
    AxisCiaConfig_default_init(&cfg);
    cfg.debug.servo_fak_statw_sm = 7;
    cfg.debug.servo_fak_pos = 100;
    cfg.is_axis_virtual = true;
    axis.set_config(&cfg);

    FbtReset    fbRst;
    fbRst.Axis = &axis;
    for(int cnt=0; cnt<35; cnt++){
        auto rstStat  = get_fb_impl_stat(fbRst);
        if (cnt <4 && cnt > 2){
            EXPECT_EQ(ax.axisStat, mcErrorStop);
        }else if (cnt == 4 )
            fbRst.Execute = (true);
        else if (cnt == 5){
            EXPECT_EQ(rstStat, FB_STAT_BUSY_ACTIVE);
        }else if (cnt == 7){
            EXPECT_EQ(rstStat, FB_STAT_DONE);
            EXPECT_EQ(ax.axisStat, mcDisabled);
        }else if (cnt == 8 ){
            fbRst.Execute = (false);
            EXPECT_EQ(rstStat, FB_STAT_DONE);
            EXPECT_EQ(ax.axisStat, mcDisabled);
        }else if (cnt == 9 ){
            EXPECT_EQ(ax.axisStat, mcDisabled);
        }else if (cnt == 10){
            EXPECT_EQ(rstStat, FB_STAT_IDLE);
            EXPECT_EQ(ax.axisStat, mcDisabled);
        }

        fbRst.runCycle();
        rstStat = get_fb_impl_stat(fbRst);
        // printf("%d,%d,%d,%f\n", cnt, ax.axisStat, rstStat, ax.fActPosition);
        if (cnt == 4){
            EXPECT_EQ(rstStat, FB_STAT_BUSY_ACTIVE);
        }else if (cnt == 5){
            EXPECT_EQ(rstStat, FB_STAT_BUSY_ACTIVE);
        }else if (cnt == 6){
            EXPECT_EQ(rstStat, FB_STAT_DONE);
        }
        axis.runCycle();
    }
}


TEST(fbReset, timeout){
    Axis axis;
    AxisImplBase& ax = (AxisImplBase&)axis;

    AxisCiaConfig cfg;
    AxisCiaConfig_default_init(&cfg);
    cfg.debug.servo_fak_statw_sm = 7;
    cfg.debug.servo_fak_pos = 100;
    cfg.is_axis_virtual = true;
    axis.set_config(&cfg);

    FbtReset    fbRst;
    fbRst.Axis = &axis;

    auto& srvFak = *get_axis_cia402_servo_fak(&axis);
    for (int i=0; i<1030; i++){
        if (i <4 && i > 2){
            fbRst.Execute = (false);
            EXPECT_EQ(ax.axisStat, mcErrorStop);
        }else if (i == 4 ){
            fbRst.Execute = (true);
            srvFak.will_fault = true;
        }else if (i == 5){
            EXPECT_EQ(get_fb_impl_stat(fbRst), FB_STAT_BUSY_ACTIVE);
        }else if (i == 1010){
            EXPECT_EQ(get_fb_impl_stat(fbRst), FB_STAT_ERROR);
            EXPECT_EQ(fbRst.ErrorID, ZMC_ERR_R_TIMEOUT);
        }else if (i == 1012){
            srvFak.will_fault = false;
            fbRst.Execute = false;
        }else if (i == 1013){
            fbRst.Execute = true;
            EXPECT_EQ(get_fb_impl_stat(fbRst), FB_STAT_IDLE);
        }else if (i == 1016){
            EXPECT_EQ(get_fb_impl_stat(fbRst), FB_STAT_DONE);
            EXPECT_EQ(ax.axisStat, mcDisabled);
        }
        fbRst.runCycle();
        axis.runCycle();
    }
}
