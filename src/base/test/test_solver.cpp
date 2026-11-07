#include <gtest/gtest.h>

#include "math.h"
#include "solver.h"

// using namespace zmc;

// TEST(solver, cube_equation_solver){}

TEST(CubeEquationSolverTest, One) {
    double roots[3];
    double coeff[] = {1, -3, 3, -1};
    int ret = solveCubic(coeff, roots);
    EXPECT_EQ(ret, 1);
    EXPECT_DOUBLE_EQ(roots[0], 1.0);
}

TEST(CubeEquationSolverTest, Double) {
    double roots[3];
    double coeff[] = {1, -5, 7, -3};
    int ret = solveCubic(coeff, roots);
    EXPECT_EQ(ret, 2);
    EXPECT_DOUBLE_EQ(roots[0], 1.0);
    EXPECT_DOUBLE_EQ(roots[1], 3.0);
}

TEST(CubeEquationSolverTest, Three) {
    double roots[3];
    double coeff[] = {1, -2, -5, 6};
    int ret = solveCubic(coeff, roots);
    EXPECT_EQ(ret, 3);
    EXPECT_DOUBLE_EQ(roots[0], 3.0);
    EXPECT_DOUBLE_EQ(roots[1], -2.0);
    EXPECT_NEAR(roots[2], 1.0, 1e-6);
}

template <int R, int R2>
double equ(double x) {
    return (x - R) * (x - R2);
};

TEST(solver, binsect) {
    // auto f = [](double x){ return (x+2)*x - 3;};
    double x;
    int ret = bisection(equ<-3, 1>, SolverArgs{-2, 2, 1e-6}, &x);
    EXPECT_EQ(ret, 1);
    EXPECT_NEAR(x, 1, 1e-6);
    bisection(equ<-3, 1>, SolverArgs{-4, 0, 1e-6}, &x);
    EXPECT_NEAR(x, -3, 1e-6);
}
TEST(solver, binsect2) {
    double x;
#define X 1000000
    bisection(equ<-3 * X, X>, SolverArgs{-2 * X, 2 * X, 1e-6 * X}, &x);
    EXPECT_NEAR(x, X, X * 1e-6);

    bisection(equ<-3 * X, X>, SolverArgs{-4 * X, 0, 1e-6 * X}, &x);
    EXPECT_NEAR(x, -3 * X, X * 1e-6);
}


TEST(solver, secant_method) {
    double x;
    secant_method(equ<-3 * X, X>, SolverArgs{-2 * X, 2 * X, 1e-6 * X}, &x);
    EXPECT_NEAR(x, X, X * 1e-6);

    secant_method(equ<-3 * X, X>, SolverArgs{-4 * X, 0, 1e-6 * X}, &x);
    // EXPECT_NEAR(x, -3 * X, X * 1e-6);
}

TEST(solver, goldenSectionSearch) {
    double x;
    golden_section_search(equ<1, 1>, SolverArgs{0, 3, 1e-6}, &x);
    EXPECT_NEAR(x, 1, 1e-6);
    golden_section_search(equ<-3 * X, X>, SolverArgs{-4 * X, 0, 1e-6 * X}, &x);
    EXPECT_NEAR(x, -3 * X, X * 1e-6);
}

TEST(solver, goldenSectionSearch2) {
#define X 1000000
    double x;
    golden_section_search(equ<X, X>, SolverArgs{0, 3 * X, 1e-6}, &x);
    EXPECT_NEAR(x, X, X * 1e-6);
#undef X
}


TEST(solver, newton_method) {
#define X 55
    double x;
    newton_method(equ<X, -X>, SolverArgs{-5, 3 * X, 1e-6}, &x);
    EXPECT_NEAR(x, X, X * 1e-6);

    newton_method(equ<X, -X>, SolverArgs{-3 * X, -2, 1e-6}, &x);
    EXPECT_NEAR(x, -X, X * 1e-6);
    int ret = newton_method(equ<X, -X>, SolverArgs{-3 * X, 3*X, 1e-6}, &x);
    printf("%d,%f\n", ret, x);
    // EXPECT_NEAR(x, -X, X * 1e-6);
#undef X
}

TEST(solver, segmented_bisection) {
    double xx[2];
    int ret = segmented_bisection(equ<-3, 1>, SolverArgs{-5, 5, 1e-6}, xx);
    EXPECT_EQ(ret, 2);
    EXPECT_NEAR(xx[0], -3, 1e-6);
    EXPECT_NEAR(xx[1], 1, 1e-6);
}