#ifndef MC_SDO_API_H
#define MC_SDO_API_H

#include <stdint.h>

#include "sdo/sdo_adapter.h"
#include "zmc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct McSdoRuntime* mc_sdo_handle_t;

typedef SdoCKey McSdoKey;
typedef SdoCUploadFn McSdoUploadFn;
typedef SdoCDownloadFn McSdoDownloadFn;
typedef SdoCAdapter McSdoAdapter;

/** Create runtime and start the locked SDO worker pthread (runs until destroy). */
ZMC_DLLPORT mc_sdo_handle_t mc_sdo_create(void);
ZMC_DLLPORT void mc_sdo_destroy(mc_sdo_handle_t handle);

/** User-controlled idle: worker sleeps and skips SDO processing while paused. */
ZMC_DLLPORT void mc_sdo_pause(mc_sdo_handle_t handle);
ZMC_DLLPORT void mc_sdo_resume(mc_sdo_handle_t handle);
ZMC_DLLPORT void mc_sdo_clear_watches(mc_sdo_handle_t handle);

/** Bind a C adapter (upload/download only). Returns 0 on success, -1 on invalid args. */
ZMC_DLLPORT int mc_sdo_bind_adapter(mc_sdo_handle_t handle, const McSdoAdapter* adapter);

/** Borrow the underlying zmc::SdoRt* for axfb_bridge_bind_sdo. */
ZMC_DLLPORT void* mc_sdo_native(mc_sdo_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* MC_SDO_API_H */
