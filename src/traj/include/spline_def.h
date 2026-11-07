/*
 * @Author: ChenSy
 * @Date: 2024-08-26 10:00:17
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-04-29 11:23:00
 */
#ifndef _SPLINE_DEF_H_
#define _SPLINE_DEF_H_



enum ENUM_SPLINE_COND {
    E_SPLINE_COND_CYCLIC,   // pos[0] = pos[N-1], vel[0] = vel[N-1], acc[0] = acc[N-1]
    E_SPLINE_COND_CLAMPPED, // vel[0], vel[N-1]
    E_SPLINE_COND_NATURE,  // acc[0] = 0, acc[N-1] = 0
    E_SPLINE_COND_ACCELER, // acc[0], acc[N-1]
    E_SPLINE_NOT_A_KNOT_COND, // jerk[1+EPS] = jerk[1-EPS], jerk[N-2-EPS] = jerk[N-2+EPS]
    E_SPLINE_BEG_VEL_ACC_COND, // acc[0], vel[0]
};

enum SPLINE_TYPE {
    SPLINE_TYPE_NULL,
    SPLINE_LINE_EQUIDISTANT, // p[N] , Equidistant
    SPLINE_LINE_FULL,    // t[N] + p[N]
    SPLINE_CUBIC_EQUIDISTANT, // p[N] + condition[2]
    SPLINE_CUBIC_COND,   // t[N] + p[N] + condition[2]
    SPLINE_CUBIC_COND2,  // {t,p}[N] + condition[2]
    SPLINE_CUBIC_FULL,   // t[N] + p[N] + v[N]
    SPLINE_CUBIC_VEL,    // t[N] + vel[N] + condition(a0) + p0
    SPLINE_CUBIC_ACC,    // t[N] + acc[N] + p0 + v0
    SPLINE_FIFTH_COND,   // {t,p,v}[N] + condition[4]
    SPLINE_FIFTH_FULL,   // t[N] + pos[N] + vel[N] + acc[N]
    SPLINE_FIFTH_XYVA,   // {t,p,v,a}[N]
};

#endif