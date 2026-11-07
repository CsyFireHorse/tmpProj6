/*
 * @Author: ChenShengyong
 * @Date: 2025-03-21 16:18:01
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-04-28 10:58:39
 */
#include "cam_interactive.h"
#include "cubic_poly.h"
#include "solver.h"
#include "math.h"
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

using namespace zmc;

int _calc_poly5_PosVelAcc(const CamXyva& lhs, const CamXyva& rhs, const double* xArr, CamPvaj* pvajArr, int arrLen){
    if (rhs.dX == lhs.dX) {
        return -1;
    }
    if (arrLen <= 0) return -2;
    FifthOrderPoly poly;
    poly.init(lhs.dX, rhs.dX, lhs.dY, rhs.dY, lhs.dV, rhs.dV, lhs.dA, rhs.dA);
    for (int i = 0; i < arrLen; i++) {
        double x = xArr[i];
        if (x < lhs.dX || x > rhs.dX) {
            return -3;
        }
        PosVelAccJerk& pva = (PosVelAccJerk&) pvajArr[i];
        poly.get_pvaj(x, pva);
    }
    return 0;
}

inline void swap(double& a, double& b) {
    double temp = a;
    a = b;
    b = temp;
}
inline void sort2(double* arr) {
    if (arr[0] > arr[1]) {
        swap(arr[0], arr[1]);
    }
}

inline
void sort3(double* arr) {
    sort2(arr);    
    sort2(arr + 1); 

    if (arr[0] > arr[1]) {
        swap(arr[0], arr[1]);
    }
}

void sort4(double* arr) {
    // if (arr[0] > arr[1]) { swap(arr[0], arr[1]); }
    // if (arr[2] > arr[3]) { swap(arr[2], arr[3]); }
    

    // if (arr[0] > arr[2]) { swap(arr[0], arr[2]); }
    // if (arr[1] > arr[3]) { swap(arr[1], arr[3]); }
    
    // if (arr[1] > arr[2]) { swap(arr[1], arr[2]); }


    double min1 = MIN(arr[0], arr[1]);
    double max1 = MAX(arr[0], arr[1]);
    double min2 = MIN(arr[2], arr[3]);
    double max2 = MAX(arr[2], arr[3]);


    arr[0] = MIN(min1, min2);
    double temp_max = MAX(min1, min2);
    

    double mid1 = MIN(max1, max2);
    double mid2 = MAX(temp_max, MIN(max1, max2));
    
    arr[1] = MIN(mid1, mid2);
    arr[2] = MAX(mid1, mid2);
    arr[3] = MAX(max1, max2);
}
void sort_4_3_2(double* arr, int len) {
    switch (len) {
        case 4:
            sort4(arr);
            break;
        case 3:
            sort3(arr);
            break;
        case 2:
            sort2(arr);
            break;
        default:
            break;
    }
}

inline double polyval4(const double* pp, double x) {
    return (((pp[0] * x + pp[1]) * x + pp[2]) * x + pp[3]) * x + pp[4];
};


inline double max(double a, double b) {
    return (a > b) ? a : b; 
}

inline double min(double a, double b) {
    return (a <= b) ? a : b; 
}
int _calc_poly5_maxPosVelAcc(const CamXyva& lhs, const CamXyva& rhs, MaxMinPosVelAcc& out);
int _calc_poly5_maxPosVelAcc(const CamXyva& lhs, const CamXyva& rhs, MaxPosVelAcc& out) {
    MaxMinPosVelAcc out2;
    int ret = _calc_poly5_maxPosVelAcc(lhs, rhs, out2);
    if(ret) return ret;
    out.maxAbsAcc = max(fabs(out2.maxAcc), fabs(out2.minAcc));
    out.maxAbsVel = max(fabs(out2.maxVel), fabs(out2.minVel));
    out.minPos = out2.minPos;
    out.maxPos = out2.maxPos;
    return 0;
}
// pos poly5
// vel poly4
// acc poly3
// jerk poly2
int _calc_poly5_maxPosVelAcc(const CamXyva& lhs, const CamXyva& rhs, MaxMinPosVelAcc& out) {
    if (rhs.dX == lhs.dX) {
        return -1;
    }

    FifthOrderPoly poly;
    poly.init(lhs.dX, rhs.dX, lhs.dY, rhs.dY, lhs.dV, rhs.dV, lhs.dA, rhs.dA);
    const double tMin = 0;
    const double tMax = 1;
    {
        double sc3 = poly.sc * poly.sc * poly.sc;
        const double CoeffsJerk[] = {60 * poly.dat[0], 24 * poly.dat[1], 6 * poly.dat[2]};
        double roots[2];
        int ret = quadtric_equation_solver(CoeffsJerk, roots);
        out.maxAcc = MAX(fabs(lhs.dA), fabs(rhs.dA));
        out.minAcc = MIN(fabs(lhs.dA), fabs(rhs.dA));
        for (int i = 0; i < ret; i++) {
            if (roots[i] <0 || roots[i] > 1) continue;
            PosVelAcc pva;
            poly.get_pva_normal(roots[i], pva);
            out.maxAcc = MAX(out.maxAcc, fabs(pva.acc));
            out.minAcc = MIN(out.minAcc, fabs(pva.acc));
        }
    }
    double candidates[5] = {tMin, tMax};
    int candidate_num = 2;
    {
        const double CoeffsAcc[] = {20 * poly.dat[0], 12 * poly.dat[1], 6 * poly.dat[2],
                                    2 * poly.dat[3]};
        double roots2[3];
        int ret2 = solveCubic(CoeffsAcc, roots2);
        out.maxVel = MAX(fabs(lhs.dV), fabs(rhs.dV));
        out.minVel = MAX(fabs(lhs.dV), fabs(rhs.dV));
        for (int i = 0; i < ret2; i++) {
            double rt = roots2[i];
            if (rt <= 1e-6 || rt >= 1-1e-6) continue;
            PosVelAcc pva;
            candidates[candidate_num] = rt;
            poly.get_pva_normal(rt, pva);
            out.maxVel = MAX(out.maxVel, fabs(pva.vel));
            out.minVel = min(out.minVel, fabs(pva.vel));
            candidate_num++;
        }
        sort_4_3_2(candidates + 1, candidate_num - 1);
    }

    if (lhs.dY > rhs.dY) {
        out.minPos = rhs.dY;
        out.maxPos = lhs.dY;
    } else {
        out.minPos = lhs.dY;
        out.maxPos = rhs.dY;
    }
    const double CoeffsVel[] = {5 * poly.dat[0], 4 * poly.dat[1], 3 * poly.dat[2], 2 * poly.dat[3], poly.dat[4]};
    for (int i = 0; i < candidate_num - 1; i++) {
        double rt;
        int ret = newton_method([&CoeffsVel](double x) { return polyval4(CoeffsVel, x); },
                            SolverArgs2{candidates[i], candidates[i + 1], 1e-18, 20, 1}, &rt);
        if (ret) {
            if (rt <0 || rt > 1) continue;
            PosVelAcc pva;
            poly.get_pva_normal(rt, pva);
            out.minPos = MIN(out.minPos, pva.pos);
            out.maxPos = MAX(out.maxPos, pva.pos);
        }
    }
    return 0;
}

int _switch_poly5_segmentType(CamXyva& lhs, CamXyva& rhs, int segType) {
    if (rhs.dX == lhs.dX) {
        return -1;
    }
    if (segType == 1) {
        double v = (rhs.dY - lhs.dY) / (rhs.dX - lhs.dX);
        lhs.dA = 0;
        lhs.dV = v;
        rhs.dA = 0;
        rhs.dV = v;
    }
    return 0;
}

int CamXyvaMethod::calc_PosVelAcc(const CamXyva& pt, const double* xArr, CamPvaj* pvajArr, int arrLen)const{
    return _calc_poly5_PosVelAcc(*this, pt, xArr, pvajArr, arrLen);
}
int CamXyvaMethod::maxPosVelAcc(const CamXyva& pt, MaxPosVelAcc& out)const{
    return _calc_poly5_maxPosVelAcc(*this, pt, out);
}
int CamXyvaMethod::maxPosVelAcc(const CamXyva& pt, MaxMinPosVelAcc& out)const{
    return _calc_poly5_maxPosVelAcc(*this, pt, out);
}

int CamXyvaMethod::switch_segmentType(CamXyva& pt, int segType){
    return _switch_poly5_segmentType(*this, pt, segType);
}
