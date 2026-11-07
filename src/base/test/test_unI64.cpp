#include <gtest/gtest.h>

#include "common/misc/un_int64.hpp"
#include "stdint.h"

using namespace zmc;

TEST(base, un_I64){
    const un_I64 a = { 0x9Abcdef12345678LL};
    EXPECT_EQ(a.high, 0x9Abcdef);
    EXPECT_EQ(a.low, 0x12345678);

}
TEST(base, un_I64_batch){

    const int64_t params_arr[][4] = {
        { 0, -30, -60, -90},
        { 0, 30, 60, 90},
        { 20, -30, -60, -90},
        { -20, 30, 60, 90},
        { 0xd0000000LL, 0xf0000000LL, 0x100000000LL, 0x110000000LL},
        { -0xd0000000LL, -0xf0000000LL, -0x100000000LL, -0x110000000LL},
        { 0x3d0000000LL, 0x3f0000000LL, 0x400000000LL, 0x410000000LL},
        { -0x3d0000000LL, -0x3f0000000LL, -0x400000000LL, -0x410000000LL},
    };

    for (int i=0; i<8; i++){
        const int64_t* params = params_arr[i];
        un_I64 un = {params[0]};
        EXPECT_EQ(un.val, params[0]);
        for (int j=1; j<4; j++){
            un.update(int64_to_32(params[j]));
            EXPECT_EQ(un.val, params[j]);
        }
    }

}
