
#include "lookahead_relaxation.h"
#include "lookaheadImpl.hpp"

int lookahead_relaxation(double* varr, const double* sArr, int N, const double* accArr,
                         const double* decArr, const double* jerkArr) {
    zmc::LookaheadVel lkhd;
    lkhd._len = N;
    lkhd.sArr = (double*)sArr;
    lkhd.accArr = (double*)accArr;
    lkhd.decArr = (double*)decArr;
    lkhd.jerkArr = (double*)jerkArr;
    return lkhd.relaxation(varr);
}