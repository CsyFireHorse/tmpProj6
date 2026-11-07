#include "stdint.h"
#include "mc_cam_table.h"
#include "cam_tappet_ser_api.h"
#include "cam_tappet_serialize.h"
// #include "vel/VelWrapCommon.h"
#include "fb/include/fb/zmc_cam_table.h"
#include <string>

int _cam_array_dump_csv(const char* filename, const CamXyva* xyvaList, int xyvaLen);

CamTappet* cam_tappet_new() {
    // cam.reserve(512);
    return new CamTappet();
}

void cam_tappet_delete(CamTappet* fb) { delete fb; }

CamXyva* cam_tappet_xyva_ptr(CamTappet* fb, int* len){
    *len = fb->xyva.size();
    return fb->xyva.data();
}
Tappet* cam_tappet_tappet_ptr(CamTappet* fb, int* len){
    *len = fb->tappets.size();
    return fb->tappets.data();
}
int cam_tappet_assign_xyva(CamTappet* fb, int idx, const CamXyva* xyva){
    if(idx<0 || idx>=fb->xyva.size()) return -1;
    fb->xyva[idx] = *xyva;
    return 0;
}
int cam_tappet_assign_tappet(CamTappet* fb, int idx, const Tappet* tappet){
    if(idx<0 || idx>=fb->tappets.size()) return -1;
    fb->tappets[idx] = *tappet;
    return 0;
}
void cam_tappet_resize(CamTappet* fb, int xyvaLen, int tappetLen){
    fb->xyva.resize(xyvaLen);
    fb->tappets.resize(tappetLen);
}
int cam_array_dump_csv(const char* filename, const CamXyva* xyvaList, int xyvaLen) {
    return  _cam_array_dump_csv(filename, xyvaList, xyvaLen);
}

void cam_tappet_to_cam_ref(const CamTappet* pCt, MC_CAM_REF cam){
    return pCt->toCamRef(cam);
}
int cam_ref_to_cam_tappet(const MC_CAM_REF cam, CamTappet* pCt){
    return pCt->fromCamRef(cam);
}

int cam_tappet_parse(CamTappet* pCt, const char* filename) {
    return pCt->parse(filename);
}

int cam_tappet_serialize(const CamTappet* pCt, const char* filename) {
    return pCt->serialize(filename);
}

int cam_tappet_parse_camfile(CamTappet* cam, const char* filename) {
    if (!cam) return -1;
    CamTappet& cf = *cam;
    int ret = cf.parseCamFile(filename);
    return ret;
}

MC_CAM_REF cam_ref_new(){
    return new MC_CAM_TABLE();
}
void cam_ref_delete(MC_CAM_REF fb){
    delete fb;
}


int cam_ref_get_pva(const MC_CAM_REF fb, double x, CamXyva* pva){
    PosVelAcc pvaTmp;
    if (!fb ||!fb->pce ||!fb->nElements) return -1;
    static_cast<const zmc::CamTable*>(fb)->getPosVelAcc(x, pvaTmp);
    pva->dX = x;
    pva->dV = pvaTmp.vel;
    pva->dA = pvaTmp.acc;
    pva->dY = pvaTmp.pos;
    return 0;
}

int cam_ref_get_pva_affine(const MC_CAM_REF fb, const PosVelAcc* masterPva, PosVelAcc* slavePva, const CamAffineCoefficient* affline){
    if (!fb || !fb->pce || !fb->nElements || !masterPva || !slavePva || !affline) return -1;
    
    // Cast to CamTable and call the affine version
    const zmc::CamTable* camTable = static_cast<const zmc::CamTable*>(fb);
    return camTable->get_pva_affine(*masterPva, *slavePva, *reinterpret_cast<const zmc::CamAffineTransform*>(affline));
}


void cam_affine_from_cam_rel(CamAffineCoefficient* affline, const CamAffineFullParam* param){
    if (!affline || !param) return;
    
    // Cast to zmc::CamAffineTransform
    zmc::CamAffineTransform* zmcAffline = reinterpret_cast<zmc::CamAffineTransform*>(affline);
    
    // Create CamRelStruct from CamAffineFullParam
    zmc::CamRelStruct camRel = {
        param->masterAbsolute,
        param->slaveAbsolute,
        param->masterEngagePos,
        param->slaveEngagePos
    };
    
    // Create CamAffineParam from CamAffineFullParam
    zmc::CamAffineParam affineParam = {
        param->MasterScaling,
        param->SlaveScaling,
        param->MasterOffset,
        param->SlaveOffset
    };
    
    // Call the template method
    zmcAffline->fromCamRel(camRel, affineParam);
}
