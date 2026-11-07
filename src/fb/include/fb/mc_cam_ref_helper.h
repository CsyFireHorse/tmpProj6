#ifndef _MC_CAM_REF_HELPER_H_
#define _MC_CAM_REF_HELPER_H_

// #include "fbt_errorcode_def.h"
#include "cam_tappet.h"
#include "mc_ref_def.h"
// #include "zmc_cam_table.h"
#include "zmc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize a MC_CAM_REF structure with default values
 * 
 * This function is an enhanced version of the original initCamRef function
 * from cam_tappet_serialize.h. It provides the same functionality with
 * additional safety checks.
 * 
 * @param pCam Pointer to the MC_CAM_REF structure to initialize
 */
ZMC_DLLPORT 
void mc_cam_ref_init(MC_CAM_REF pCam);

/**
 * @brief Validate the MC_CAM_REF structure
 * 
 * This function performs comprehensive validation of the MC_CAM_REF structure,
 * checking for various error conditions that could affect cam table operations.
 * 
 * @param camRef Pointer to the MC_CAM_REF structure to validate
 * @return MC_ERROR_CODE indicating the validation result
 *         - ZMC_NO_ERROR if validation passes
 *         - ZMC_NO_CAM_REF_TYPE if camRef is NULL or invalid type
 *         - SMC_CAM_TABLE_DOES_NOT_COVER_MASTER_SCALE if master scale is invalid
 *         - SMC_CAM_TABLE_EMPTY_MASTER_RANGE if master range is empty
 *         - SMC_CAM_TABLE_INVALID_MASTER_MINMAX if master min/max values are invalid
 *         - SMC_CAM_TABLE_INVALID_SLAVE_MINMAX if slave min/max values are invalid
 *         - ZMC_CAM_TABLE_INVALID_DATA if cam table data is invalid
 */
ZMC_DLLPORT 
unsigned short mc_cam_ref_validate(const MC_CAM_REF camRef);

/**
 * @brief Create a new MC_CAM_REF from XYVA data
 * 
 * This function is an enhanced version of the original makeCamRef function
 * from cam_tappet_serialize.h. It provides the same functionality with
 * additional safety checks.
 * 
 * @param pXyva Pointer to XYVA data array
 * @param len Length of the XYVA data array
 * @return MC_CAM_REF New CAM_REF structure (caller must free memory)
 */
ZMC_DLLPORT 
MC_CAM_REF mc_cam_ref_create(const CamXyva* pXyva, int len);

ZMC_DLLPORT 
MC_CAM_REF mc_cam_ref_create_with_tappet(const CamXyva* pXyva, int len, const Tappet* pTappet, int tappetLen);

ZMC_DLLPORT 
int mc_cam_ref_fill_xyva(MC_CAM_REF camRef, const CamXyva* pXyva, int len);

ZMC_DLLPORT 
int mc_cam_ref_fill_tappet(MC_CAM_REF camRef, const Tappet* pTappet, int tappetLen);


#ifdef __cplusplus
}
#endif

#endif // _MC_CAM_REF_HELPER_H_ 