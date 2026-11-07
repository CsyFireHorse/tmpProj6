#ifndef _VEL_TEST_DATA_HPP_
#define _VEL_TEST_DATA_HPP_


#include "vel_test_helper.hpp"

namespace zmc{


namespace my_namespace {
    // global constexpr variant
    constexpr int MAX_SIZE = 100;
    constexpr double PI = 3.141592653589793;
    constexpr const char* VERSION = "1.0.0";
    
    // static constexpr
    class MyClass {
        static constexpr int BUFFER_SIZE = 256;
    };
}

namespace test_vel{
    constexpr double T=0;
    constexpr VelSgParams2 trapz_param[] = {
        {0, 10, 0, 0, 3, 5, 5, 0, T, LADDER_30_SEGMENT},   // 30
        {0, 10, 1, 2, 3, 5, 5, 0, T, LADDER_30_SEGMENT},   // 30
        {0, 10, 1, 2, 3, 5, 4, 0, T, LADDER_30_SEGMENT},   // 30
        {5, 15, 1, 2, 3, 5, 5, 0, T, LADDER_30_SEGMENT},   // 30
        {0, 10, 4, 2, 3, 5, 5, 0, T, LADDER_31_SEGMENT},   // 31
        {0, 10, 4, 2, 3, 6, 5, 0, T, LADDER_31_SEGMENT},   // 31
        {0, 10, 2, 4, 3, 5, 5, 0, T, LADDER_32_SEGMENT},   // 31
        {0, 10, 2, 4, 3, 6, 5, 0, T, LADDER_32_SEGMENT},   // 31
        {0, 10, 4, 2, 2, 5, 5, 0, T, LADDER_24_SEGMENT},   // 24
        {0, 10, 2, 4, 2, 5, 5, 0, T, LADDER_25_SEGMENT},   // 25
        {0, 1, 0, 0, 3, 5, 5, 0, T, LADDER_20_SEGMENT},    // 20
        {0, 1, 1, 0, 3, 5, 5, 0, T, LADDER_20_SEGMENT},    // 20
        {0, 5, 1, 3, 3, 5, 5, 0, T, LADDER_21_SEGMENT},    // 21
        {0, 5, 3, 1, 3, 5, 5, 0, T, LADDER_22_SEGMENT},    // 22
        {0, 5, 3, 3, 3, 5, 5, 0, T, LADDER_10_SEGMENT},    // 10
        {0, 0, 0, 0, 3, 5, 5, 0, T, LADDER_20_SEGMENT},    // 20
        {0, 0.9, 0, 3, 4, 5, 4, 0, T, LADDER_11_SEGMENT},  // 11
        {0, 0.5, 2, 0, 3, 6, 4, 0, T, LADDER_12_SEGMENT},  // 12

        // {0, 10, -10, 2, 1, 5, 10, 0, T, LADDER_31_SEGMENT},    // 31
        // {0, 10, 2, -10, 1, 5, 10, 0, T, LADDER_32_SEGMENT},    // 32
        // {0, 10, 10, -2, 1, 5, 10, 0, T, LADDER_31_SEGMENT},    // 31
        // {0, 10, -2, 10, 1, 5, 10, 0, T, LADDER_32_SEGMENT},    // 32

        // {100000, 1, 2, 3, 5, 5, 0, T, LADDER_30_SEGMENT}, // 30
        // {100, 1, 2, 300, 50000, 30000, 0, T, LADDER_30_SEGMENT}, // 30
        // {1000, 10000, 9000, 11000, 100, 30000, 0, T, LADDER_20_SEGMENT}, // 30
    };
}
}
#endif