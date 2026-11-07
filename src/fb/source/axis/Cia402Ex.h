#ifndef _ZMC_CIA402EX_H_
#define _ZMC_CIA402EX_H_

#include "common/ecat/Cia402Easy.hpp"
#include "stdint.h"

namespace zmc{
union Cia402SmEx{
    struct  {
        uint8_t sm : 4;
        uint8_t hsm : 4;
    };
    uint8_t val;
    inline bool is_home_error() const { return hsm & 4; }
    inline bool is_home_done() const { return hsm & 2; }
    inline bool is_halt_done() const { return hsm & 1; }
};

inline int getHomeHaltStatus(uint16_t statWord) {
    return (Cia402Easy::is_home_error(statWord) << 2) |
        (Cia402Easy::is_home_done(statWord) << 1) | Cia402Easy::is_halt_done(statWord);
}

inline uint8_t statusWord2SwSmEx(uint16_t wStatusWord){
    auto sm = Cia402Easy::get_sm(wStatusWord);
    Cia402SmEx smEx = {sm, getHomeHaltStatus(wStatusWord)};
    return smEx.val;
}

}
#endif