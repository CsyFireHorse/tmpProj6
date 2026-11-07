#include "axis/AxisRunTmpl.hpp"
#include "fbt_axis_impl.h"
#include "fb_cpp_def.h"
#include "fb_cpp_def.h"
#include "stdio.h"
#include "camGenHelper.hpp"
#include "cam_tappet_serialize.h"
#include "zmc_cam_table.h"
#include "common/misc/CmdArgsToy.hpp"

using namespace zmc;



void CamTableTappet_demo2(){
    AxisMotEmu axisArr[2];

    FbtMoveVelocity fbMov;
    FbtCamTableSelect fbTbl;
    FbtCamIn camIn;
    #define TAPPET_ID_NUM 5
    FbtGetTappetValue getTappetValue[TAPPET_ID_NUM];

    fbMov.Axis = &axisArr[0];
    fbMov.Acceleration = 1000;
    fbMov.Deceleration = 1000;
    fbMov.Velocity = 100;

    fbTbl.Master = &axisArr[0];
    fbTbl.Slave = &axisArr[1];
    camIn.Master = &axisArr[0];
    camIn.Slave = &axisArr[1];

    #define CAM_XYVA_LEN 21
    CamXyva dat[CAM_XYVA_LEN] = {0};
    fillXyvaToy((Xyva*)dat, CAM_XYVA_LEN, 90);

    SMC_CAMTappet tappets[] = {
        // 类型, 动作, 延迟, 持续时间, 组ID, 位置, 激活
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
    
    auto camRef = (MC_CAM_REF) &table;
    fbTbl.CamTable = camRef;


    for (int i = 0; i < TAPPET_ID_NUM; i++){
        getTappetValue[i].iID = i+1;
        getTappetValue[i].Tappets = &camIn.Tappets;
    }
    getTappetValue[3].bInitValue = true;

    int verbose = 1;
    int fbStat;
    for (int i = 0; i <= 300; i++) {
        if (i==3) fbTbl.Execute = true;
        if (i==6){
            fbMov.Execute = true;
            camIn.Execute = true;
            camIn.CamTableID = fbTbl.CamTableID;
        }
        // fbPow.runCycle();
        // fbPow2.runCycle();
        fbMov.runCycle();
        fbTbl.runCycle();
        camIn.runCycle();
        int tappetBitValue = 0;
        for (int j = 0; j < TAPPET_ID_NUM; j++) {
            getTappetValue[j].runCycle();
            tappetBitValue |= getTappetValue[j].bTappet << j;
        }
        for (int j=0; j<2; j++){
            auto ax =  (AxisImplBase&) axisArr[j];
            if (j==0){
                fbStat = fbMov.Busy * 1 + fbMov.InVelocity * 2 + fbMov.Error * 4 + fbMov.CommandAborted * 5;
            }else{
                fbStat = camIn.EndOfProfile * 3 + camIn.Busy * 2 + camIn.InSync * 1 + camIn.Error * 4;
            }
            PosVelAcc pva = {ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration};
            printf("%d, %d, %d, %d, %f, %f, %f, 0x%x\n", i, j, ax.axisStat, fbStat, pva.pos, pva.vel, pva.acc, tappetBitValue);
        }
        axisArr[0].runCycle();
        axisArr[1].runCycle();
    }
}