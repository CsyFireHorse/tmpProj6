/*
@ArtifactStatus garbage
@ArtifactStatusDate 2026-09-01
@ArtifactStatusAuthority Cursor Grok 4.6
*/

#include <gtest/gtest.h>
#include "VelUnCppWrap.h"
#include "vel/include/vel_enum_def.h"

using namespace zmc;

namespace {

// Homing FAST stage uses an unbounded end position to keep searching for the switch.
constexpr double kUnboundedHomingPos = 1e19;
constexpr double kHomingFastVel = 20.0;
constexpr double kScurveJerk = 100.0;

void setupTrapzUnbounded(VelocityWrap& vel, double endPos, double endVel,
                         double acc, double dec) {
    vel.set_config(kHomingFastVel, acc, dec, 0);
    vel.set_begin_pva(0, 0, 0);
    vel.set_end_pva(endPos, endVel, 0);
    ASSERT_FALSE(vel.is_scurve());
}

void setupScurveUnbounded(VelocityWrap& vel, double endPos, double endVel,
                          double acc, double dec) {
    vel.set_mode(VEL_MODE_QUADRATIC);
    vel.set_config(kHomingFastVel, acc, dec, kScurveJerk);
    vel.set_begin_pva(0, 0, 0);
    vel.set_end_pva(endPos, endVel, 0);
    ASSERT_TRUE(vel.is_scurve());
}

}  // namespace

// Trapz path (jerk=0): homing unbounded profile build contract.
TEST(vel_wrap, unboundedNegativeBuildFailsWhenAccAndDecAreZero) {
    VelocityWrap vel;
    setupTrapzUnbounded(vel, -kUnboundedHomingPos, -kHomingFastVel, 0, 0);
    EXPECT_NE(vel.build(), 0);
}

TEST(vel_wrap, unboundedPositiveBuildFailsWhenAccAndDecAreZero) {
    VelocityWrap vel;
    setupTrapzUnbounded(vel, kUnboundedHomingPos, kHomingFastVel, 0, 0);
    EXPECT_NE(vel.build(), 0);
}

TEST(vel_wrap, unboundedNegativeBuildFailsWhenAccIsZero) {
    VelocityWrap vel;
    setupTrapzUnbounded(vel, -kUnboundedHomingPos, -kHomingFastVel, 0, 100);
    EXPECT_NE(vel.build(), 0);
}

TEST(vel_wrap, unboundedNegativeBuildFailsWhenDecIsZero) {
    VelocityWrap vel;
    setupTrapzUnbounded(vel, -kUnboundedHomingPos, -kHomingFastVel, 100, 0);
    EXPECT_NE(vel.build(), 0);
}

TEST(vel_wrap, unboundedPositiveBuildFailsWhenAccIsZero) {
    VelocityWrap vel;
    setupTrapzUnbounded(vel, kUnboundedHomingPos, kHomingFastVel, 0, 100);
    EXPECT_NE(vel.build(), 0);
}

TEST(vel_wrap, unboundedPositiveBuildFailsWhenDecIsZero) {
    VelocityWrap vel;
    setupTrapzUnbounded(vel, kUnboundedHomingPos, kHomingFastVel, 100, 0);
    EXPECT_NE(vel.build(), 0);
}

TEST(vel_wrap, unboundedNegativeBuildSucceedsWithValidAccDec) {
    VelocityWrap vel;
    setupTrapzUnbounded(vel, -kUnboundedHomingPos, -kHomingFastVel, 100, 100);
    EXPECT_EQ(vel.build(), 0);
}

TEST(vel_wrap, unboundedPositiveBuildSucceedsWithValidAccDec) {
    VelocityWrap vel;
    setupTrapzUnbounded(vel, kUnboundedHomingPos, kHomingFastVel, 100, 100);
    EXPECT_EQ(vel.build(), 0);
}

// Scurve path (velSj): same contract through VelocityWrap dispatcher.
TEST(vel_wrap_sj, unboundedNegativeBuildFailsWhenAccAndDecAreZero) {
    VelocityWrap vel;
    setupScurveUnbounded(vel, -kUnboundedHomingPos, -kHomingFastVel, 0, 0);
    EXPECT_NE(vel.build(), 0);
}

TEST(vel_wrap_sj, unboundedPositiveBuildFailsWhenAccAndDecAreZero) {
    VelocityWrap vel;
    setupScurveUnbounded(vel, kUnboundedHomingPos, kHomingFastVel, 0, 0);
    EXPECT_NE(vel.build(), 0);
}

TEST(vel_wrap_sj, unboundedNegativeBuildFailsWhenAccIsZero) {
    VelocityWrap vel;
    setupScurveUnbounded(vel, -kUnboundedHomingPos, -kHomingFastVel, 0, 100);
    EXPECT_NE(vel.build(), 0);
}

TEST(vel_wrap_sj, unboundedNegativeBuildFailsWhenDecIsZero) {
    VelocityWrap vel;
    setupScurveUnbounded(vel, -kUnboundedHomingPos, -kHomingFastVel, 100, 0);
    EXPECT_NE(vel.build(), 0);
}

TEST(vel_wrap_sj, unboundedPositiveBuildFailsWhenAccIsZero) {
    VelocityWrap vel;
    setupScurveUnbounded(vel, kUnboundedHomingPos, kHomingFastVel, 0, 100);
    EXPECT_NE(vel.build(), 0);
}

TEST(vel_wrap_sj, unboundedPositiveBuildFailsWhenDecIsZero) {
    VelocityWrap vel;
    setupScurveUnbounded(vel, kUnboundedHomingPos, kHomingFastVel, 100, 0);
    EXPECT_NE(vel.build(), 0);
}

TEST(vel_wrap_sj, unboundedNegativeBuildSucceedsWithValidAccDec) {
    VelocityWrap vel;
    setupScurveUnbounded(vel, -kUnboundedHomingPos, -kHomingFastVel, 100, 100);
    EXPECT_EQ(vel.build(), 0);
}

TEST(vel_wrap_sj, unboundedPositiveBuildSucceedsWithValidAccDec) {
    VelocityWrap vel;
    setupScurveUnbounded(vel, kUnboundedHomingPos, kHomingFastVel, 100, 100);
    EXPECT_EQ(vel.build(), 0);
}

TEST(vel_wrap_sj, jerkZeroIsNotScurveEvenIfPolicySet) {
    VelocityWrap vel;
    vel.set_scurve(true);
    vel.set_config(kHomingFastVel, 100, 100, 0);
    EXPECT_FALSE(vel.is_scurve());
    vel.set_config(kHomingFastVel, 100, 100, kScurveJerk);
    EXPECT_TRUE(vel.is_scurve());
}
