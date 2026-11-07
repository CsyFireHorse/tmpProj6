/*
@ArtifactStatus garbage
@ArtifactStatusDate 2026-09-17
@ArtifactStatusAuthority GPT-5.6 Sol
*/
#ifndef _ZMC_ECAT_STATE_ADAPTER_H_
#define _ZMC_ECAT_STATE_ADAPTER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** EtherCAT state callbacks without thread or lifecycle ownership. */
typedef int32_t (*EcatCGetSlaveCountFn)(void *ctx, uint16_t *slave_count);
typedef int32_t (*EcatCGetSlaveStateFn)(void *ctx, uint16_t slave_id,
                                        uint8_t *slave_state);
typedef int32_t (*EcatCGetMasterStateFn)(void *ctx, uint8_t *master_state);

/** Adapter used exclusively by synchronous EtherCAT state queries. */
typedef struct EcatStateAdapter {
  void *ctx;
  EcatCGetSlaveCountFn get_slave_count;
  EcatCGetSlaveStateFn get_slave_state;
  EcatCGetMasterStateFn get_master_state;
} EcatStateAdapter;

#ifdef __cplusplus
}
#endif

#endif // _ZMC_ECAT_STATE_ADAPTER_H_
