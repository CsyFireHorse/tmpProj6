#include "cam_interactive_api.h"
#include "stdio.h"

void switch_segment_type_demo() {
    CamXyva lhs{0.0, 0.0, 2.0, 0.0}; 
    CamXyva rhs{10.0, 5.0, 3.0, 0.0};

    printf("切换前：\n左段V=%.1f A=%.1f\n右段V=%.1f A=%.1f\n", lhs.dV, lhs.dA, rhs.dV, rhs.dA);

    if (0 == switch_poly5_segmentType(&lhs, &rhs, 1)) {
        printf("\n切换后：\n左段V=%.1f A=%.1f\n右段V=%.1f A=%.1f\n", lhs.dV, lhs.dA, rhs.dV,
               rhs.dA);
    }
}

void calc_poly_max_PosVelAcc_demo() {
    CamXyva lhs{0.0, 2.0, -5.0, 3.0};  
    CamXyva rhs{10.0, 5.0, 8.0, -4.0}; 

    MaxPosVelAcc result;
    if (0 == calc_poly5_maxPosVelAcc(&lhs, &rhs, &result)) {
        printf("minPos, maxPos: %.4f ~ %.4f\n", result.minPos, result.maxPos);
        printf("maxAbsVel: %.4f\n", result.maxAbsVel);
        printf("maxAbsAcc: %.4f\n", result.maxAbsAcc);
    }
}

void cam_calc_poly5_pva() {
    CamXyva lhs{0.0, 2.0, -5.0, 3.0};  
    CamXyva rhs{10.0, 5.0, 8.0, -4.0}; 
    #define N (101)
    CamPvaj points[N];
    double xArr[N];

    for (int i = 0; i < N; i++) {
        xArr[i] = i*0.1;
    }

    if (calc_poly5_PosVelAcc(&lhs, &rhs, xArr, points, N)) return;
    printf("x,y,v,a,j\n");
    for (int i = 0; i < N; i++) {
        printf("%f,%.5f,%.5f,%.5f,%.5f\n", xArr[i], points[i].pos, points[i].vel, points[i].acc, points[i].jerk);
    }
    
}