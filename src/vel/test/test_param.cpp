#if 0
#include <gtest/gtest.h>
#include "VelToyTrapz.h"
# include "../source/vel_par_trapz.h"
#include "vel_forward_trapz.h"
#include "stdio.h"
#include "math.h"

using namespace zmc;


TEST(vel_param, trapz){
    TrapzParams vt;
    vt.set_config(10, 3, 4, 0);
    vt.set_begin_end_pos_vel(0, 20, 1, 0);
    vt.init();

    double vm = 5.0;
    double sc = vt.nseg20_vm2s(vm);
    double vmr = vt.nseg20_s2vm(sc);
    EXPECT_NEAR(vmr, vm, 1e-6);
}
TEST(vel_param, trapz_t23){
    TrapzParams vt;
    vt.set_config(10, 3, 4, 0);
    vt.set_begin_end_pos_vel(0, 20, 3, 2);
    vt.init();
    double vm = 1.0;
    double sc = vt.nseg23_vm2s(vm);
    double vmr = vt.nseg23_s2vm(sc);
    EXPECT_NEAR(vmr, vm, 1e-6);

    static const double params[][4] = { {20, 3, 2, -1 },  {20, 3, 1, -2 },  {20, 3, -2, -1 },  {20, -1, 3,  -2 }
    ,  {20, -3, 3,  -2 },  {20, -2, 3,  -2.5}};
    for (int i=0; i<6; i++){
        const auto& par = params[i];
        vt.set_begin_end_pos_vel(0, par[0], par[1], par[2]);
        double vm = par[3];
        double sc = vt.nseg23_vm2s(vm);
        double vmr = vt.nseg23_s2vm(sc);
        EXPECT_NEAR(-vmr, vm, 1e-6);
    }
}


TEST(vel_param, trapz_time){
    TrapzParams vt;
    vt.set_config(10, 3, 3.2, 0);
    vt.set_begin_end_pos_vel(0, 20, 1, 0);
    vt.init();

    double vm = 5.0;
    double tc = vt.nseg30_vm2t(vm);
    double vmr = vt.nseg30_t2vm(tc);
    EXPECT_NEAR(vmr, vm, 1e-6);

    vt.set_begin_end_pos_vel(0, 20, 4, 2);
    double vm2 = 3.0;
    double tc2 = vt.nseg31_vm2t(vm2);
    double vmr2 = vt.nseg31_t2vm(tc2);
    EXPECT_NEAR(vmr2, vm2, 1e-6);
    {
    vt.set_begin_end_pos_vel(0, 20, 2, 4);
    double vm2 = 3.0;
    double tc2 = vt.nseg32_vm2t(vm2);
    double vmr2 = vt.nseg32_t2vm(tc2);
    EXPECT_NEAR(vmr2, vm2, 1e-6);
    }
    {
    vt.set_begin_end_pos_vel(0, 20, 4, 3);
    double vm2 = 2.0;
    double tc2 = vt.nseg34_vm2t(vm2);
    double vmr2 = vt.nseg34_t2vm(tc2);
    EXPECT_NEAR(vmr2, vm2, 1e-6);
    }
}
#endif