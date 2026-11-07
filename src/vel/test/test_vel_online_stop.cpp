#include <gtest/gtest.h>

#include "math.h"
#include "velocity_utils.hpp"
#include "vel_stop_online.hpp"

using namespace zmc;

TEST(velStopOnline, cycleInternalStopReportsZeroAcc) {
    const double dec = 15.0;
    const double dt = 0.1;
    const zmc::PosVelAcc pva = {0.0, 1.0, 0.0};
    // |vel| < dec * dt triggers in-cycle stop with stationary endpoint.
    const zmc::PosVelAcc next = calc_next_decel_pva(pva, dec, dt);
    EXPECT_NEAR(next.pos, pva.vel * pva.vel / 2 / dec, 1e-6);
    EXPECT_NEAR(next.vel, 0.0, 1e-6);
    EXPECT_NEAR(next.acc, 0.0, 1e-6);
}

TEST(velStopOnline, fullCycleDecKeepsConstantDecAcc) {
    const double dec = 15.0;
    const double dt = 0.1;
    const zmc::PosVelAcc pva = {0.0, 3.0, 0.0};
    const zmc::PosVelAcc next = calc_next_decel_pva(pva, dec, dt);
    EXPECT_NEAR(next.vel, pva.vel - dec * dt, 1e-6);
    EXPECT_NEAR(next.acc, -dec, 1e-6);
}

TEST(velOnlineChecker, rejectsInCycleStopWithoutErrorStopState) {
    VelOnlineChecker chk;
    chk.Ts = 0.1;
    const double dec = 15.0;
    const zmc::PosVelAcc start = {0.0, 1.0, 0.0};
    EXPECT_EQ(chk.check(start.pos, start.vel, start.acc), VEL_CHECK_NOP);

    const zmc::PosVelAcc stop = calc_next_decel_pva(start, dec, chk.Ts);
    EXPECT_NE(chk.check(stop.pos, stop.vel, stop.acc), VEL_CHECK_NOP);
}

TEST(velOnlineChecker, allowsInCycleStopWithErrorStopState) {
    VelOnlineChecker chk;
    chk.Ts = 0.1;
    const double dec = 15.0;
    const zmc::PosVelAcc start = {0.0, 1.0, 0.0};
    EXPECT_EQ(chk.check(start.pos, start.vel, start.acc), VEL_CHECK_NOP);

    const zmc::PosVelAcc stop = calc_next_decel_pva(start, dec, chk.Ts);
    chk.set_state(VEL_ONLINE_AXIS_ERROR_STOP);
    EXPECT_EQ(chk.check(stop.pos, stop.vel, stop.acc), VEL_CHECK_NOP);
}

TEST(velOnlineChecker, rejectsErrorStopStopWhenDistanceRequiresMoreThanTs) {
    VelOnlineChecker chk;
    chk.Ts = 0.1;
    EXPECT_EQ(chk.check(0.0, 9.0, 0.0), VEL_CHECK_NOP);

    // Same-direction displacement that would need t > Ts for constant decel to zero.
    chk.set_state(VEL_ONLINE_AXIS_ERROR_STOP);
    EXPECT_NE(chk.check(1.0, 0.0, 0.0), VEL_CHECK_NOP);
}

TEST(velOnlineChecker, errorStopNonStationaryUsesOriginalFormula) {
    VelOnlineChecker chk;
    chk.Ts = 0.1;
    EXPECT_EQ(chk.check(0.0, 9.0, 0.0), VEL_CHECK_NOP);

    chk.set_state(VEL_ONLINE_AXIS_ERROR_STOP);
    const int ret = chk.check(0.45, 8.0, -15.0);
    EXPECT_NE(ret, VEL_CHECK_NOP);
}

TEST(velOnlineChecker, stationaryHoldAfterInCycleStopPassesWithoutReset) {
    VelOnlineChecker chk;
    chk.Ts = 0.1;
    const double dec = 15.0;
    const zmc::PosVelAcc start = {0.0, 1.0, 0.0};
    EXPECT_EQ(chk.check(start.pos, start.vel, start.acc), VEL_CHECK_NOP);

    const zmc::PosVelAcc stop = calc_next_decel_pva(start, dec, chk.Ts);
    chk.set_state(VEL_ONLINE_AXIS_ERROR_STOP);
    EXPECT_EQ(chk.check(stop.pos, stop.vel, stop.acc), VEL_CHECK_NOP);

    chk.set_state(VEL_ONLINE_AXIS_ERROR_STOP);
    EXPECT_EQ(chk.check(stop.pos, stop.vel, stop.acc), VEL_CHECK_NOP);
}

TEST(velOnlineChecker, allowsNegligibleDisplacementInCycleStop) {
    VelOnlineChecker chk;
    chk.Ts = 0.1;
    EXPECT_EQ(chk.check(7.0, 10.0, 10.0), VEL_CHECK_NOP);

    chk.set_state(VEL_ONLINE_AXIS_ERROR_STOP);
    EXPECT_EQ(chk.check(7.0, 0.0, 0.0), VEL_CHECK_NOP);
}
