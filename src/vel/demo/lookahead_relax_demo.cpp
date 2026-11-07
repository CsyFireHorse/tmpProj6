/*
 * @Author: ChenShengyong
 * @LastEditors: Chen Shengyong
 * @Date: 2022-06-16 13:49:27
 * @LastEditTime: 2022-07-22 16:12:56
 * @Description: Modify here please
 */

#include "lookahead_relaxation.h"
#include "stdio.h"
// #include "velocity.h"
#include "TrapzVel.h"
#include "stdint.h"

using namespace zmc;


#define LEN(x) sizeof(x) / sizeof(*(x))

typedef struct LookaheadDemoParams {
    int N;
    double* varr;
    double* sarr;
    double vmax;
    double acc;
    double dec;
    double jerk;
    double* varr2;
} LookaheadDemoParams;

// double velocity_get_rend_distance(velocity_handle insert_vh);

template <int N>
struct LookaheadDemoParams2 {
    double sArr[N];
    double accArr[N];
    double decArr[N];
    double jerkArr[N];
    double vArr[N + 1];
    inline int len() const { return N; }
};

template <typename T>
void _lookahead_demo(T& par) {
    lookahead_relaxation(par.vArr, par.sArr, par.len(), par.accArr, par.decArr, par.jerkArr);
    for (int i = 0; i < par.len() + 1; i++) {
        printf("%f\t", par.vArr[i]);
    }
    printf("\n");
}

void lookahead_demo() {
    LookaheadDemoParams2<5> par = {// 0 1 2.236068 1 1.414214 0
                                   {1, 2, 3, 2, 1},
                                   {1, 1, 1, 1, 1},
                                   {1, 1, 1, 1, 1},
                                   {0, 0, 0, 0, 0},
                                   {0, 1, 3, 1, 2, 0}};

    LookaheadDemoParams2<5> par2 = {// 0 1 3 1 2 0
                                    {1, 2, 3, 2, 1},
                                    {3, 3, 3, 3, 3},
                                    {3, 3, 3, 3, 3},
                                    {0, 0, 0, 0, 0},
                                    {0, 1, 3, 1, 2, 0}};

    LookaheadDemoParams2<5> par3 = {// 0, 1, 2.1641589825484533, 1, 1.3650971698084906, 0
                                    {1, 2, 3, 2, 1},
                                    {1, 1, 1, 1, 1},
                                    {1, 1, 1, 1, 1},
                                    {10, 10, 10, 10, 10},
                                    {0, 1, 3, 1, 2, 0}};

    LookaheadDemoParams2<4> par4 = {// [0, 7.524233833096018, 10, 7.524233833096018, 0]
                                    {10, 10, 10, 10},
                                    {3, 3, 3, 3},
                                    {3, 3, 3, 3},
                                    {5, 5, 5, 5},
                                    {0, 10, 10, 10, 0}};

    LookaheadDemoParams2<5> par5 = {// [0, 1, 2.1641589825484533, 1, 1.3650971698084906, 0]
                                    {1, 2, 3, 2, 1},
                                    {3, 3, 3, 3, 3},
                                    {3, 3, 3, 3, 3},
                                    {10, 10, 10, 10, 10},
                                    {0, 10, 10, 10, 10, 0}};
    _lookahead_demo(par5);
}

void _lookahead_vel_demo(LookaheadDemoParams& par);

void lookahead_vel_demo() {
    // 31,22,32
    double varr[] = {0, 10, 10, 0};
    double sarr[] = {5, 3, 4};

    int N = LEN(varr);
    double acc = 3, dec = 3, jerk = 5, vmax = 10;
    LookaheadDemoParams par = {N, varr, sarr, vmax, acc, dec, jerk};
    _lookahead_vel_demo(par);
}

void lookahead_vel_demo1b() {
    // 31,51,32
    double varr[] = {0, 10, 10, 0};
    double sarr[] = {5, 3, 4};

    int N = LEN(varr);
    double acc = 3, dec = 3, jerk = 20, vmax = 10;
    LookaheadDemoParams par = {N, varr, sarr, vmax, acc, dec, jerk};
    _lookahead_vel_demo(par);
}

void lookahead_vel_demo3() {
    // 31 31 31 ... 22 22 32 32 32
    double varr[] = {0, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 0};
    double sarr[] = {5, 3, 4, 4, 4, 3, 3, 4, 4, 4, 3};

    int N = LEN(varr);
    double acc = 3, dec = 3, jerk = 20, vmax = 10;
    LookaheadDemoParams par = {N, varr, sarr, vmax, acc, dec, jerk};
    _lookahead_vel_demo(par);
}
void lookahead_vel_demo3b() {
    double varr[] = {0, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 0};
    double sarr[] = {5, 3, 4.7123889803846897, 4, 4.7123889803846897,
                     3, 3, 4.7123889803846897, 4, 4.7123889803846897,
                     3};

    int N = LEN(varr);
    double acc = 3, dec = 3, jerk = 20, vmax = 10;
    LookaheadDemoParams par = {N, varr, sarr, vmax, acc, dec, jerk};
    _lookahead_vel_demo(par);
}



template<typename T>
void _lookahead_vel_demo_tmpl(LookaheadDemoParams& par) {
    int ret = lookahead_relaxation(par.varr, par.sarr, par.N, &par.acc, &par.dec, &par.jerk);
    if (ret){
        printf("lookahead_relaxation failed\n");
        return;
    }
    double plan_time;
    auto handle = T();

    printf("idx,time,pos,vel,acc\n");
    double tc = 0, pc = 0;
    int jc = 0;
    double Ts = 0.02;
    for (int i=0;i < par.N-1; i++){
        handle.set_config(par.vmax, par.acc, par.dec, par.jerk);
        handle.set_begin_pva(0, par.sarr[i]);
        handle.set_end_pva(par.varr[i],  par.varr[i+1]);
        if (handle.build()){
            printf("build failed\n");
            return;
        }
        plan_time = handle.get_end_time();

        if (plan_time> 1e6 || plan_time < -1e6){
            printf("plan_time not right\n");
            return;
        }
        int CNT = int((tc+plan_time)/Ts);
        double sc, vc, ac;

        for (uint32_t j = jc+1; j <= CNT; j++)
        {
            double tm = Ts * j - tc;
            handle.get_slice(tm, &sc, &vc, &ac);
            printf("%d,%f,%f,%f,%f\n", j, tm+tc, sc+pc, vc, ac);
        }
        tc += plan_time;
        jc = CNT;
        pc += handle.get_end_pos();
    }
    // velocity_delete(handle);
}

void _lookahead_vel_demo(LookaheadDemoParams& par) {
    return _lookahead_vel_demo_tmpl<TrapzVel>(par);
};
