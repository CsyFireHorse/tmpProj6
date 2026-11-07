#ifndef _FBT_AXIS_MORPH_H_
#define _FBT_AXIS_MORPH_H_

#include "fbt_axis_cia402.h"
//

namespace zmc {

// Variable axis supporting free encoder, based on AxisCia402Fak
class AxisMorph : public AxisCia402 {
   public:
    AxisMorph();
    void set_encoder_pos(int diEncoderPosition);
    void runCycle();
};

}  // namespace zmc

#endif