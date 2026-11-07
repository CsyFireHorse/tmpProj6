/**
 * History:
 * ================================================================
 * 2025-07-30 ChenSy <ChenSy@zlg.cn> created.
 *
 */
	
#ifndef _FB_CNC_AXIS_DEF_H_
#define _FB_CNC_AXIS_DEF_H_

#include "fbt_common.h"
#include "mc_ref_def.h"
#include "mc_cnc_enum_def.h"
// #include "mc_cnc_geoinfo_def.h"
#include "mc_cnc_posinfo_def.h"


namespace zmc{
    
#pragma pack(push, AW_PLC_NATIVE_ALIGN)

// SMC_ControlAxisByPos function block structure
typedef struct FbtControlAxisByPosStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_INT iStatus; // SMC_INT_STATUS
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
    MC_ERROR iErrorID;
    ZMC_BOOL bStopIpo;
}FbtControlAxisByPosStruct;

// SMC_ControlAxisByPosVel function block structure
typedef struct FbtControlAxisByPosVelStruct{
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
    MC_ERROR iErrorID;
    ZMC_BOOL bStopIpo;
}FbtControlAxisByPosVelStruct;

// SMC_ControlAxisByVel function block structure
typedef struct FbtControlAxisByVelStruct{
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
    MC_ERROR iErrorID;
    ZMC_BOOL bStopIpo;
}FbtControlAxisByVelStruct;


#pragma pack(pop)

class FbtControlAxisByPos : public FbtControlAxisByPosStruct {
public:
    FbtControlAxisByPos();
    ~FbtControlAxisByPos();
    void runCycle();
};

class FbtControlAxisByPosVel : public FbtControlAxisByPosVelStruct {
public:
    FbtControlAxisByPosVel();
    ~FbtControlAxisByPosVel();
    void runCycle();
};

class FbtControlAxisByVel : public FbtControlAxisByVelStruct {
public:
    FbtControlAxisByVel();
    ~FbtControlAxisByVel();
    void runCycle();
};

}
#endif 