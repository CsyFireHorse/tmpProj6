#include "solver.h"
#include <stdio.h>


// Define the function whose zero crossing is to be found.
static double f(double x) {
    return x * x - 4; // Example function: x^2 - 4.
}
void bisection_demo() {
    double a = 0, b = 5, epsilon = 0.0001;
    SolverArgs args = {a, b, epsilon};
    double root;
    int ret = bisection(f, args, &root);
    printf("%d,%f\n", ret, root);
}
