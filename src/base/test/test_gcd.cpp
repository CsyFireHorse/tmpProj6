#include <gtest/gtest.h>
#include "common/misc/math_gcd.hpp"
using namespace zmc;


TEST(solver, gcd)
{
    EXPECT_EQ(gcd(3, 5), 1);
    EXPECT_EQ(gcd(1, 15), 1);
    EXPECT_EQ(gcd(60, 24), 12);
    EXPECT_EQ(gcd(33, 33), 33);

    // EXPECT_EQ(gcd(60, -24), 12);
    // EXPECT_EQ(gcd(-60, -24), 12);
    // EXPECT_EQ(gcd(-60, 24), 12);
    // EXPECT_EQ(gcd(-1, 15), 1);
    // EXPECT_EQ(gcd(1, -15), 1);
    // EXPECT_EQ(gcd(-1, -15), 1);
}
