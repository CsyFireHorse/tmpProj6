/* Typed status-client C API implementation. */
#include "axfb_status_api.h"

#include <cstring>
#include <new>
#include <vector>

#include "common/zmc/fbt_errorcode_def.h"
#include "qcmd/MotionCmdMirror.hpp"

struct AxFbStatusClient {
    AxFbStatusClient() : mirror() {}

    zmc::MotionStatusMirror mirror;
};

int axfb_status_create(axfb_status_handle_t* out) {
    if (!out) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    *out = new (std::nothrow) AxFbStatusClient();
    return *out ? ZMC_NO_ERROR : ZMC_ERR_BRIDGE_INVALID_ARG;
}

void axfb_status_destroy(axfb_status_handle_t handle) { delete handle; }

int axfb_status_apply_packet(axfb_status_handle_t handle, const MCStatusPacket* packet) {
    if (!handle || !packet) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    static_assert(sizeof(MCStatusPacket) == sizeof(zmc::StatusPacket),
                  "MCStatusPacket must match StatusPacket layout");
    zmc::StatusPacket status = {};
    std::memcpy(&status, packet, sizeof(status));
    handle->mirror.applyPacket(status);
    return ZMC_NO_ERROR;
}

int axfb_status_reset(axfb_status_handle_t handle) {
    if (!handle) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    handle->mirror = zmc::MotionStatusMirror();
    return ZMC_NO_ERROR;
}

int axfb_status_get_axis_state(axfb_status_handle_t handle, int axis_id, AxisState* out) {
    if (!handle || !out) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    return handle->mirror.getAxisState(axis_id, *out);
}

int axfb_status_get_axis_state2(axfb_status_handle_t handle, int axis_id, AxisState2* out) {
    if (!handle || !out) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    return handle->mirror.getAxisState2(axis_id, *out);
}

int axfb_status_get_flow_status(axfb_status_handle_t handle, FlowCtrlStatus* out) {
    if (!handle || !out) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    return handle->mirror.getFlowStatus(*out);
}

int axfb_status_get_cmd_results(axfb_status_handle_t handle, CmdResult* out,
                                uint32_t* inout_len) {
    if (!handle || !inout_len) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    std::vector<CmdResult> native;
    const int ret = handle->mirror.getCmdResults(native);
    if (ret != ZMC_NO_ERROR) {
        *inout_len = 0;
        return ret;
    }
    const uint32_t count = static_cast<uint32_t>(native.size());
    if (*inout_len < count) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    if (count > 0 && !out) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    for (uint32_t i = 0; i < count; ++i) {
        out[i] = native[i];
    }
    *inout_len = count;
    return ZMC_NO_ERROR;
}

int axfb_status_get_io_results(axfb_status_handle_t handle, CmdResult* out,
                               uint32_t* inout_len) {
    if (!handle || !inout_len) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    std::vector<CmdResult> native;
    const int ret = handle->mirror.getIoResults(native);
    if (ret != ZMC_NO_ERROR) {
        *inout_len = 0;
        return ret;
    }
    const uint32_t count = static_cast<uint32_t>(native.size());
    if (*inout_len < count) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    if (count > 0 && !out) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    for (uint32_t i = 0; i < count; ++i) {
        out[i] = native[i];
    }
    *inout_len = count;
    return ZMC_NO_ERROR;
}

int axfb_status_current_tick_for_axis(axfb_status_handle_t handle, int axis, uint32_t* out) {
    if (!handle || !out) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    *out = handle->mirror.currentTickForAxis(axis);
    return ZMC_NO_ERROR;
}

int axfb_status_current_axis_state(axfb_status_handle_t handle, int axis, uint8_t* out_state,
                                   int* out_has_state) {
    if (!handle || !out_state || !out_has_state) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    bool has_state = false;
    *out_state = handle->mirror.currentAxisState(axis, has_state);
    *out_has_state = has_state ? 1 : 0;
    return ZMC_NO_ERROR;
}

int axfb_status_ack_seq(axfb_status_handle_t handle, uint32_t* out) {
    if (!handle || !out) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    *out = handle->mirror.ackSeq();
    return ZMC_NO_ERROR;
}

int axfb_status_ack_tick(axfb_status_handle_t handle, uint32_t* out) {
    if (!handle || !out) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    *out = handle->mirror.ackTick();
    return ZMC_NO_ERROR;
}
