#include "cubic_poly.h"
#include "gtest/gtest.h"
#include "FifthOrderPolyWrap.h"

using namespace zmc;

#define LEN(x) sizeof(x) / sizeof(*(x))

TEST(polySpline, CubicPoly) {
    CubicPoly cp;
    double t0 = 1, t1 = 3;
    const double arr[] = {0, 5, 1, 2};
    cp.init(t0, t1, arr[0], arr[1], arr[2], arr[3]);
    double dat[] = {cp.val(t0), cp.val(t1), cp.diff(t0), cp.diff(t1)};
    for (int i = 0; i < 4; i++) EXPECT_NEAR(dat[i], arr[i], 1e-8);
}
TEST(polySpline, CubicPoly2) {
    CubicPoly cp;
    double t0 = 1, t1 = 3;
    const double arr[] = {0, 5, 1, 2};
    const double tpva2Arr[][6] = {
        {1, 3, 0, 5, 1, 2},
        {1, 3, 2, 3, 4, 5},
        {1, 3, 3, 2, 1, -2},
        {101,325, 100,500, 1000, 200}
    };
    for (int i=0; i<LEN(tpva2Arr); i++){
        const double *arr = tpva2Arr[i];
        cp.init(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);
        double dat[] = {arr[0], arr[1], cp.val(arr[0]), cp.val(arr[1]), cp.diff(arr[0]), cp.diff(arr[1])};
        for (int i = 2; i < 6; i++) EXPECT_NEAR(dat[i], arr[i], 1e-8);
    }
}



TEST(polySpline, FifthPoly) {
    FifthOrderPoly cp;
    double t0 = 1, t1 = 3;
    const double arr[] = {0, 5, 1, 2, 1, -1};
    cp.init(t0, t1, arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]);
    PosVelAcc pva = cp.get_pva(t0);
    PosVelAcc pva2 = cp.get_pva(t1);
    double dat[] = {pva.pos, pva2.pos, pva.vel, pva2.vel, pva.acc, pva2.acc};
    for (int i = 0; i < 6; i++) EXPECT_NEAR(dat[i], arr[i], 1e-8);
}


TEST(polySpline, FifthPoly2) {
    FifthOrderPoly cp;
    const double tpva2Arr[][8] = {
        {1, 3, 0, 5, 1, 2, 1, -1},
        {1, 3, 2, 3, 4, 5, 6, 8},
        {1, 3, 3, 2, 1, -2, -4 , -8},
        {101,325, 100,500, 1000,200, 100,-111},
        {325,101, 100,500, 1000,200, 100,-111}
    };
    for (int i=0; i<LEN(tpva2Arr); i++){
        const double *arr = tpva2Arr[i];
        cp.init(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5],arr[6], arr[7]);
        PosVelAcc pva = cp.get_pva(arr[0]);
        PosVelAcc pva2 = cp.get_pva(arr[1]);
        double dat[] = {arr[0], arr[1], pva.pos, pva2.pos, pva.vel, pva2.vel, pva.acc, pva2.acc};
        for (int i = 2; i < 8; i++) EXPECT_NEAR(dat[i], arr[i], 1e-8);
    }
}

TEST(FifthOrderPolyWrap, base) {
    FifthOrderPolyWrap   cp;
    const double tpva2Arr[][8] = {
        {1, 3, 0, 5, 1, 2, 1, -1},
        {1, 3, 2, 3, 4, 5, 6, 8},
        {1, 3, 3, 2, 1, -2, -4 , -8},
        {101,325, 100,500, 1000,200, 100,-111},
        {325,101, 100,500, 1000,200, 100,-111}
    };
    for (int i=0; i<LEN(tpva2Arr); i++){
        const double *arr = tpva2Arr[i];
        cp.init(arr[0], arr[1], arr[2], arr[3], arr[4], arr[5],arr[6], arr[7]);
        PosVelAcc pva = cp.get_pva(arr[0]);
        PosVelAcc pva2 = cp.get_pva(arr[1]);
        double dat[] = {arr[0], arr[1], pva.pos, pva2.pos, pva.vel, pva2.vel, pva.acc, pva2.acc};
        for (int i = 2; i < 8; i++) EXPECT_NEAR(dat[i], arr[i], 1e-8);
    }
}