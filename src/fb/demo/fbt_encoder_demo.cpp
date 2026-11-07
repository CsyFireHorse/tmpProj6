#include "AxPou.hpp"
#include "axPouTmplRunner.hpp"
#include "pou/AxisManager.h"
#include "fbt_axis_encoder.h"
#include "fbt_axis_virtual.h"
#include "fb_cpp_def.h"
#include <iostream>
#include <unordered_map>
#include <string>
#include <cstring>
#include "common/misc/CmdArgsToy.hpp"

using namespace zmc;

class EncoderTask : public PouTaskBase {
public:
    EncoderTask() {
        auto& args = CmdArgsToySingleton::get();
        encoder_axis = AxPouTaskBinderAuto::getAxisByIndex(0);
        slave_axis = AxPouTaskBinderAuto::getAxisByIndex(1);
        
        power.Axis = slave_axis;
        // Configure gear
        gear.Master = encoder_axis;
        gear.Slave = slave_axis;
        gear.RatioNumerator = args["ratio_num"].get<int>(1);
        gear.RatioDenominator = args["ratio_den"].get<int>(1);
        gear.Acceleration = args["acceleration"].get<double>(100);
        gear.Deceleration = args["deceleration"].get<double>(100);
        
        // Store configuration parameters
        bit_width = args["bit_width"].get<int>(8);
        steps = args["steps"].get<int>(5);
    }
    
    void runCycle() override {
        // Generate encoder position based on cycle count
        int val = (cycle_count * steps);
        if (bit_width == 8) {
            val &= 0xff;
        } else if (bit_width == 12) {
            val &= 0xfff;
        } else if (bit_width == 16) {
            val &= 0xffff;
        } else if (bit_width == 24) {
            val &= 0xffffff;
        }
        power.Enable = true;
        power.bRegulatorOn = true;
        power.bDriveStart = true;
        power.runCycle();
        // Set encoder position
        ((AxisEncoder*)encoder_axis)->set_encoder_pos(val);
        
        // Control gear execution
        gear.Execute = (cycle_count >= 5) && power.Status;
        
        // Run cycles
        gear.runCycle();        
        cycle_count++;
    }
    
private:
    AXIS_REF encoder_axis;
    AXIS_REF slave_axis;
    FbtGearIn gear;
    FbtPower power;
    int bit_width;
    int steps;
    int cycle_count = 0;
};

int encoder_gear_demo(int argc, char** argv) {
    const std::string iniContentDefault = R"(
[base]
axis_num = 2
[axis_0]
cycle_time = 10000
# is_axis_virtual = true
id = 0
bit_width = 8
debug.eAxisMtype = AXIS_TYPE_ENCODER
[axis_1]
cycle_time = 10000
is_axis_virtual = true
id = 1
)";
    return pou_auto_tmpl_demo<EncoderTask>(argc, argv, iniContentDefault, PouTmplDefaults{{"verbose_flag", "1"}});
}
