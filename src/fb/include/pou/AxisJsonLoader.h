#ifndef _AXIS_JSON_LOADER_H_
#define _AXIS_JSON_LOADER_H_

#include "AxisCiaConfig.hpp"

namespace zmc {

int load_axis_json(const char* jsonFile, AxisCiaConfig& cfg);

}  // namespace zmc
#endif