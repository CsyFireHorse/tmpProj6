/*
 * @Author: ChenShengyong 
 * @Date: 2025-03-21 16:18:01
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-04-28 11:15:17
 */
#ifndef _CAM_INTERACTIVE_H_
#define _CAM_INTERACTIVE_H_

#include "cam_tappet.h"

struct CamXyvaMethod:public CamXyva{
    int calc_PosVelAcc(const CamXyva&, const double* xArr, CamPvaj* pvajArr, int arrLen)const;
    int maxPosVelAcc(const CamXyva&, MaxPosVelAcc& out)const;
    int maxPosVelAcc(const CamXyva&, MaxMinPosVelAcc& out)const;
    int switch_segmentType(CamXyva&, int segType);
};

#endif