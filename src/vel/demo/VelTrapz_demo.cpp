#include "TrapzVel.h"
#include "stdio.h"


using namespace zmc;
void _velTrapz_demo(double S_0 = 0, double S_d=10){
    TrapzVel vt;
    vt.set_config(10, 5, 4, 0);
    vt.set_begin_pva(S_0, 1);
    vt.set_end_pva(S_d, 0);
    if (vt.build()){
        printf("build failed\n");
        return;
    }
    double planTime;
    planTime = vt.get_end_time();
    printf("idx,time,pos,vel,acc\n");
    PosVelAcc pva;
    for (int i=0; i<50; i++){
        double tm = i*0.1;
        vt.get_slice(tm, pva);
        printf("%d,%f,%f,%f,%f\n", i, tm, pva.pos, pva.vel, pva.acc);
    }
}

void velTrapz_demo(){
    return _velTrapz_demo();
}
void velTrapz_invert_demo(){
    return _velTrapz_demo(-4, -10);
}

