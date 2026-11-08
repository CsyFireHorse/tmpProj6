#include "cam_interactive.h"
#include "stdio.h"

#include "zmc_cam_table.h"
#include "camGenHelper.hpp"

#include "fb_cpp_def.h"

void cam_xyva_switch_segment_type_demo() {
    CamXyva lhs = {0.0, 0.0, 2.0, 0.0};  
    CamXyva rhs = {10.0, 5.0, 3.0, 0.0}; 

    printf("Before switch:\n"
           "Left segment V=%.1f A=%.1f\n"
           "Right segment V=%.1f A=%.1f\n", lhs.dV, lhs.dA, rhs.dV, rhs.dA);

    if (0 == ((CamXyvaMethod&)lhs).switch_segmentType(rhs, 1)) {
        printf("\nAfter switch:\n"
               "Left segment V=%.1f A=%.1f\n"
               "Right segment V=%.1f A=%.1f\n", lhs.dV, lhs.dA, rhs.dV, rhs.dA);
    }
}

void cam_xyva_max_PosVelAcc_demo() {
    // Create two adjacent cam segments.
    CamXyva lhs{270, 200, 0, 0};   
    CamXyva rhs{360, 0, 10, 0};  
    
    
    MaxPosVelAcc result;
    if (0 == ((CamXyvaMethod&)lhs).maxPosVelAcc(rhs, result)) {
        printf("minPos, maxPos: %.18f ~ %.18f\n", result.minPos, result.maxPos);
        printf("maxAbsVel: %.18f\n", result.maxAbsVel);
        printf("maxAbsAcc: %.18f\n", result.maxAbsAcc);
    }
}

void cam_xyva_calc_pva_demo() {
    // CamXyva lhs = {0.0, 2.0, -5.0, 3.0};   
    // CamXyva rhs = {10.0, 5.0, 8.0, -4.0};  
    CamXyva lhs{180, 360, 0, 0};   
    CamXyva rhs{360, 0, 2, 0};  
    #define N (101)
    CamPvaj points[N];
    double xArr[N];

    double dx = (rhs.dX - lhs.dX)/(N - 1);
    for (int i = 0; i < N; i++) {
        xArr[i] = lhs.dX + i*dx;
    }

    if (((CamXyvaMethod&)lhs).calc_PosVelAcc(rhs, xArr, points, N)) return;
    printf("x,y,v,a,j\n");
    for (int i = 0; i < N; i++) {
        printf("%f,%.5f,%.5f,%.5f,%.5f\n", xArr[i], points[i].pos, points[i].vel, points[i].acc, points[i].jerk);
    }
    
}
