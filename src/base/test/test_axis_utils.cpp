#include <gtest/gtest.h>
// #include "common/misc/un_int64.hpp"
#include "stdint.h"
#include "axis_utils.h"

using namespace zmc;


// Test updateEncoderPos function - simple example
TEST(base, updateEncoder_simple){
    // 8-bit example: normal case
    int32_t val = 0x12345678;  // high=0x123456, low=0x78
    int32_t result = updateEncoderPos(8, val, 0x7A);
    EXPECT_EQ(result, 0x1234567A);
    
    // 32-bit example: direct return
    result = updateEncoderPos(32, 0x12345678, 0x9ABCDEF0);
    EXPECT_EQ(result, 0x9ABCDEF0);
}

// Test updateEncoderPos function - batch test with parameter arrays
TEST(base, updateEncoder_batch){
    // Test parameters: {BitWidth, inputVal, encVal, expectedResult}
    const uint32_t test_params[][4] = {
        // 8-bit tests
        {8, 0x12345678, 0x7A, 0x1234567A},              // Normal case
        {8, 0x123456C0, 0x10, 0x12345710},              // Positive overflow
        {8, 0x12345610, 0xC0, 0x123455C0},              // Negative overflow
        {8, 0x12345640, 0x41, 0x12345641},              // At Low threshold
        {8, 0x123456BF, 0xBE, 0x123456BE},              // Below High threshold
        {8, 0x00000000, 0x00, 0x00000000},              // Zero case
        
        // 12-bit tests  
        {12, 0x12345678, 0x67A, 0x1234567A},            // Normal case
        {12, 0x12345C00, 0x100, 0x12346100},            // Positive overflow
        {12, 0x12345100, 0xC00, 0x12344C00},            // Negative overflow
        {12, 0x12345400, 0x401, 0x12345401},            // At Low threshold
        
        // 16-bit tests
        {16, 0x12345678, 0x567A, 0x1234567A},           // Normal case
        {16, 0x1234C000, 0x1000, 0x12351000},           // Positive overflow
        {16, 0x12341000, 0xC000, 0x1233C000},           // Negative overflow
        {16, 0xf234C000, 0x1000, 0xf2351000},           // Positive overflow
        {16, 0xf2341000, 0xC000, 0xf233C000},           // Negative overflow
        {16, 0xfffffedc, 0x0012, 0x12},  
        {16, 0x12, 0xfedc, 0xfffffedc},   
        {16, 0x12344000, 0x4001, 0x12344001},           // At Low threshold
        
        // 24-bit tests
        {24, 0x12345678, 0x34567A, 0x1234567A},         // Normal case
        {24, 0x12C00000, 0x100000, 0x13100000},         // Positive overflow
        {24, 0x12100000, 0xC00000, 0x11C00000},         // Negative overflow
        {24, 0x12400000, 0x400001, 0x12400001},         // At Low threshold
        
        // 32-bit tests (direct return)
        {32, 0x12345678, 0x9ABCDEF0, 0x9ABCDEF0},       // Normal case
        {32, 0x12345678, 0x80000000, 0x80000000},       // Negative value
        {32, 0x12345678, 0xFFFFFFFF, 0xFFFFFFFF},       // Max value
        {32, 0x12345678, 0x00000000, 0x00000000},       // Zero
    };
    
    const int test_count = sizeof(test_params) / sizeof(test_params[0]);
    
    for (int i = 0; i < test_count; i++) {
        int BitWidth = test_params[i][0];
        int32_t inputVal = test_params[i][1];
        int32_t encVal = test_params[i][2];
        int32_t expected = test_params[i][3];
        
        int32_t result = updateEncoderPos(BitWidth, inputVal, encVal);
        EXPECT_EQ(result, expected) << "Test case " << i << " failed: "
                                   << "BitWidth=" << BitWidth 
                                   << ", inputVal=0x" << std::hex << inputVal
                                   << ", encVal=0x" << std::hex << encVal
                                   << ", expected=0x" << std::hex << expected
                                   << ", actual=0x" << std::hex << result;
    }
}


