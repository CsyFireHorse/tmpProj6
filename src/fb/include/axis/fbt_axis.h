#ifndef _ZMC_FBT_AXIS_H_
#define _ZMC_FBT_AXIS_H_

#include "fbt_axis_cia402.h"
#include "fbt_axis_virtual.h"
#include "fbt_axis_encoder.h"

class AxisPulse:public AxisBase{};
class AxisDrivePosControl:public AxisBase{};

#endif