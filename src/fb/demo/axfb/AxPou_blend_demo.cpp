#include "AxPou.hpp"
#include "fb_cpp_def.h"
// #include "common/get_file_path.hpp"
#include <iostream>
#include "fbt_axis_morph.h"
// #include "fbt_impl_enum_def.h"
#include "mc_enum_def.h"
#include "axPouTmplRunner.hpp"
#include "AxVerboseFlag.h"

using namespace zmc;

class BlendingMoveTask : public PouTaskBase {
public:
    BlendingMoveTask() {
        auto& args = CmdArgsToySingleton::get();
        axis = AxPouTaskBinderAuto::getAxisByIndex(0);

        move.Position = args["move.Position"].get<double>(10.0);
        move.Velocity = args["move.Velocity"].get<double>(1.0);
        move.Acceleration = args["move.Acceleration"].get<double>(1.0);
        move.Deceleration = args["move.Deceleration"].get<double>(1.0);
        move.Jerk = 0.0;
        move.Direction = MC_DIRECTION::mcPositiveDirection;
        move.BufferMode = mcAborting;
        
        move2.Position = args["move2.Position"].get<double>(20.0);
        move2.Velocity = args["move2.Velocity"].get<double>(5.0); // 150
        move2.Acceleration = args["move2.Acceleration"].get<double>(1.0);
        move2.Deceleration = args["move2.Deceleration"].get<double>(1.0);
        move2.Jerk = 0.0;
        move2.Direction = MC_DIRECTION::mcPositiveDirection;
        move2.BufferMode = static_cast<MC_BUFFER_MODE>(args["move2.BufferMode"].get<int>(mcBlendingHigh));

        delay_idx = args["delay_idx"].get<int>(11);

        axis->bind_extra_data(&extraData);
    }
    int getIndex() const {
        return _index;
    }
    void runCycle() override {

        // Power control
        power.Axis = axis;
        power.Enable = true;
        power.bDriveStart = true;
        power.bRegulatorOn = true;
        power.runCycle();

        move.Axis = axis;
        move.Execute = power.Status && (getIndex() >= 10);
        move.runCycle();

        move2.Axis = axis;
        move2.Execute = power.Status && (getIndex() >= delay_idx + 10);
        move2.runCycle();

        extraData = move.Busy + move.Active + move.Done*3 + move.Error*4 + move.CommandAborted*5 +
            16*(move2.Busy + move2.Active + move2.Done*3 + move2.Error*4 + move2.CommandAborted*5);
        _index++;
    }

private:
    AXIS_REF axis;
    FbtPower power;
    FbtMoveAbsolute move;
    FbtMoveAbsolute move2;
    // Configurable parameters
    int delay_idx = 0;
    int _index = 0;
    // int _verboseFlag = 9;
    int extraData = 0;
};
  

int blending_demo(int argc, char** argv) {
    return pou_auto_tmpl_demo<BlendingMoveTask>(argc, argv, iniContentDefault, PouTmplDefaults{{"verbose_flag", "9"}});
}

#define LEN(x) sizeof(x)/sizeof(x[0])
void blending_demo_low_speed() {
    char* argv[] = {"", "--move2.Velocity", "150.0", "--move2.BufferMode", "5"};

    blending_demo(LEN(argv), argv);
}

void blending_demo_high_speed() {
    char* argv[] = {"", "--move2.Velocity", "500.0"};
    blending_demo(LEN(argv), argv);
}
void blending_dynamic_demo() {
    char* argv[] = {"", "blending", "--cycles", "2500", "--delay_idx", "500", "--move2.Velocity", "3", 
    "--move2.Acceleration", "3", "--move2.Deceleration", "3", 
    "--base.axis_num", "1",
    "--axis_0.is_axis_virtual", "1",
    "--axis_0.cycle_time", "5000",
    "--axis_0.increments_units_in_application_num", "51200",
    "--axis_0.increments_units_in_application_den", "1"};

    blending_demo(LEN(argv), argv);
}