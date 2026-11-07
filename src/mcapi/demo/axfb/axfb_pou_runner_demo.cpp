#include "axfb_pou_runner_api.h"
// #include "AxPou.hpp"
#include "common/misc/pou_base.hpp"
#include "fb_cpp_def.h"
#include <iostream>
#include <stdexcept>

using namespace zmc;

namespace {

const char* kIniContent = R"(
[base]
axis_num = 1
[axis_0]
cycle_time = 10000
is_axis_virtual = true
)";

class SimpleTask : public PouTaskBase {
public:
    SimpleTask() {
        axis = static_cast<AXIS_REF>(axfb_pou_runner_get_axis_by_index(0));
        if (!axis) {
            throw std::runtime_error("No axis available");
        }

        power.Axis = axis;
        move.Axis = axis;
        move.Position = 100.0;
        move.Velocity = 10.0;
        move.Acceleration = 20.0;
        move.Deceleration = 20.0;
        move.Jerk = 0.0;
    }

    void runCycle() override {
        power.Enable = true;
        power.bDriveStart = true;
        power.bRegulatorOn = true;
        power.runCycle();

        move.Execute = power.Status;
        move.runCycle();
    }

private:
    AXIS_REF axis = nullptr;
    FbtPower power;
    FbtMoveAbsolute move;
};

}  // namespace

int axfb_pou_runner_demo(int argc, char** argv) {
    auto runner = axfb_pou_runner_create("", kIniContent);
    if (!runner) {
        std::cerr << "Failed to create AxPouTaskBinderAuto runner" << std::endl;
        return -1;
    }

    axfb_pou_runner_set_cycles(runner, 200);
    axfb_pou_runner_set_verbose_flag(runner, 1);
    axfb_pou_runner_set_cycle_time(runner, 10000);

    SimpleTask task;
    axfb_pou_runner_bind_task(runner, 0, &task);
    axfb_pou_runner_run(runner);
    axfb_pou_runner_destroy(runner);

    return 0;
}

