#include "SplineCppWrap.h"
#include "stdio.h"
#include "common/misc/ZmcAllocator.hpp"
#include "spline_def.h"
#include "camGenHelper.hpp"

using namespace zmc;

#define LEN(arr) (sizeof(arr)/sizeof(arr[0]))
void PoliFitterWrap_demo(){
    // double tt[] = { 0, 2, 5, 8, 9};
    // double yy[] = { 0, 3, 5, 9, 10};
    // double vv[] = { 0, 3, 5, 1, 0};
    double tt[] = { 0, 0.1, 0.2, 0.3};
    double yy[] = { 0, 100, 50, 200};
    double vv[] = { 0, 250, 500, 0};
    PoliFitterWrap pfw;
    AllocatorArray<512> alloc;
    pfw.set_allocator(&alloc);
    pfw.set_ptype(SPLINE_CUBIC_FULL);
    // pfw.set_ptype(SPLINE_LINE_FULL);
    pfw.set_xx_yy_vv(LEN(tt), tt, yy, vv);
    int ret = pfw.build();
    double sc, vc, ac;
    printf("idx,time,pos,vel,acc\n");
    for (int i = 0; i < 38; i++) {
        double tm = i * 0.02;
        pfw.get_pos_vel_acc(tm, &sc,&vc,&ac);
        printf("%d,%f,%f,%f,%f\n", i, tm, sc, vc, ac);
    }
}
void PoliFitterWrap_pos_demo(){
    double tt[] = { 0, 0.1, 0.2, 0.3};
    double yy[] = { 0, 100, 50, 200};
    PoliFitterWrap pfw;
    AllocatorArray<512> alloc;
    pfw.set_allocator(&alloc);
    pfw.set_ptype(SPLINE_CUBIC_COND);
    pfw.set_condition_type(E_SPLINE_BEG_VEL_ACC_COND);
    double cond[2] = {0, 0};
    pfw.set_condition_value(cond, 2);
    pfw.set_xx_yy(LEN(tt), tt, yy);
    int ret = pfw.build();
    if (ret){
        return;
    }
    double sc, vc, ac;
    printf("idx,time,pos,vel,acc\n");
    for (int i = 0; i < 38; i++) {
        double tm = i * 0.01;
        pfw.get_pos_vel_acc(tm, &sc,&vc,&ac);
        printf("%d,%f,%f,%f,%f\n", i, tm, sc, vc, ac);
    }
}


void PoliFitterWrap_xyva_demo(){
    // double tt[] = { 0, 2, 5, 8, 9};
    // double yy[] = { 0, 3, 5, 9, 10};
    // double vv[] = { 0, 3, 5, 1, 0};
    // double aa[] = {0, 1,2, 1, 0};
    double tt[] = { 0, 45, 90, 135, 180, 225, 270, 315, 360};
    double yy[] = { 0, 70, 100, 70,  0,  -70, -100, -70, 0};
    double vv[] = { 100, 70, 0, -70,  -100,  -70, 0, 70, 100};
    double aa[] = { 0, 70, 100, 70,  0,  -70, -100, -70, 0};
    #define NUM (9)
    Xyva xyva[9];
    for (int i=0;i<NUM; i++){
        xyva[i] = Xyva{ tt[i], yy[i], vv[i], aa[i]};
    }
    fillXyvaToy(xyva, NUM);
    PoliFitterWrap pfw;
    AllocatorArray<512> alloc;
    pfw.set_allocator(&alloc);
    pfw.set_ptype(SPLINE_FIFTH_XYVA);
    pfw.set_xyva(NUM*4, (const double*) xyva);
    int ret = pfw.build();
    double sc, vc, ac;
    printf("idx,time,pos,vel,acc\n");
    for (int i = 0; i < 75; i++) {
        double tm = i * 5;
        pfw.get_pos_vel_acc(tm, &sc,&vc,&ac);
        printf("%d,%f,%f,%f,%f\n", i, tm, sc, vc, ac);
    }
}

