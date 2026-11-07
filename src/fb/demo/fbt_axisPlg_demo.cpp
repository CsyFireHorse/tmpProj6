#include "fbt_axis_cia402.h"
#include "fbt_axis_impl.h"
#include "stdio.h"
#include "fb_cpp_def.h"

using namespace zmc;
typedef AxisCia402 Axis;

void fbt_axisPlg_demo() {
    Axis axis;
    AxisCiaConfig cfg;
    AxisImplBase& ax = *get_axis_impl(&axis);
    AxisCiaConfig_default_init(&cfg);
    cfg.is_axis_virtual = true;
    cfg.debug.servo_fak_pos = 100;
    axis.set_config(&cfg);
    printAxisHeader(3);
    for (int i = 0; i < 10; i++) {
        axis.set_input(nullptr);
        axis.runCycle();
        // axis.get_output(nullptr);
        axis.print(3);
    }
}

void fbt_axisPlg_move_demo() {
    Axis axis;
    AxisCiaConfig cfg;
    AxisImplBase& ax = *get_axis_impl(&axis);
    AxisCiaConfig_default_init(&cfg);
    cfg.is_axis_virtual = true;
    axis.set_config(&cfg);

    FbtPower power;
    power.Axis = &axis;

    FbtMoveAbsolute mov;
    mov.Axis = &axis;
    mov.Position = 50;
    mov.Velocity = 100;
    mov.Acceleration = 300;
    mov.Deceleration = 300;
    mov.Jerk = 100;

    printAxisHeader(3);
    for (int i = 0; i < 80; i++) {
        if (i==1){
            power.Enable = true;
            power.bRegulatorOn = true;
            power.bDriveStart = true;
        }else if (i==8)
            mov.Execute = true;
        power.runCycle();
        mov.runCycle();

        axis.set_input(nullptr);
        axis.runCycle();
        // axis.get_output(nullptr);
        axis.print(3);
    }
}