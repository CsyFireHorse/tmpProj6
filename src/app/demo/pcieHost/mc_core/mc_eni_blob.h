#ifndef MC_PCIEHOST_MC_ENI_BLOB_H
#define MC_PCIEHOST_MC_ENI_BLOB_H

/*
 * ENI descriptor blob cache.
 *
 * mc_pdo_thread asks the master for ENI info each time the bus reaches
 * OP and then calls mc_eni_load_pending_descriptor() once to snapshot
 * the descriptor. Today the blob just stores PI byte sizes / cycle time
 * from MC_EcatGetEniInfo; later iterations can expand it with parsed
 * PDO maps and servo masks.
 */

#include "mc_zecm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Refresh blob from current MC_EcatGetEniInfo. Returns 0 on success. */
int mc_eni_load_pending_descriptor(void);

/* Read-only view of the most recently loaded blob. Returns NULL if no
 * descriptor has been loaded yet. */
const ecat_eni_info_ex* mc_eni_blob_get(void);

/* Reset the blob. Useful when the bus drops out of OP. */
void mc_eni_blob_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* MC_PCIEHOST_MC_ENI_BLOB_H */
