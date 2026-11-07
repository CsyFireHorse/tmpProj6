#ifndef _CAM_TAPPET_H_
#define _CAM_TAPPET_H_

#include "stdint.h"

typedef struct CamXyva {
    double dX, dY, dV, dA;
}CamXyva;

// same as SMC_CAMTappet
typedef struct Tappet {
    uint16_t ctt; // SMC_CAMTAPPETTYPE: pos, all, neg
    uint16_t cta; // TAPPETACTION: on, off,invert
    uint32_t dwDelay; 
    uint32_t dwDuration;
    uint16_t iGroupID;
    double x;
    uint32_t dwActive; // internal value, 0xFFFFFFFF
}Tappet;

void zmc_tappet_init(Tappet* tappet);

typedef struct MaxPosVelAcc {
    double minPos;
    double maxPos;
    double maxAbsVel;
    double maxAbsAcc;
}MaxPosVelAcc;

typedef struct MaxMinPosVelAcc {
    double minPos;
    double maxPos;
    double minVel;
    double maxVel;
    double minAcc;
    double maxAcc;
}MaxMinPosVelAcc;


typedef struct CamPvaj {
    double pos, vel, acc,jerk;
}CamPvaj;

typedef struct DoubleArr4 {
    double arr[4];
}DoubleArr4;

#endif