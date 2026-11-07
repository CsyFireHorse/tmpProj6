#ifndef _VEL_TEST_HELPER_HPP_
#define _VEL_TEST_HELPER_HPP_

#include <gtest/gtest.h>
#include <stdio.h>
#include "VelWrapCommon.h"
// #include "VelCppWrap.h"
#include "math.h"
#include "velocity_utils.hpp"
#include "scurve_base.h"

namespace zmc{

struct VelSgParams2{
    double S_0;
    double S_d;
    double V_0;
    double V_d;
    double V_m;
    double A_m;
    double A_n;
    double Sparam;
    double Tc;
    int nseg;

    void invert(){
        S_0 = -S_0;
        S_d = -S_d;
        V_0 = -V_0;
        V_d = -V_d;
    }
    void shift_pos(double ofs){
        S_0 += ofs;
        S_d += ofs;
    }
    void swap_am_an(){
        auto tmp = A_m;
        A_m = A_n;
        A_n = tmp;
    }
};


struct VelSgParams3 {
    // double S_0;
    double S_d;
    double V_0;
    double V_d;
    double A_0;
    // double A_d;
    double V_m;
    double A_m;
    double A_n;
    double Sparam; // use as jerk
    double Tc;
    int nseg;

    void invert(){
        S_d = -S_d;
        V_0 = -V_0;
        V_d = -V_d;
        A_0 = -A_0;
    }
    void swap_am_an(){
        auto bak = A_m;
        A_m = A_n;
        A_n = bak;
    }
};

template<typename P, typename T, int TOL = 20>
void VelSgParams_vel_check(const P& par, T& vt, double Ts, bool verbose = false){
    const double TOL_VAL = 1.0/(((uint64_t)1)<<TOL);
    VelOnlineChecker chk;
    chk.Ts = Ts;
    double plan_time = vt.get_end_time();
    int CNT = ceil(plan_time / Ts);
    PosVelAcc pva;
    for (int i = 0; i <= CNT; i++) {
        double tm = Ts * i;
        vt.get_slice(tm, pva);
        if (i == 0) {
            EXPECT_NEAR(pva.pos, par.S_0, TOL_VAL);
            EXPECT_NEAR(pva.vel, par.V_0, TOL_VAL);
        }
        if (verbose) {
            printf("  sample=%d tm=%.17g pos=%.17g vel=%.17g acc=%.17g\n",
                   i, tm, pva.pos, pva.vel, pva.acc);
        }
        if (i != CNT)
            ASSERT_EQ(0, chk.check(pva.pos, pva.vel, pva.acc));
    }
    EXPECT_NEAR(pva.pos, par.S_d, TOL_VAL);
    EXPECT_NEAR(pva.vel, par.V_d, TOL_VAL);

    // EXPECT_EQ(vt.get_nseg(), int(par.nseg));
    // EXPECT_EQ(vt.check(), 0);

    // double plan_time, plan_v_max;
    // vt.get_plan(&plan_time, &plan_v_max);
    // if (par.Tc)
    //     EXPECT_NEAR(plan_time, par.Tc, 1e-6);

    // // double plan_time = vt.get_rend_time();
    // int CNT = ceil(plan_time/Ts);
    // double sc, vc, ac;
    // for (int i = 0; i <= CNT; i++)
    // {
    //     double tm = Ts * i;
    //     vt.get_slice(tm, &sc, &vc, &ac);
    //     if (i==0){
    //         EXPECT_NEAR(sc, par.S_0, 1e-6);
    //         EXPECT_NEAR(vc, par.V_0, 1e-6);
    //     }

    // }
    // EXPECT_EQ(pos_vel_check(vt, Ts), 0);
}

template <typename T, typename P, int TOL = 20>
void VelSgParams_to_vel_check(T& vt, const P& par, double Ts = 0.01,
                              bool verbose = false) {
    // VelocityWrap vt;
    vt.set_config(par.V_m, par.A_m, par.A_n, par.Sparam);
    vt.set_begin_pva(par.S_0, par.V_0);
    vt.set_end_pva(par.S_d, par.V_d);
    int errCode = vt.build();
    ASSERT_EQ(errCode, 0);
    VelSgParams_vel_check<P, T, TOL>(par, vt, Ts, verbose);
}


}
#endif
