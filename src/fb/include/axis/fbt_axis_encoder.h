#ifndef _FBT_AXIS_ENCODER_H_
#define _FBT_AXIS_ENCODER_H_

// #include "pdo_struct.h"
#include "AxisRef.h"
#include "fbt_axis_opaque.h"
#include "AxisCiaConfig.hpp"

namespace zmc{

class AxisEncoder:public AxisBase{  
public:
    AxisEncoder();
    ~AxisEncoder();
    void set_encoder_pos(int diEncoderPosition);
    void runCycle();
};

}
#endif