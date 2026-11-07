#include "AxisGroupMotbufRobot.h"
#include <vector>
#include "MotCmd.hpp"
#include "fb_cpp_def.h"
#include "fbmEnum.h"
#include "AxPou.hpp"

namespace zmc {


int AxisGroupMotbufRobot::pushCommand(const CommandStructDefault& cmd) {
    return 0;
}

void AxisGroupMotbufRobot::runCycleTop() {
}

void AxisGroupMotbufRobot::runCycleBottom() {

}

void AxisGroupMotbufRobot::reset_runtime() {
    // The robot motbuf currently owns no runtime state.
}


}  // namespace zmc
