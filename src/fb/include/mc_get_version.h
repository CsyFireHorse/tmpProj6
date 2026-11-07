#ifndef _ZMC_GET_VERSION_H_
#define _ZMC_GET_VERSION_H_


#include "zmc_common.h"
#ifdef __cplusplus
extern "C" {
#endif

ZMC_DLLPORT int zmc_get_version_major();
ZMC_DLLPORT int zmc_get_version_minor();
ZMC_DLLPORT int zmc_get_version_patch();
ZMC_DLLPORT int zmc_get_version_number();
ZMC_DLLPORT const char* zmc_get_version();
ZMC_DLLPORT const char* zmc_get_full_version();
ZMC_DLLPORT const char* zmc_get_description();
ZMC_DLLPORT const char* zmc_get_error_code_string(int val);

#ifdef __cplusplus
}
#endif

#endif