#include "fb/mc_cam_ref_helper.h"
#include "fb/cam_tappet_serialize.h"
#include "fbt_errorcode_def.h"
#include "zmc_cam_table.h"

using namespace zmc;
void mc_cam_ref_init(MC_CAM_REF pCam) {
    if (pCam == nullptr) {
        return;
    }
    ((zmc::CamTable*)pCam)->init();
}

/**
 * @brief Create a new MC_CAM_REF from XYVA data
 */
MC_CAM_REF mc_cam_ref_create(const CamXyva* pXyva, int len) {
    if (pXyva == nullptr || len <= 0) {
        return nullptr;
    }
    return (MC_CAM_REF) new CamTable(pXyva, len);
}

MC_CAM_REF mc_cam_ref_create_with_tappet(const CamXyva* pXyva, int len, const Tappet* pTappet, int tappetLen){
    if (pXyva == nullptr || len <= 0 || pTappet == nullptr || tappetLen <= 0) {
        return nullptr;
    }
    return (MC_CAM_REF) new CamTable(pXyva, len, pTappet, tappetLen);
}
/**
 * @brief Validate the MC_CAM_REF structure
 */
unsigned short mc_cam_ref_validate(const MC_CAM_REF camRef) {
    return ((CamTable*)camRef)->validate();
}

 
int mc_cam_ref_fill_xyva(MC_CAM_REF camRef, const CamXyva* pXyva, int len){
    return ((CamTable*)camRef)->fillXyva(pXyva, len);
}

 
int mc_cam_ref_fill_tappet(MC_CAM_REF camRef, const Tappet* pTappet, int tappetLen){
    return ((CamTable*)camRef)->fillTappet(pTappet, tappetLen);
}