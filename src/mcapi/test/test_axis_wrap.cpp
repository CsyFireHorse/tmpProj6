#include <gtest/gtest.h>
#include "math.h"
#include "stdio.h"
#include "axis_cia402_api.h"

TEST(AxisCia402Handle, run){
    axis_cia402_handle_t axWr = axis_cia402_new();
    for (int i=0; i<5; i++){
        axis_cia402_runcycle(axWr);
    }
    axis_cia402_delete(axWr);    
}

TEST(AxisCia402Handle, db_deinit){
    AxisCia402InpWrap axWr;
    AxisCia402InpRef ax = &axWr;
    axis_cia402_init(ax);
    for (int i=0; i<5; i++){
        axis_cia402_runcycle(ax);
    }
    axis_cia402_deinit(ax);  
    axis_cia402_deinit(ax);  
}
