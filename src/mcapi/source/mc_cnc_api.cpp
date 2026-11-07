/**
 * History:
 * ================================================================
 * 2025-01-27 ChenSy <ChenSy@zlg.cn> created.
 *
 */

#include "mc_cnc_axis_api.h"
#include "fb_cnc_axis_def.h"
#include "fb_cnc_int_def.h"

#include <new>

using namespace zmc;

int MC_ControlAxisByPos_create(McControlAxisByPos* pFb){
    static_assert(sizeof(FbtControlAxisByPos) == sizeof(McControlAxisByPos), "ControlAxisByPos size not equal");
    new (pFb) FbtControlAxisByPos();
    return 0;
}

int MC_ControlAxisByPos_exec(McControlAxisByPos* pFb){
    ((FbtControlAxisByPos*) pFb)->runCycle();
    return 0;
}

int MC_ControlAxisByPos_destroy(McControlAxisByPos* pFb){
    ((FbtControlAxisByPos*)pFb)->~FbtControlAxisByPos();
    return 0;
}

int MC_ControlAxisByPosVel_create(McControlAxisByPosVel* pFb){
    static_assert(sizeof(FbtControlAxisByPosVel) == sizeof(McControlAxisByPosVel), "ControlAxisByPosVel size not equal");
    new (pFb) FbtControlAxisByPosVel();
    return 0;
}

int MC_ControlAxisByPosVel_exec(McControlAxisByPosVel* pFb){
    ((FbtControlAxisByPosVel*) pFb)->runCycle();
    return 0;
}

int MC_ControlAxisByPosVel_destroy(McControlAxisByPosVel* pFb){
    ((FbtControlAxisByPosVel*)pFb)->~FbtControlAxisByPosVel();
    return 0;
}

int MC_ControlAxisByVel_create(McControlAxisByVel* pFb){
    static_assert(sizeof(FbtControlAxisByVel) == sizeof(McControlAxisByVel), "ControlAxisByVel size not equal");
    new (pFb) FbtControlAxisByVel();
    return 0;
}

int MC_ControlAxisByVel_exec(McControlAxisByVel* pFb){
    ((FbtControlAxisByVel*) pFb)->runCycle();
    return 0;
}

int MC_ControlAxisByVel_destroy(McControlAxisByVel* pFb){
    ((FbtControlAxisByVel*)pFb)->~FbtControlAxisByVel();
    return 0;
}
