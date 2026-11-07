#include "AxPou.hpp"
#include "axPouTmplRunner.hpp"
#include "pou/AxisManager.h"
#include "fb_cpp_def.h"
#include "common/misc/CmdArgsToy.hpp"
#include "fbt_axis_base.h"
#include "fbt_axis_opaque.h"
#include <iostream>

using namespace zmc;

class HomingTask : public PouTaskBase {
public:
    HomingTask() {
        axis = AxPouTaskBinderAuto::getAxisByIndex(0);
        power.Axis = axis;
        homing.Axis = axis;

        auto& args = CmdArgsToySingleton::get();
        // Parameters (defaults, can be overridden via CLI)
        homing.fHomePosition = args["homing.fHomePosition"].get<double>(100.0);
        homing.fVelocityFast = args["homing.fVelocityFast"].get<double>(100.0);
        homing.fVelocitySlow = args["homing.fVelocitySlow"].get<double>(20.0);
        homing.fAcceleration = args["homing.fAcceleration"].get<double>(500.0);
        homing.fDeceleration = args["homing.fDeceleration"].get<double>(500.0);
        homing.fJerk = args["homing.fJerk"].get<double>(0.0);
        homing.nDirection = (MC_DIRECTION) args["homing.nDirection"].get<int>((int)mcNegativeDirection);
        homing.nHomingMode = FAST_BSLOW_S_STOP; // default mode
        homing.bReturnToZero = args["homing.bReturnToZero"].get<bool>(false);
        homing.fSignalDelay = args["homing.fSignalDelay"].get<double>(0.0);
        homing.nHomingMode = (MC_HOMING_MODE) args["homing.nHomingMode"].get<int>(0);

        // Reference switch window around 0.0 (simulate a cam/limit switch)
        refCenter = args["refCenter"].get<double>(10.0);
        refWidth  = args["refWidth"].get<double>(0.5); // +/- window
    }

    void runCycle() override {
        // Power on
        power.Enable = true;
        power.bDriveStart = true;
        power.bRegulatorOn = true;
        power.runCycle();
        auto ax = (AxisImplBase*) axis;
        // Simulate reference switch based on actual position window
        double pos = ax->fActPosition;
        bool inWindow = (pos >= (refCenter - refWidth)) && (pos <= (refCenter + refWidth));
        homing.bReferenceSwitch = pos > refCenter; // inWindow;

        // Execute homing when power is on
        homing.bExecute = power.Status;
        homing.runCycle();

        // axis->print(1);
    }

    AXIS_REF axis;
    FbtPower power;
    FbtHoming homing;
    double refCenter = 120.0;
    double refWidth = 0.5;
};

int pou_homing_demo(int argc, char** argv) {
    return pou_auto_tmpl_demo<HomingTask>(argc, argv, iniContentDefault, PouTmplDefaults{{"verbose_flag", "1"}});
}


