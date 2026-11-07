#include "FbtMotBuf.h"
// #include "PfbMoveDiscrete.h"
#include <stdio.h>
#include <iostream>
#include "AxisStatInfo.h"
#include "FbtNodeAll.h"

using namespace zmc;

void fbtmotbuf_demo() {
    std::cout << "=== Basic FbtNode Interface Demo ===" << std::endl;
    FbtMotBuf buf;
    buf.pAxisInfo->axisTs = 0.02;  // 20ms cycle

    PfbMoveDiscrete mov;
    // mov.setMoveMode(mcMoveAbsolute);
    mov.setBuf(&buf);
    mov.setPosition(40.0);
    mov.setVelConfig(100.0, 200, 200, 0);
    mov.setBufferMode(mcAborting);

    double pos, vel, acc;
    for (int i = 0; i < 50; ++i) {
        if (i == 2) {
            mov.setExecute(true);
            mov.runCycle();
            // buf.addPfbToQueue((FbImplBase*) &mov);
        }
        buf.runCycle();
        buf.get_commands(pos, vel, acc);
        int st = mov.isBusy() + mov.isActive() + mov.isDone() * 3;
        if (1) {
            printf("Cycle %d: pos=%.2f, vel=%.2f, acc=%.2f,%d\n", i, pos, vel, acc, st);
        }
        if (buf.isEmpty() && i > 50) {
            std::cout << "Motion completed at cycle " << i << std::endl;
            break;
        }
    }
}

void fbtmotbuf_demo20() {
    FbtMotBuf buf;
    buf.pAxisInfo->axisTs = 0.02;  // 20ms cycle
    PfbMoveDiscrete mov;
    mov.setMotionType(0);
    mov.setBuf(&buf);
    mov.setPosition(40.0);
    mov.setVelConfig(100.0, 200, 200, 0);
    mov.setBufferMode(mcAborting);

    double pos, vel, acc;
    for (int i = 0; i < 100; ++i) {
        if (i == 2) {
            mov.setExecute(true);
        } else if (i == 45) {
            mov.setExecute(false);
        } else if (i == 60) {
            mov.setMotionType(1);
            mov.setExecute(true);
        } 
        mov.runCycle();
        buf.runCycle();
        buf.get_commands(pos, vel, acc);

        int st = mov.isBusy() + mov.isActive() + mov.isDone() * 3;
        printf("Cycle %d: pos=%.2f, vel=%.2f, acc=%.2f, %d\n", i, pos, vel, acc, st);
    }
}



void fbtmotbuf_demo2() {
    FbtMotBuf buf;
    buf.pAxisInfo->axisTs = 0.02;  // 20ms cycle
    PfbMoveDiscrete mov;
    mov.setMotionType(0);
    mov.setBuf(&buf);
    mov.setPosition(40.0);
    mov.setVelConfig(100.0, 200, 200, 0);
    mov.setBufferMode(mcAborting);

    PfbMoveDiscrete mov2;
    mov2.setMotionType(1);
    mov2.setBuf(&buf);
    mov2.setPosition(-30.0);
    mov2.setVelConfig(80.0, 160, 160, 0);
    mov2.setBufferMode(mcAborting);
    double pos, vel, acc;
    for (int i = 0; i < 120; ++i) {
        if (i == 2) {
            mov.setExecute(true);
        } else if (i == 45) {
            mov.setExecute(false);
        } else if (i == 22) {
            mov2.setMotionType(1);
            mov2.setExecute(true);
        } 
        mov.runCycle();
        mov2.runCycle();
        buf.runCycle();
        buf.get_commands(pos, vel, acc);

        int st = mov.isBusy() + mov.isActive() + mov.isDone() * 3 + mov.isCommandAborted() * 4;
        printf("Cycle %d: pos=%.2f, vel=%.2f, acc=%.2f, %d\n", i, pos, vel, acc, st);
    }
}
