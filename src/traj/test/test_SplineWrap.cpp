
#include "SplineCppWrap.h"
#include "SplineWrap.h"
#include "common/misc/ZmcAllocator.hpp"
#include "gtest/gtest.h"
#include "math.h"

using namespace zmc;

#define PI (3.14159265358979)
TEST(SplineCppWrap, Linear) {
    int len = 5;
    double tt[] = {0, 2, 5, 8, 10};
    double yy[] = {0, 3, 7, 9, 10};
    AllocatorArray<512> alloc;
    SplineCppWrap csp;
    csp.set_allocator(&alloc);
    csp.set_xx_yy(len, tt, yy);
    csp.set_ptype(SPLINE_LINE_FULL);
    int err = csp.build();
    EXPECT_EQ(err, 0);
    double pos, vel, acc;
    for (int i = 0; i < len; i++) {
        double tm = tt[i];
        csp.get_pos_vel_acc(tm, &pos, &vel, &acc);
        EXPECT_NEAR(pos, yy[i], 1e-8);
        // EXPECT_NEAR(vel, vv[i], 1e-8);
    }
}

TEST(SplineCppWrap, Cubic) {
    int len = 4;
    double tt[] = {0, 2, 5, 8};
    double yy[] = {0, 3, 6, 9};
    double vv[] = {0, 3, 5, 1};
    AllocatorArray<512> alloc;
    SplineCppWrap csp;
    csp.set_allocator(&alloc);
    csp.set_xx_yy_vv(len, tt, yy, vv);
    csp.set_ptype(SPLINE_CUBIC_FULL);
    int err = csp.build();
    EXPECT_EQ(err, 0);
    double pos, vel, acc;
    for (int i = 0; i < len; i++) {
        double tm = tt[i];
        csp.get_pos_vel_acc(tm, &pos, &vel, &acc);
        EXPECT_NEAR(pos, yy[i], 1e-8);
        EXPECT_NEAR(vel, vv[i], 1e-8);
    }
}

TEST(SplineCppWrap, CubicCond) {
    int len = 5;
    double tt[] = {0, 2, 5, 8, 9, 11};
    double yy[] = {0, 3, 7, 9, 10, 11};
    double cond[] = {0, 1};
    AllocatorArray<512> alloc;
    SplineCppWrap csp;
    csp.set_allocator(&alloc);
    csp.set_xx_yy(len, tt, yy);
    csp.set_condition_value(cond, 2);
    csp.set_condition_type(1);
    csp.set_ptype(SPLINE_CUBIC_COND);
    int err = csp.build();
    EXPECT_EQ(err, 0);
    double pos, vel, acc;
    for (int i = 0; i < len; i++) {
        double tm = tt[i];
        csp.get_pos_vel_acc(tm, &pos, &vel, &acc);
        EXPECT_NEAR(pos, yy[i], 1e-8);
        // EXPECT_NEAR(vel, vv[i], 1e-8);
    }
}

TEST(SplineCppWrap, CubicCondSin) {
    int len = 360;
    double tt[360] = {};
    double yy[360] = {};
    double cond[] = {0, 1};

    for (int i = 0; i < 360; i++) {
        tt[i] = i;
        yy[i] = sin((i) / 180.0 * PI);
    }
    SplineCppWrap csp;
    csp.set_default_allocator();
    csp.set_xx_yy(len, tt, yy);
    csp.set_condition_value(cond, 2);
    csp.set_condition_type(1);
    csp.set_ptype(SPLINE_CUBIC_COND);
    int err = csp.build();
    EXPECT_EQ(err, 0);
    double pos, vel, acc;
    for (int i = 0; i < len; i++) {
        double tm = tt[i];
        csp.get_pos_vel_acc(tm, &pos, &vel, &acc);
        EXPECT_NEAR(pos, yy[i], 1e-8);
        // EXPECT_NEAR(vel, vv[i], 1e-8);
    }
}

TEST(SplineCppWrap, Fifth) {
    double tt[] = {0, 2, 5, 8, 9};
    double yy[] = {0, 3, 6, 9, 11};
    double vv[] = {0, 3, 5, 1, 0};
    double aa[] = {0, 1, 2, 3, -1};
    int len = 5;
    AllocatorArray<512> alloc;
    SplineCppWrap csp;
    csp.set_allocator(&alloc);
    csp.set_xx_yy_vv_aa(len, tt, yy, vv, aa);
    csp.set_ptype(SPLINE_FIFTH_FULL);
    int err = csp.build();
    EXPECT_EQ(err, 0);
    double pos, vel, acc;
    for (int i = 0; i < len; i++) {
        double tm = tt[i];
        csp.get_pos_vel_acc(tm, &pos, &vel, &acc);
        EXPECT_NEAR(pos, yy[i], 1e-8);
        EXPECT_NEAR(vel, vv[i], 1e-8);
        EXPECT_NEAR(acc, aa[i], 1e-8);
    }
}

TEST(SplineCppWrap, FifthXyva) {
    double xyva[][4] = {{0, 0, 0, 0}, {2, 3, 3, 1}, {5, 6, 5, 2}, {8, 9, 1, 3}, {9, 11, 0, -1}

    };
    int len = 5;
    AllocatorArray<512> alloc;
    SplineCppWrap csp;
    csp.set_allocator(&alloc);
    csp.set_xyva(len * 4, (double*)xyva);
    csp.set_ptype(SPLINE_FIFTH_XYVA);
    int err = csp.build();
    EXPECT_EQ(err, 0);
    double pos, vel, acc;
    for (int i = 0; i < len; i++) {
        double tm = xyva[i][0];
        csp.get_pos_vel_acc(tm, &pos, &vel, &acc);
        EXPECT_NEAR(pos, xyva[i][1], 1e-8);
        EXPECT_NEAR(vel, xyva[i][2], 1e-8);
        EXPECT_NEAR(acc, xyva[i][3], 1e-8);
    }
}

TEST(SplineCppWrap, Cubic_linear) {
    int len = 5;
    double tt[] = {0, 2, 5, 8, 9};
    double yy[] = {0, 3, 6, 9, 10};
    double vv[] = {0, 3, 5, 2, 1};
    AllocatorArray<512> alloc;
    SplineCppWrap csp;
    csp.set_allocator(&alloc);
    csp.set_xx_yy(len, tt, yy);
    csp.set_ptype(SPLINE_LINE_FULL);
    int err = csp.build();
    EXPECT_EQ(err, 0);
    double pos, vel, acc;
    for (int i = 0; i < len; i++) {
        double tm = tt[i];
        csp.get_pos_vel_acc(tm, &pos, &vel, &acc);
        EXPECT_NEAR(pos, yy[i], 1e-8);
        // EXPECT_NEAR(vel, vv[i], 1e-8);
    }

    csp.set_xx_yy_vv(len, tt, yy, vv);
    csp.set_ptype(SPLINE_CUBIC_FULL);
    csp.build();
    for (int i = 0; i < len; i++) {
        double tm = tt[i];
        csp.get_pos_vel_acc(tm, &pos, &vel, &acc);
        EXPECT_NEAR(pos, yy[i], 1e-8);
        EXPECT_NEAR(vel, vv[i], 1e-8);
    }
}

