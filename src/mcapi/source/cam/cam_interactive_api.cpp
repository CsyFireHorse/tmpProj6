/*
 * @Author: ChenShengyong
 * @Date: 2025-03-21 16:18:01
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-03-28 13:36:23
 */
#include "cam_interactive.h"
#include "cam_interactive_api.h"

// using namespace zmc;


int calc_poly5_PosVelAcc(const CamXyva* lhs, const CamXyva* rhs, const double* xArr,
                         CamPvaj* pvajArr, int arrLen) {
    return ((const CamXyvaMethod*)lhs)->calc_PosVelAcc(*rhs, xArr, pvajArr, arrLen);
}

int calc_poly5_maxPosVelAcc(const CamXyva* lhs, const CamXyva* rhs, MaxPosVelAcc* out) {
    return ((const CamXyvaMethod*)lhs)->maxPosVelAcc(*rhs, *out);
}

int switch_poly5_segmentType(CamXyva* lhs, CamXyva* rhs, int segType) {
    return ((CamXyvaMethod*)lhs)->switch_segmentType(*rhs, segType);
}

int calc_poly5_maxPosVelAcc_WithSegType(const CamXyva* lhs, const CamXyva* rhs, MaxPosVelAcc* out, int segType){
    if (segType == 1){
        double v = (rhs->dY - lhs->dY) / (rhs->dX - lhs->dX);
        out->maxPos = rhs->dY > lhs->dY ? rhs->dY: lhs->dY;
        out->minPos = rhs->dY > lhs->dY ? lhs->dY: rhs->dY;
        out->maxAbsVel = v >0 ? v: -v;
        out->maxAbsAcc = 0;
    }else{
        return calc_poly5_maxPosVelAcc(lhs, rhs, out);
    }
};