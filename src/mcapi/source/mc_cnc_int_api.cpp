/**
 * History:
 * ================================================================
 * 2025-01-27 ChenSy <ChenSy@zlg.cn> created.
 *
 */

#include "mc_cnc_int_api.h"
#include "fb_cnc_axis_def.h"
#include "fb_cnc_int_def.h"

#include <new>

using namespace zmc;

// SMC_Interpolator function block implementation
int MC_Interpolator_create(McInterpolator* pFb){
    static_assert(sizeof(FbtInterpolator) == sizeof(McInterpolator), "Interpolator size not equal");
    new (pFb) FbtInterpolator();
    return 0;
}

int MC_Interpolator_exec(McInterpolator* pFb){
    ((FbtInterpolator*) pFb)->runCycle();
    return 0;
}

int MC_Interpolator_destroy(McInterpolator* pFb){
    ((FbtInterpolator*)pFb)->~FbtInterpolator();
    return 0;
}

// SMC_TRAFO_Gantry2 function block implementation
int MC_TrafoGantry2_create(McTrafoGantry2* pFb){
    static_assert(sizeof(FbtTrafoGantry2) == sizeof(McTrafoGantry2), "TrafoGantry2 size not equal");
    new (pFb) FbtTrafoGantry2();
    return 0;
}

int MC_TrafoGantry2_exec(McTrafoGantry2* pFb){
    ((FbtTrafoGantry2*) pFb)->runCycle();
    return 0;
}

int MC_TrafoGantry2_destroy(McTrafoGantry2* pFb){
    ((FbtTrafoGantry2*)pFb)->~FbtTrafoGantry2();
    return 0;
}

// SMC_TRAFOF_Gantry2 function block implementation
int MC_TrafoFGantry2_create(McTrafoFGantry2* pFb){
    static_assert(sizeof(FbtTrafoFGantry2) == sizeof(McTrafoFGantry2), "TrafoFGantry2 size not equal");
    new (pFb) FbtTrafoFGantry2();
    return 0;
}

int MC_TrafoFGantry2_exec(McTrafoFGantry2* pFb){
    ((FbtTrafoFGantry2*) pFb)->runCycle();
    return 0;
}

int MC_TrafoFGantry2_destroy(McTrafoFGantry2* pFb){
    ((FbtTrafoFGantry2*)pFb)->~FbtTrafoFGantry2();
    return 0;
}

// SMC_CNC_REF function block implementation
int MC_CncRef_create(McCncRef* pFb){
    static_assert(sizeof(FbtCncRef) == sizeof(McCncRef), "CncRef size not equal");
    new (pFb) FbtCncRef();
    return 0;
}

int MC_CncRef_exec(McCncRef* pFb){
    ((FbtCncRef*) pFb)->runCycle();
    return 0;
}

int MC_CncRef_destroy(McCncRef* pFb){
    ((FbtCncRef*)pFb)->~FbtCncRef();
    return 0;
}


// SMC_CheckVelocities function block implementation
int MC_CheckVelocities_create(McCheckVelocities* pFb){
    static_assert(sizeof(FbtCheckVelocities) == sizeof(McCheckVelocities), "CheckVelocities size not equal");
    new (pFb) FbtCheckVelocities();
    return 0;
}

int MC_CheckVelocities_exec(McCheckVelocities* pFb){
    ((FbtCheckVelocities*) pFb)->runCycle();
    return 0;
}

int MC_CheckVelocities_destroy(McCheckVelocities* pFb){
    ((FbtCheckVelocities*)pFb)->~FbtCheckVelocities();
    return 0;
}

// SMC_NCDecoder function block implementation
int MC_NCDecoder_create(McNCDecoder* pFb){
    static_assert(sizeof(FbtNCDecoder) == sizeof(McNCDecoder), "NCDecoder size not equal");
    new (pFb) FbtNCDecoder();
    return 0;
}

int MC_NCDecoder_exec(McNCDecoder* pFb){
    ((FbtNCDecoder*) pFb)->runCycle();
    return 0;
}

int MC_NCDecoder_destroy(McNCDecoder* pFb){
    ((FbtNCDecoder*)pFb)->~FbtNCDecoder();
    return 0;
} 