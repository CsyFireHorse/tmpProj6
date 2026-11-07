#include "fb_cpp_def.h"
#include "fbt_axis_cia402.h"

using namespace zmc;
// typedef AxisCia402 Axis;

#define LEN(x) (sizeof(x) / sizeof(*x))

void fbt_follow_full_demo() {
    AxisCia402 axis;
    axis.get_config_ref().cycle_time = 100000;
    axis.get_config_ref().is_axis_virtual = true; // only use for simulation
    axis.reload_config();
    AXIS_REF pAx = &axis;
    // AxisImpl& ax =(AxisImpl&) axis;
    FbtPower power;
    FbtFollowPosition follow;
    FbtHalt halt;

    const double posArr[] = {0.000000,  0.200000,  0.800000,  1.800000,  3.200000,  5.000000,
                             7.200000,  9.800000,  12.800000, 16.200000, 20.000000, 24.000000,
                             28.000000, 32.000000, 36.000000, 40.000000, 44.000000, 48.000000,
                             52.000000, 56.000000, 60.000000, 64.000000, 68.000000, 72.000000,
                             76.000000, 80.000000, 83.800000, 87.200000, 90.200000, 92.800000,
                             95.000000, 96.800000, 98.200000, 99.200000, 99.800000, 100.00000};

    int j = 0;

    for (int i=0; i<120; i++){
        power.Axis = pAx;
        follow.Axis = pAx;
        halt.Axis = pAx;
        halt.Deceleration = 5000.0;

        if (i == 1) {
            power.Enable = true;
            power.bRegulatorOn = true;
            power.bDriveStart = true;
        } else if (i >= 10 && power.Status) {
            follow.bExecute = true;
            if (j < LEN(posArr)) follow.fSetPosition = posArr[j];
            j++;
        }

        if (i == 100) {
            halt.Execute = true;
        }
        if (follow.bCommandAborted) {
            // printf(" follow stopped\n");
        }
        power.runCycle();
        follow.runCycle();
        halt.runCycle();
        axis.runCycle();
    }
}
