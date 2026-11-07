#include "mc_get_version.h"
#include "zal_version.h"
#include "errorCode2str.hpp"

extern "C" {
    static const char* const zal_versiont_str = ZAL_VERSION_STR;
    static const char* const zal_versiont_full_str = "version:" ZAL_VERSION_STR " author:" ZAL_AUTHOR " date:" ZAL_CODE_DATE;
    int zmc_get_version_major() {
        return ZAL_VERSION_MAJOR;
    }

    int zmc_get_version_minor() {
        return ZAL_VERSION_MINOR;
    }

    int zmc_get_version_patch() {
        return ZAL_VERSION_PATCH;
    }
    int zmc_get_version_number(){
        return ZAL_VERSION_MAJOR * 10000 + ZAL_VERSION_MINOR * 100 + ZAL_VERSION_PATCH;
    }
    const char* zmc_get_version() {
        return zal_versiont_str;
    }
    const char* zmc_get_full_version() {
        return zal_versiont_full_str;
    }

    const char* zmc_get_description() {
        return ZAL_DESCRIPTION;
    }
    const char* zmc_get_error_code_string(int val) {
        return MC_ERROR_CODEToString((MC_ERROR_CODE) val); 
    }

}