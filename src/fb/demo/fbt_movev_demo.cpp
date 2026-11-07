#include "fb_cpp_def.h"
#include "stdio.h"
#include "axis/AxisRunTmpl.hpp"

using namespace zmc;

class AxMoveVel:public AxDbFbTmpl<FbtPower, FbtMoveVelocity, 1>{
    public:
    double pos0 = -30.;
    double pos = 1.;

    AxMoveVel(){
        bindAxis(0);
    }

    void preRunCycle() override {
        if (idx == 3){
            mov.Enable = true;
            mov.bRegulatorOn = true;
            mov.bDriveStart = true;
        }else if (idx == 8){
            set_fb_vel_config(mov2, 50, 1000, 1000, 0);
            mov2.Execute = true;
        }else if (idx == 70) {
            mov2.Execute = false;
        }else if(idx == 71){
            mov2.Execute = true;
            set_fb_vel_config(mov2, 80, 2000, 1000, 0);
            mov2.Direction = mcNegativeDirection;
        }
        mov.runCycle();
        mov2.runCycle();
    }
};

void ax_movev_demo0(){
    AxMoveVel axm;
    axm.verbose_flag = 1;
    axm.count_to_exit = 200;
    axm.run();
}

void ax_movev_demo(){
    AxMoveVel axm;
    axm.axRef(0).fSWLimitPositive = 10;
    axm.axRef(0).fSWLimitNegative = -10;
    axm.axRef(0).fSWLimitDeceleration = 5000;
    axm.reload_config(0);
    axm.verbose_flag = 1;
    axm.count_to_exit = 500;
    axm.run();
}