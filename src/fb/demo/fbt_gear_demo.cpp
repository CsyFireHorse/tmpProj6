#include "AxPou.hpp"
#include "axPouTmplRunner.hpp"
#include "pou/AxisManager.h"
#include "fb_cpp_def.h"
#include "stdio.h"
#include "fbt_axis_impl.h"
#include "fbt_axis_base.h"
#include "fbt_axis.h"
#include "common/misc/CmdArgsToy.hpp"

using namespace zmc;

class GearTask : public PouTaskBase {
public:
    GearTask() {
        auto& args = CmdArgsToySingleton::get();
        master_axis = AxPouTaskBinderAuto::getAxisByIndex(0);
        slave_axis = AxPouTaskBinderAuto::getAxisByIndex(1);
        
        move.Axis = master_axis;
        move.Distance = args["move.Distance"].get<double>(300);
        move.Velocity = args["move.Velocity"].get<double>(100);
        move.Acceleration = args["move.Acceleration"].get<double>(200);
        move.Deceleration = args["move.Deceleration"].get<double>(200);
        move.Jerk = args["move.Jerk"].get<double>(0);
        
        // Configure gear
        gear.Slave = slave_axis;
        gear.Master = master_axis;
        gear.RatioNumerator = args["gear.RatioNumerator"].get<int>(1);
        gear.RatioDenominator = args["gear.RatioDenominator"].get<int>(1);
        gear.Acceleration = args["gear.Acceleration"].get<double>(200);
        gear.Deceleration = args["gear.Deceleration"].get<double>(200);
        
        idx_gearOut_exec = args["idx_gearOut_exec"].get<int>(100);
        // Power control for both axes
        power_master.Axis = master_axis;
        power_slave.Axis = slave_axis;
        gear_out.Slave = slave_axis;
    }
    
    void runCycle() override {
        // Power control for master axis
        power_master.Enable = true;
        power_master.bDriveStart = true;
        power_master.bRegulatorOn = true;
        power_master.runCycle();
        
        // Power control for slave axis
        power_slave.Enable = true;
        power_slave.bDriveStart = true;
        power_slave.bRegulatorOn = true;
        power_slave.runCycle();
        
        // Control movement sequence

        move.Execute = power_master.Status && idx >= 7;
        gear.Execute = power_slave.Status && idx >= 8;

        move.runCycle();
        gear.runCycle();
        gear_out.Execute = idx >= idx_gearOut_exec;
        gear_out.runCycle();

        // master_axis->print(1);
        // slave_axis->print(1);
        
        idx++;
    }
    
private:
    AXIS_REF master_axis;
    AXIS_REF slave_axis;
    FbtPower power_master;
    FbtPower power_slave;
    FbtMoveRelative move;
    FbtGearIn gear;
    FbtGearOut gear_out;
    int idx = 0;
    int idx_gearOut_exec = 0;
};


int ax_gear_demo(int argc, char** argv) {
    const std::string iniContentDefault = R"(
[base]
axis_num = 2
[axis_0]
cycle_time = 20000
is_axis_virtual = true
id = 0
[axis_1]
cycle_time = 20000
is_axis_virtual = true
id = 1
)";
    return pou_auto_tmpl_demo<GearTask>(argc, argv, iniContentDefault, PouTmplDefaults{{"verbose_flag", "1"}});
}
