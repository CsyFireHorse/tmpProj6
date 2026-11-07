#include "AxisJsonLoader.h"
#include "common/get_file_path.hpp"
using namespace zmc;

int axis_load_json_demo(){
    AxisCiaConfig cfg;
    auto eni_file = _get_file_path("mhelper/data/motor_cia402_0.json");
    int ret = load_axis_json(eni_file.c_str(), cfg);
    // cfg.id;
    return ret;
}