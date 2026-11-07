#include <gtest/gtest.h>
#include "fb_cpp_def.h"
#include "AxisRunTmpl.hpp"
#include "math.h"
#include "stdio.h"

using namespace zmc;


TEST(motNode, gearIn) {
    AxDbMotTmpl<FbtMoveRelative, FbtGearIn, 2> asw;
    auto& cfg = asw.get_config_ref(0);
    cfg.cycle_time = 50000;
    cfg.debug.servo_fak_pos = 4;
    asw.reload_config(0);

    auto& cfg2 = asw.get_config_ref(1);
    cfg2.cycle_time = 50000;
    cfg2.debug.servo_fak_pos = 4;
    asw.reload_config(1);
    auto& ax = asw.axRef(0);
    auto& ax2 = asw.axRef(1);

    asw.mov.Distance = 6;
    set_fb_vel_config(asw.mov, 5, 3, 3, 0);
    asw.mov.Axis = (AXIS_REF) &ax;

    set_fb_acc_config(asw.mov2, 5, 5, 0);
    asw.mov2.RatioNumerator = -1;
    asw.mov2.RatioDenominator = 1;
    asw.mov2.Master = (AXIS_REF) &ax;
    asw.mov2.Slave = (AXIS_REF) &ax2;

    asw.count_to_exit = 100;
    // asw.verbose_flag = 1;
    asw.run([&](){
        int i = asw.getIndex();
        if (i==2)
            asw.mov2.Execute = true;
        else if (i==4)
            asw.mov.Execute = true;
        else if(i>=30 && i<40){
            EXPECT_TRUE(asw.mov2.InGear);
            EXPECT_NEAR(asw.axRef(0).fActVelocity, -asw.axRef(1).fActVelocity, 1e-4);
        }
        EXPECT_EQ(0, asw.check_set_pva(0));
        EXPECT_EQ(0, asw.check_set_pva(1));
    });
    EXPECT_EQ(asw.axRef(0).fSetPosition, 10);
    EXPECT_EQ(asw.axRef(1).fSetPosition, -2);
}
