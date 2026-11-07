
#include "AxPou.hpp"
#include "axPouTmplRunner.hpp"
#include "pou/AxisManager.h"
#include "fb_cpp_def.h"
#include <iostream>

#include "fbt_axis_morph.h"
#include "fbt_impl_enum_def.h"

using namespace zmc;

/**
 * @brief Simple task that uses AxisManager to get axes
 */
class AutoTask : public PouTaskBase {
public:
    AutoTask() {
        auto& args = CmdArgsToySingleton::get();
        axis = AxPouTaskBinderAuto::getAxisByIndex(0);

        power.Axis = axis;
        move.Axis = axis;
        move.Position = args["move.Position"].get<double>(100);
        move.Velocity = args["move.Velocity"].get<double>(10);
        move.Acceleration = args["move.Acceleration"].get<double>(20);
        move.Deceleration = args["move.Deceleration"].get<double>(20);
        move.Jerk = args["move.Jerk"].get<double>(0);
    }
    
    void runCycle() override {
        // if (!axis) return;
        power.Enable = true;
        power.bDriveStart = true;
        power.bRegulatorOn = true;
        power.runCycle();
        // Use the axis for movement
        move.Execute = power.Status;
        move.runCycle();
        
        // axis->print(1);
    }
    
    AXIS_REF axis;
    FbtPower power;
    FbtMoveAbsolute move;
};



int pou_auto_demo(int argc, char** argv) {
    const std::string iniContentDefault = R"(
[base]
axis_num = 1
[axis_0]
cycle_time = 10000
is_axis_virtual = true
)";
    return pou_auto_tmpl_demo<AutoTask>(argc, argv, iniContentDefault, PouTmplDefaults{{"verbose_flag", "1"}});
}