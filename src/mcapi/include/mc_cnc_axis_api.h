/**
 * History:
 * ================================================================
 * 2025-08-07 ChenSy <ChenSy@zlg.cn> created.
 *
 */
#ifndef _ZMC_CNC_AXIS_API_H_
#define _ZMC_CNC_AXIS_API_H_

#include "zmc_types_def.h"
#include "zmc_enum_cdef.h"
#include "mc_ref_def.h"
#include "AxisRef.h"
#include "zmc_common.h"
#include "mc_cnc_posinfo_def.h"
// #include "fb/mc_cnc_enum_def.h"

#ifndef AW_PLC_NATIVE_ALIGN
#define AW_PLC_NATIVE_ALIGN 8
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, AW_PLC_NATIVE_ALIGN)

// SMC_ControlAxisByPos function block structure
typedef struct McControlAxisByPos{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_INT iStatus;
    ZMC_BOOL bEnable;
    ZMC_BOOL bAvoidGaps;
    ZMC_LREAL fSetPosition;
    ZMC_LREAL fGapVelocity;
    ZMC_LREAL fGapAcceleration;
    ZMC_LREAL fGapDeceleration;
    ZMC_LREAL fGapJerk;
    // output
    ZMC_BOOL bBusy;
    ZMC_BOOL bCommandAborted;
    ZMC_BOOL bError;
    ZMC_ERROR iErrorID;
    ZMC_BOOL bStopIpo;
}McControlAxisByPos;

// SMC_ControlAxisByPosVel function block structure
typedef struct McControlAxisByPosVel{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_INT iStatus;
    ZMC_BOOL bEnable;
    ZMC_BOOL bAvoidGaps;
    ZMC_LREAL fSetPosition;
    ZMC_LREAL fSetVelocity;
    ZMC_LREAL fGapVelocity;
    ZMC_LREAL fGapAcceleration;
    ZMC_LREAL fGapDeceleration;
    ZMC_LREAL fGapJerk;
    // output
    ZMC_BOOL bBusy;
    ZMC_BOOL bCommandAborted;
    ZMC_BOOL bError;
    ZMC_ERROR iErrorID;
    ZMC_BOOL bStopIpo;
}McControlAxisByPosVel;

// SMC_ControlAxisByVel function block structure
typedef struct McControlAxisByVel{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_INT iStatus;
    ZMC_BOOL bEnable;
    ZMC_BOOL bAvoidGaps;
    ZMC_LREAL fSetVelocity;
    ZMC_LREAL fGapAcceleration;
    ZMC_LREAL fGapDeceleration;
    ZMC_LREAL fGapJerk;
    // output
    ZMC_BOOL bBusy;
    ZMC_BOOL bCommandAborted;
    ZMC_BOOL bError;
    ZMC_ERROR iErrorID;
    ZMC_BOOL bStopIpo;
}McControlAxisByVel;


#pragma pack(pop)

// Function declarations for SMC_ControlAxisByPos
ZMC_DLLPORT int MC_ControlAxisByPos_create(McControlAxisByPos*);
ZMC_DLLPORT int MC_ControlAxisByPos_exec(McControlAxisByPos*);
ZMC_DLLPORT int MC_ControlAxisByPos_destroy(McControlAxisByPos*);

// Function declarations for SMC_ControlAxisByPosVel
ZMC_DLLPORT int MC_ControlAxisByPosVel_create(McControlAxisByPosVel*);
ZMC_DLLPORT int MC_ControlAxisByPosVel_exec(McControlAxisByPosVel*);
ZMC_DLLPORT int MC_ControlAxisByPosVel_destroy(McControlAxisByPosVel*);

// Function declarations for SMC_ControlAxisByVel
ZMC_DLLPORT int MC_ControlAxisByVel_create(McControlAxisByVel*);
ZMC_DLLPORT int MC_ControlAxisByVel_exec(McControlAxisByVel*);
ZMC_DLLPORT int MC_ControlAxisByVel_destroy(McControlAxisByVel*);

#ifdef __cplusplus
}
#endif

#endif // _MC_CNC_API_H_ 