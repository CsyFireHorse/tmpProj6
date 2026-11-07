#ifndef _ZMC_UN_INT64_HPP_
#define _ZMC_UN_INT64_HPP_


#include "stdint.h"

namespace zmc{


union un_I64{
    int64_t val;
    struct{
        uint32_t low;
        uint32_t high;
    };
    struct{
        int32_t iLow;
        int32_t iHigh;
    };
    inline void update(int inp){ 
        uint32_t unp = (uint32_t) inp;
        // maybe exist bug
        // increase and cross 0xFFFFFFFF, overflow
        if (low >= 0xC0000000 && unp < 0x40000000)
            high += 1;
        // decrease and cross 0x00000000
        if (unp >= 0xC0000000 && low < 0x40000000)
            high -= 1;
        low = unp;
    }
};
inline int32_t int64_to_32(int64_t x){
    un_I64 un = {x};
    return un.iLow;
}


template<unsigned int BY=2>
union un_I64Tmpl{
    int64_t val;
    struct{
        uint32_t low;
        uint32_t high;
    };
    struct{
        int32_t iLow;
        int32_t iHigh;
    };
    static_assert(BY<=4 && BY>=1, "BY is not in {1,2,3,4}");
    inline void update(int inp){ 
        uint32_t unp = (uint32_t) inp;
        // increase and cross 0xFFFFFFFF, overflow
        const int Low  = 0x40 * (1<<((BY-1)*8));
        const int High  = 0xC0 * (1<<((BY-1)*8));
        if (low >= High && unp < Low)
            high += 1;
        // decrease and cross 0x00000000
        if (unp >= High && low < Low)
            high -= 1;
        low = unp;
    }
    inline int64_t getValue()const{
        return low + ((int64_t) high) * (1<<(BY*8));
    }
    inline int32_t getLowValue()const{
        int64_t ret = low + ((int64_t) high) * (1<<(BY*8));
        return ret & 0xffffffff;
    }
    static inline int32_t getNewValue(int newVal, int oldVal, int& turns){
        uint32_t unp = (uint32_t) newVal;
        // increase and cross 0xFFFFFFFF, overflow
        const int Low  = 0x40 * (1<<((BY-1)*8));
        const int High  = 0xC0 * (1<<((BY-1)*8));
        if (oldVal >= High && unp < Low)
            turns += 1;
        // decrease and cross 0x00000000
        if (unp >= High && oldVal < Low)
            turns -= 1;
        int64_t ret = unp + ((int64_t) turns) * (1<<(BY*8));
        return ret & 0xffffffff;
    }
};


}

#endif