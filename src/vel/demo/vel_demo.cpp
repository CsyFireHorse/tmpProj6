#include <iostream>
#include "vel_sin_square.hpp"
#include "stdio.h"

using namespace zmc;
void vel_sin_demo() {
    VelSinSquarePlanner planner;
    planner.set_config(5, M_PI, M_PI * 5 / 4, 0);
    planner.set_begin_pva(0, 1, 0);
    planner.set_end_pva(26, 0, 0);

    // planner.build_by_phasing(1.0, 0.0, 5.0, 2.0, 3.0, 2.0);
    planner.build();

    printf("index,time,pos,vel,acc\n");
    for (int i=0; i<400; i++){
        double t = i *0.02;
        auto state = planner.get_slice(t);
        std::printf("%d,%-4.2f,%-7.5f,%-5.4f,%-6.4f\n", i,
                   t, state.pos, state.vel, state.acc);
    }
}