/*
 * @Description: Transitional host errno offset for mst_utils thin wrappers.
 *                Shared utils return base ZMC_ERR_*; mst boundary adds fixed offset.
 */

#ifndef ZMC_UTILS_ERRNO_H
#define ZMC_UTILS_ERRNO_H

#include "common/zmc/mc_errno.h"

#ifndef ZMC_ERR_UTILS_HOST_OFFSET
#define ZMC_ERR_UTILS_HOST_OFFSET 1000
#endif

/* Map shared-utils result to host-facing MCCommResult at mst_utils return boundary. */
static inline MCCommResult zmc_utils_errno_to_host(MCCommResult code)
{
    if (code == MC_COMM_S_OK) {
        return code;
    }
    return (MCCommResult)((int)code + ZMC_ERR_UTILS_HOST_OFFSET);
}

#endif /* ZMC_UTILS_ERRNO_H */
