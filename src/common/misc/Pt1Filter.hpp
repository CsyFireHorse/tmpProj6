#ifndef _ZMC_PT1_FILTER_HPP_
#define _ZMC_PT1_FILTER_HPP_

#include "stdint.h"
#include "stdio.h"

namespace zmc {

struct RcFilter {
    double _k = 1;
    double _val = 0;
    double step(double val) {
        _val = _k * val + (1 - _k) * _val;
        return _val;
    }
};

struct RcLimitFilter {
    double _k = 1;
    double _val = 0;
    double _limit = 100;
    double step(double val) {
        // double valNew = _k * val + (1 - _k) * _val;
        double valDif = _k * (val - _val);
        if (valDif > _limit) {
            _val += _limit;
        } else if (valDif < -_limit) {
            _val -= _limit;
        } else {
            _val += valDif;
        }
        return _val;
    }
};
inline double RcFilterCalc(double valNew, double valOld, double scale) {
    return valOld + scale * (valNew - valOld);
}
}  // namespace zmc
#endif
