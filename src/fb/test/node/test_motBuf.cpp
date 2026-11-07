#include <gtest/gtest.h>

#include "AxisRunTmpl.hpp"
#include "fb_cpp_def.h"
#include "fbt_motion_traits.hpp"
#include "math.h"
#include "stdio.h"

using namespace zmc;

TEST(motbuf, move_abs_loop){
    AxisMotEmu axFak;
    auto& ax = get_axis_impl_ref((AXIS_REF) &axFak);
    FbtMoveAbsolute fbMov;
    fbMov.Axis = (AXIS_REF) &axFak;
    int direction = 1;
    fbMov.Position = 5 * direction;
    set_fb_vel_config(fbMov, 100000, 400000, 400000, 0);

    axFak.get_config_ref().cycle_time = 100000;
    axFak.reload_config();

    // printf("idx,stat,fb,pos,vel,acc\n");
    bool nextToRun = false;
    for (int i=0; i<13555; i++){ 
        if (i==3){
            fbMov.Execute = true;
        }else if (fbMov.Done && !nextToRun) {
            fbMov.Execute = false;
            direction *= -1;
            fbMov.Position = 5 * direction;
            nextToRun = true;
        }else if(nextToRun){
            fbMov.Execute = true;
            nextToRun = false; 
        }
        fbMov.runCycle();
        axFak.runCycle();
        auto axStat = ax.axisStat;
        EXPECT_NE(get_move_stat(fbMov), FB_STAT_ERROR);
    }
}

TEST(motbuf, axis_is_null) {
    AxSgMotTmpl<FbtMoveRelative> axFb;
    axFb.mov.Distance = 5;
    axFb.count_to_exit = 20;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 5)
            axFb.mov.Execute = true;
        else if (i == 6) {
            EXPECT_EQ(axFb.mov.Error, true);
            EXPECT_EQ(axFb.mov.ErrorID, ZMC_AXIS_IS_NO_AXIS_REF); 
        }
    });
}

template<typename T>
class AxSgMotTestTmpl:public AxSgMotTmpl<T, 1>{
public:
    void preRunCycle()override{}
};

TEST(motbuf, move_rerun) {
    AxSgMotTestTmpl<FbtMoveRelative> axFb;
    axFb.get_config_ref(0).debug.servo_fak_pos = 3;
    axFb.get_config_ref(0).cycle_time = 200000;
    axFb.reload_config(0);
    axFb.bindAxis(0);
    axFb.mov.Distance = 5;
    set_fb_vel_config(axFb.mov, 5, 3, 3, 0);
    axFb.count_to_exit = 100;
    axFb.run([&](){
        auto i = axFb.getIndex();
        if (i == 5)
            axFb.mov.Execute = true;
        axFb.fbRunCycle();
        axFb.fbRunCycle();
        EXPECT_EQ(axFb.check_set_pva(0), 0);
    });
    EXPECT_NEAR(axFb.axRef(0).fSetPosition, 8, 1e-6);
}

TEST(motbuf, move_skip_error) {
    AxSgMotTestTmpl<FbtMoveRelative> axFb;
    axFb.get_config_ref(0).debug.servo_fak_pos = 3;
    axFb.reload_config(0);
    axFb.bindAxis(0);
    axFb.count_to_exit = 100;
    axFb.mov.Distance = 5;
    set_fb_vel_config(axFb.mov, 5, 3, 3, 0);
    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);
        if (i == 5)
            axFb.mov.Execute = true;
        else if(i==6)
            EXPECT_EQ(ax.axisStat, mcDiscreteMotion);
        else if(i==11){
            EXPECT_EQ(ax.axisStat, mcErrorStop);
            EXPECT_EQ(ax.dwErrorID, ZMC_FB_WASNT_CALLED_DURING_MOTION);
        }else if(i==12){
            EXPECT_EQ(axFb.mov.Error, true);
            EXPECT_EQ(axFb.mov.ErrorID, ZMC_FB_WASNT_CALLED_DURING_MOTION);
        }
        if (i != 10)
            axFb.fbRunCycle();

        axFb.check_set_pva(0);
    });
}



TEST(motbuf, move_skip_error_restart) {
    AxSgMotTestTmpl<FbtMoveRelative> axFb;
    axFb.get_config_ref(0).debug.servo_fak_pos = 3;
    axFb.reload_config(0);
    axFb.bindAxis(0);
    axFb.count_to_exit = 100;
    axFb.mov.Distance = 5;
    set_fb_vel_config(axFb.mov, 5, 3, 3, 0);
    // axFb.verbose_flag = 1;
    FbtReset fbRst;
    axFb.run([&](){
        auto i = axFb.getIndex();
        auto& ax = axFb.axRef(0);
        if (i == 5)
            axFb.mov.Execute = true;
        else if(i==6)
            EXPECT_EQ(ax.axisStat, mcDiscreteMotion);
        else if(i==11){
            EXPECT_EQ(ax.axisStat, mcErrorStop);
            EXPECT_EQ(ax.dwErrorID, ZMC_FB_WASNT_CALLED_DURING_MOTION);
        }else if(i==12){
            EXPECT_EQ(axFb.mov.Error, true);
            EXPECT_EQ(axFb.mov.ErrorID, ZMC_FB_WASNT_CALLED_DURING_MOTION);
        }else if(i >= 15 && i<18){ 
            axFb.mov.Execute = false;
            fbRst.Axis = axFb.axisRef(0);
            fbRst.Execute = true;
            fbRst.runCycle();
        }else if(i==18){
            EXPECT_EQ(axFb.mov.Error, false);
            EXPECT_EQ(ax.axisStat, mcStandstill);
        }else if(i==19){
            axFb.powerOn(0);
        }else if(i==20){
            EXPECT_EQ(ax.axisStat, mcStandstill);
        }else if(i==22){
            axFb.mov.Execute = true;
        }else if(i==26){
            EXPECT_EQ(ax.axisStat, mcDiscreteMotion);
        }
        if (i != 10)
            axFb.fbRunCycle();

        axFb.check_set_pva(0);
    });
}
