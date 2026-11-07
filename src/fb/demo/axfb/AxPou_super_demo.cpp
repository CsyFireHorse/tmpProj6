#include "AxPou.hpp"
#include "fb_cpp_def.h"
#include "common/get_file_path.hpp"
#include <iostream>
#include "common/misc/CmdArgsToy.hpp"
#include "fbt_axis_morph.h"
#include "fbt_impl_enum_def.h"
#include "axPouTmplRunner.hpp"

using namespace zmc;

class MoveSuperImposedTask : public PouTaskBase {
public:
    MoveSuperImposedTask() {
        auto& args = CmdArgsToySingleton::get();
        axis = AxPouTaskBinderAuto::getAxisByIndex(0);

        set_fb_vel_config(moveAbs, 
            args["moveAbs.Velocity"].get<double>(20.0), 
            args["moveAbs.Acceleration"].get<double>(50.0), 
            args["moveAbs.Deceleration"].get<double>(50.0), 
            args["moveAbs.Jerk"].get<double>(0.0));
        moveAbs.Position = args["moveAbs.Position"].get<double>(100.0);

        set_fb_acc_config(moveSuper, 
            args["moveSuper.Acceleration"].get<double>(30.0), 
            args["moveSuper.Deceleration"].get<double>(30.0), 
            args["moveSuper.Jerk"].get<double>(0.0));
        moveSuper.VelocityDiff = args["moveSuper.VelocityDiff"].get<double>(10.0);
        moveSuper.Distance = args["moveSuper.Distance"].get<double>(50.0);
    }
    int getIndex() const {
        return cnt;
    }
    void runCycle() override {
        power.Axis = axis;
        power.Enable = true;
        power.bDriveStart = true;
        power.bRegulatorOn = true;
        power.runCycle();

        moveAbs.Axis = axis;
        moveAbs.Execute = power.Status;
        moveAbs.runCycle();

        moveSuper.Axis = axis;
        moveSuper.Execute = power.Status && getIndex()>10;
        moveSuper.runCycle();

        // axis->print(1);
        cnt++;
    }
private:
    AXIS_REF axis;
    FbtPower power;
    FbtMoveAbsolute moveAbs;
    FbtMoveSuperImposed moveSuper;
    int cnt = 0;
};

int move_superimposed_demo(int argc, char** argv) {
    return pou_auto_tmpl_demo<MoveSuperImposedTask>(argc, argv, iniContentDefault, PouTmplDefaults{{"verbose_flag", "1"}});
}

void move_superimposed_demo0() {
    std::string iniContent = R"(
[base]
axis_num = 1
[axis_0]
cycle_time = 50000
is_axis_virtual = true
)";
    
    AxPouTaskBinder pou("", iniContent);
    MoveSuperImposedTask task;
    pou.bind(0, &task);
    pou.count_to_exit = 500;
    pou.run();
}
