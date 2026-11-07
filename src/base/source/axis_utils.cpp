#include "axis_utils.h"

namespace zmc{
    
int32_t updateEncoderPos(int BitWidth, int32_t val, int encVal){
    if (BitWidth == 32 || BitWidth == 0){
        return encVal;
    }
    // BitWidth only in {8,12,16,24,28}
    // Extract low and high parts from val
    uint32_t mask = (1U << BitWidth) - 1;
    uint32_t oldLow = val & mask;
    int32_t high = val >> BitWidth;
    
    uint32_t newLow = ((uint32_t)encVal) & mask;
    
    // Detection thresholds based on getNewValue logic
    // Use 3/4 and 1/4 of the range for overflow detection
    const uint32_t Low = 1U << (BitWidth - 2);      // 1/4 of range
    const uint32_t High = 3U * (1U << (BitWidth - 2)); // 3/4 of range
    
    // Check for overflow (increase and cross max value)
    if (oldLow >= High && newLow < Low)
        high += 1;
    // Check for underflow (decrease and cross 0)
    if (newLow >= High && oldLow < Low)
        high -= 1;
    
    // Return new 32-bit value: high bits + low bits
    return (((uint32_t)high) << BitWidth) | (newLow & mask);
}

int32_t updateEncoder(int BitWidth, int32_t& val, int encVal){
    val = updateEncoderPos(BitWidth, val, encVal);
    return val;
}

}