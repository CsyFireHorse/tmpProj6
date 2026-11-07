#ifndef AXFB_BRIDGE_API_H
#define AXFB_BRIDGE_API_H

#include <stddef.h>
#include <stdint.h>

#include "common/zmc/mc_motion_packet.h"
#include "zmc_common.h"

struct ZmcPioOffsetLayout;
typedef struct SdoCAdapter SdoCAdapter;
typedef struct EcatStateAdapter EcatStateAdapter;

#ifdef __cplusplus
extern "C" {
#endif

#define AXFB_BRIDGE_PACKET_PAYLOAD_SIZE MC_CMD_PACKET_PAYLOAD_MAX

typedef struct AxFbBridgeWrap* axfb_bridge_handle_t;

/* Bridge packet keeps the public API name while reusing the shared packet layout. */
typedef MCCmdPacket AxFbBridgePacket;

/* C representation of one configured IO alias metadata entry. */
typedef struct AxFbIoMetadataEntry {
    uint32_t mask;
    uint8_t kind;
    uint8_t source_kind;
    uint16_t io_number;
    uint16_t index;
    uint8_t slave_idx;
    uint8_t subindex;
    uint16_t bit_offset;
    uint8_t bit_size;
    uint8_t is_input;
} AxFbIoMetadataEntry;

ZMC_DLLPORT int axfb_bridge_create(axfb_bridge_handle_t* out);
ZMC_DLLPORT int axfb_bridge_configure(axfb_bridge_handle_t handle, const char* ini_path,
                                      const char* ini_content, unsigned int axis_num);
ZMC_DLLPORT int axfb_bridge_load_eni(axfb_bridge_handle_t handle, const char* eni_path);
ZMC_DLLPORT int axfb_bridge_reset(axfb_bridge_handle_t handle);
ZMC_DLLPORT void axfb_bridge_destroy(axfb_bridge_handle_t handle);

// SDO runtime lives inside the bridge; bind only the transport adapter.
ZMC_DLLPORT int axfb_bridge_bind_sdo(axfb_bridge_handle_t handle, void* sdo_rt);
ZMC_DLLPORT int axfb_bridge_bind_sdo_adapter(axfb_bridge_handle_t handle,
                                             const SdoCAdapter* adapter);
ZMC_DLLPORT int axfb_bridge_bind_ecat_state_adapter(axfb_bridge_handle_t handle,
                                                    const EcatStateAdapter* adapter);
ZMC_DLLPORT void* axfb_bridge_native_sdo(axfb_bridge_handle_t handle);
ZMC_DLLPORT int axfb_bridge_start_sdo(axfb_bridge_handle_t handle);
ZMC_DLLPORT int axfb_bridge_stop_sdo(axfb_bridge_handle_t handle);
ZMC_DLLPORT int axfb_bridge_register_opmode_watches(axfb_bridge_handle_t handle, uint8_t opmode);
ZMC_DLLPORT int axfb_bridge_apply_pio_layout(axfb_bridge_handle_t handle,
                                             const ZmcPioOffsetLayout* layout);
ZMC_DLLPORT int axfb_bridge_get_io_metadata(axfb_bridge_handle_t handle,
                                            AxFbIoMetadataEntry* out, uint32_t* inout_len);
ZMC_DLLPORT int axfb_bridge_servo_count(axfb_bridge_handle_t handle, int* out);
ZMC_DLLPORT int axfb_bridge_pdo_layout_ready(axfb_bridge_handle_t handle, int* out);
ZMC_DLLPORT int axfb_bridge_auto_bind(axfb_bridge_handle_t handle, int bind_begin, int servo_num);
ZMC_DLLPORT int axfb_bridge_manual_bind(axfb_bridge_handle_t handle, int axis_count,
                                        const int* bind_axis_arr, int servo_num);

ZMC_DLLPORT int axfb_bridge_run_cycle(axfb_bridge_handle_t handle, const void* inp, void* oup);
ZMC_DLLPORT int axfb_bridge_run_cycle_parsed(axfb_bridge_handle_t handle, const void* proc_img_inp,
                                             void* proc_img_oup);

ZMC_DLLPORT int axfb_bridge_axis_count(axfb_bridge_handle_t handle, int* out);
ZMC_DLLPORT int axfb_bridge_get_address(axfb_bridge_handle_t handle, int axis_idx, int* out);
ZMC_DLLPORT int axfb_bridge_get_actual_position(axfb_bridge_handle_t handle, int axis_idx,
                                                int* out);

ZMC_DLLPORT int axfb_bridge_push_cmd_packet(axfb_bridge_handle_t handle,
                                            const AxFbBridgePacket* packet);
ZMC_DLLPORT int axfb_bridge_pop_status_packet(axfb_bridge_handle_t handle,
                                              AxFbBridgePacket* packet);
ZMC_DLLPORT int axfb_bridge_consume_exit_flag(axfb_bridge_handle_t handle);
ZMC_DLLPORT int axfb_bridge_cmd_queue_size(axfb_bridge_handle_t handle, size_t* out);
ZMC_DLLPORT int axfb_bridge_cmd_queue_empty(axfb_bridge_handle_t handle, int* out);
ZMC_DLLPORT int axfb_bridge_cmd_queue_full(axfb_bridge_handle_t handle, int* out);
ZMC_DLLPORT int axfb_bridge_status_queue_size(axfb_bridge_handle_t handle, size_t* out);
ZMC_DLLPORT int axfb_bridge_status_queue_empty(axfb_bridge_handle_t handle, int* out);
ZMC_DLLPORT int axfb_bridge_status_queue_full(axfb_bridge_handle_t handle, int* out);

#ifdef __cplusplus
}
#endif

#endif  // AXFB_BRIDGE_API_H
