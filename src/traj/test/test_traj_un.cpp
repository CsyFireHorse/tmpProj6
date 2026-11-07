/*
 * @ArtifactStatus garbage
 * @ArtifactStatusDate 2026-08-27
 * @ArtifactStatusAuthority AI
 *
 * Reuse one TrajWrap across line/circ while comparing against a fresh
 * reference of the same geometry. CountingAllocator is bound only to the
 * reused object so cache expand / reuse / destroy pairing can surface.
 *
 * AI modification signature: Cursor Grok 4.6
 */
#include "TrajCppWrap.h"
#include "common/misc/ZmcAllocator.hpp"
#include "common/zmc/zmc_predef.h"
#include "gtest/gtest.h"
#include "math.h"

using namespace zmc;

namespace {

const double kPi = 3.14159265358979323846;
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

// Configure a 2-axis absolute 3-4-5 line; caller builds.
void configure_line_2d(TrajWrap &traj) {
    const double begin[] = {0.0, 0.0};
    const double end[] = {3.0, 4.0};
    traj.set_begin_pos(begin);
    traj.set_line_param(end, false);
}

// Configure the shared BORDER half-circle; caller builds.
void configure_circ_2d(TrajWrap &traj) {
    const double begin[] = {1.0, 0.0};
    const double aux[] = {0.0, 1.0};
    const double end[] = {-1.0, 0.0};
    traj.set_begin_pos(begin);
    traj.set_circ_param(aux, end, false, false, CIRC_MODE_BORDER);
}

// Configure the shared four-axis absolute line; caller builds.
void configure_line_4d(TrajWrap &traj) {
    const double begin[] = {1.0, 2.0, 3.0, 4.0};
    const double end[] = {2.0, 4.0, 5.0, 8.0};
    traj.set_begin_pos(begin);
    traj.set_line_param(end, false);
}

// Eight-axis line: first four axes match the 4d case, trailing axes stay put.
void configure_line_8d(TrajWrap &traj) {
    const double begin[] = {1.0, 2.0, 3.0, 4.0, 0.0, 0.0, 0.0, 0.0};
    const double end[] = {2.0, 4.0, 5.0, 8.0, 0.0, 0.0, 0.0, 0.0};
    traj.set_begin_pos(begin);
    traj.set_line_param(end, false);
}

// Primary check: reused object must match a fresh same-geometry reference.
void expect_matches(const TrajWrap &actual, const TrajWrap &fresh, int axisNum,
                    double d) {
    EXPECT_EQ(actual.get_traj_type(), fresh.get_traj_type());
    EXPECT_NEAR(actual.get_plan(), fresh.get_plan(), kTolerance);

    double actualEnd[8] = {};
    double freshEnd[8] = {};
    actual.get_end_pos(actualEnd);
    fresh.get_end_pos(freshEnd);
    for (int i = 0; i < axisNum; ++i) {
        EXPECT_NEAR(actualEnd[i], freshEnd[i], kTolerance);
    }

    double actualPos[8] = {};
    double actualDif[8] = {};
    double freshPos[8] = {};
    double freshDif[8] = {};
    actual.get_slice(d, actualPos, actualDif);
    fresh.get_slice(d, freshPos, freshDif);
    for (int i = 0; i < axisNum; ++i) {
        EXPECT_NEAR(actualPos[i], freshPos[i], kTolerance);
        EXPECT_NEAR(actualDif[i], freshDif[i], kTolerance);
    }
}

}  // namespace

// Same object: line then circ twice; first circ owns CircCache, later switches reuse it.
TEST(TrajUn, ReuseLineThenCirc2d) {
    CountingAllocator allocator;
    {
        TrajWrap traj(2);
        traj.set_allocator(&allocator);

        configure_line_2d(traj);
        ASSERT_EQ(traj.build(), 0);
        {
            TrajWrap fresh(2);
            configure_line_2d(fresh);
            ASSERT_EQ(fresh.build(), 0);
            expect_matches(traj, fresh, 2, 2.5);
        }
        EXPECT_EQ(allocator.allocateCount, 0);
        EXPECT_EQ(allocator.deallocateCount, 0);

        // First circ expands heap to CircCache.
        configure_circ_2d(traj);
        ASSERT_EQ(traj.build(), 0);
        {
            TrajWrap fresh(2);
            configure_circ_2d(fresh);
            ASSERT_EQ(fresh.build(), 0);
            expect_matches(traj, fresh, 2, kPi / 2.0);
        }
        EXPECT_EQ(allocator.allocateCount, 1);
        EXPECT_EQ(allocator.deallocateCount, 0);

        configure_line_2d(traj);
        ASSERT_EQ(traj.build(), 0);
        {
            TrajWrap fresh(2);
            configure_line_2d(fresh);
            ASSERT_EQ(fresh.build(), 0);
            expect_matches(traj, fresh, 2, 2.5);
        }
        EXPECT_EQ(allocator.allocateCount, 1);
        EXPECT_EQ(allocator.deallocateCount, 0);

        configure_circ_2d(traj);
        ASSERT_EQ(traj.build(), 0);
        {
            TrajWrap fresh(2);
            configure_circ_2d(fresh);
            ASSERT_EQ(fresh.build(), 0);
            expect_matches(traj, fresh, 2, kPi / 2.0);
        }
        EXPECT_EQ(allocator.allocateCount, 1);
        EXPECT_EQ(allocator.deallocateCount, 0);
    }
    // Destructor must return every block this counting allocator issued.
    EXPECT_EQ(allocator.deallocateCount, allocator.allocateCount);
}

// Circ first, then line, then circ again; CircCache stays and is reused.
TEST(TrajUn, ReuseCircThenLine2d) {
    CountingAllocator allocator;
    {
        TrajWrap traj(2);
        traj.set_allocator(&allocator);

        configure_circ_2d(traj);
        ASSERT_EQ(traj.build(), 0);
        {
            TrajWrap fresh(2);
            configure_circ_2d(fresh);
            ASSERT_EQ(fresh.build(), 0);
            expect_matches(traj, fresh, 2, kPi / 2.0);
        }
        EXPECT_EQ(allocator.allocateCount, 1);
        EXPECT_EQ(allocator.deallocateCount, 0);

        configure_line_2d(traj);
        ASSERT_EQ(traj.build(), 0);
        {
            TrajWrap fresh(2);
            configure_line_2d(fresh);
            ASSERT_EQ(fresh.build(), 0);
            expect_matches(traj, fresh, 2, 2.5);
        }
        EXPECT_EQ(allocator.allocateCount, 1);
        EXPECT_EQ(allocator.deallocateCount, 0);

        configure_circ_2d(traj);
        ASSERT_EQ(traj.build(), 0);
        {
            TrajWrap fresh(2);
            configure_circ_2d(fresh);
            ASSERT_EQ(fresh.build(), 0);
            expect_matches(traj, fresh, 2, kPi / 2.0);
        }
        EXPECT_EQ(allocator.allocateCount, 1);
        EXPECT_EQ(allocator.deallocateCount, 0);
    }
    EXPECT_EQ(allocator.deallocateCount, allocator.allocateCount);
}

// TrajWrap(4) then set_allocator: rebind frees the default block and reallocates
// line cache from the counting allocator (alloc=1), then circ expands once more.
TEST(TrajUn, ReuseFourAxisLineThenTwoAxisCirc) {
    CountingAllocator allocator;
    {
        TrajWrap traj(4);
        traj.set_allocator(&allocator);
        EXPECT_EQ(allocator.allocateCount, 1);
        EXPECT_EQ(allocator.deallocateCount, 0);

        configure_line_4d(traj);
        ASSERT_EQ(traj.build(), 0);
        {
            TrajWrap fresh(4);
            configure_line_4d(fresh);
            ASSERT_EQ(fresh.build(), 0);
            expect_matches(traj, fresh, 4, 2.5);
        }

        // Shrink axis count, then expand cache for CircCache via the new allocator.
        ASSERT_EQ(traj.init_axis_num(2), 0);
        configure_circ_2d(traj);
        ASSERT_EQ(traj.build(), 0);
        {
            TrajWrap fresh(2);
            configure_circ_2d(fresh);
            ASSERT_EQ(fresh.build(), 0);
            expect_matches(traj, fresh, 2, kPi / 2.0);
        }
        // CircCache is larger than the 4-axis line cache: one expand.
        EXPECT_EQ(allocator.allocateCount, 2);
        EXPECT_EQ(allocator.deallocateCount, 1);
        const int allocAfterFirstCirc = allocator.allocateCount;

        ASSERT_EQ(traj.init_axis_num(4), 0);
        configure_line_4d(traj);
        ASSERT_EQ(traj.build(), 0);
        {
            TrajWrap fresh(4);
            configure_line_4d(fresh);
            ASSERT_EQ(fresh.build(), 0);
            expect_matches(traj, fresh, 4, 2.5);
        }
        EXPECT_EQ(allocator.allocateCount, allocAfterFirstCirc);

        ASSERT_EQ(traj.init_axis_num(2), 0);
        configure_circ_2d(traj);
        ASSERT_EQ(traj.build(), 0);
        {
            TrajWrap fresh(2);
            configure_circ_2d(fresh);
            ASSERT_EQ(fresh.build(), 0);
            expect_matches(traj, fresh, 2, kPi / 2.0);
        }
        // Second circ must reuse the already-expanded cache.
        EXPECT_EQ(allocator.allocateCount, allocAfterFirstCirc);
    }
    EXPECT_EQ(allocator.deallocateCount, allocator.allocateCount);
}

// TrajWrap(8) then set_allocator: rebind takes one 192-byte block; circ fits in it.
TEST(TrajUn, ReuseEightAxisLineThenCirc) {
    CountingAllocator allocator;
    {
        TrajWrap traj(8);
        traj.set_allocator(&allocator);
        EXPECT_EQ(allocator.allocateCount, 1);
        EXPECT_EQ(allocator.deallocateCount, 0);

        configure_line_8d(traj);
        ASSERT_EQ(traj.build(), 0);
        {
            TrajWrap fresh(8);
            configure_line_8d(fresh);
            ASSERT_EQ(fresh.build(), 0);
            expect_matches(traj, fresh, 8, 2.5);
        }

        ASSERT_EQ(traj.init_axis_num(2), 0);
        configure_circ_2d(traj);
        ASSERT_EQ(traj.build(), 0);
        {
            TrajWrap fresh(2);
            configure_circ_2d(fresh);
            ASSERT_EQ(fresh.build(), 0);
            expect_matches(traj, fresh, 2, kPi / 2.0);
        }
        // 192-byte line cache already covers CircCache; allocate count stays 1.
        EXPECT_EQ(allocator.allocateCount, 1);

        ASSERT_EQ(traj.init_axis_num(8), 0);
        configure_line_8d(traj);
        ASSERT_EQ(traj.build(), 0);
        {
            TrajWrap fresh(8);
            configure_line_8d(fresh);
            ASSERT_EQ(fresh.build(), 0);
            expect_matches(traj, fresh, 8, 2.5);
        }
        EXPECT_EQ(allocator.allocateCount, 1);
    }
    EXPECT_EQ(allocator.deallocateCount, allocator.allocateCount);
}
