#include <gtest/gtest.h>

#include "VelGearFollower.h"
#include "common/misc/TrapzVelToyForwarder.hpp"
#include "stdio.h"


using namespace zmc;

TEST(VelGearFollowerTest, BasicGearFollowing) {
    // Create the master-axis motion planner.
    ToyTrapzVelPlanner masterPlanner;
    masterPlanner.set_config(5, 3, 3, 0);
    masterPlanner.set_begin_pva(10, 0);
    masterPlanner.set_end_pva(20, 0);
    masterPlanner.build();

    // Create the slave-axis follower.
    const double Ts = 0.1;
    VelGearFollower follower;
    follower.set_config(6, 3, 3, 0);
    follower.set_begin_pva(0, 0, 0);
    follower.set_num_den(1, 1);  // 1:1 following ratio.
    follower.set_Ts(Ts);

    PosVelAcc masterPva, slavePva;
    // std::vector<double> expectedPositions = {0, 0.15, 0.6, 1.35, 2.4};  // First five expected positions.

    for (int i = 0; i < 5; i++) {
        double tm = i * Ts;
        masterPlanner.get_slice(tm, masterPva);
        follower.follow(masterPva);
        follower.runCycle();

        int ret = follower.get_cur_slice(slavePva);
        // printf("%d,%f,%f,%f,%f,%d,%f,%f,%f\n", i, tm, masterPva.pos, masterPva.vel,
        // masterPva.acc, ret, slavePva.pos, slavePva.vel, slavePva.acc);
        // Verify that the slave position is within the expected error range.
        EXPECT_NEAR(masterPva.pos, 10 + slavePva.pos, 0.01) << "Position mismatch at step " << i;

        // Verify that the slave velocity follows the master velocity.
        if (i > 0) {  // Skip the initial state.
            EXPECT_NEAR(slavePva.vel, masterPva.vel, 0.1) << "Velocity mismatch at step " << i;
        }

        // Verify the motion state.
        if (i >= 2) {
            EXPECT_EQ(ret, GEAR_STATUS_GEAR_IN) << "Gear status mismatch at step " << i;
        }
    }
}
