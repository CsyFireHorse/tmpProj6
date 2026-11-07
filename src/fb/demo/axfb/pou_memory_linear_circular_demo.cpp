#include "pou_memory_demo.hpp"

#include "AxPou.hpp"
#include "axPouTmplRunner.hpp"
#include "axgrp/AxisGrpZmot.h"
#include "axgrp/AxisGrpZmotRef.h"
#include "axgrp/FbtGroupAdmin.h"
#include "axgrp/FbtMoveCircular.h"
#include "axgrp/FbtMoveLinear.h"
#include "common/misc/CmdArgsToy.hpp"
#include "fb_cpp_def.h"
#include "memory/MemoryPouTypeLayout.hpp"
#include "pou_memory_task.h"

using namespace zmc;

namespace {

// Fields shared by the typed size check and the runtime POU data block.
#define MEMORY_POU_LINEAR_CIRCULAR_DEMO_FIELDS \
    AXIS_REF axis0_;                           \
    AXIS_REF axis1_;                           \
    AXIS_REF axis2_;                           \
    AxisGrpZmotRef groupRef_;                  \
    FbtPower power0;                           \
    FbtPower power1;                           \
    FbtPower power2;                           \
    FbtAddAxisToGroup add0;                    \
    FbtAddAxisToGroup add1;                    \
    FbtAddAxisToGroup add2;                    \
    FbtGroupEnable enable;                     \
    FbtMoveLinear linear;                      \
    FbtMoveCircular circular

static const char kMemoryPouLinearCircularIni[] = R"(
[base]
axis_num = 3
[axis_0]
cycle_time = 10000
is_axis_virtual = true
[axis_1]
cycle_time = 10000
is_axis_virtual = true
id = 1
[axis_2]
cycle_time = 10000
is_axis_virtual = true
id = 2
)";

// Default script: power axes, configure the group, then issue a line and a half circle.
static const char kMemoryPouLinearCircularDefaultScript[] =
    "power0.Enable = true; "
    "power0.bRegulatorOn = true; "
    "power0.bDriveStart = true; "
    "power1.Enable = true; "
    "power1.bRegulatorOn = true; "
    "power1.bDriveStart = true; "
    "power2.Enable = true; "
    "power2.bRegulatorOn = true; "
    "power2.bDriveStart = true; "
    "delay 6; "
    "add0.Execute = true; "
    "add1.Execute = true; "
    "add2.Execute = true; "
    "delay 2; "
    "enable.Execute = true; "
    "delay 3; "
    "linear.MotionType = 0; "
    "linear.Position.a0 = 1; "
    "linear.Position.a1 = 0; "
    "linear.Position.a2 = 0; "
    "linear.Velocity = 20; "
    "linear.Acceleration = 40; "
    "linear.Deceleration = 40; "
    "linear.Execute = true; "
    "delay 60; "
    "circular.MotionType = 0; "
    "circular.AuxPoint.a0 = 0; "
    "circular.AuxPoint.a1 = 1; "
    "circular.AuxPoint.a2 = 0; "
    "circular.EndPosition.a0 = -1; "
    "circular.EndPosition.a1 = 0; "
    "circular.EndPosition.a2 = 0; "
    "circular.CircMode = 0; "
    "circular.Velocity = 20; "
    "circular.Acceleration = 40; "
    "circular.Deceleration = 40; "
    "circular.Execute = true";

class MemoryPouLinearCircularTask : public MemoryPouTask {
   public:
    typedef struct PouData {
        MEMORY_POU_LINEAR_CIRCULAR_DEMO_FIELDS;
    } PouData;

    MemoryPouLinearCircularTask() {
        axis0_ = AxPouTaskBinderAuto::getAxisByIndex(0);
        axis1_ = AxPouTaskBinderAuto::getAxisByIndex(1);
        axis2_ = AxPouTaskBinderAuto::getAxisByIndex(2);
        groupRef_ = &group_;

        errorCode_ = loadTypeLayout(kMemoryPouTypeLayout);
        if (errorCode_ != MEMOP_OK) {
            return;
        }
        errorCode_ = configureInit(kMemoryPouLinearCircularInitConfig);
        if (errorCode_ != MEMOP_OK) {
            return;
        }
        if (memoryOperator_.rootSize() != sizeof(PouData)) {
            errorCode_ = MEMOP_ERR_LAYOUT;
            return;
        }
        bindPouMemory(&axis0_, sizeof(PouData));

        bindStaticReferences();

        const CmdArgsToy& args = CmdArgsToySingleton::get();
        // Prefer CLI; fall back to static default (PouTmplDefaults c_str is ephemeral).
        const char* script = args.has_key("memory.script")
                                 ? args["memory.script"].get<const char*>(NULL)
                                 : kMemoryPouLinearCircularDefaultScript;
        if (script == NULL) {
            errorCode_ = MEMOP_ERR_INIT;
            return;
        }
        errorCode_ = parseScheduledScript(script);
    }

   protected:
    void runCycle() override {
        cmdAdvance();
        power0.runCycle();
        power1.runCycle();
        power2.runCycle();
        add0.runCycle();
        add1.runCycle();
        add2.runCycle();
        enable.runCycle();
        linear.runCycle();
        circular.runCycle();
        group_.runCycle();
    }

   private:
    void bindStaticReferences() {
        power0.Axis = axis0_;
        power1.Axis = axis1_;
        power2.Axis = axis2_;

        add0.AxisGroup = groupRef_;
        add1.AxisGroup = groupRef_;
        add2.AxisGroup = groupRef_;
        add0.Axis = axis0_;
        add1.Axis = axis1_;
        add2.Axis = axis2_;

        enable.AxisGroup = groupRef_;
        linear.AxisGroup = groupRef_;
        circular.AxisGroup = groupRef_;
    }

    AxisGrpZmot group_;
    // struct {
    MEMORY_POU_LINEAR_CIRCULAR_DEMO_FIELDS;
    // };

    static const char kMemoryPouLinearCircularInitConfig[];
};

const char MemoryPouLinearCircularTask::kMemoryPouLinearCircularInitConfig[] = R"(
{
    AXIS_REF axis0_;
    AXIS_REF axis1_;
    AXIS_REF axis2_;
    AxisGrpZmotRef groupRef_;
    FbtPower power0;
    FbtPower power1;
    FbtPower power2;
    FbtAddAxisToGroup add0;
    FbtAddAxisToGroup add1;
    FbtAddAxisToGroup add2;
    FbtGroupEnable enable;
    FbtMoveLinear linear;
    FbtMoveCircular circular;
};
)";

#undef MEMORY_POU_LINEAR_CIRCULAR_DEMO_FIELDS

}  // namespace

int pou_memory_linear_circular_demo(int argc, char** argv) {
    AxPouTaskBinderAuto* runner = make_pou_config<AxPouTaskBinderAuto>(
        argc, argv, kMemoryPouLinearCircularIni,
        PouTmplDefaults{{"verbose_flag", "1"}, {"cycles", "200"}});
    if (runner == NULL) {
        return -1;
    }

    MemoryPouLinearCircularTask task;
    int rc = task.errorCode();
    if (rc == MEMOP_OK) {
        runner->bind(0, &task);
        runner->run();
        rc = task.errorCode();
    }

    delete runner;
    return rc;
}
