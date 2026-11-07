#include <utility>
#include <vector>

#include "AxisToy.hpp"
#include "TrapzVel.h"
#include "VelGearFollower.h"
#include "VelPosFollower.h"
#include "common/misc/TrapzVelToyForwarder.hpp"
#include "stdio.h"
using namespace zmc;

using VelToyPosGear = VelToyGearTmpl<VelPosFollower>;
using VelMove = VelToyMoveTmpl<TrapzVel>;

void velToyTrapz_demo() {
    double S_0 = 10;
    double S_d = 25;

    AxisToy ax;
    ax.set_pos_vel_acc(S_0, 0, 0);
    VelMove vMove;
    vMove.Axis = &ax;
    vMove.set_config(10, 5, 4, 0);
    vMove.set_end_pva(S_d, 0);

    ax.printHeader(1);
    for (int i = 0; i < 50; i++) {
        if (i == 2) vMove.Execute = true;
        vMove.runCycle();
        ax.runCycle();
        ax.print(1);
    }
}

void VelToyPosFollower_demo() {
    // const double Ts = 0.1;
    AxisToy axArr[2];
    axArr[0].pos = 5;

    VelToyMoveTmpl<ToyTrapzVelPlanner> vMove;
    vMove.Axis = &axArr[0];
    vMove.set_config(5, 3, 3, 0);
    vMove.set_end_pva(20, 0);

    VelToyPosGear vGear;
    vGear.Master = &axArr[0];
    vGear.Slave = &axArr[1];
    vGear.set_config(6, 4, 4, 0);

    axArr[0].printHeader(1);
    for (int i = 0; i < 82; i++) {
        if (i == 2) {
            vMove.Execute = true;
            vGear.Execute = true;
        }
        vMove.runCycle();
        vGear.runCycle();
        axArr[0].runCycle();
        axArr[1].runCycle();
        axArr[0].print(1);
        axArr[1].print(1);
    }
}
