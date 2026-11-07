#ifndef _VEL_RAMP_TYPE_HPP_
#define _VEL_RAMP_TYPE_HPP_


#include "str2hash.hpp"
#include "stdint.h"

namespace zmc{

typedef enum VelRampType_t:int16_t{
    TRAPEZ,
    SINSQUARE,
    QUADRATIC_RAMP,
    QUADRATIC_SMOOTH_RAMP,
    UNKNOWN_RAMP = -1,
} VelRampType;


inline VelRampType VelRampTypeFromString(const char* str) {
    const unsigned int hash = strhash(str);
    switch(hash) {
        case strhash("TRAPEZ"): return TRAPEZ;
        case strhash("SINSQUARE"): return SINSQUARE;
        case strhash("QUADRATIC_RAMP"): return QUADRATIC_RAMP;
        case strhash("QUADRATIC_SMOOTH_RAMP"): return QUADRATIC_SMOOTH_RAMP;
        default: return UNKNOWN_RAMP;
    }
}
}

#endif