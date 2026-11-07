#include "cubic_poly.h"
#include "stdio.h"
#include "FifthOrderPolyWrap.h"

using namespace zmc;
void CubicPoly_demo(){
    CubicPoly cp;
    // cp.init(1, 4, 0, 5, 1, 2);
    double t0 = 1, t1 = 3;
    cp.init(t0, t1, 0, 5, 1, 2);
    double dat[] = { cp.val(t0), cp.val(t1), cp.diff(t0), cp.diff(t1)};
    printf("%f,%f,%f,%f\n", dat[0], dat[1], dat[2], dat[3]);
}

void CubicPoly2_demo(){
    CubicPoly2 cp;
    double t0 = 1, t1 = 3;
    cp.init(t0, t1, 0, 5, 1, 2);
    double dat[] = { cp.val(t0), cp.val(t1), cp.diff(t0), cp.ddiff(t0)};
    printf("%f,%f,%f,%f\n", dat[0], dat[1], dat[2], dat[3]);
}


void FifthPoly_demo(){
    FifthOrderPoly cp;
    // cp.init(1, 4, 0, 5, 1, 2);
    double t0 = 1, t1 = 3;
    cp.init(t0, t1, 0, 5, 1, 2, 1, -1);
    double dat[] = { cp.val(t0), cp.val(t1), cp.diff(t0), cp.diff(t1), cp.ddiff(t0), cp.ddiff(t1)};
    printf("%f,%f,%f,%f,%f,%f\n", dat[0], dat[1], dat[2], dat[3], dat[4], dat[5]);
}

void FifthPoly_demo2(){
    FifthOrderPoly cp;
    // double t0 = 1, t1 = 3;
    // Xyva tpva = {t0, 0, 1, 1};
    // Xyva tpva2 = {t1, 5, 2, -1};
    double t0 = 101, t1 = 325;
    Xyva tpva = {t0, 100, 1000, 100};
    Xyva tpva2 = {t1, 500, 200, -111};
    cp.init(tpva.x, tpva2.x, tpva.y, tpva2.y, tpva.v, tpva2.v, tpva.a, tpva2.a);
    double dat[] = { t0,cp.val(t0), cp.diff(t0), cp.ddiff(t0), t1, cp.val(t1), cp.diff(t1), cp.ddiff(t1)};
    printf("%f,%f,%f,%f\n", dat[0], dat[1], dat[2], dat[3]);
    printf("%f,%f,%f,%f\n", dat[4], dat[5], dat[6], dat[7]);
}

void FifthOrderPolyWrap_demo(){
    FifthOrderPolyWrap cp;
    cp.init(1, 3, 0, 5, 1, 2, 1, -1);
    PosVelAcc pva = cp.get_pva(2);
    printf("%f,%f,%f\n", pva.pos, pva.vel, pva.acc);
}