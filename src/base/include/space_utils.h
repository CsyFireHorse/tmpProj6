/*
 * @Author: ChenShengyong
 * @LastEditors: Chen Shengyong
 * @Date: 2022-06-09 11:14:35
 * @LastEditTime: 2022-10-10 11:30:24
 * @Description: Modify here please
 */

#ifndef SPACE_UTILS_H_
#define SPACE_UTILS_H_

#define DOUBLE2_ASSIGN(x, y) *((Double2*)x) = *((Double2*)y)
typedef class Double2 {
    double d[2];
} Double2;
#define DOUBLE3_ASSIGN(x, y) *((Double3*)x) = *((Double3*)y)
typedef class Double3 {
    double d[3];
} Double3;

double get_square_sum(const double* a, unsigned int N);
double get_norm(const double* a, unsigned int N);
double get_cosine(const double* a, const double* b, int N);
double get_db_point_distance(const double* a, const double* b, int N);
bool pt2d_arc_is_clockwise(const double* pt1, const double* pt2, const double* pt3);
double get_r(double a, double b);

#endif