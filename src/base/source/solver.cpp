#include "solver.h"
#include "stdio.h"
#include <cmath>
#include <complex>
#include <vector>
#include <functional>

using namespace std;

#define M_PI 3.14159265358979323846
#define GOLDEN (0.618033988749895) // (std::sqrt(5)-1) / 2
#define RESPHI (1 - GOLDEN)
#define SQRT3  (1.732050807568877)
#define SQRT3DIV2  (0.866025403784439)

// Ax^2+Bx+C = 0
// \retval  0: no solution
// \retval  1: one solution
// \retval  2: two solution
int quadtric_equation_solver(const double* equ, double* ret) {
    double A = equ[0];
    double B = equ[1];
    double C = equ[2];

    if (abs(A) < 1e-6) {
        if (abs(B) < 1e-6) return 0;
        ret[0] = -C / B;
        return 1;
    }
    double d = B * B - 4 * A * C;
    if (d < -1e-6)
        return 0;
    else if (abs(d) < 1e-6) {
        ret[0] = -B / (2 * A);
        return 1;
    }
    double sd = sqrt(d);
    ret[0] = (-B - sd) / 2 / A;
    ret[1] = (-B + sd) / 2 / A;
    return 2;
}
double quadtric_equation_solver2(double a, double b, double c){
    double d = b*b -4 *a * c;
    // assert d >= 0
    // assert a != 0
    double sd = sqrt(d);
    // xx =[(-b-sd)/2/a, (-b+sd)/2/a]

    return (-b+sd)/2/a;
}


// x^3 + px + q = 0
int cube_equation_solver(double p, double q, double* ret){
    double Dt = q*q/4+p*p*p/27;
    if (Dt>0){
        double sdt = sqrt(Dt);
        double x = cbrt(-q/2+sdt) + cbrt(-q/2-sdt);
        *ret = x;
        return 1;
    }else if (Dt == 0){
        *ret = cbrt(q/2.0);
        ret[1] = -2 * cbrt(q/2.0);
        return 2;
    }else{
        complex<double> z1(Dt, 0);
        complex<double> sdt = pow(z1,1.0/2);

        complex<double> x = pow(-q/2+sdt, 1.0/3) + pow(-q/2-sdt, 1.0/3);
        ret[0] = x.real();

        complex<double> w(-1.0/2, SQRT3DIV2);
        complex<double> w2(-1.0/2, -SQRT3DIV2);
        complex<double> x1 = w*pow(-q/2+ sdt, 1.0/3) + w2*pow(-q/2-sdt, 1.0/3);
        ret[1] = x1.real();
        complex<double> x2 = w*w*pow(-q/2+ sdt, 1.0/3) + w2*w2*pow(-q/2-sdt, 1.0/3);
        ret[2] = x2.real();
        return 3;
    }
}

// Cardano method
int solveCubic(const double* equ, double* ret) {
    double a = equ[0];
    double b = equ[1];
    double c = equ[2];
    double d = equ[3];
    vector<double> roots;
    if (abs(a) < 1e-12) return quadtric_equation_solver(equ + 1, ret);

    // convert to t³ + pt + q = 0
    double p = (3 * a * c - b * b) / (3 * a * a);
    double q = (2 * b * b * b - 9 * a * b * c + 27 * a * a * d) / (27 * a * a * a);

    double delta = q * q / 4 + p * p * p / 27;

    if (delta > 1e-6) {
        double u = cbrt(-q / 2 + sqrt(delta));
        double v = cbrt(-q / 2 - sqrt(delta));
        ret[0] = u + v - b / (3 * a);
        return 1;
    } else if (delta < -1e-6) {
        double theta = acos(-q * sqrt(-27 / (p * p * p)) / 2);
        for (int k = 0; k < 3; ++k) {
            ret[k] = 2 * sqrt(-p / 3) * cos((theta + 2 * M_PI * k) / 3) - b / (3 * a);
        }
        return 3;
    } else {
        ret[0] = cbrt(q / 2) - b / (3 * a);
        if (fabs(q) < 1e-6)
            return 1;
        ret[1] = cbrt(q / 2) * -2 - b / (3 * a);
        return 2;
    }
    return 0;
}

template<int L>
double evalPoly(const double* coeffs, double t) {
    double result = coeffs[0];
    for (int i = 1; i < L; ++i) {
        result += coeffs[i] * pow(t, i);
    }
    return result;
}


int solveCubicIter(const double* coeffs, double* ret, double t0, double t1) {
    const int steps = 1000;
    const double epsilon = 1e-6;
    double step = (t1 - t0) / steps;
    auto f = [&](double t) { return evalPoly<4>(coeffs, t); };

    for (int i = 0; i < steps; ++i) {
        double a = t0 + i * step;
        double b = a + step;
        double fa = f(a);
        double fb = f(b);

        if (fa * fb > 0) continue;

        // binsect
        double left = a, right = b;
        while (right - left > epsilon) {
            double mid = (left + right) / 2;
            double fmid = f(mid);
            if (fmid == 0) break;
            (fa * fmid < 0) ? right = mid : left = mid;
        }
        ret[0] = (left + right) / 2;
    }
    return 0;
}


template<typename T>
int bisectionTmpl(T func, const SolverArgs& args, double* root) {
    double a = args.xMin;
    double b = args.xMax;
    double fa = func(a);
    double fb = func(b);
    if (fa * fb > 0) {
        return 0;
    }else if(fa == 0) {
        *root = a;
        return 1; 
    }else if(fb == 0){
        *root = b;
        return 1;
    }

    double c = a;
    while ((b - a) >= args.epsilon) {
        c = (a + b) / 2;


        if (func(c) == 0.0) {
            break;
        }

        if (func(c) * func(a) < 0) {
            b = c;
        } else {
            a = c;
        }
    }

    *root = c;
    return 1;
}

int bisection(Func func, const SolverArgs& args, double* root) {
    return bisectionTmpl<Func>(func, args, root);
}
int bisection(std::function<double(double)> func, const SolverArgs& args, double* root) {
    return bisectionTmpl<std::function<double(double)>>(func, args, root);
}

int secant_method(Func func, SolverArgs args, double* root) {
    double a = args.xMin;
    double b = args.xMax;
    double epsilon = args.epsilon;

    double fa = func(a);
    double fb = func(b);

    if (std::abs(fa) < epsilon) {
        *root = a;
        return 0;
    }

    if (std::abs(fb) < epsilon) {
        *root = b;
        return 0;
    }

    if (fa * fb > 0) {
        return 0;
    }

    double c;
    while (std::abs(b - a) >= args.epsilon) {
        c = b - fb * (b - a) / (fb - fa);
        double fc = func(c);

        if (std::abs(fc) < epsilon) {
            *root = c;
            return 1; 
        }

        a = b;
        fa = fb;
        b = c;
        fb = fc;
    }

    *root = b;
    return 1;
}



int golden_section_search(Func func, SolverArgs args, double* root) {
    // const double phi = (1 + std::sqrt(5)) / 2;
    double a = args.xMin;
    double b = args.xMax;
    double epsilon = args.epsilon;

    double c = b - (b - a) * GOLDEN;
    double d = a + (b - a) * GOLDEN;

    while ((b - a) > epsilon) {
        if (std::abs(func(c)) < std::abs(func(d))) {
            b = d;
        } else {
            a = c;
        }
        c = b - (b - a) * GOLDEN;
        d = a + (b - a) * GOLDEN;
    }

    *root = (b + a) / 2;
    return 0;
}



template<typename T>
int newton_method_tmpl(T func, const SolverArgs2& args, double* root) {
    const double h = 1e-14; 
    double x = (args.xMax + args.xMin)/2; 
    double delta = 1.0;
    // printf("x0=%f\n", x);
    for (int i = 0; i < args.max_iterations && std::abs(delta) > args.epsilon; ++i) {
        double fval = func(x);

        double df = (func(x + h) - func(x - h)) / (2 * h);
        if (args.cfg & 1 && (x < args.xMin || x > args.xMax)) {
            return 0;  // over of range
        }
        if (std::abs(df) < 1e-16) {
            return 0;
        }
        
        delta = fval / df;
        x -= delta;
    }
    
    *root = x;
    return std::abs(func(x)) < args.epsilon ? 1 : 0;
}


int newton_method(Func func, const SolverArgs2& args, double* root) {
    return newton_method_tmpl<Func>(func, args, root);
}

// std::function 
int newton_method(std::function<double(double)> func, 
                const SolverArgs2& args, double* root) {
    return newton_method_tmpl<std::function<double(double)>>(func, args, root);
}
#define SolverArgToArg2(x) SolverArgs2{x.xMin, x.xMax, x.epsilon, 100, 0, 100}

int newton_method(Func func, const SolverArgs& args, double* root) {
    return newton_method_tmpl<Func>(func, SolverArgToArg2(args), root);
}

// std::function 
int newton_method(std::function<double(double)> func, 
                const SolverArgs& args, double* root) {
    return newton_method_tmpl<std::function<double(double)>>(func, SolverArgToArg2(args), root);
}

int segmented_bisection(Func func, const SolverArgs& args, double* roots){
    return segmented_bisection(func, SolverArgToArg2(args), roots);
}

int segmented_bisection(Func func, const SolverArgs2& args, double* roots){
    int j=0;
    double step = (args.xMax - args.xMin) / args.steps;
    for (int i = 0; i < args.steps; ++i) {
        double a = args.xMin + i * step;
        double b = a + step;
        double fa = func(a);
        double fb = func(b);
        
        if (fa * fb > 0) continue;
        if (fa == 0) {
            // roots[j++] = a;
            continue; 
        }else if(fb == 0){
            roots[j++] = b;
            continue; 
        }
        // binsect
        double left = a, right = b;
        while (right - left > args.epsilon) {
            double mid = (left + right) / 2;
            double fmid = func(mid);
            if (fmid == 0) break;
            (fa * fmid < 0) ? right = mid : left = mid;
        }
        roots[j++] = (left + right) / 2;
    }
    return j;
}