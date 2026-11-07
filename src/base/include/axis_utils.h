#ifndef _ZMC_AXIS_UTILS_H_
#define _ZMC_AXIS_UTILS_H_
#include <stdint.h>

namespace zmc{

int32_t updateEncoderPos(int BitWidth, int32_t val, int encVal);
int32_t updateEncoder(int BitWidth, int32_t& val, int encVal);

}
#endif