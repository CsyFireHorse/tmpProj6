#include <gtest/gtest.h>
#include "fbt_axis.h"
#include "fbt_axis_impl.h"
#include "fb_cpp_def.h"
#include "fbt_fb_base.hpp"
#include "fbt_axis_cia402.h"
#include "axMoveTestTmpl.hpp"

using namespace zmc;

class PowerAndMove:public PouTaskBase{
public:
    AXIS_REF axis;
    FbtPower power;
    FbtMoveAbsolute move;
    int cnt = 0;
    PowerAndMove() {}
    void runCycle()override{
        if (!cnt){
            power.Axis = axis;
            power.Enable = true;
            power.bRegulatorOn = true;
            power.bDriveStart = true;

            move.Axis = axis;
            move.Position = 10.0;
            move.Velocity = 10.0;
            move.Acceleration = 30.0;
            move.Deceleration = 30.0;
        }
        if (cnt == 6) move.Execute = true;
        power.runCycle();
        move.runCycle();

        AxisImpl& ax = *((AxisImpl*)axis);
        if (cnt == 5) {
            EXPECT_EQ(ax.axisStat, mcStandstill);
            EXPECT_TRUE(power.Status);
        }else if (cnt == 6) {
            EXPECT_EQ(ax.axisStat, mcDiscreteMotion);
            EXPECT_TRUE(move.Busy);
        }else if (cnt == 40) {
            EXPECT_EQ(ax.axisStat, mcStandstill);
            EXPECT_TRUE(move.Done);
            EXPECT_NEAR(ax.fSetPosition, 10.0, 1e-6);
            EXPECT_NEAR(ax.fActPosition, 10.0, 1e-6);
        }
        cnt++;
    }
};


TEST(AxisVirtual, PowerAndMoveAbs) {
    ConfigDict config = {
        {"base.count_to_exit", "50"},
        {"axis_0.cycle_time", "50000"}
    };
    ax_move_test_tmpl_dict<PowerAndMove, AxisCia402Virt>(config);
}

TEST(AxisCia402Virt, PowerAndMoveAbs) {
    ConfigDict config = {
        {"base.count_to_exit", "50"},
        {"axis_0.cycle_time", "50000"}
    };
    ax_move_test_tmpl_dict<PowerAndMove, AxisCia402Virt>(config);
}

TEST(AxisCia402Fak, PowerAndMoveAbs) {
    ConfigDict config = {
        {"base.count_to_exit", "51"},
        {"axis_0.cycle_time", "50000"}
    };
    ax_move_test_tmpl_dict<PowerAndMove, AxisCia402Virt>(config);
}

class PowerAndMoveVel:public PouTaskBase{
    public:
    AXIS_REF axis;
    FbtPower power;
    FbtMoveVelocity move;
    int cnt = 0;
    PowerAndMoveVel() {
        move.Velocity = 8.0;
        move.Acceleration = 30.0;
        move.Deceleration = 30.0;
    }
    void runCycle()override{
        if (!cnt){
            power.Axis = axis;
            power.Enable = true;
            power.bRegulatorOn = true;
            power.bDriveStart = true;

            move.Axis = axis;
        }
        if (cnt == 6) move.Execute = true;
        power.runCycle();
        move.runCycle();

        AxisImpl& ax = *((AxisImpl*)axis);
        if (cnt == 6) {
            EXPECT_EQ(ax.axisStat, mcContinuousMotion);
            EXPECT_TRUE(move.Busy);
        }else if (cnt > 20 ) {
            EXPECT_EQ(ax.axisStat, mcContinuousMotion);
            EXPECT_TRUE(move.InVelocity);
            EXPECT_NEAR(ax.fSetVelocity, 8.0, 1e-6);
            EXPECT_NEAR(ax.fActVelocity, 8.0, 1e-6);
        }
        // axis->print(4);
        cnt++;
    }
};

TEST(AxisVirtual, PowerAndMoveVel) {
    ConfigDict config = {
        {"base.count_to_exit", "50"},
        {"axis_0.cycle_time", "50000"},
        {"axis_0.increments_units_in_application_num", "3"},
        {"axis_0.increments_units_in_application_den", "11"}
    };
    ax_move_test_tmpl_dict<PowerAndMoveVel, AxisCia402Virt>(config);
}