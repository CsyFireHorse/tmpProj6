/*
 * @Author: ChenSy
 * @Date: 2024-06-19 11:17:24
 * @Last Modified by: Cheny
 * @Last Modified time: 2024-09-04 11:44:57
 */
#ifndef _MC_CAM_TABLE_IMPL_H_
#define _MC_CAM_TABLE_IMPL_H_

#include "stdint.h"
#include "zmc_types_def.h"


typedef enum SMC_CAM_VAR_TYPE_t {
    mcCamVarTypeUnused = 0,  // <unused>
    mcCamVarTypeINT,         // ZMC_INT
    mcCamVarTypeUINT,        // ZMC_UINT
    mcCamVarTypeDINT,        // DINT
    mcCamVarTypeUDINT,       // ZMC_UDINT
    mcCamVarTypeREAL,        // ZMC_REAL
    mcCamVarTypeLREAL,       // ZMC_LREAL
} SMC_CAM_VAR_TYPE;


// A XYVA-cam is represented by an array of variables of type SMC_CAMXYVA describing each point of
// a cam.
typedef struct SMC_CAMXYVA_t {
    ZMC_LREAL dX;  // master position
    ZMC_LREAL dY;  // slave position
    ZMC_LREAL dV;  // first derivative dY/dX; slave velocity
    ZMC_LREAL dA;  // slave acceleration
} SMC_CAMXYVA;


typedef enum SMC_CAM_Interpolation_Quality_t {
    mcCamInterplQualLinear = 1,  // Linear interpolation
    mcCamInterplQualCubic = 3,   // Cubic interpolation
} SMC_CAM_Interpolation_Quality;


#endif