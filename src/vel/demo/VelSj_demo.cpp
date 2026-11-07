#include "stdio.h"
#include "velocity_utils.hpp"
#include "VelUnCppWrap.h"

using namespace zmc;

struct VelParam2{
    double S_0;
    double S_d;
    double V_0;
    double V_d;
    double A_0;
    double A_d;
    double V_m;
    double A_m;
    double A_n;
    double J;
};


void velSj_demo(){
    VelParam2 param = {
        0, 10, 0, 1, 0, 0, 8, 5, 5, 3
    };
    build_print_velocity<VelocityWrap>(param);
}

void VelocityWrap_demo(){
    VelParam2 param = {
        0, 10, 0, 1, 0, 0, 
        8, 5, 5, 0
    };
    build_print_velocity<VelocityWrap>(param);
}


void VelocityWrap2_demo(){
    VelParam2 param = {
        0, 10, 0, 1, 0, 0, 
        8, 5, 5, 1
    };
    build_print_velocity<VelocityWrap>(param);
}