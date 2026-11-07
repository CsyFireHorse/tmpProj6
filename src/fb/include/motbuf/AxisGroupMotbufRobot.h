#ifndef ZMC_AXIS_GROUP_MOTBUF_ROBOT_H
#define ZMC_AXIS_GROUP_MOTBUF_ROBOT_H

#include <vector>

#include "AxisMotbufBase.h"
#include "MotCmd.hpp"
#include "fbmEnum.h"

namespace zmc {


// Multi-axis group buffer: manages multiple groups, each with own queue and axis params
class AxisGroupMotbufRobot : public AxisMotbufBase {
   public:
    int pushCommand(const CommandStructDefault& cmd);
    void runCycleTop();
    void runCycleBottom();
    void reset_runtime();
};

}  // namespace zmc

#endif
