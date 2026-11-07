#if 0
#include <gtest/gtest.h>
#include "VelUnCppWrap.h"
#include "stdio.h"
#include "vel/source/VelFollower.h"
// #include "velocity_utils.hpp"

using namespace zmc;

TEST(velFollow, base){

    VelocityWrap vw;
    vw.set_config(5, 3, 3, 0);
    vw.set_begin_pva(10, 0);
    vw.set_end_pva(20, 0);
    vw.build();

    const double Ts = 0.1;
    VelGearFollower egt;
    egt.set_config(2, 3, 3, 0);
    egt.set_begin_end_pos_vel(0, 0, 0, 0);
    egt.set_begin_pos(0, 0, 0);
    egt.num = 1;
    egt.den = 1;
    egt.Ts = Ts;

    PosInfo mPos = {5, -4, 0};
    double pos, vel, acc;
    for (int i = -2; i < 42; i++) {
        double tm = i * Ts;
        vw.get_slice(tm, &mPos.pos, &mPos.vel, &mPos.acc);
        egt.follow(mPos);
        egt.build();
        egt.get_slice(tm, &pos, &vel, &acc);
        // printf("%d,%f,%f,%f,%f,%f,%f,%f\n", i, tm, mPos.pos, mPos.vel, mPos.acc, pos, vel ,acc);
        EXPECT_NEAR(pos, mPos.pos - 10, 1e-6);
        EXPECT_NEAR(vel, mPos.vel, 1e-6);
    }
    EXPECT_NEAR(pos, 10, 1e-6);
    EXPECT_NEAR(vel, 0, 1e-6);
}
#endif