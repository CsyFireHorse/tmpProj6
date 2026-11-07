#include "cubic_spline.h"
#include "gtest/gtest.h"


TEST(CubicSpline, clamp) {
    int N = 7;
    double tt[] = {0, 1, 1.5, 2, 2.5, 3, 4};
    double yy[] = {0, 17, 26, 23, 26, 17, 0};
    double pts[2] = {1, -1};
    CubicSpline csp(N, tt, yy, pts, E_SPLINE_COND_CLAMPPED);
    double pos, vel, acc;
    for (int i = 0; i < N; i++) {
        double tm = tt[i];
        csp.get_pos_vel_acc(tm, &pos, &vel, &acc);
        EXPECT_NEAR(pos, yy[i], 1e-8);
        if (i == 0) EXPECT_NEAR(vel, pts[0], 1e-8);
        if (i == N - 1) EXPECT_NEAR(vel, pts[1], 1e-8);
        // EXPECT_NEAR(acc, aa[i], 1e-8);
    }
}

TEST(CubicSpline, nature) {
    int N = 7;
    double tt[] = {0, 1, 1.5, 2, 2.5, 3, 4};
    double yy[] = {0, 17, 26, 23, 26, 17, 11};
    double pts[2] = {1, -1};
    CubicSpline csp(N, tt, yy, pts, E_SPLINE_COND_NATURE);
    double pos, vel, acc;
    for (int i = 0; i < N; i++) {
        double tm = tt[i];
        csp.get_pos_vel_acc(tm, &pos, &vel, &acc);
        EXPECT_NEAR(pos, yy[i], 1e-8);
        if (i == 0) EXPECT_NEAR(acc, pts[0], 1e-8);
        if (i == N - 1) EXPECT_NEAR(acc, pts[1], 1e-8);
        // EXPECT_NEAR(acc, aa[i], 1e-8);
    }
}
