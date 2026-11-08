#include "cam_interactive.h"
#include "stdio.h"
#include "zmc_cam_table.h"
#include "camGenHelper.hpp"
#include "fb_cpp_def.h"
#include "axis/fbt_axis_cia402.h"
#include "impl/fbt_axis_base.h"

using namespace zmc;

void CamTableFitter_demo(){
    CamTableFitter fitter;

    #define CAM_XYVA_LEN 21
    CamXyva dat[CAM_XYVA_LEN] = {0};
    fillXyvaToy((Xyva*)dat, CAM_XYVA_LEN, 90);

    CamTable table(dat, CAM_XYVA_LEN/2+1);

    fitter.pCamTable = &table;

    PosVelAcc pva;
    #define N (90)
    double dx = (table.xEnd - table.xStart) / N;

    for (int i = 0; i <= N; i++) {
        double x = table.xStart + i * dx;
        int interval = fitter.get_pva_or_build(x, pva);
        printf("%d, %f, %d, %f, %f, %f\n", i, x, interval, pva.pos, pva.vel, pva.acc);
    }
}


void CamTableTappet_demo0(){
    CamTappetChecker fitter;

    #define CAM_XYVA_LEN 21
    CamXyva dat[CAM_XYVA_LEN] = {0};
    fillXyvaToy((Xyva*)dat, CAM_XYVA_LEN, 90);

    SMC_CAMTappet tappets[] = {
        // Type, action, delay, duration, group ID, position, activation.
        {TAPPET_pos, TAPPETACTION_on, 0, 0, 1, 10., 0xffffffff}, 
        {TAPPET_pos, TAPPETACTION_on, 0, 0, 2, 20., 0xffffffff}, 
        {TAPPET_all, TAPPETACTION_off, 0, 0, 1, 30., 0xffffffff}, 
        {TAPPET_all, TAPPETACTION_on, 0, 0, 3, 50., 0xffffffff}, 
        {TAPPET_all, TAPPETACTION_inv, 0, 0, 4, 50., 0xffffffff}, 
        {TAPPET_all, TAPPETACTION_inv, 0, 0, 5, 60., 0xffffffff}, 
        {TAPPET_neg, TAPPETACTION_off, 0, 0, 4, 70., 0xffffffff}, 
        {TAPPET_pos, TAPPETACTION_off, 0, 0, 2, 90., 0xffffffff}  
    };

    CamTable table(dat, CAM_XYVA_LEN/2+1, (const Tappet*)tappets, sizeof(tappets)/sizeof(tappets[0]));

    fitter.pCamTable = &table;

    PosVelAcc pva;
    #define N (90)
    double dx = (table.xEnd - table.xStart) / N;

    SMC_TappetData tappetData;
    #define TAPPET_ID_NUM 5
    FbtGetTappetValue getTappetValue[TAPPET_ID_NUM];
    for (int i = 0; i < TAPPET_ID_NUM; i++){
        getTappetValue[i].iID = i+1;
        getTappetValue[i].Tappets = &tappetData;
    }
    getTappetValue[3].bInitValue = true;

    int verbose = 1;
    for (int i = 0; i <= N; i++) {
        double x = table.xStart + i * dx;
        int interval = fitter.get_pva_or_build(x, pva);
        int triggered_tappet_num = fitter.checkProcessTappet(&tappetData);
        int tappetBitValue = 0;
        for (int j = 0; j < TAPPET_ID_NUM; j++) {
            getTappetValue[j].runCycle();
            tappetBitValue |= getTappetValue[j].bTappet << j;
        }
        for (int j = 0; j < triggered_tappet_num; j++) {
            if (!verbose) break;
            auto pTappet = tappetData.pTaps[j]; 
            printf("tappet gid=%d triggered %s at %f, value: %d\n", pTappet->iGroupID, getTappetActionName(pTappet->cta), pTappet->x, getTappetValue[pTappet->iGroupID-1].bTappet);
        }
        printf("%d, %f, %d, %f, %f, %f, 0x%x\n", i, x, interval, pva.pos, pva.vel, pva.acc, tappetBitValue);
    }
}

