#if 0
#include "vel_forward_trapz.h"
#include "vel_forward_trapz_rev.h"

using namespace zmc;
void VelForwarderTrapzT30_demo() {
    VelParam par;
    par.set_config(5, 3, 4, 0);
    par.set_begin_end_pos_vel(-1, -21, -1, -2);
    VelForwarderTrapzT30 vft;
    vft.pPar = &par;
    vft.nseg = LADDER_30_SEGMENT;
    vft.vm = -5;
    vft.am = -3;
    vft.an = -4;
    vft._buildTimeSegment();
    vft._cumsum();
    // vft.get_plan(3);
    double tm = vft.get_end_time();
    double ds = vft.get_distance(3);
}

void FuncVelTrapzRev_demo0() {
    VelTrapzRev vft;
    vft.set_config(5, 3, 4, 0);
    vft.set_begin_end_pos_vel(-1, -21, -1, -2);
    int ret = vft.build();
    ret == 0;
    double tm;
    vft.get_plan(&tm, nullptr, nullptr, nullptr);
    // double tm = vft.get_end_time();
}

void FuncVelTrapzTime_demo() {
    VelTrapzRev vft;
    vft.set_config(15, 3, 3, 0);
    vft.set_begin_end_pos_vel(7.746481481481481, 12, 5.433333333333333, 2);
    vft.param.set_total_time(1.094444444444444 + 0.05);
    int ret = vft.build();
    double tm;
    vft.get_plan(&tm, nullptr, nullptr, nullptr);
    // double tm = vft.get_end_time();
}
void FuncVelTrapzRev_demo() {
    VelTrapzRev vft;
    vft.set_config(15, 3, 3, 0);
    vft.set_begin_end_pos_vel(8.214721759259263, 12, 5.168333333333331, 2);
    vft.param.set_total_time(1.0061111111111108 + 0.05);
    int ret = vft.build();
    double tm;
    vft.get_plan(&tm, nullptr, nullptr, nullptr);
    // double tm = vft.get_end_time();
}
#endif