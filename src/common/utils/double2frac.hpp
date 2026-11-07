#ifndef _ZMC_DOUBLE2FRAC_HPP_
#define _ZMC_DOUBLE2FRAC_HPP_
#include <cmath>
#include <limits>

namespace zmc {
union NumDenUnion{
    struct {
        int num;
        int den;
    };
    int numDen[2];
};    
// convert double x to fraction num/den
// |x - num/den| is small, and den <= maxDen
inline 
bool doubleToFraction(double x, int& num, int& den,
                      int maxDen = 1000000)
{
    if (std::isnan(x) || std::isinf(x)) {
        return false; // cannot convert NaN/Inf
    }

    // handle sign
    int sign = (x < 0) ? -1 : 1;
    x = std::fabs(x);

    if (x == 0.0) {
        num = 0;
        den = 1;
        return true;
    }

    // continued fraction expansion
    long long n0 = 0, d0 = 1; // previous
    long long n1 = 1, d1 = 0; // current

    double value = x;
    while (true) {
        long long a = static_cast<long long>(std::floor(value));
        long long n2 = a * n1 + n0;
        long long d2 = a * d1 + d0;

        if (d2 > maxDen) {
            break; // stop when denominator too large
        }

        n0 = n1; d0 = d1;
        n1 = n2; d1 = d2;

        double frac = value - static_cast<double>(a);
        if (frac == 0.0) {
            break; // exact representation
        }
        value = 1.0 / frac;
    }

    num = sign * n1;
    den = d1;
    return true;
}
}
#endif