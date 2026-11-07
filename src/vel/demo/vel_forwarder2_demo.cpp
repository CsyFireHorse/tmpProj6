#include <iostream>

#include "stdio.h"
#include "vel/source/VelForwarder2.hpp"


using namespace zmc;

void vel_forwarder2_demo0() {
    VelForwarderGeneric<7> forwarder;
    VelParam velParam;
    velParam.set_config(5, 3, 3, 10);
    velParam.set_begin_pva(0, 1, 0);
    velParam.set_end_pva(26, 0, 0);
    SjRelaxParam relaxParam;
    relaxParam.vMax = 1.9;
    relaxParam.aMax = 3;
    relaxParam.aMin = -3;
    relaxParam.plainTime = 5;
    relaxParam.seg_type = 50;

    forwarder.build(relaxParam, velParam);
    double tc = forwarder.get_end_time();
    double Ts = 0.02;
    int N = int(tc / Ts);
    printf("idx,time,pos,vel,acc\n");
    for (int i = 0; i <= N; i++) {
        double t = i * Ts;
        PosVelAcc pva;
        forwarder.get_slice(t, pva);
        std::printf("%d,%-4.2f,%-7.5f,%-5.4f,%-6.4f\n", i, t, pva.pos, pva.vel, pva.acc);
    }
}

void vel_forwarder2_demo() {
    VelForwarderGeneric<7> forwarder;
    VelParam velParam;
    velParam.set_config(5, 3, 3, 10);
    velParam.set_begin_pva(0, 1, 0);
    velParam.set_end_pva(26, 0, 0);
    SjRelaxParam relaxParam;
    relaxParam.vMax = 1.9;
    relaxParam.aMax = 3;
    relaxParam.aMin = -3;
    relaxParam.aMid = 1;
    relaxParam.plainTime = 0;
    relaxParam.seg_type = 79;

    forwarder.build(relaxParam, velParam);
    double tc = forwarder.get_end_time();
    double Ts = 0.02;
    int N = int(tc / Ts);
    printf("idx,time,pos,vel,acc\n");
    for (int i = 0; i <= N; i++) {
        double t = i * Ts;
        PosVelAcc pva;
        forwarder.get_slice(t, pva);
        std::printf("%d,%-4.2f,%-7.5f,%-5.4f,%-6.4f\n", i, t, pva.pos, pva.vel, pva.acc);
    }
}