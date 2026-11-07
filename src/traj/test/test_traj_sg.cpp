#include "TrajCppWrap.h"
#include "gtest/gtest.h"

using namespace zmc;

namespace {

const double kTolerance = 1e-9;

}  // namespace

// Absolute and relative single-axis endpoints are available before build.
TEST(TrajSg, EndPosBeforeAndAfterBuild) {
    TrajWrap absTraj;
    const double begin = 3.0;
    absTraj.set_begin_pos(&begin);
    absTraj.set_sg_param(8.0, false);

    double pos = 0.0;
    absTraj.get_end_pos(&pos);
    EXPECT_NEAR(pos, 8.0, kTolerance);
    ASSERT_EQ(absTraj.build(), 0);
    absTraj.get_end_pos(&pos);
    EXPECT_NEAR(pos, 8.0, kTolerance);

    TrajWrap relTraj;
    relTraj.set_begin_pos(&begin);
    relTraj.set_sg_param(5.0, true);
    relTraj.get_end_pos(&pos);
    EXPECT_NEAR(pos, 8.0, kTolerance);
    ASSERT_EQ(relTraj.build(), 0);
    relTraj.get_end_pos(&pos);
    EXPECT_NEAR(pos, 8.0, kTolerance);
    EXPECT_NEAR(relTraj.get_sg_direct(), 1.0, kTolerance);
}
