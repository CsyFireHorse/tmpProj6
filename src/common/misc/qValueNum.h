#ifndef _ZMC_Q_VALUE_NUM_H_
#define _ZMC_Q_VALUE_NUM_H_

#include "stdint.h"

namespace zmc {

template<unsigned int DenBits, typename T = int32_t>
class QValueNum {
public:
    T value_;
    static T fromDouble(double value) {
        return static_cast<T>(value * (1 << DenBits));
    }

    static T fromFloat(float value) {
        return static_cast<T>(value * (1 << DenBits));
    }

    static T fromInt(int value) {
        return static_cast<T>(value * (1 << DenBits));
    }

    double toDouble() const {
        return static_cast<double>(value_) / (1 << DenBits);
    }

    float toFloat() const {
        return static_cast<float>(value_) / (1 << DenBits);
    }
};

}
#endif