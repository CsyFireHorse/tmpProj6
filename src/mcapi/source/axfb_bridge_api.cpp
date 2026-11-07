#include "axfb_bridge_api.h"

#include <cstddef>
#include <cstring>
#include <new>
#include <string>
#include <vector>

#include "bridge/AxisFbBridge.h"
#include "bridge/AxisFbBridgeWithThread.h"
#include "common/container/FbCmdQueue.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "motbuf/MotionCmdQueueBase.hpp"

struct AxFbBridgeWrap {
    zmc::AxisFbBridge bridge;
};

static_assert(AXFB_BRIDGE_PACKET_PAYLOAD_SIZE == zmc::MAX_PAYLOAD_SIZE,
              "C bridge packet payload size must match the C++ queue packet payload size");
static_assert(sizeof(AxFbBridgePacket) == sizeof(zmc::CmdPacket),
              "C bridge packet layout must match CmdPacket layout");
static_assert(offsetof(AxFbBridgePacket, payload) == offsetof(zmc::CmdPacket, payload),
              "C bridge packet payload offset must match CmdPacket payload offset");
static_assert(sizeof(AxFbIoMetadataEntry) == sizeof(zmc::IoMetadataEntry),
              "C bridge IO metadata layout must match IoMetadataEntry layout");

namespace {

bool valid_packet(const AxFbBridgePacket* packet) {
    return packet && packet->payload_len <= AXFB_BRIDGE_PACKET_PAYLOAD_SIZE;
}

void copy_to_cmd_packet(const AxFbBridgePacket& src, zmc::CmdPacket& dst) {
    dst.seq = src.seq;
    dst.payload_len = src.payload_len;
    std::memcpy(dst.payload, src.payload, sizeof(dst.payload));
}

void copy_from_status_packet(const zmc::StatusPacket& src, AxFbBridgePacket& dst) {
    dst.seq = src.seq;
    dst.payload_len = src.payload_len;
    std::memcpy(dst.payload, src.payload, sizeof(dst.payload));
}

zmc::MotionCmdQueueBase* queue_of(axfb_bridge_handle_t handle) {
    if (!handle) {
        return nullptr;
    }
    return &handle->bridge.command_interface();
}

}  // namespace

int axfb_bridge_create(axfb_bridge_handle_t* out) {
    if (!out) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    *out = nullptr;
    *out = new (std::nothrow) AxFbBridgeWrap();
    return *out ? 0 : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_bridge_configure(axfb_bridge_handle_t handle, const char* ini_path,
                          const char* ini_content, unsigned int axis_num) {
    if (!handle) {
        return ZMC_ERR_BRIDGE_INVALID_HANDLE;
    }
    return handle->bridge.configure(ini_path, ini_content, axis_num);
}

int axfb_bridge_load_eni(axfb_bridge_handle_t handle, const char* eni_path) {
    if (!handle) {
        return ZMC_ERR_BRIDGE_INVALID_HANDLE;
    }
    return handle->bridge.load_eni(eni_path);
}

int axfb_bridge_reset(axfb_bridge_handle_t handle) {
    if (!handle) {
        return ZMC_ERR_BRIDGE_INVALID_HANDLE;
    }
    static_assert(sizeof(zmc::AxisFbBridgeWithThread) == sizeof(zmc::AxisFbBridge),
                  "AxisFbBridgeWithThread must equal to AxisFbBridge");
    return ((zmc::AxisFbBridgeWithThread*)(&handle->bridge))->reset();
}

void axfb_bridge_destroy(axfb_bridge_handle_t handle) { delete handle; }

int axfb_bridge_bind_sdo(axfb_bridge_handle_t handle, void* sdo_rt) {
    (void)handle;
    (void)sdo_rt;
    if (!handle) {
        return ZMC_ERR_BRIDGE_INVALID_HANDLE;
    }
    return ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_bridge_bind_sdo_adapter(axfb_bridge_handle_t handle, const SdoCAdapter* adapter) {
    if (!handle) {
        return ZMC_ERR_BRIDGE_INVALID_HANDLE;
    }
    return handle->bridge.bind_sdo_adapter(adapter);
}

int axfb_bridge_bind_ecat_state_adapter(axfb_bridge_handle_t handle,
                                        const EcatStateAdapter* adapter) {
    if (!handle) {
        return ZMC_ERR_BRIDGE_INVALID_HANDLE;
    }
    return handle->bridge.bind_ecat_state_adapter(adapter);
}

void* axfb_bridge_native_sdo(axfb_bridge_handle_t handle) {
    if (!handle) {
        return nullptr;
    }
    return handle->bridge.native_sdo();
}

int axfb_bridge_start_sdo(axfb_bridge_handle_t handle) {
    if (!handle) {
        return ZMC_ERR_BRIDGE_INVALID_HANDLE;
    }
    static_assert(sizeof(zmc::AxisFbBridgeWithThread) == sizeof(zmc::AxisFbBridge),
                  "AxisFbBridgeWithThread must equal to AxisFbBridge");
    return ((zmc::AxisFbBridgeWithThread*)(&handle->bridge))->start_sdo();
}

int axfb_bridge_stop_sdo(axfb_bridge_handle_t handle) {
    if (!handle) {
        return ZMC_ERR_BRIDGE_INVALID_HANDLE;
    }
    return ((zmc::AxisFbBridgeWithThread*)(&handle->bridge))->stop_sdo();
}

int axfb_bridge_register_opmode_watches(axfb_bridge_handle_t handle, uint8_t opmode) {
    if (!handle) {
        return ZMC_ERR_BRIDGE_INVALID_HANDLE;
    }
    return handle->bridge.register_opmode_watches(opmode);
}

int axfb_bridge_apply_pio_layout(axfb_bridge_handle_t handle, const ZmcPioOffsetLayout* layout) {
    if (!handle) {
        return ZMC_ERR_BRIDGE_INVALID_HANDLE;
    }
    return handle->bridge.apply_pio_layout(layout);
}

int axfb_bridge_get_io_metadata(axfb_bridge_handle_t handle, AxFbIoMetadataEntry* out,
                                uint32_t* inout_len) {
    if (!handle || !inout_len) {
        return !handle ? ZMC_ERR_BRIDGE_INVALID_HANDLE : ZMC_ERR_BRIDGE_INVALID_ARG;
    }

    std::vector<zmc::IoMetadataEntry> native;
    const int ret = handle->bridge.get_io_metadata(native);
    if (ret != ZMC_NO_ERROR) {
        *inout_len = 0;
        return ret;
    }

    const uint32_t count = static_cast<uint32_t>(native.size());
    if (*inout_len < count || (count > 0u && !out)) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    if (count > 0u) {
        std::memcpy(out, native.data(), count * sizeof(AxFbIoMetadataEntry));
    }
    *inout_len = count;
    return ZMC_NO_ERROR;
}

int axfb_bridge_servo_count(axfb_bridge_handle_t handle, int* out) {
    if (!handle || !out) {
        return !handle ? ZMC_ERR_BRIDGE_INVALID_HANDLE : ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    *out = handle->bridge.servo_count();
    return 0;
}

int axfb_bridge_pdo_layout_ready(axfb_bridge_handle_t handle, int* out) {
    if (!handle || !out) {
        return !handle ? ZMC_ERR_BRIDGE_INVALID_HANDLE : ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    *out = handle->bridge.pdo_layout_ready() ? 1 : 0;
    return 0;
}

int axfb_bridge_auto_bind(axfb_bridge_handle_t handle, int bind_begin, int servo_num) {
    if (!handle) {
        return ZMC_ERR_BRIDGE_INVALID_HANDLE;
    }
    return handle->bridge.auto_bind(bind_begin, servo_num);
}

int axfb_bridge_manual_bind(axfb_bridge_handle_t handle, int axis_count, const int* bind_axis_arr,
                            int servo_num) {
    if (!handle || axis_count < 0 || (axis_count > 0 && !bind_axis_arr)) {
        return !handle ? ZMC_ERR_BRIDGE_INVALID_HANDLE : ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    return handle->bridge.manual_bind(axis_count, bind_axis_arr, servo_num);
}

int axfb_bridge_run_cycle(axfb_bridge_handle_t handle, const void* inp, void* oup) {
    if (!handle || !inp || !oup) {
        return !handle ? ZMC_ERR_BRIDGE_INVALID_HANDLE : ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    return handle->bridge.run_cycle(inp, oup);
}

int axfb_bridge_run_cycle_parsed(axfb_bridge_handle_t handle, const void* proc_img_inp,
                                 void* proc_img_oup) {
    if (!handle || !proc_img_inp || !proc_img_oup) {
        return !handle ? ZMC_ERR_BRIDGE_INVALID_HANDLE : ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    return handle->bridge.run_cycle_parsed(static_cast<const zmc::proc_img_i_addr*>(proc_img_inp),
                                           static_cast<zmc::proc_img_o_addr*>(proc_img_oup));
}

int axfb_bridge_axis_count(axfb_bridge_handle_t handle, int* out) {
    if (!handle || !out) {
        return !handle ? ZMC_ERR_BRIDGE_INVALID_HANDLE : ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    *out = handle->bridge.axis_count();
    return 0;
}

int axfb_bridge_get_address(axfb_bridge_handle_t handle, int axis_idx, int* out) {
    if (!handle || !out) {
        return !handle ? ZMC_ERR_BRIDGE_INVALID_HANDLE : ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    *out = handle->bridge.get_address(axis_idx);
    return 0;
}

int axfb_bridge_get_actual_position(axfb_bridge_handle_t handle, int axis_idx, int* out) {
    if (!handle || !out) {
        return !handle ? ZMC_ERR_BRIDGE_INVALID_HANDLE : ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    *out = handle->bridge.get_actual_position(axis_idx);
    return 0;
}

int axfb_bridge_push_cmd_packet(axfb_bridge_handle_t handle, const AxFbBridgePacket* packet) {
    zmc::MotionCmdQueueBase* queue = queue_of(handle);
    if (!queue || !valid_packet(packet)) {
        if (!handle) {
            return ZMC_ERR_BRIDGE_INVALID_HANDLE;
        }
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    zmc::CmdPacket cmd;
    copy_to_cmd_packet(*packet, cmd);
    return queue->push_cmd_packet(cmd) ? 0 : ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED;
}

int axfb_bridge_pop_status_packet(axfb_bridge_handle_t handle, AxFbBridgePacket* packet) {
    zmc::MotionCmdQueueBase* queue = queue_of(handle);
    if (!queue || !packet) {
        return !handle ? ZMC_ERR_BRIDGE_INVALID_HANDLE : ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    zmc::StatusPacket status;
    if (!queue->pop_status_packet(status)) {
        return ZMC_ERR_MOTCMD_CMD_RESULTS_EMPTY;
    }
    copy_from_status_packet(status, *packet);
    return 0;
}

int axfb_bridge_consume_exit_flag(axfb_bridge_handle_t handle) {
    zmc::MotionCmdQueueBase* queue = queue_of(handle);
    if (!queue) {
        return ZMC_ERR_BRIDGE_INVALID_HANDLE;
    }
    return queue->consume_exit_flag() ? 1 : 0;
}

#if 0
int axfb_bridge_cmd_queue_size(axfb_bridge_handle_t handle, size_t* out) {
    zmc::MotionCmdQueueBase* queue = queue_of(handle);
    if (!queue || !out) {
        return !queue ? ZMC_ERR_BRIDGE_INVALID_HANDLE : ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    *out = queue->cmd_queue_size();
    return 0;
}

int axfb_bridge_cmd_queue_empty(axfb_bridge_handle_t handle, int* out) {
    zmc::MotionCmdQueueBase* queue = queue_of(handle);
    if (!queue || !out) {
        return !queue ? ZMC_ERR_BRIDGE_INVALID_HANDLE : ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    *out = queue->cmd_queue_empty() ? 1 : 0;
    return 0;
}

int axfb_bridge_cmd_queue_full(axfb_bridge_handle_t handle, int* out) {
    zmc::MotionCmdQueueBase* queue = queue_of(handle);
    if (!queue || !out) {
        return !queue ? ZMC_ERR_BRIDGE_INVALID_HANDLE : ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    *out = queue->cmd_queue_full() ? 1 : 0;
    return 0;
}

int axfb_bridge_status_queue_size(axfb_bridge_handle_t handle, size_t* out) {
    zmc::MotionCmdQueueBase* queue = queue_of(handle);
    if (!queue || !out) {
        return !queue ? ZMC_ERR_BRIDGE_INVALID_HANDLE : ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    *out = queue->status_queue_size();
    return 0;
}

int axfb_bridge_status_queue_empty(axfb_bridge_handle_t handle, int* out) {
    zmc::MotionCmdQueueBase* queue = queue_of(handle);
    if (!queue || !out) {
        return !queue ? ZMC_ERR_BRIDGE_INVALID_HANDLE : ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    *out = queue->status_queue_empty() ? 1 : 0;
    return 0;
}

int axfb_bridge_status_queue_full(axfb_bridge_handle_t handle, int* out) {
    zmc::MotionCmdQueueBase* queue = queue_of(handle);
    if (!queue || !out) {
        return !queue ? ZMC_ERR_BRIDGE_INVALID_HANDLE : ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    *out = queue->status_queue_full() ? 1 : 0;
    return 0;
}
#endif
