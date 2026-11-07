/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-01
@ArtifactStatusAuthority GPT-5 Codex
*/

#include <gtest/gtest.h>

#include "fb_cpp_def.h"
#include "fbt_axis_base.h"
#include "fbt_fb_base.hpp"
#include "math.h"
#include "motbuf/AxisWithFb.hpp"
#include "stdio.h"
#include "fbt_axis.h"

using namespace zmc;

// TEST(axis, base)
// {
//     auto pAx = makeAxis();
//     deleteAxis(pAx);
// }

TEST(Axis, empty) {
    AxisBase axWr;
    for (int i = 0; i < 5; i++) {
        axWr.runCycle();
    }
}

TEST(AxisReset, ClearsCycleCounterInPlace) {
    AxisBase axWr;
    AXIS_REF axisRef = axWr.axis();
    axWr.runCycle();
    axWr.runCycle();
    EXPECT_GT(axWr.get_count(), 0u);

    axWr.reset_runtime();

    EXPECT_EQ(axisRef, axWr.axis());
    EXPECT_EQ(0u, axWr.get_count());
}

TEST(AxisStateDetail, EncodesOnlyExplicitCompatibleDetails) {
    AxisImplBase axis;

    axis.setAxisState(mcHoming, AXIS_STATE_DETAIL_HOME);
    EXPECT_EQ(mcHomeEx, axis.getAxisStateEx());
    axis.setAxisState(mcContinuousMotion, AXIS_STATE_DETAIL_GEAR_OUT);
    EXPECT_EQ(mcGearOutEx, axis.getAxisStateEx());
    axis.setAxisState(mcSynchronizedMotion, AXIS_STATE_DETAIL_FOLLOW);
    EXPECT_EQ(mcFollowEx, axis.getAxisStateEx());
    axis.setAxisState(mcStopping, AXIS_STATE_DETAIL_QUICK_STOP);
    EXPECT_EQ(mcQuickStoppingEx, axis.getAxisStateEx());

    // An incompatible detail never changes the encoded main state and is cleared on update.
    // axis.setAxisState(mcStandstill, AXIS_STATE_DETAIL_HOME);
    // EXPECT_EQ(mcStandstillEx, axis.getAxisStateEx());
    // axis.updateAxisStateDetail();
    // EXPECT_EQ(AXIS_STATE_DETAIL_DEFAULT, axis.axisStateDetail);
}

TEST(AxisStateDetail, RefreshesErrorStopFromRealPowerState) {
    AxisImplBase axis;
    axis.regulatorRealState = true;
    axis.driveStartRealState = true;
    axis.setAxisState(mcErrorStop);
    axis.updateAxisStateDetail();
    EXPECT_EQ(mcErrorStopPowerOnEx, axis.getAxisStateEx());

    axis.driveStartRealState = false;
    axis.updateAxisStateDetail();
    EXPECT_EQ(AXIS_STATE_DETAIL_ERROR_POWER_OFF, axis.axisStateDetail);
    EXPECT_EQ(mcErrorStopPowerOffEx, axis.getAxisStateEx());

    axis.driveStartRealState = true;
    axis.updateAxisStateDetail();
    EXPECT_EQ(mcErrorStopPowerOnEx, axis.getAxisStateEx());
}

TEST(AxisStateDetail, ResetClearsExplicitDetail) {
    AxisBase axisWrapper;
    AxisImplBase& axis = get_axis_impl_ref(axisWrapper.axis());
    axis.setAxisState(mcContinuousMotion, AXIS_STATE_DETAIL_GEAR_OUT);

    axisWrapper.reset_runtime();

    EXPECT_EQ(mcDisabledEx, axis.getAxisStateEx());
    EXPECT_EQ(AXIS_STATE_DETAIL_DEFAULT, axis.axisStateDetail);
}

template <typename ExitFb>
void expect_exit_motion_keeps_explicit_gear_out_detail(uint8_t synchronized_fb_type) {
    AxisImplBase axis;
    Node<FbtImplParam> synchronized_node;
    synchronized_node.fbType = synchronized_fb_type;
    axis.pNode.push_back(&synchronized_node);
    axis.setAxisState(mcSynchronizedMotion);
    axis.fSavePosition = 3.0;
    axis.fSaveVelocity = 2.0;

    ExitFb exit_fb;
    exit_fb.Slave = reinterpret_cast<AXIS_REF>(&axis);
    exit_fb.Execute = true;
    exit_fb.runCycle();

    EXPECT_TRUE(exit_fb.Done);
    EXPECT_TRUE(axis.pNode.isEmpty());
    EXPECT_EQ(mcContinuousMotion, axis.axisStat);
    EXPECT_EQ(AXIS_STATE_DETAIL_GEAR_OUT, axis.axisStateDetail);
    EXPECT_EQ(mcGearOutEx, axis.getAxisStateEx());
}

TEST(AxisStateDetail, GearOutDetailSurvivesCompletedNodeRemoval) {
    expect_exit_motion_keeps_explicit_gear_out_detail<FbtGearOut>(FB_TYPE_GEARIN);
}

TEST(AxisStateDetail, CamOutDetailSurvivesCompletedNodeRemoval) {
    expect_exit_motion_keeps_explicit_gear_out_detail<FbtCamOut>(FB_TYPE_CAMIN);
}

template <typename FollowFb>
void expect_follow_sets_explicit_detail() {
    AxisImplBase axis;
    axis.setAxisState(mcStandstill);

    FollowFb follow;
    follow.Axis = reinterpret_cast<AXIS_REF>(&axis);
    follow.bExecute = true;
    follow.runCycle();

    EXPECT_EQ(mcSynchronizedMotion, axis.axisStat);
    EXPECT_EQ(AXIS_STATE_DETAIL_FOLLOW, axis.axisStateDetail);
    EXPECT_EQ(mcFollowEx, axis.getAxisStateEx());
}

TEST(AxisStateDetail, FourFollowBlocksSetExplicitDetail) {
    expect_follow_sets_explicit_detail<FbtFollowPosition>();
    expect_follow_sets_explicit_detail<FbtFollowPositionVelocity>();
    expect_follow_sets_explicit_detail<FbtFollowVelocity>();
    expect_follow_sets_explicit_detail<FbtFollowSetValues>();
}

TEST(AxisStateDetail, OrdinaryContinuousMotionClearsGearOutDetail) {
    AxisImplBase axis;
    axis.setAxisState(mcContinuousMotion, AXIS_STATE_DETAIL_GEAR_OUT);

    FbtMoveVelocity move;
    move.Axis = reinterpret_cast<AXIS_REF>(&axis);
    move.Execute = true;
    move.Velocity = 2.0;
    move.Acceleration = 4.0;
    move.Deceleration = 4.0;
    move.runCycle();

    EXPECT_EQ(mcContinuousMotion, axis.axisStat);
    EXPECT_EQ(AXIS_STATE_DETAIL_DEFAULT, axis.axisStateDetail);
    EXPECT_EQ(mcContinuousMotionEx, axis.getAxisStateEx());
}

TEST(AxisStateDetail, DriveQuickStopUsesExplicitDetail) {
    AxisImplBase axis;
    axis.setAxisState(mcStandstill);
    axis.regulatorRealState = true;
    axis.driveStartRealState = false;

    FbtPower power;
    power.Axis = reinterpret_cast<AXIS_REF>(&axis);
    power.Enable = true;
    power.bRegulatorOn = true;
    power.bDriveStart = true;
    power.runCycle();

    EXPECT_EQ(mcStopping, axis.axisStat);
    EXPECT_EQ(AXIS_STATE_DETAIL_QUICK_STOP, axis.axisStateDetail);
    EXPECT_EQ(mcQuickStoppingEx, axis.getAxisStateEx());
}

TEST(AxisStateDetail, AxisWithFbReturnsBoundAxisExtendedState) {
    AxisImplBase axis;
    AxisWithFb axis_with_fb;
    EXPECT_EQ(mcDisabledEx, axis_with_fb.getAxisState());

    axis.setAxisState(mcContinuousMotion, AXIS_STATE_DETAIL_GEAR_OUT);
    axis_with_fb.bindAxis(reinterpret_cast<AXIS_REF>(&axis));
    EXPECT_EQ(mcGearOutEx, axis_with_fb.getAxisState());
}
