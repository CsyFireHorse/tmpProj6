#if 0
# include "../source/vel_par_trapz.h"

#include "vel_forward_trapz.h"
#include "stdio.h"
#include "assert.h"
using namespace zmc;
void velParam_demo(){
    TrapzParams vt;
    vt.set_config(10, 3, 3, 0);
    vt.set_begin_end_pos_vel(0, 20, 1, 0);
    vt.init();

    double vm = 5.0;
    double tc = vt.nseg30_vm2t(vm);
    double vmr = vt.nseg30_t2vm(tc);
    printf("%f,%f\n", tc, vmr);
    assert(abs(vmr - vm)<= 1e-6);

    vt.set_begin_end_pos_vel(0, 20, 4, 3);
    double vm2 = 2.0;
    double tc2 = vt.nseg34_vm2t(vm2);
    double vmr2 = vt.nseg34_t2vm(tc2);
    printf("%f,%f\n", tc2, vmr2);
    assert(abs(vmr2 - vm2)<= 1e-6);
}
#endif