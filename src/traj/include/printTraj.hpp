#ifndef _ZMC_PRINT_TRAJ_HPP_
#define _ZMC_PRINT_TRAJ_HPP_
#include "math.h"
#include "stdio.h"

namespace zmc {
#define MAX_AXIS_NUM 32

template <typename T>
void printTrajRow(T& vt, double v = 0.5) {
    double dst = vt.get_plan();
    int axis_num = vt.get_axis_num();
    printf("idx,time,axId,pos,vel,acc\n");

    double posArr[MAX_AXIS_NUM], velArr[MAX_AXIS_NUM], accArr[MAX_AXIS_NUM];
    double tEnd = dst / v;
    double dt = 1;
    int N = ceil(tEnd / dt) + 1;
    double acc = 0;
    for (int i = 0; i < N; i++) {
        double tm = i * dt;
        double s = v * tm;
        if (s > dst) v = 0;
        vt.get_slice3(s, v, acc, posArr, velArr, accArr);
        printf("%d,%f,%d,%f,%f,%f\n", i, tm, 32, s, v, acc);
        for (int j = 0; j < axis_num; j++) {
            printf("%d,%f,%d,%f,%f,%f\n", i, tm, j, posArr[j], velArr[j], accArr);
        }
    }
}

template <typename T>
void printTraj1d(T& vt, double v = 0.5) {
    double dst = vt.get_plan();
    int axis_num = vt.get_axis_num();
    if (axis_num != 1) return;
    printf("idx,time,s,v,pos0,vel0\n");
    double posArr[1], velArr[1];
    double tEnd = dst / v;
    double dt = 1;
    int N = ceil(tEnd / dt) + 1;
    for (int i = 0; i < N; i++) {
        double tm = i * dt;
        double s = v * tm;
        if (s > dst) v = 0;
        vt.get_slice2(s, v, posArr, velArr);
        printf("%d,%f,%f,%f,%f,%f\n", i, tm, s, v, posArr[0], velArr[0]);
    }
}

template <typename T>
void printTraj2d(T& vt, double v = 0.5) {
    double dst = vt.get_plan();
    int axis_num = vt.get_axis_num();
    if (axis_num != 2) return;
    printf("idx,time,s,v,pos0,pos1,vel0,vel1\n");
    double posArr[2], velArr[2];
    double tEnd = dst / v;
    double dt = 1;
    int N = ceil(tEnd / dt) + 3;
    for (int i = 0; i < N; i++) {
        double tm = i * dt;
        double s = v * tm;
        if (s > dst) v = 0;
        vt.get_slice2(s, v, posArr, velArr);
        printf("%d,%f,%f,%f,%f,%f,%f,%f\n", i, tm, s, v, posArr[0], posArr[1], velArr[0],
               velArr[1]);
    }
}
template <typename T>
void printTraj3d(T& vt, double v = 0.5) {
    double dst = vt.get_plan();
    int axis_num = vt.get_axis_num();
    if (axis_num != 3) return;
    printf("idx,time,s,v,pos0,pos1,pos2,vel0,vel1,vel2\n");
    double posArr[3], velArr[3];
    double tEnd = dst / v;
    double dt = 1;
    int N = ceil(tEnd / dt) + 3;
    for (int i = 0; i < N; i++) {
        double tm = i * dt;
        double s = v * tm;
        if (s > dst) v = 0;
        vt.get_slice3(s, v, 0, posArr, velArr, nullptr);
        printf("%d,%f,%f,%f,%f,%f,%f,%f,%f,%f\n", i, tm, s, v, posArr[0], posArr[1], posArr[2],
               velArr[0], velArr[1], velArr[2]);
    }
}

template <typename T>
void printTraj(T& vt, double v = 0.5) {
    double dst = vt.get_plan();
    int axis_num = vt.get_axis_num();
    if (axis_num == 2)
        return printTraj2d(vt, v);
    else if (axis_num == 1)
        return printTraj1d(vt, v);
    else
        return printTraj3d(vt, v);
}

}  // namespace zmc
#endif