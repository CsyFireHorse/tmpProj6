/*
 * @Author: ChenShengyong 
 * @Date: 2025-03-21 16:18:01
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-09-04 10:26:29
 */
#ifndef _ZMC_CAM_TAPPET_SERIALIZE_H_
#define _ZMC_CAM_TAPPET_SERIALIZE_H_

#include "zmc_common.h"
#include "stdint.h"
#include "cam_tappet.h"
#include "vel/VelWrapCommon.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct CamTappet CamTappet;
typedef struct CamAffineCoefficient {
    double masterScale; 
    double masterIntercept; 
    double slaveScale; 
    double slaveIntercept; 
}CamAffineCoefficient;

ZMC_DLLPORT CamTappet* cam_tappet_new();
ZMC_DLLPORT void cam_tappet_delete(CamTappet* fb);
ZMC_DLLPORT CamXyva* cam_tappet_xyva_ptr(CamTappet* fb, int* len);
ZMC_DLLPORT Tappet* cam_tappet_tappet_ptr(CamTappet* fb, int* len);
ZMC_DLLPORT int cam_tappet_assign_xyva(CamTappet* fb, int idx, const CamXyva* xyva);
ZMC_DLLPORT int cam_tappet_assign_tappet(CamTappet* fb, int idx, const Tappet* tappet);
ZMC_DLLPORT void cam_tappet_resize(CamTappet* fb, int xyvaLen, int tappetLen);

// [Base]
// 1,0,0,4,0.0,10.0,2
// [Cam]
// 0.000000,3.500000,6.000000,7.000000
// 2.000000,-2.987000,6.500000,0.000000
// 5.500000,5.000000,3.500000,0.000000
// 10.000000,2.000000,0.500000,0.000000
// [Tappet]
// 256,2,3,1.500000,0,0
// 511,0,1,2.500000,0,0
ZMC_DLLPORT int cam_tappet_parse(CamTappet* fb, const char* filename);
ZMC_DLLPORT int cam_tappet_serialize(const CamTappet* fb, const char* filename);

ZMC_DLLPORT int cam_tappet_parse_camfile(CamTappet* cam, const char* filename);
ZMC_DLLPORT int cam_array_dump_csv(const char* filename, const CamXyva* xyvaList, int xyvaLen);


typedef struct MC_CAM_TABLE MC_CAM_TABLE;
typedef MC_CAM_TABLE* MC_CAM_REF;
ZMC_DLLPORT void cam_tappet_to_cam_ref(const CamTappet*, MC_CAM_REF);
ZMC_DLLPORT int cam_ref_to_cam_tappet(const MC_CAM_REF cam, CamTappet* pCt);

ZMC_DLLPORT MC_CAM_REF cam_ref_new();
ZMC_DLLPORT void cam_ref_delete(MC_CAM_REF fb);

ZMC_DLLPORT int cam_ref_get_pva(const MC_CAM_REF fb, double x, CamXyva* pva);
ZMC_DLLPORT int cam_ref_get_pva_affine(const MC_CAM_REF fb, const PosVelAcc* masterPva, PosVelAcc* slavePva, const CamAffineCoefficient* affline);


typedef struct CamAffineFullParam{
    double MasterScaling;
    double SlaveScaling;
    double MasterOffset;
    double SlaveOffset;
    bool masterAbsolute; // true: absolute position, false: relative position
    bool slaveAbsolute;
    double masterEngagePos; // position of master axis when cam table starts to cam in
    double slaveEngagePos; // position of slave axis when cam table starts to cam in
}CamAffineFullParam;

ZMC_DLLPORT void cam_affine_from_cam_rel(CamAffineCoefficient* affline, const CamAffineFullParam*  param);


#ifdef __cplusplus
} // extern "C" {
#endif

#endif