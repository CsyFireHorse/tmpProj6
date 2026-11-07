#include "gtest/gtest.h"
#include "poly_spline.h"

using namespace zmc;

TEST(polySpline0, allocator) {
    printf("before allocator\n");
    {
        std::allocator<char> alloc;
        auto ptr = alloc.allocate(100);
        for (int i = 0; i < 100; i++) ptr[0] = i;
        alloc.deallocate(ptr, 100);
    }
    printf("after allocator\n");
}

TEST(polySpline0, LinearSplineProfile) {
    int len = 5;
    double tt[] = {0, 2, 5, 8, 10};
    double yy[] = {0, 3, 7, 9, 10};
    LinearSplineProfile csp(len, tt, yy);
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

TEST(polySpline, LinearSplineProfile) {
    int len = 5;
    double tt[] = {0, 2, 5, 8, 10};
    double yy[] = {0, 3, 7, 9, 10};
    LinearSplineProfile csp(len, tt, yy);
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


TEST(polySpline, CubicSplineProfile) {
    int len = 4;
    double tt[] = {0, 2, 5, 8};
    double yy[] = {0, 3, 6, 9};
    double vv[] = {0, 3, 5, 1};
    CubicSplineProfile csp(len, tt, yy, vv);
    printf("before CubicSplineProfile build\n");
    int err = csp.build();
    printf("after CubicSplineProfile build\n");
    EXPECT_EQ(err, 0);
    double pos, vel, acc;
    for (int i = 0; i < len; i++) {
        double tm = tt[i];
        csp.get_pos_vel_acc(tm, &pos, &vel, &acc);
        EXPECT_NEAR(pos, yy[i], 1e-8);
        EXPECT_NEAR(vel, vv[i], 1e-8);
    }
}


TEST(polySpline, FifthSplineProfile) {
    double tt[] = {0, 2, 5, 8, 9};
    double yy[] = {0, 3, 6, 9, 11};
    double vv[] = {0, 3, 5, 1, 0};
    double aa[] = {0, 1, 2, 3, -1};
    int len = 5;
    FifthOrderSplineProfile csp(len, tt, yy, vv, aa);
    printf("before FifthSplineProfile build\n");
    int err = csp.build();
    printf("after FifthSplineProfile build\n");
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

TEST(polySpline, FifthSplineProfile2) {
    double tt[501];
    double yy[501] = {0, 3, 6, 9, 11};
    double vv[501] = {0, 3, 5, 1, 0};
    double aa[501] = {0, 1, 2, 3, -1};
    for (int i = 0; i < 501; i++) {
        tt[i] = i;
    }

    int len = 501;
    FifthOrderSplineProfile csp(len, tt, yy, vv, aa);
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
