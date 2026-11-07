/*
 * @Author: ChenSy
 * @LastEditors: ChenSy
 * @Date: 2022-03-24 16:01:11
 * @LastEditTime: 2022-07-22 16:06:49
 * @Description: Modify here please
 */

#ifndef ZS_SOLVER_V0_H_
#define ZS_SOLVER_V0_H_

#include <math.h>
#include "stdlib.h"
#include <functional>


namespace zmc{

typedef std::function<double(double x)> func_iter;


// Ax^2 + Bx + C = 0
double quadtric_equation_solver2(double A, double B, double C);
double quadtric_equation_solver(const double* equ);

// x^3 + px + q = 0
double cube_equation_solver(double p, double q);
//  x^3 + px^2 + q = 0
double cube_equation_solver2(double p, double q);
double cubeEquation_solver(const double *pq, int idx);

double binsectItermethod(func_iter pf, double xmin, double xmax, double tol);
double secantItermethod(func_iter pf, double xx1,double xx2,double xeps);
double secantItermethod2(func_iter pf, double xx1,double xx2,double xeps);

// double goldenSectionSearch(double(*p)(double x),double l, double u, double tol);
double goldenSectionSearch(func_iter pf, double l, double u, double tol);

}

#endif  // ZS_SOLVER_H_
/* end of file */
