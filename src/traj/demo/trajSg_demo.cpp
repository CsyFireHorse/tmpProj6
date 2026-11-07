#include "TrajCppWrap.h"
#include "printTraj.hpp"
#include "stdio.h"

using namespace zmc;
void TrajSg_demo() {
    TrajWrap vt;
    vt.set_sg_param(5, true);
    const double begPos = 3;
    vt.set_begin_pos(&begPos);
    vt.build();
    double planTime = vt.get_plan();
    printf("idx,distance,pos,dif\n");
    double pos, vel;
    for (int i = 0; i < 30; i++) {
        double tm = i * 0.2;
        vt.get_slice(tm, &pos, &vel);
        printf("%d,%f,%f,%f\n", i, tm, pos, vel);
    }
}

void TrajLinear_demo() {
    TrajWrap vt;

    const double beginPos[] = {2, 1};
    vt.init_axis_num(2);
    vt.set_begin_pos(beginPos);
    double endPosArr[] = {5, 12};

    vt.set_line_param(endPosArr, true);  // relative
    vt.build();
    printTrajRow(vt, 0.5);
}

void TrajCircBorder2D_demo() {
    TrajWrap vt;

    vt.init_axis_num(2);

    const double beginPos[] = {1, -1};
    vt.set_begin_pos(beginPos);

    const double auxPoint[] = {-0.5, 1.732 / 2.0};
    const double endPoint[] = {-1, 1};

    vt.set_circ_param(auxPoint, endPoint, false, false, CIRC_MODE_BORDER);
    const int ret = vt.build();
    if (ret != 0) {
        printf("TrajCircBorder2D_demo build failed: ret=%d\n", ret);
        return;
    }
    printTraj(vt, 0.1);
}
