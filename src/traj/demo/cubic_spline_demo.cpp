/*
 * @Author: ChenSy
 * @Date: 2022-10-21 13:07:25
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-04-29 11:25:01
 */
#include "cubic_spline.h"
#include "stdio.h"

void CubicSpline_demo() {
    int N = 7;
    double tt[] = {0, 1, 1.5, 2, 2.5, 3, 4};
    double yy[] = {0, 17, 26, 23, 26, 17, 0};
    double pts[2] = {0, 0};
    CubicSpline csp(N, tt, yy, pts, E_SPLINE_COND_CLAMPPED);
    for (int i = 0; i < 16 + 1; i++) {
        double tm = i * 0.25;
        double sc = csp.get_pos(tm);
        double vc = csp.get_velocity(tm);
        double ac = csp.get_acc(tm);
        printf("%d,%f,%f,%f,%f\n", i, tm, sc, vc, ac);
    }
    // [0    1.4545    5.2955   10.7386   17.0000   22.9403   26.0000   24.6619   23.0000   24.6619
    // 26.0000   22.9403   17.0000   10.7386    5.2955    1.4545         0]
}

void CubicSpline_demo2() {
    int N = 7;
    double tt[] = {0, 1, 1.5, 2, 2.5, 3, 4};
    double yy[] = {0, 17, 26, 23, 26, 17, 0};
    double pts[2] = {0, 0};
    CubicSpline csp(N, tt, yy, pts, E_SPLINE_COND_CYCLIC);
    for (int i = 0; i < 16 + 1; i++) {
        double tm = i * 0.25;
        double sc = csp.get_pos(tm);
        double vc = csp.get_velocity(tm);
        double ac = csp.get_acc(tm);
        printf("%d,%f,%f,%f,%f\n", i, tm, sc, vc, ac);
    }
}

void CubicSpline_demo3() {
    int N = 7;
    double tt[] = {0, 1, 1.5, 2, 2.5, 3, 4};
    double yy[] = {0, 17, 26, 23, 26, 17, 0};
    double pts[2] = {1, -1};
    CubicSpline csp(N, tt, yy, pts, E_SPLINE_COND_NATURE);
    for (int i = 0; i < 16 + 1; i++) {
        double tm = i * 0.25;
        double sc = csp.get_pos(tm);
        double vc = csp.get_velocity(tm);
        double ac = csp.get_acc(tm);
        printf("%d,%f,%f,%f,%f\n", i, tm, sc, vc, ac);
    }
}

void spline_smooth_demo() {
    double arr[] = {1, 2, 3, 4, 5};
    int len = 5;
    double out[30];
    double ratio = 5;

    int a = spline_smooth(0, arr, len, out, ratio);
    for (int i = 0; i < len * ratio; i++) {
        printf("%f,", out[i]);
    }
}

void spline_to_bezier_demo() {
    double arr[] = {1, 2, 3, 4};
    double arr2[4] = {};
    Cubic_poly cp;

    int len = 5;
    cp.from_bezier(arr);
    cp.to_bezier(arr2);

    for (int i = 0; i < 4; i++) {
        printf("%f,", arr2[i] - arr[i]);
    }
}