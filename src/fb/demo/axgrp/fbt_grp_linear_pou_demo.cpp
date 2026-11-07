#include "AxPou.hpp"
#include "axPouTmplRunner.hpp"
#include "axgrp/AxisGrpZmot.h"
#include "axgrp/FbtGroupAdmin.h"
#include "axgrp/FbtMoveLinear.h"
#include "fb_cpp_def.h"


using namespace zmc;

namespace {

const std::string kGrpLinearIni = R"(
[base]
axis_num = 2
[axis_0]
cycle_time = 10000
is_axis_virtual = true
[axis_1]
cycle_time = 10000
is_axis_virtual = true
id = 1
)";

class GrpMoveLinearTask : public PouTaskBase {
   public:
    GrpMoveLinearTask() {
        auto& args = CmdArgsToySingleton::get();
        axis0_ = AxPouTaskBinderAuto::getAxisByIndex(0);
        axis1_ = AxPouTaskBinderAuto::getAxisByIndex(1);

        power0_.Axis = axis0_;
        power1_.Axis = axis1_;

        addGrp0_.AxisGroup = &group_;
        addGrp1_.AxisGroup = &group_;
        addGrp0_.Axis = axis0_;
        addGrp1_.Axis = axis1_;

        enGrp_.AxisGroup = &group_;

        move_.AxisGroup = &group_;
        move_.MotionType = args["move.MotionType"].get<int>(0);
        move_.Position.v[0] = args["move.Position0"].get<double>(3.0);
        move_.Position.v[1] = args["move.Position1"].get<double>(4.0);
        move_.Velocity = args["move.Velocity"].get<double>(80.0);
        move_.Acceleration = args["move.Acceleration"].get<double>(400.0);
        move_.Deceleration = args["move.Deceleration"].get<double>(400.0);
        move_.Jerk = args["move.Jerk"].get<double>(0.0);
    }

    void runCycle() override {
        power0_.Enable = true;
        power0_.bDriveStart = true;
        power0_.bRegulatorOn = true;
        power0_.runCycle();

        power1_.Enable = true;
        power1_.bDriveStart = true;
        power1_.bRegulatorOn = true;
        power1_.runCycle();

        bool isPowered = power0_.Status && power1_.Status;

        if (cnt_ == 1) {
            addGrp0_.Execute = true;
            addGrp1_.Execute = true;
        }

        enGrp_.Execute = (cnt_ > 2) && isPowered;
        move_.Execute = (cnt_ > 10) && isPowered;

        // re execute after idle
        if (cnt_ == 62) {
            move_.Execute = false;
            move_.MotionType = 1;
        } else if (cnt_ > 62) {
            move_.Execute = true;
        }

        addGrp0_.runCycle();
        addGrp1_.runCycle();
        enGrp_.runCycle();
        move_.runCycle();
        group_.runCycle();
        cnt_++;
    }

   private:
    AXIS_REF axis0_ = nullptr;
    AXIS_REF axis1_ = nullptr;
    AxisGrpZmot group_;
    FbtPower power0_;
    FbtPower power1_;
    FbtAddAxisToGroup addGrp0_;
    FbtAddAxisToGroup addGrp1_;
    FbtGroupEnable enGrp_;
    FbtMoveLinear move_;
    int cnt_ = 0;
};

}  // namespace

int pou_grp_linear_demo(int argc, char** argv) {
    return pou_auto_tmpl_demo<GrpMoveLinearTask>(argc, argv, kGrpLinearIni,
                                                 PouTmplDefaults{{"verbose_flag", "1"}});
}
