#include "TrajCppWrap.h"
#include "common/zmc/fbt_errorcode_def.h"
#include "gtest/gtest.h"
#include "math.h"

using namespace zmc;

namespace {

const double kPi = 3.14159265358979323846;
const double kTolerance = 1e-9;

void expect_xy(const double* value, double x, double y) {
    EXPECT_NEAR(value[0], x, kTolerance);
    EXPECT_NEAR(value[1], y, kTolerance);
}

void set_border_half_circle(TrajWrap& traj, bool pathChoice) {
    const double begin[] = {1.0, 0.0, 0.0};
    const double aux[] = {0.0, 1.0, 0.0};
    const double end[] = {-1.0, 0.0, 0.0};

    traj.set_begin_pos(begin);
    traj.set_circ_param(aux, end, pathChoice, false, CIRC_MODE_BORDER);
}

}  // namespace

// BORDER mode creates the arc that passes through the auxiliary point.
TEST(TrajCirc, Border2dAbsoluteHalfCircle) {
    TrajWrap traj(2);
    set_border_half_circle(traj, false);

    ASSERT_EQ(traj.build(), 0);
    EXPECT_NEAR(traj.get_plan(), kPi, kTolerance);

    double pos[2] = {};
    double tangent[2] = {};
    traj.get_slice(0.0, pos, tangent);
    expect_xy(pos, 1.0, 0.0);
    expect_xy(tangent, 0.0, 1.0);

    traj.get_slice(kPi / 2.0, pos, tangent);
    expect_xy(pos, 0.0, 1.0);
    expect_xy(tangent, -1.0, 0.0);

    traj.get_end_pos(pos);
    expect_xy(pos, -1.0, 0.0);
}

// get_slice3 combines tangential acceleration with the inward normal acceleration.
TEST(TrajCirc, Border2dKinematics) {
    TrajWrap traj(2);
    set_border_half_circle(traj, false);

    ASSERT_EQ(traj.build(), 0);

    double pos[2] = {};
    double vel[2] = {};
    double acc[2] = {};
    traj.get_slice3(0.0, 2.0, 3.0, pos, vel, acc);

    expect_xy(pos, 1.0, 0.0);
    expect_xy(vel, 0.0, 2.0);
    expect_xy(acc, -4.0, 3.0);
}

// Relative auxiliary and end points are translated from the configured beginning position.
TEST(TrajCirc, Border2dRelativeCoordinates) {
    TrajWrap traj(2);
    const double begin[] = {10.0, 20.0, 0.0};
    const double auxDiff[] = {-1.0, 1.0, 0.0};
    const double endDiff[] = {-2.0, 0.0, 0.0};

    traj.set_begin_pos(begin);
    traj.set_circ_param(auxDiff, endDiff, false, true, CIRC_MODE_BORDER);

    ASSERT_EQ(traj.build(), 0);
    EXPECT_NEAR(traj.get_plan(), kPi, kTolerance);

    double pos[2] = {};
    double tangent[2] = {};
    traj.get_slice(kPi / 2.0, pos, tangent);
    expect_xy(pos, 9.0, 21.0);
    expect_xy(tangent, -1.0, 0.0);

    traj.get_end_pos(pos);
    expect_xy(pos, 8.0, 20.0);
}

// PathChoice is intentionally ignored by BORDER mode.
TEST(TrajCirc, BorderIgnoresPathChoice) {
    TrajWrap clockwise(2);
    TrajWrap counterClockwise(2);
    set_border_half_circle(clockwise, false);
    set_border_half_circle(counterClockwise, true);

    ASSERT_EQ(clockwise.build(), 0);
    ASSERT_EQ(counterClockwise.build(), 0);
    EXPECT_NEAR(clockwise.get_plan(), counterClockwise.get_plan(), kTolerance);

    double clockwisePos[2] = {};
    double clockwiseTangent[2] = {};
    double counterClockwisePos[2] = {};
    double counterClockwiseTangent[2] = {};
    clockwise.get_slice(kPi / 4.0, clockwisePos, clockwiseTangent);
    counterClockwise.get_slice(kPi / 4.0, counterClockwisePos, counterClockwiseTangent);

    expect_xy(counterClockwisePos, clockwisePos[0], clockwisePos[1]);
    expect_xy(counterClockwiseTangent, clockwiseTangent[0], clockwiseTangent[1]);
}

// A 3D arc remains in the plane defined by its three input points.
TEST(TrajCirc, Border3dPlanarCircle) {
    TrajWrap traj(3);
    const double begin[] = {1.0, 0.0, 7.0};
    const double aux[] = {0.0, 1.0, 7.0};
    const double end[] = {-1.0, 0.0, 7.0};

    traj.set_begin_pos(begin);
    traj.set_circ_param(aux, end, false, false, CIRC_MODE_BORDER);

    ASSERT_EQ(traj.build(), 0);
    EXPECT_NEAR(traj.get_plan(), kPi, kTolerance);

    double pos[3] = {};
    double vel[3] = {};
    double acc[3] = {};
    traj.get_slice3(kPi / 2.0, 2.0, 3.0, pos, vel, acc);

    expect_xy(pos, 0.0, 1.0);
    EXPECT_NEAR(pos[2], 7.0, kTolerance);
    expect_xy(vel, -2.0, 0.0);
    EXPECT_NEAR(vel[2], 0.0, kTolerance);
    expect_xy(acc, -3.0, -4.0);
    EXPECT_NEAR(acc[2], 0.0, kTolerance);
}

// CENTER mode uses PathChoice to choose the clockwise or counter-clockwise arc.
TEST(TrajCirc, Center2dAbsolutePathChoice) {
    const double begin[] = {1.0, 0.0, 0.0};
    const double center[] = {0.0, 0.0, 0.0};
    const double end[] = {0.0, 1.0, 0.0};
    TrajWrap clockwise(2);
    TrajWrap counterClockwise(2);

    clockwise.set_begin_pos(begin);
    clockwise.set_circ_param(center, end, false, false, CIRC_MODE_CENTER);
    counterClockwise.set_begin_pos(begin);
    counterClockwise.set_circ_param(center, end, true, false, CIRC_MODE_CENTER);

    ASSERT_EQ(clockwise.build(), 0);
    ASSERT_EQ(counterClockwise.build(), 0);
    EXPECT_NEAR(clockwise.get_plan(), 1.5 * kPi, kTolerance);
    EXPECT_NEAR(counterClockwise.get_plan(), 0.5 * kPi, kTolerance);

    double pos[2] = {};
    double tangent[2] = {};
    clockwise.get_slice(0.5 * kPi, pos, tangent);
    expect_xy(pos, 0.0, -1.0);
    expect_xy(tangent, -1.0, 0.0);

    counterClockwise.get_slice(0.25 * kPi, pos, tangent);
    expect_xy(pos, sqrt(0.5), sqrt(0.5));
    expect_xy(tangent, -sqrt(0.5), sqrt(0.5));
}

// CENTER mode translates both the center and endpoint in relative coordinates.
TEST(TrajCirc, Center2dRelativeCoordinates) {
    TrajWrap traj(2);
    const double begin[] = {10.0, 20.0, 0.0};
    const double centerDiff[] = {-1.0, 0.0, 0.0};
    const double endDiff[] = {-1.0, 1.0, 0.0};

    traj.set_begin_pos(begin);
    traj.set_circ_param(centerDiff, endDiff, true, true, CIRC_MODE_CENTER);

    ASSERT_EQ(traj.build(), 0);
    EXPECT_NEAR(traj.get_plan(), 0.5 * kPi, kTolerance);

    double pos[2] = {};
    traj.get_slice(0.25 * kPi, pos, nullptr);
    expect_xy(pos, 9.0 + sqrt(0.5), 20.0 + sqrt(0.5));

    traj.get_end_pos(pos);
    expect_xy(pos, 9.0, 21.0);
}

// CENTER mode creates a deterministic right-handed plane basis in 3D.
TEST(TrajCirc, Center3dCounterClockwisePlane) {
    TrajWrap traj(3);
    const double begin[] = {1.0, 0.0, 7.0};
    const double center[] = {0.0, 0.0, 7.0};
    const double end[] = {0.0, 1.0, 7.0};

    traj.set_begin_pos(begin);
    traj.set_circ_param(center, end, true, false, CIRC_MODE_CENTER);

    ASSERT_EQ(traj.build(), 0);
    EXPECT_NEAR(traj.get_plan(), 0.5 * kPi, kTolerance);

    double pos[3] = {};
    double tangent[3] = {};
    traj.get_slice(0.25 * kPi, pos, tangent);
    expect_xy(pos, sqrt(0.5), sqrt(0.5));
    EXPECT_NEAR(pos[2], 7.0, kTolerance);
    expect_xy(tangent, -sqrt(0.5), sqrt(0.5));
    EXPECT_NEAR(tangent[2], 0.0, kTolerance);
}

// A small programmed center error is projected onto the perpendicular bisector.
TEST(TrajCirc, CenterProjectsSmallCenterError) {
    TrajWrap traj(2);
    const double begin[] = {1.0, 0.0, 0.0};
    const double center[] = {0.001, -0.001, 0.0};
    const double end[] = {0.0, 1.0, 0.0};

    traj.set_begin_pos(begin);
    traj.set_circ_param(center, end, true, false, CIRC_MODE_CENTER);

    ASSERT_EQ(traj.build(), 0);
    EXPECT_NEAR(traj.get_plan(), 0.5 * kPi, kTolerance);

    double pos[2] = {};
    traj.get_slice(0.25 * kPi, pos, nullptr);
    expect_xy(pos, sqrt(0.5), sqrt(0.5));
}

// CENTER mode rejects geometry that cannot define a unique arc.
TEST(TrajCirc, CenterRejectsInvalidGeometry) {
    const double begin[] = {1.0, 0.0, 0.0};
    const double center[] = {0.0, 0.0, 0.0};
    const double oppositeEnd[] = {-1.0, 0.0, 0.0};
    const double offsetCenter[] = {0.02, -0.02, 0.0};
    const double quarterEnd[] = {0.0, 1.0, 0.0};
    TrajWrap halfCircle(2);
    TrajWrap impreciseCenter(2);
    TrajWrap coincidentEnd(2);

    halfCircle.set_begin_pos(begin);
    halfCircle.set_circ_param(center, oppositeEnd, true, false, CIRC_MODE_CENTER);
    EXPECT_EQ(halfCircle.build(), ZMC_ERR_TRAJ_BUILD_FAILED);

    impreciseCenter.set_begin_pos(begin);
    impreciseCenter.set_circ_param(offsetCenter, quarterEnd, true, false, CIRC_MODE_CENTER);
    EXPECT_EQ(impreciseCenter.build(), ZMC_ERR_TRAJ_BUILD_FAILED);

    coincidentEnd.set_begin_pos(begin);
    coincidentEnd.set_circ_param(center, begin, true, false, CIRC_MODE_CENTER);
    EXPECT_EQ(coincidentEnd.build(), ZMC_ERR_TRAJ_BUILD_FAILED);
}

// CENTER mode keeps the same two- or three-axis restriction as other circular modes.
TEST(TrajCirc, CenterRejectsUnsupportedAxisCount) {
    const double center[] = {0.0, 0.0, 0.0};
    const double end[] = {0.0, 1.0, 0.0};

    TrajWrap oneAxis(1);
    oneAxis.set_circ_param(center, end, true, false, CIRC_MODE_CENTER);
    EXPECT_EQ(oneAxis.build(), ZMC_ERR_TRAJ_INVALID_AXIS_NUM);

    TrajWrap fourAxes(4);
    fourAxes.set_circ_param(center, end, true, false, CIRC_MODE_CENTER);
    EXPECT_EQ(fourAxes.build(), ZMC_ERR_TRAJ_INVALID_AXIS_NUM);
}

// Collinear points do not define a valid BORDER arc.
TEST(TrajCirc, BorderRejectsCollinearPoints) {
    TrajWrap traj(2);
    const double begin[] = {1.0, 0.0, 0.0};
    const double aux[] = {0.0, 0.0, 0.0};
    const double end[] = {-1.0, 0.0, 0.0};

    traj.set_begin_pos(begin);
    traj.set_circ_param(aux, end, false, false, CIRC_MODE_BORDER);

    EXPECT_EQ(traj.build(), ZMC_ERR_TRAJ_BUILD_FAILED);
}

// Circular trajectories only accept two or three axes.
TEST(TrajCirc, BorderRejectsUnsupportedAxisCount) {
    const double aux[] = {0.0, 1.0, 0.0};
    const double end[] = {-1.0, 0.0, 0.0};

    TrajWrap oneAxis(1);
    oneAxis.set_circ_param(aux, end, false, false, CIRC_MODE_BORDER);
    EXPECT_EQ(oneAxis.build(), ZMC_ERR_TRAJ_INVALID_AXIS_NUM);

    TrajWrap fourAxes(4);
    fourAxes.set_circ_param(aux, end, false, false, CIRC_MODE_BORDER);
    EXPECT_EQ(fourAxes.build(), ZMC_ERR_TRAJ_INVALID_AXIS_NUM);
}

// Unknown circular modes are rejected before mode-specific construction.
TEST(TrajCirc, RejectsInvalidMode) {
    TrajWrap traj(2);
    const double begin[] = {1.0, 0.0, 0.0};
    const double aux[] = {0.0, 1.0, 0.0};
    const double end[] = {-1.0, 0.0, 0.0};

    traj.set_begin_pos(begin);
    traj.set_circ_param(aux, end, false, false, static_cast<SMC_CIRC_MODE>(99));

    EXPECT_EQ(traj.build(), ZMC_ERR_TRAJ_INVALID_MODE);
}

// Circular endpoints are available from the programmed command before build.
TEST(TrajCirc, EndPosBeforeBuild) {
    TrajWrap absTraj(2);
    set_border_half_circle(absTraj, false);

    double pos[2] = {};
    absTraj.get_end_pos(pos);
    expect_xy(pos, -1.0, 0.0);
    ASSERT_EQ(absTraj.build(), 0);
    absTraj.get_end_pos(pos);
    expect_xy(pos, -1.0, 0.0);

    TrajWrap relTraj(2);
    const double begin[] = {10.0, 20.0, 0.0};
    const double auxDiff[] = {-1.0, 1.0, 0.0};
    const double endDiff[] = {-2.0, 0.0, 0.0};
    relTraj.set_begin_pos(begin);
    relTraj.set_circ_param(auxDiff, endDiff, false, true, CIRC_MODE_BORDER);
    relTraj.get_end_pos(pos);
    expect_xy(pos, 8.0, 20.0);
    ASSERT_EQ(relTraj.build(), 0);
    relTraj.get_end_pos(pos);
    expect_xy(pos, 8.0, 20.0);
}
