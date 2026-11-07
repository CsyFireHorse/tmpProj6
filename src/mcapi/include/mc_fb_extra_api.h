/**
 * History:
 * ================================================================
 * 2025-04-14 ChenSy <ChenSy@zlg.cn> created.
 *
 */
#ifndef _MC_FB_EXTRA_API_H_
#define _MC_FB_EXTRA_API_H_

#include "zmc_types_def.h"
#include "zmc_enum_cdef.h"
#include "mc_ref_def.h"
#include "AxisRef.h"
#include "zmc_common.h"

#ifndef AW_PLC_NATIVE_ALIGN
#define AW_PLC_NATIVE_ALIGN 8
#endif

#ifdef __cplusplus
extern "C" {
#endif
class McGetTappetValue;
class McCamIn;

ZMC_DLLPORT int setTappetFromCamIn(McGetTappetValue*, McCamIn*);

#ifdef __cplusplus
}
#endif

#endif