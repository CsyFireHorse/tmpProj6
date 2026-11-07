/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-17
@ArtifactStatusAuthority GPT-5.6 Sol
*/
#ifndef _ZMC_SDO_ADAPTER_H_
#define _ZMC_SDO_ADAPTER_H_

#include <stdint.h>

#include "ecat/ecat_state_adapter.h"

#ifdef __cplusplus
extern "C" {
#endif

/** SDO object identity for C adapter callbacks. */
typedef struct SdoCKey {
    uint16_t slave_id;
    uint16_t index;
    uint8_t subindex;
} SdoCKey;

typedef int32_t (*SdoCUploadFn)(void* ctx, const SdoCKey* key, uint32_t* len, void* data,
                               uint32_t timeout_ms);
typedef int32_t (*SdoCDownloadFn)(void* ctx, const SdoCKey* key, uint32_t len,
                                 const void* data, uint32_t timeout_ms);

/** SDO transport adapter only: no thread or lifecycle ownership. */
typedef struct SdoCAdapter {
    void* ctx;
    SdoCUploadFn upload;
    SdoCDownloadFn download;
    EcatCGetSlaveStateFn get_slave_state;
} SdoCAdapter;

#ifdef __cplusplus
}
#endif

#endif  // _ZMC_SDO_ADAPTER_H_
