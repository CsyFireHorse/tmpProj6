#ifndef _ZMC_POLYVAL_H_
#define _ZMC_POLYVAL_H_


namespace zmc{

// ax^2 + b
inline double polyval1(double x, double a, double b=0){
    return a*x + b;
}
// ax^2 + bx + c
inline double polyval2(double x, double a, double b, double c=0){
    return (a*x + b)*x + c;
}
// ax^3 + bx^2 + cx + d
inline double polyval3(double x, double a, double b, double c, double d=0){
    return ((a*x + b)*x + c ) * x + d;
}

inline double polyval2(const double* pp, double x){
    return (x*pp[0]+pp[1])*x + pp[2];
};

inline double polyval3(const double* pp, double x){
    return ((pp[0]*x+pp[1])*x + pp[2])*x+ pp[3];
};

inline double polyval4(const double* pp, double x){
    return (((pp[0]*x+pp[1])*x + pp[2])*x + pp[3])*x + pp[4];
};

}

#endif