#include "SplineWrap.h"
#include "spline_def.h"
#include "common/misc/ZmcAllocator.hpp"
#include "gtest/gtest.h"
#include "math.h"

using namespace zmc;

#define PI (3.14159265358979)


TEST(zmc_spline_t, CubicCondSin) {
    int len = 360;
    double tt[360] = {};
    double yy[360] = {};
    double cond[] = {0, 1};

    for (int i = 0; i < 360; i++) {
        tt[i] = i;
        yy[i] = sin((i) / 180.0 * PI);
    }
    zmc_spline_t csp = zmc_spline_new();
    zmc_spline_set_default_allocator(csp);
    zmc_spline_set_xx_yy(csp, len, tt, yy);
    zmc_spline_set_condition_value(csp, cond, 2);
    zmc_spline_set_condition_type(csp, E_SPLINE_COND_CLAMPPED);
    zmc_spline_set_ptype(csp, SPLINE_CUBIC_COND);
    int err = zmc_spline_build(csp);
    EXPECT_EQ(err, 0);
    double pos, vel, acc;
    for (int i = 0; i < len; i++) {
        double tm = tt[i];
        zmc_spline_get_pos_vel_acc(csp, tm, &pos, &vel, &acc);
        EXPECT_NEAR(pos, yy[i], 1e-8);
        // EXPECT_NEAR(vel, vv[i], 1e-8);
    }
}