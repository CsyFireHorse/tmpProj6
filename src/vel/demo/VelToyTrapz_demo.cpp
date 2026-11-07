#if 0
#include "VelToyTrapz.h"
#include "stdio.h"


using namespace zmc;
void velToyTrapz_demo(){
    VelToyTrapz vt;
    vt.set_config(10, 5, 5, 0);
    vt.set_begin_end_pos_vel(0, 10, 1, 0);
    vt.build();
    double planTime, vMax;
    vt.get_plan(&planTime, &vMax, nullptr, nullptr);
    printf("%f,%f\n", planTime, vMax);
    double pos, vel, acc;
    for (int i=0; i<10; i++){
        double tm = i*0.5;
        vt.get_slice(tm, &pos, &vel, &acc);
        printf("%d,%f,%f,%f,%f\n", i, tm, pos, vel, acc);
    }
}
#endif