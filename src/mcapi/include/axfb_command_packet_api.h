/* Typed command-packet C API. */
#ifndef AXFB_COMMAND_PACKET_API_H
#define AXFB_COMMAND_PACKET_API_H

#include <stdint.h>

#include "common/zmc/mc_motion_packet.h"
#include "zmc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AXFB_CMD_MAX_AXES 64u

/* Clear wire fields (MotionPacket::reset). */
ZMC_DLLPORT int axfb_cmd_packet_init(MCCmdPacket* packet);
/* Append FBM_RESET item (MotionCmdPacket::reset). */
ZMC_DLLPORT int axfb_cmd_packet_reset(MCCmdPacket* packet);

ZMC_DLLPORT int axfb_cmd_packet_can_append(const MCCmdPacket* packet, uint16_t param_len,
                                           int* out);
ZMC_DLLPORT int axfb_cmd_packet_set_axis(MCCmdPacket* packet, int axis);
ZMC_DLLPORT int axfb_cmd_packet_set_base_axis(MCCmdPacket* packet, int axis);
ZMC_DLLPORT int axfb_cmd_packet_set_master_axis(MCCmdPacket* packet, int axis);
ZMC_DLLPORT int axfb_cmd_packet_set_units(MCCmdPacket* packet, double units);
ZMC_DLLPORT int axfb_cmd_packet_set_speed(MCCmdPacket* packet, double speed);
ZMC_DLLPORT int axfb_cmd_packet_set_accel(MCCmdPacket* packet, double accel);
ZMC_DLLPORT int axfb_cmd_packet_set_decel(MCCmdPacket* packet, double decel);
ZMC_DLLPORT int axfb_cmd_packet_set_jerk(MCCmdPacket* packet, double jerk);
ZMC_DLLPORT int axfb_cmd_packet_set_fast_decel(MCCmdPacket* packet, double fast_decel);
ZMC_DLLPORT int axfb_cmd_packet_set_speed_ratio(MCCmdPacket* packet, double ratio);
ZMC_DLLPORT int axfb_cmd_packet_set_direction(MCCmdPacket* packet, int direction);
ZMC_DLLPORT int axfb_cmd_packet_set_buffer_mode(MCCmdPacket* packet, int buffer_mode);
ZMC_DLLPORT int axfb_cmd_packet_set_flow_config(MCCmdPacket* packet, uint32_t period,
                                                uint32_t batch_size, uint8_t verbose_flag);
ZMC_DLLPORT int axfb_cmd_packet_power_on(MCCmdPacket* packet);
ZMC_DLLPORT int axfb_cmd_packet_power_off(MCCmdPacket* packet);
ZMC_DLLPORT int axfb_cmd_packet_move_abs(MCCmdPacket* packet, double position);
ZMC_DLLPORT int axfb_cmd_packet_move_rel(MCCmdPacket* packet, double distance);
ZMC_DLLPORT int axfb_cmd_packet_move_add(MCCmdPacket* packet, int pos_len, const double* pos_arr);
ZMC_DLLPORT int axfb_cmd_packet_home(MCCmdPacket* packet, double position);
ZMC_DLLPORT int axfb_cmd_packet_homing(MCCmdPacket* packet);
ZMC_DLLPORT int axfb_cmd_packet_gear_in(MCCmdPacket* packet);
ZMC_DLLPORT int axfb_cmd_packet_gear_out(MCCmdPacket* packet);
ZMC_DLLPORT int axfb_cmd_packet_set_position(MCCmdPacket* packet, double position);
ZMC_DLLPORT int axfb_cmd_packet_datum(MCCmdPacket* packet, int mode);
ZMC_DLLPORT int axfb_cmd_packet_move_velocity(MCCmdPacket* packet, int direction);
ZMC_DLLPORT int axfb_cmd_packet_forward(MCCmdPacket* packet);
ZMC_DLLPORT int axfb_cmd_packet_reverse(MCCmdPacket* packet);
ZMC_DLLPORT int axfb_cmd_packet_stop(MCCmdPacket* packet);
ZMC_DLLPORT int axfb_cmd_packet_halt(MCCmdPacket* packet);
ZMC_DLLPORT int axfb_cmd_packet_multi_move_abs(MCCmdPacket* packet, int pos_len,
                                               const double* pos_arr);
ZMC_DLLPORT int axfb_cmd_packet_multi_move_rel(MCCmdPacket* packet, int pos_len,
                                               const double* pos_arr);
ZMC_DLLPORT int axfb_cmd_packet_group_select(MCCmdPacket* packet, int idx);
ZMC_DLLPORT int axfb_cmd_packet_group_set_axes(MCCmdPacket* packet, int n,
                                               const int* axis_idx_arr);
ZMC_DLLPORT int axfb_cmd_packet_group_enable(MCCmdPacket* packet);
ZMC_DLLPORT int axfb_cmd_packet_group_disable(MCCmdPacket* packet);
ZMC_DLLPORT int axfb_cmd_packet_group_reset(MCCmdPacket* packet);

#ifdef __cplusplus
}
#endif

#endif  // AXFB_COMMAND_PACKET_API_H
