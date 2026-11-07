#include "solver.h"
#include <stdio.h>


// 定义需要求解过零点的函数
static double f(double x) {
    return x * x - 4; // 示例函数 x^2 - 4
}
void bisection_demo() {
    double a = 0, b = 5, epsilon = 0.0001;
    SolverArgs args = {a, b, epsilon};
    double root;
    int ret = bisection(f, args, &root);
    printf("%d,%f\n", ret, root);
}