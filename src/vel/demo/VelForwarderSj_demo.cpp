#if 0
#include "vel_forward_sj2.h"
#include "stdio.h"

using namespace zmc;
void VelForwarderSjT44_demo() {
    VelParam par;
    par.set_config(5, 2, 2, 2);
    par.set_begin_pva(-0.75, 1, 0);
    par.set_end_pva(0.75, 1, 0);
    VelForwarderS44 vft;  // VelForwarderS44
    vft.pPar = &par;
    vft.nseg = SLADDER_44_SEGMENT;
    for (int i = 0; i < 70; i++) {
        vft.am = 0.8 + i * 0.01;
        vft.an = vft.am;
        vft.vm = 1.0 - vft.am / 1 * vft.am / 2;
        if (vft.vm < 0) continue;
        vft._buildTimeSegment();
        vft._cumsum();
        // vft.get_plan(3);
        double tm = vft.get_end_time();
        double ds = vft.get_distance(tm - 0.000001) - 0.75;
        printf("%d,%f,%f,%f\n", i, vft.am, tm, ds);
    }
    // range of tm is very special
    //  tMin >= 1.35 ,  tm not in (2.0, 2.6)
}
#endif