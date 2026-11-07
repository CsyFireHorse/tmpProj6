#if 0
/*
 * @Author: ChenShengyong
 * @Date: 2024-07-03 20:23:11
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-08-28 09:12:00
 */
#include "VelStopper.h"
#include "stdio.h"

using namespace zmc;
void velDeceler_demo() {
    VelDeceler vt;
    vt.set_config(10, 5, 5, 0);
    vt.set_begin_end_pos_vel(3, 0, 10, 0);
    vt.build();
    double planTime, vMax;
    vt.get_plan(&planTime, &vMax, nullptr, nullptr);
    printf("idx,time,pos,vel,acc\n");
    double pos, vel, acc;
    for (int i = 0; i < 20; i++) {
        double tm = i * 0.2;
        vt.get_slice(tm, &pos, &vel, &acc);
        printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    }
}

// (V_0, V_d)
// (2, 5)
// (5, 2)
// (-2, -5)
// (-5, -2)
// (-5, 2)
// (-2, 5)
// (2, -5)
// (5, -2)
// (4, -4)
void velAcceler_demo() {
    VelAcceler vt;
    vt.set_config(10, 5, 4, 0);
    vt.set_begin_end_pos_vel(3, 0, 2, -4);
    vt.build();
    double planTime, vMax;
    vt.get_plan(&planTime, &vMax, nullptr, nullptr);
    printf("idx,time,pos,vel,acc\n");
    double pos, vel, acc;
    for (int i = 0; i < 20; i++) {
        double tm = i * 0.2;
        vt.get_slice(tm, &pos, &vel, &acc);
        printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    }
}

void velAcceler_rev_demo() {
    VelAcceler vt;
    vt.set_config(10, 5, 4, 0);
    vt.set_begin_end_pos_vel(3, 0, 2, -4);
    vt.build();
    double planTime, vMax;
    vt.get_plan(&planTime, &vMax, nullptr, nullptr);
    printf("idx,time,pos,vel,acc\n");
    double pos, vel, acc;
    double tmOfs = 0;
    for (int i = 0; i < 35; i++) {
        double tm = i * 0.2;
        if (i == 10) {
            vt.chop_left_time(tm - tmOfs);
            vt.set_param(VEL_INNER_END_VEL, 3);
            vt.build();
            tmOfs += tm - tmOfs;
        } else if (i == 23) {
            vt.chop_left_time(tm - tmOfs);
            vt.set_param(VEL_INNER_END_VEL, 1);
            vt.set_param(VEL_INNER_MAX_DEC, 1);
            vt.build();
            tmOfs += tm - tmOfs;
        }

        vt.get_slice(tm - tmOfs, &pos, &vel, &acc);
        printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    }
}
#endif