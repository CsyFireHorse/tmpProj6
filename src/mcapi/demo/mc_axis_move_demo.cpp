#include "mc_fb_api.h"
#include "axis_cia402_api.h"
#include "stdio.h"
#include "McAxis.hpp"
#include "AxisRunTmpl.hpp"

using namespace zmc;

using McAxisRunTmpl = AxisRunTmpl<McAxis>;

class AxMoveApiDemo2 : public McAxisRunTmpl {
public:
    McPower fbPow;
    McMoveAbsolute fbMov;

    AxMoveApiDemo2() {
        // Initialize axis
        auto& cfg = axArr[0].get_config_ref();
        cfg.debug.servo_fak_pos = 1;
        cfg.cycle_time = 50000;
        cfg.is_axis_virtual = true;
        axArr[0].reload_config();

        this->count_to_exit = 70;
        MC_Power_create(&fbPow);
        MC_MoveAbsolute_create(&fbMov);
    }

    ~AxMoveApiDemo2() {
        MC_Power_destroy(&fbPow);
        MC_MoveAbsolute_destroy(&fbMov);
    }

    void preRunCycle() {
        auto* axRef = this->axisRef(0);
        if (idx == 2) {
            fbPow.Axis = axRef;
            fbPow.Enable = true;
            fbPow.bRegulatorOn = true;
            fbPow.bDriveStart = true;
        } else if (idx == 8) {
            fbMov.Axis = axRef;
            fbMov.Velocity = 20;
            fbMov.Acceleration = 40;
            fbMov.Deceleration = 40;
            fbMov.Execute = true;
        }
        MC_Power_exec(&fbPow);
        MC_MoveAbsolute_exec(&fbMov);
    }
};

void mc_api_axis_move_demo() {
    AxMoveApiDemo2 axm;
    axm.fbMov.Position = 10;
    axm.verbose_flag = true;
    axm.run();
}
