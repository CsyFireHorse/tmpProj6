#ifndef _SOLVER_H_
#define _SOLVER_H_

#include <functional>

// P0x^2+P1x+P2 = 0
// \retval  0: no solution
// \retval  1: one solution
// \retval  2: two solution
int quadtric_equation_solver(const double* equ, double* ret);
// x^3 + px + q = 0
int cube_equation_solver(double p, double q, double* ret);
// P0x^3+P1x^2+P2x+P3 = 0
int solveCubic(const double* equ, double* ret);

struct SolverArgs {
    double xMin;
    double xMax;
    double epsilon;
};
struct SolverArgs2 {
    double xMin;
    double xMax;
    double epsilon;
    int max_iterations;// = 1000;
    unsigned char cfg;//  = 0;
    int steps;//  = 100;
};

typedef double (*Func)(double);

// 
// \param[in]  func  function pointer
// \param[in]  args    parameters of solver
// \param[out] root    root
// \retval  0: no solution
// return  nunber of roots
int bisection(Func func, const SolverArgs& args, double* root);
int secant_method(Func func, SolverArgs args, double* root);
int golden_section_search(Func func, SolverArgs args, double* root);

int bisection(std::function<double(double)> func, const SolverArgs& args, double* root);

int segmented_bisection(Func func, const SolverArgs& args, double* roots);
int segmented_bisection(Func func, const SolverArgs2& args, double* roots);

int newton_method(Func func, const SolverArgs& args, double* root);
// std::function 
int newton_method(std::function<double(double)> func, const SolverArgs& args, double* root);
int newton_method(Func func, const SolverArgs2& args, double* root);
int newton_method(std::function<double(double)> func, const SolverArgs2& args, double* root);

#include "solver_v0.h"
#endif
