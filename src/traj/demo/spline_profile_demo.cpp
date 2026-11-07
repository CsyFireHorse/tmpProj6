#include "poly_spline.h"
#include "stdio.h"
using namespace zmc;


void LinearSplineProfile_demo(){
    int len = 5;
    double tt[] = { 0, 2, 5, 8, 10};
    double yy[] = { 0, 3, 7, 9, 10};
    LinearSplineProfile csp(len, tt, yy);
    csp.build();
    double pos, vel, acc;
    printf("idx,time,pos,vel,acc\n");
    for (int i=0; i<20; i++){
        double tm = i * 0.5;
        csp.get_pos_vel_acc(tm, &pos, &vel, &acc);
        printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    }
}

void CubicSplineProfile_demo(){

    int len = 4;
    double tt[] = { 0, 2, 5, 8};
    double yy[] = { 0, 3, 6, 9};
    double vv[] = { 0, 3, 5, 1};
    CubicSplineProfile csp(len, tt, yy, vv);
    csp.build();
    double pos, vel, acc;
    for (int i=0; i<20; i++){
        double tm = i * 0.5;
        csp.get_pos_vel_acc(tm, &pos, &vel, &acc);
        printf("%d,%f,%f,%f\n", i, tm, pos, vel);
    }
}

void CubicRecursionProfile_demo(){
    int len = 5;
    double tt[] = { 0, 2, 5, 8, 9};
    double yy[] = { 0, 3, 7, 9, 10};
    double vv[] = { 0, 3};
    CubicRecursionProfile csp(len, tt, yy, vv);
    csp.build();
    double pos, vel, acc;
    for (int i=0; i<1005; i++){
        double tm = i * 0.01;
        csp.get_pos_vel_acc(tm, &pos, &vel, &acc);
        printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    }
}

void FifthSplineProfile_demo(){
    double tt[] = { 0, 2, 5, 8, 9};
    double yy[] = { 0, 3, 6, 9, 11};
    double vv[] = { 0, 3, 5, 1, 0};
    double aa[] = { 0, 1, 2, 3, -1};
    int len = 5;
    FifthOrderSplineProfile csp(len, tt, yy, vv, aa);
    csp.build();
    double pos, vel, acc;
    for (int i=0; i<24; i++){
        double tm = i * 0.5;
        csp.get_pos_vel_acc(tm, &pos, &vel, &acc);
        printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    }
}