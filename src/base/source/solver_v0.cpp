/*
 * @Author: ChenShengyong
 * @LastEditors: Chen Shengyong
 * @Date: 2022-03-24 16:00:29
 * @LastEditTime: 2022-07-22 16:06:35
 * @Description: Modify here please
 */

#include "solver_v0.h"
#include <complex>
#include "common/misc/polyval.h"

using std::complex;
namespace zmc{

double quadtric_equation_solver(const double* equ)
{
    double a = equ[0];
    double b = equ[1];
    double c = equ[2];
    return quadtric_equation_solver2(a, b, c);
}

double quadtric_equation_solver2(double a, double b, double c){
    double d = b*b -4 *a * c;
    // assert d >= 0
    // assert a != 0
    double sd = sqrt(d);
    // xx =[(-b-sd)/2/a, (-b+sd)/2/a]

    return (-b+sd)/2/a;
}

double cube_equation_solver(double p, double q){
    // x^3 + px + q = 0
    double Dt = q*q/4+p*p*p/27;
    if (Dt>0){
        double sdt = sqrt(Dt);
        double x = cbrt(-q/2+sdt) + cbrt(-q/2-sdt);
        if (0>0){
            // complex<double> w(-1.0/2, sqrt(3)/2);
            // complex<double> w2(-1.0/2, -sqrt(3)/2);
            // x = w^idx*cbrt(-q/2+sqrt(Dt))+w2^idx*cbrt(-q/2-sqrt(Dt);
        }
        return x;
    }else if (Dt == 0){
        return cbrt(q/2.0);
        // return -2 * cbrt(q/2.0);
    }else{
        complex<double> z1(Dt, 0);
        complex<double> sdt = pow(z1,1.0/2);
        int idx = 2;
        if (idx == 0){
            complex<double> x = pow(-q/2+sdt, 1.0/3) + pow(-q/2-sdt, 1.0/3);
            return x.real();
        }else{
            complex<double> w(-1.0/2, sqrt(3)/2);
            complex<double> w2(-1.0/2, -sqrt(3)/2);
            complex<double> x = pow(w, idx)*pow(-q/2+ sdt, 1.0/3) + pow(w2, idx)*pow(-q/2-sdt, 1.0/3);
            return x.real();
        }
    }
    return -1;
}

//  x^3 + px^2 + q = 0
double cube_equation_solver2(double p, double q){
    return 1./cube_equation_solver(p/q, 1/q);
}

double cubeEquation_solver(const double *pq, int idx){
    // assert pq[0] != 0
    double a = pq[1] / pq[0];
    double b = pq[2] / pq[0];
    double c = pq[3] / pq[0];
    if (a == 0)
        return cube_equation_solver(b, c);
    else if (b == 0)
        return cube_equation_solver2(a, c);
    else
        // todo
        return cube_equation_solver(b, c);
}

double binsectItermethod(func_iter pf, double xmin, double xmax, double tol){
    double x0 = xmin;
    double y0 = pf(x0);
    double x = xmax;
    double y = pf(x);
    double xn;
    // assert y*y0<=0
    while (1){
        xn = (x0+x)/2;
        // yield xn
        double yn = pf(xn);
        // printf("%f,%f\t", xn, yn);
        if (yn*y>0){
            // x, y = xn, yn;
            x = xn,
            y = yn;
        }
        else{
            // x0, y0 = xn, yn;
            x0 = xn,
            y0 = yn;
        }
        if (abs(x-x0)<tol)
            break;
    }
    return xn;
}

double secantItermethod(func_iter pf, double xx1,double xx2,double xeps)
{
    double x0 = xx1;
    double x = xx2;
    double y0 = pf(x0);
    double y, xn = x;
    while (1) {
        y = pf(x);
        if (abs(x0 - x)>xeps) {
            xn = x - (x-x0)*y/(y-y0);
            // yield xn
            y0=y; x0=x; x=xn;
            // x, x0, y0= xn, x, y;
        }
        else {
            return xn;
        }
    }
}


double secantItermethod2(func_iter pf, double xx1,double xx2,double xeps)
{
    double x0 = xx1;
    double x = xx2;
    double y0 = pf(x0);
    double y,xn = x;
    while (1) {
        y = pf(x);
        if (abs(x0 - x)>xeps) {
            xn = x - (x-x0)*y/(y-y0);
            if (xn < xx1){
                xn = (xx1 + x)/2;
            }else if (xn > xx2){
                xn = (xx2 + x)/2;
            }
            // yield xn
            y0=y; x0=x; x=xn;
            // x, x0, y0= xn, x, y;
        }
        else {
            return xn;
        }
    }
}

static const double GOLD = 0.618033988749895;
static const double RESPHI = 1 - GOLD;

double _goldenSectionSearch0(double(*p)(double x),double& l, double& u, double& x, double tol)
{
	//1)
	double xlk = l;
	double xuk = u;
	//2)
	double I1 = xuk - xlk;
	double Ikp1 = I1 * GOLD;
	double xak = xuk - Ikp1;
	double xbk = xlk + Ikp1;
	double fak = p(xak);
	double fbk = p(xbk);
	//3)
	while(true)
	{
		double Ikp2 = Ikp1 * GOLD;
		if(fak >= fbk)
		{
			xlk = xak;
			xuk = xuk;
 
			xak = xbk;
			xbk = xlk + Ikp2;
 
			fak = fbk;
			fbk = p(xbk);
		}
		else
		{
			xlk = xlk;
			xuk = xbk;
 
			xak = xuk - Ikp2;
			xbk = xak;
 
			fbk = fak;
			fak = p(xak);
		}
		Ikp1 = Ikp2;
		//4)
		if(Ikp2 < tol || xak > xbk)
		{
			if(fak > fbk)
				x = 0.5 * (xbk + xuk);
			if(fak == fbk)
				x = 0.5 * (xak + xbk);
			if(fak < fbk)
				x = 0.5 * (xlk + xak);
 
			l = xlk;
			u = xuk;
			return p(x);
		} 
	}
}

/**
 * Golden section search.
 *
 * @author Darran Zhang @ codelast.com
 * @param f     
 * @param x1  The left bound of current bounds.
 * @param x2  An interior point between (x1, x3).
 * @param x3  The right bound of current bounds.
 * @param tau The error tolerance to judge the convergence, recommended value is e^0.5, where e is is the required absolute precision of f(x).
 */
  template <typename Func>
  double _goldenSectionSearch(Func f, double x1, double x2, double x3, double tau) {
    double x4;
    if (x3 - x2 > x2 - x1) {    // 
      x4 = x2 + RESPHI * (x3 - x2);   // [x1, x2, x3, x3]
    } else {    // b <= a; the left half-interval is longer.
      x4 = x2 - RESPHI * (x2 - x1);   // [x1, x4, x2, x3]
    }

    // x1 <  x2, x4 < x3
    if (tau* abs(x3 - x1) > (abs(x2 - x4))) {    // Check whether the convergence condition is met.
      return (x3 + x1) / 2;
    }
    if (abs(x3 - x1) < tau * (abs(x2) + abs(x4))) {    // Check whether the convergence condition is met.
    //   return (x3 + x1) / 2;   // The convergence condition is met; return the midpoint as the minimum.
    }

    // assert (function.func(x4) != function.func(x2));

    /* The convergence condition is not met; continue searching. */
    if (f(x4) < f(x2)) {    // The function value at x4 is as shown by f4b.
      if (x3 - x2 > x2 - x1) {    // b > a; the right half-interval is longer.
        return _goldenSectionSearch(f, x2, x4, x3, tau);  // Continue with x2, x4, and x3 as the new points.
      } else {    // b <= a; the left half-interval is longer.
        return _goldenSectionSearch(f, x1, x4, x2, tau);  // Continue with x1, x4, and x2 as the new points.
      }
    } else {    // The function value at x4 is as shown by f4a.
      if (x3 - x2 > x2 - x1) {    // b > a; the right half-interval is longer.
        return _goldenSectionSearch(f, x1, x2, x4, tau);  // Continue with x1, x2, and x4 as the new points.
      } else {  // b <= a; the left half-interval is longer.
        return _goldenSectionSearch(f, x4, x2, x3, tau);  // Continue with x4, x2, and x3 as the new points.
      }
    }
}

double goldenSectionSearch0(double(*f)(double),double l, double u, double tol)
{
    double x = l + (u-l)*GOLD;
    _goldenSectionSearch(f, l, u, x, tol);
    return x;
}

// double goldenSectionSearch(double(*f)(double),double l, double u, double tol)
// {
//     double x = l + (u-l)*GOLD;
//     return _goldenSectionSearch(f, l, x, u, tol);
// }

double goldenSectionSearch(func_iter pf, double l, double u, double tol){
    double x = l + (u-l)*GOLD;
    return _goldenSectionSearch(pf, l, x, u, tol);
}
} /* namespace zmc */
