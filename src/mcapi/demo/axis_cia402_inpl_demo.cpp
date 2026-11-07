#include "axis_cia402_api.h"


void axis_cia402_inpl_demo(){
    AxisCia402InpWrap axWr;
    AxisCia402InpRef ax = &axWr;
    axis_cia402_init(ax);
    for (int i=0; i<5; i++){
        axis_cia402_runcycle(ax);
    }
    axis_cia402_deinit(ax);   
}