#include "pou_memory_demo.hpp"

#include "AxPou.hpp"
#include "axPouTmplRunner.hpp"
#include "common/misc/CmdArgsToy.hpp"
#include "fb_cpp_def.h"
#include "fbt_axis_morph.h"
#include "fbt_impl_enum_def.h"
#include "memory/MemoryPouTypeLayout.hpp"
#include "pou_memory_task.h"

using namespace zmc;

namespace {

// Fields shared by the typed size check and the anonymous data block.
#define MEMORY_POU_DEMO_FIELDS \
    AXIS_REF axis;             \
    FbtPower power;            \
    FbtHalt halt;              \
    FbtMoveRelative moveRel;   \
    FbtMoveAdditive moveAdd;   \
    FbtMoveAbsolute move

static const char kMemoryPouDemoInitConfig[] = R"(
{
    AXIS_REF axis;
    FbtPower power;
    FbtHalt halt;
    FbtMoveRelative moveRel;
    FbtMoveAdditive moveAdd;
    FbtMoveAbsolute move;
};
)";

// Default script: immediate power/move params, delay 6 ticks, then execute move.
static const char kMemoryPouDemoDefaultScript[] =
    "power.Enable = true; "
    "power.bRegulatorOn = true; "
    "power.bDriveStart = true; "
    "move.Position = 15; "
    "move.Velocity = 20; "
    "move.Acceleration = 40; "
    "move.Deceleration = 40; "
    "delay 6; "
    "move.Execute = true; "
    "delay 150; "
    "out move.Done; "
    "expect move.Done = true";

class MemoryPouDemoTask : public MemoryPouTask {
   public:
    typedef struct PouData {
        MEMORY_POU_DEMO_FIELDS;
    } PouData;
    MemoryPouDemoTask() {
        axis = AxPouTaskBinderAuto::getAxisByIndex(0);

        errorCode_ = loadTypeLayout(kMemoryPouTypeLayout);
        if (errorCode_ != MEMOP_OK) {
            return;
        }
        errorCode_ = configureInit(kMemoryPouDemoInitConfig);
        if (errorCode_ != MEMOP_OK) {
            return;
        }

        if (memoryOperator_.rootSize() != sizeof(PouData)) {
            errorCode_ = MEMOP_ERR_LAYOUT;
            return;
        }
        bindPouMemory(&axis, sizeof(PouData));

        const CmdArgsToy& args = CmdArgsToySingleton::get();
        // Prefer CLI; fall back to static default (PouTmplDefaults c_str is ephemeral).
        const char* script = args.has_key("memory.script")
                                 ? args["memory.script"].get<const char*>(NULL)
                                 : kMemoryPouDemoDefaultScript;
        if (script == NULL) {
            errorCode_ = MEMOP_ERR_INIT;
            return;
        }
        errorCode_ = parseScheduledScript(script);

        power.Axis = axis;
        move.Axis = axis;
    }

   protected:
    void runCycle() override {
        cmdAdvance();
        power.runCycle();
        halt.runCycle();
        moveRel.runCycle();
        moveAdd.runCycle();
        move.runCycle();
    }

   private:
    MEMORY_POU_DEMO_FIELDS;
};

#undef MEMORY_POU_DEMO_FIELDS

}  // namespace

int pou_memory_demo(int argc, char** argv) {
    AxPouTaskBinderAuto* runner = make_pou_config<AxPouTaskBinderAuto>(
        argc, argv, iniContentDefault, PouTmplDefaults{{"verbose_flag", "1"}, {"cycles", "200"}});
    if (runner == NULL) {
        return -1;
    }

    MemoryPouDemoTask task;
    int rc = task.errorCode();
    if (rc == MEMOP_OK) {
        runner->bind(0, &task);
        runner->run();
        rc = task.errorCode();
    }

    delete runner;
    return rc;
}
