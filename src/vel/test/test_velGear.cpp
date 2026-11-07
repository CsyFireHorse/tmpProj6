#include <gtest/gtest.h>

#include "VelGearFollower.h"
#include "common/misc/TrapzVelToyForwarder.hpp"
#include "stdio.h"


using namespace zmc;

TEST(VelGearFollowerTest, BasicGearFollowing) {
    // 创建主轴运动规划器
    ToyTrapzVelPlanner masterPlanner;
    masterPlanner.set_config(5, 3, 3, 0);
    masterPlanner.set_begin_pva(10, 0);
    masterPlanner.set_end_pva(20, 0);
    masterPlanner.build();

    // 创建从轴跟随器
    const double Ts = 0.1;
    VelGearFollower follower;
    follower.set_config(6, 3, 3, 0);
    follower.set_begin_pva(0, 0, 0);
    follower.set_num_den(1, 1);  // 1:1 跟随比
    follower.set_Ts(Ts);

    PosVelAcc masterPva, slavePva;
    // std::vector<double> expectedPositions = {0, 0.15, 0.6, 1.35, 2.4};  // 前5个期望位置点

    for (int i = 0; i < 5; i++) {
        double tm = i * Ts;
        masterPlanner.get_slice(tm, masterPva);
        follower.follow(masterPva);
        follower.runCycle();

        int ret = follower.get_cur_slice(slavePva);
        // printf("%d,%f,%f,%f,%f,%d,%f,%f,%f\n", i, tm, masterPva.pos, masterPva.vel,
        // masterPva.acc, ret, slavePva.pos, slavePva.vel, slavePva.acc);
        // 验证从轴位置是否在预期误差范围内
        EXPECT_NEAR(masterPva.pos, 10 + slavePva.pos, 0.01) << "Position mismatch at step " << i;

        // 验证从轴速度是否跟随主轴速度
        if (i > 0) {  // 跳过初始状态
            EXPECT_NEAR(slavePva.vel, masterPva.vel, 0.1) << "Velocity mismatch at step " << i;
        }

        // 验证运动状态
        if (i >= 2) {
            EXPECT_EQ(ret, GEAR_STATUS_GEAR_IN) << "Gear status mismatch at step " << i;
        }
    }
}