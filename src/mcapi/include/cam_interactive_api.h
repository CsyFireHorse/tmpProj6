/*
 * @Author: ChenShengyong 
 * @Date: 2025-03-21 16:18:01
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-03-25 17:40:49
 */
#ifndef _CAM_INTERACTIVE_API_H_
#define _CAM_INTERACTIVE_API_H_

#include "zmc_common.h"
#include "cam_tappet.h"


#ifdef __cplusplus
extern "C" {
#endif


// \brief calculate  pos, vel, acc of multiple points of a segment
// \argument[in] lhs: left segment
// \argument[in] rhs: right segment
// \argument[in] xArr: right segment
// \argument[out]  xyva  (dX is input, dY, dV, dA are output)
// \argument[in] outLen: length of xyva
// \return 0 for success, else for error
ZMC_DLLPORT int calc_poly5_PosVelAcc(const CamXyva* lhs, const CamXyva* rhs, const double* xArr, CamPvaj* pvajArr, int arrLen);

// \brief calculate max pos, vel, acc of a segment
// \argument[in] lhs: left segment
// \argument[in] rhs: right segment
// \argument[out] out: max pos, max abs vel, max abs acc
// \return 0 for success, else for error
ZMC_DLLPORT int calc_poly5_maxPosVelAcc(const CamXyva* lhs, const CamXyva* rhs, MaxPosVelAcc* out);

ZMC_DLLPORT int calc_poly5_maxPosVelAcc_WithSegType(const CamXyva* lhs, const CamXyva* rhs, MaxPosVelAcc* out, int segType);


// \brief switch segment type
// \argument[inout] lhs: left segment
// \argument[inout] rhs: right segment
// \argument[in] segType: 0 for Poly5, 1 for line
// \return 0 for success, else for error
ZMC_DLLPORT int switch_poly5_segmentType(CamXyva* lhs, CamXyva* rhs, int segType);


#ifdef __cplusplus
}
#endif

#endif