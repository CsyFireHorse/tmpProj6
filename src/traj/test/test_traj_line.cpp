#include "TrajCppWrap.h"
#include "common/misc/ZmcAllocator.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "common/zmc/zmc_predef.h"
#include "gtest/gtest.h"
#include "math.h"

using namespace zmc;

namespace {

const double kTolerance = 1e-9;

// Count cache requests while returning one reusable aligned buffer.
class CountingAllocator : public PmAllocator {
   public:
    void *allocate(int n) override {
        ++allocateCount;
        return n <= static_cast<int>(sizeof(buffer)) ? buffer : nullptr;
    }

    void deallocate(void *p, int n) override {
        (void)p;
        (void)n;
        ++deallocateCount;
    }

    int allocateCount = 0;
    int deallocateCount = 0;

   private:
    alignas(double) uint8_t buffer[3 * MC_MOTION_MAX_AXES * sizeof(double)];
};

}  // namespace

// A four-axis absolute line evaluates every output axis from one path coordinate.
TEST(TrajLine, FourAxisAbsoluteKinematics) {
    TrajWrap traj;
    const double begin[] = {1.0, 2.0, 3.0, 4.0};
    const double end[] = {2.0, 4.0, 5.0, 8.0};

    ASSERT_EQ(traj.init_axis_num(4), 0);
    traj.set_begin_pos(begin);
    traj.set_line_param(end, false);
    ASSERT_EQ(traj.build(), 0);
    EXPECT_NEAR(traj.get_plan(), 5.0, kTolerance);

    double pos[4] = {};
    double vel[4] = {};
    double acc[4] = {};
    traj.get_slice3(2.5, 10.0, 15.0, pos, vel, acc);

    const double expectedPos[] = {1.5, 3.0, 4.0, 6.0};
    const double expectedVel[] = {2.0, 4.0, 4.0, 8.0};
    const double expectedAcc[] = {3.0, 6.0, 6.0, 12.0};
    for (int i = 0; i < 4; ++i) {
        EXPECT_NEAR(pos[i], expectedPos[i], kTolerance);
        EXPECT_NEAR(vel[i], expectedVel[i], kTolerance);
        EXPECT_NEAR(acc[i], expectedAcc[i], kTolerance);
    }

    double actualEnd[4] = {};
    traj.get_end_pos(actualEnd);
    for (int i = 0; i < 4; ++i) {
        EXPECT_NEAR(actualEnd[i], end[i], kTolerance);
    }
}

// Relative line input is translated from the configured N-dimensional beginning point.
TEST(TrajLine, FourAxisRelativeCoordinates) {
    TrajWrap traj;
    const double begin[] = {10.0, 20.0, 30.0, 40.0};
    const double diff[] = {-1.0, 2.0, -2.0, 4.0};
    const double expectedEnd[] = {9.0, 22.0, 28.0, 44.0};

    ASSERT_EQ(traj.init_axis_num(4), 0);
    traj.set_begin_pos(begin);
    traj.set_line_param(diff, true);
    ASSERT_EQ(traj.build(), 0);
    EXPECT_NEAR(traj.get_plan(), 5.0, kTolerance);

    double end[4] = {};
    traj.get_end_pos(end);
    for (int i = 0; i < 4; ++i) {
        EXPECT_NEAR(end[i], expectedEnd[i], kTolerance);
    }
}

// The configured maximum axis count uses the same cache layout as smaller high-dimensional lines.
TEST(TrajLine, MaximumAxisCount) {
    TrajWrap traj;
    double begin[MC_MOTION_MAX_AXES] = {};
    double diff[MC_MOTION_MAX_AXES] = {};
    double pos[MC_MOTION_MAX_AXES] = {};
    double vel[MC_MOTION_MAX_AXES] = {};
    double acc[MC_MOTION_MAX_AXES] = {};

    for (int i = 0; i < MC_MOTION_MAX_AXES; ++i) {
        begin[i] = static_cast<double>(i);
        diff[i] = i == MC_MOTION_MAX_AXES - 1 ? 8.0 : 0.0;
    }

    ASSERT_EQ(traj.init_axis_num(MC_MOTION_MAX_AXES), 0);
    traj.set_begin_pos(begin);
    traj.set_line_param(diff, true);
    ASSERT_EQ(traj.build(), 0);
    traj.get_slice3(4.0, 2.0, 3.0, pos, vel, acc);

    for (int i = 0; i < MC_MOTION_MAX_AXES - 1; ++i) {
        EXPECT_NEAR(pos[i], begin[i], kTolerance);
        EXPECT_NEAR(vel[i], 0.0, kTolerance);
        EXPECT_NEAR(acc[i], 0.0, kTolerance);
    }
    EXPECT_NEAR(pos[MC_MOTION_MAX_AXES - 1], begin[MC_MOTION_MAX_AXES - 1] + 4.0,
                kTolerance);
    EXPECT_NEAR(vel[MC_MOTION_MAX_AXES - 1], 2.0, kTolerance);
    EXPECT_NEAR(acc[MC_MOTION_MAX_AXES - 1], 3.0, kTolerance);
}

// Embedded storage keeps the same line behavior for one, two, and three axes.
TEST(TrajLine, EmbeddedDimensionRegression) {
    for (uint8_t axisNum = 1; axisNum <= 3; ++axisNum) {
        TrajWrap traj;
        const double begin[] = {1.0, 2.0, 3.0};
        const double diff[] = {0.0, 0.0, 6.0};
        double activeDiff[3] = {0.0, 0.0, 0.0};
        activeDiff[axisNum - 1] = diff[2];

        ASSERT_EQ(traj.init_axis_num(axisNum), 0);
        traj.set_begin_pos(begin);
        traj.set_line_param(activeDiff, true);
        ASSERT_EQ(traj.build(), 0);

        double pos[3] = {};
        traj.get_slice(3.0, pos, nullptr);
        for (int i = 0; i < axisNum; ++i) {
            const double expected = begin[i] + (i == axisNum - 1 ? 3.0 : 0.0);
            EXPECT_NEAR(pos[i], expected, kTolerance);
        }
    }
}

// A zero-length line remains at the beginning point with zero derivatives.
TEST(TrajLine, ZeroLengthAndDistanceClamping) {
    TrajWrap zeroLength;
    const double begin[] = {1.0, 2.0, 3.0, 4.0};
    const double zeroDiff[] = {0.0, 0.0, 0.0, 0.0};

    ASSERT_EQ(zeroLength.init_axis_num(4), 0);
    zeroLength.set_begin_pos(begin);
    zeroLength.set_line_param(zeroDiff, true);
    ASSERT_EQ(zeroLength.build(), 0);

    double pos[4] = {};
    double vel[4] = {};
    double acc[4] = {};
    zeroLength.get_slice3(10.0, 2.0, 3.0, pos, vel, acc);
    for (int i = 0; i < 4; ++i) {
        EXPECT_NEAR(pos[i], begin[i], kTolerance);
        EXPECT_NEAR(vel[i], 0.0, kTolerance);
        EXPECT_NEAR(acc[i], 0.0, kTolerance);
    }

    TrajWrap clamped;
    const double diff[] = {4.0, 0.0, 0.0, 0.0};
    ASSERT_EQ(clamped.init_axis_num(4), 0);
    clamped.set_begin_pos(begin);
    clamped.set_line_param(diff, true);
    ASSERT_EQ(clamped.build(), 0);
    clamped.get_slice(-1.0, pos, nullptr);
    EXPECT_NEAR(pos[0], begin[0], kTolerance);
    clamped.get_slice(10.0, pos, nullptr);
    EXPECT_NEAR(pos[0], begin[0] + 4.0, kTolerance);
}

// Invalid dimensions and insufficient allocator capacity return trajectory errors.
TEST(TrajLine, RejectsInvalidAxisCountAndAllocationFailure) {
    TrajWrap traj;
    EXPECT_EQ(traj.init_axis_num(0), ZMC_ERR_TRAJ_INVALID_AXIS_NUM);
    EXPECT_EQ(traj.get_axis_num(), 0);
    EXPECT_EQ(traj.init_axis_num(65), ZMC_ERR_TRAJ_INVALID_AXIS_NUM);
    EXPECT_EQ(traj.get_axis_num(), 0);

    AllocatorArray<64> smallAllocator;
    traj.set_allocator(&smallAllocator);
    EXPECT_EQ(traj.init_axis_num(4), ZMC_ERR_TRAJ_ALLOC_FAILED);
    EXPECT_EQ(traj.get_axis_num(), 0);
}

// Stable dimensions reuse the cache while a larger dimension requests one expansion.
TEST(TrajLine, ReusesAndExpandsSharedCache) {
    CountingAllocator allocator;
    TrajWrap traj;
    traj.set_allocator(&allocator);

    ASSERT_EQ(traj.init_axis_num(4), 0);
    EXPECT_EQ(allocator.allocateCount, 1);
    ASSERT_EQ(traj.init_axis_num(4), 0);
    EXPECT_EQ(allocator.allocateCount, 1);
    ASSERT_EQ(traj.init_axis_num(8), 0);
    EXPECT_EQ(allocator.allocateCount, 2);
    EXPECT_EQ(allocator.deallocateCount, 1);
}

// Absolute and relative endpoints are available before build.
TEST(TrajLine, EndPosBeforeBuild) {
    TrajWrap absTraj;
    const double begin[] = {1.0, 2.0, 3.0, 4.0};
    const double absEnd[] = {2.0, 4.0, 5.0, 8.0};

    ASSERT_EQ(absTraj.init_axis_num(4), 0);
    absTraj.set_begin_pos(begin);
    absTraj.set_line_param(absEnd, false);

    double pos[4] = {};
    absTraj.get_end_pos(pos);
    for (int i = 0; i < 4; ++i) {
        EXPECT_NEAR(pos[i], absEnd[i], kTolerance);
    }

    TrajWrap relTraj;
    const double diff[] = {-1.0, 2.0, -2.0, 4.0};
    const double expectedEnd[] = {9.0, 22.0, 28.0, 44.0};
    const double relBegin[] = {10.0, 20.0, 30.0, 40.0};

    ASSERT_EQ(relTraj.init_axis_num(4), 0);
    relTraj.set_begin_pos(relBegin);
    relTraj.set_line_param(diff, true);
    relTraj.get_end_pos(pos);
    for (int i = 0; i < 4; ++i) {
        EXPECT_NEAR(pos[i], expectedEnd[i], kTolerance);
    }
    ASSERT_EQ(relTraj.build(), 0);
    relTraj.get_end_pos(pos);
    for (int i = 0; i < 4; ++i) {
        EXPECT_NEAR(pos[i], expectedEnd[i], kTolerance);
    }
}

// A later beginning point updates the relative endpoint without rebuild.
TEST(TrajLine, RelativeEndPosFollowsBeginPos) {
    TrajWrap traj;
    const double begin[] = {10.0, 20.0, 30.0, 40.0};
    const double nextBegin[] = {1.0, 2.0, 3.0, 4.0};
    const double diff[] = {-1.0, 2.0, -2.0, 4.0};
    const double expectedEnd[] = {0.0, 4.0, 1.0, 8.0};

    ASSERT_EQ(traj.init_axis_num(4), 0);
    traj.set_line_param(diff, true);
    traj.set_begin_pos(begin);
    traj.set_begin_pos(nextBegin);

    double pos[4] = {};
    traj.get_end_pos(pos);
    for (int i = 0; i < 4; ++i) {
        EXPECT_NEAR(pos[i], expectedEnd[i], kTolerance);
    }
}
