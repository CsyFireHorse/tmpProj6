/*
 * @Author: ChenShengyong
 * @LastEditors: Chen Shengyong
 * @Date: 2022-06-09 11:14:01
 * @LastEditTime: 2022-10-08 15:26:22
 * @Description: Modify here please
 */
#include <math.h>
#include "space_utils.h"

double get_square_sum(const double* arr, unsigned int N){
    double s = 0;
    for (int i=0; i<N;i++){
        s += arr[i] * arr[i];
    }
    return s;
}
double get_norm(const double* arr, unsigned int N){
    double s = 0;
    for (int i=0; i<N;i++){
        s += arr[i] * arr[i];
    }
    return sqrt(s);
}
double get_cosine(const double* a, const double *b, int N){
    double c = 0;
    double sa = 0, sb = 0;
    for (int i=0; i<N;i++){
        c += a[i] * b[i];
        sa += a[i] * a[i];
        sb += b[i] * b[i];
    }
    if (sa == 0 || sb == 0)
        return 1;
    return c/(1e-8+sqrt(sa*sb));
}

double get_db_point_distance(const double* a, const double *b, int N){
    double c = 0;
    for (int i=0; i<N;i++){
        c += (a[i] - b[i])*(a[i] - b[i]);
    }
    return sqrt(c);
}

double get_r(double a, double b){
    return sqrt(a*a+b*b);
}

/**
 * @brief Determine if an arc defined by three points is clockwise
 * @param pt1 Start point coordinates [x,y]
 * @param pt2 Middle point coordinates on the arc [x,y]
 * @param pt3 End point coordinates [x,y]
 * @return bool - true for clockwise, false for counter-clockwise
 * @details Direction is determined by cross product of vectors:
 *          - Cross product > 0: counter-clockwise
 *          - Cross product < 0: clockwise
 *          - Cross product = 0: collinear points, treated as counter-clockwise
 */
bool pt2d_arc_is_clockwise(const double* pt1, const double* pt2, const double* pt3)
{
    double x1 = pt1[0], x2 = pt2[0], x3 = pt3[0];
    double y1 = pt1[1], y2 = pt2[1], y3 = pt3[1];
    
    // Calculate cross product of vectors pt1->pt2 and pt1->pt3
    double cross_product = (x2-x1)*(y3-y1) - (y2-y1)*(x3-x1);
    
    // cross_product < 0 indicates clockwise direction
    return cross_product < 0;
}