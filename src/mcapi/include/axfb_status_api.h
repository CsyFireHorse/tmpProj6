/* Typed status-client C API. */
#ifndef AXFB_STATUS_API_H
#define AXFB_STATUS_API_H

#include <stdint.h>

#include "common/zmc/mc_motion_packet.h"
#include "motbuf/MotionCmdTypes.hpp"
#include "zmc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AxFbStatusClient* axfb_status_handle_t;

ZMC_DLLPORT int axfb_status_create(axfb_status_handle_t* out);
ZMC_DLLPORT void axfb_status_destroy(axfb_status_handle_t handle);
ZMC_DLLPORT int axfb_status_apply_packet(axfb_status_handle_t handle,
                                         const MCStatusPacket* packet);
ZMC_DLLPORT int axfb_status_reset(axfb_status_handle_t handle);
ZMC_DLLPORT int axfb_status_get_axis_state(axfb_status_handle_t handle, int axis_id,
                                           AxisState* out);
ZMC_DLLPORT int axfb_status_get_axis_state2(axfb_status_handle_t handle, int axis_id,
                                            AxisState2* out);
ZMC_DLLPORT int axfb_status_get_flow_status(axfb_status_handle_t handle, FlowCtrlStatus* out);
ZMC_DLLPORT int axfb_status_get_cmd_results(axfb_status_handle_t handle, CmdResult* out,
                                            uint32_t* inout_len);
ZMC_DLLPORT int axfb_status_get_io_results(axfb_status_handle_t handle, CmdResult* out,
                                           uint32_t* inout_len);
ZMC_DLLPORT int axfb_status_current_tick_for_axis(axfb_status_handle_t handle, int axis,
                                                  uint32_t* out);
ZMC_DLLPORT int axfb_status_current_axis_state(axfb_status_handle_t handle, int axis,
                                               uint8_t* out_state, int* out_has_state);
ZMC_DLLPORT int axfb_status_ack_seq(axfb_status_handle_t handle, uint32_t* out);
ZMC_DLLPORT int axfb_status_ack_tick(axfb_status_handle_t handle, uint32_t* out);

#ifdef __cplusplus
}
#endif

#endif  // AXFB_STATUS_API_H
