/* Typed command-packet C API implementation over MCCmdPacket. */
#include "axfb_command_packet_api.h"

#include "common/zmc/fbt_errorcode_def.h"
#include "common/utils/double2frac.hpp"
#include "qcmd/MotionCmdPacket.hpp"

namespace {

zmc::MotionCmdPacket* as_cmd_packet(MCCmdPacket* packet) {
    static_assert(sizeof(MCCmdPacket) == sizeof(zmc::MotionCmdPacket),
                  "MCCmdPacket must match MotionCmdPacket layout");
    return reinterpret_cast<zmc::MotionCmdPacket*>(packet);
}

const zmc::MotionCmdPacket* as_cmd_packet_const(const MCCmdPacket* packet) {
    return reinterpret_cast<const zmc::MotionCmdPacket*>(packet);
}

bool valid_axis_count(int n) { return n > 0 && static_cast<unsigned>(n) <= AXFB_CMD_MAX_AXES; }

}  // namespace

int axfb_cmd_packet_init(MCCmdPacket* packet) {
    if (!packet) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    // Clear wire fields; MotionCmdPacket::reset() appends FBM_RESET instead.
    static_cast<zmc::MotionPacket*>(as_cmd_packet(packet))->reset();
    return ZMC_NO_ERROR;
}

int axfb_cmd_packet_reset(MCCmdPacket* packet) {
    return packet ? as_cmd_packet(packet)->reset() : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_can_append(const MCCmdPacket* packet, uint16_t param_len, int* out) {
    if (!packet || !out) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    *out = as_cmd_packet_const(packet)->canAppend(param_len) ? 1 : 0;
    return ZMC_NO_ERROR;
}

int axfb_cmd_packet_set_axis(MCCmdPacket* packet, int axis) {
    return packet ? as_cmd_packet(packet)->setAxis(axis) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_set_base_axis(MCCmdPacket* packet, int axis) {
    return packet ? as_cmd_packet(packet)->setBaseAxis(axis) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_set_master_axis(MCCmdPacket* packet, int axis) {
    return packet ? as_cmd_packet(packet)->setMasterAxis(axis) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_set_units(MCCmdPacket* packet, double units) {
    if (!packet) return ZMC_ERR_BRIDGE_INVALID_ARG;
    // The public C ABI keeps a double input; the ratio payload carries num/den.
    // GLM-5.3 (opencode), 2026-10-23.
    zmc::NumDenUnion numDen = {{1, 1}};
    zmc::doubleToFraction(units, numDen.num, numDen.den);
    return as_cmd_packet(packet)->setUnitsNumDen(numDen.num, numDen.den);
}

int axfb_cmd_packet_set_speed(MCCmdPacket* packet, double speed) {
    return packet ? as_cmd_packet(packet)->setSpeed(speed) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_set_accel(MCCmdPacket* packet, double accel) {
    return packet ? as_cmd_packet(packet)->setAccel(accel) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_set_decel(MCCmdPacket* packet, double decel) {
    return packet ? as_cmd_packet(packet)->setDecel(decel) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_set_jerk(MCCmdPacket* packet, double jerk) {
    return packet ? as_cmd_packet(packet)->setJerk(jerk) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_set_fast_decel(MCCmdPacket* packet, double fast_decel) {
    return packet ? as_cmd_packet(packet)->setFastDecel(fast_decel) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_set_speed_ratio(MCCmdPacket* packet, double ratio) {
    return packet ? as_cmd_packet(packet)->setSpeedRatio(ratio) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_set_direction(MCCmdPacket* packet, int direction) {
    return packet ? as_cmd_packet(packet)->setDirection(direction) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_set_buffer_mode(MCCmdPacket* packet, int buffer_mode) {
    return packet ? as_cmd_packet(packet)->setBufferMode(buffer_mode) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_set_flow_config(MCCmdPacket* packet, uint32_t period, uint32_t batch_size,
                                    uint8_t verbose_flag) {
    if (!packet) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    FlowCtrlConfig config = {};
    config.period = period;
    config.batchSize = batch_size;
    config.verboseFlag = verbose_flag;
    return as_cmd_packet(packet)->setFlowConfig(config);
}

int axfb_cmd_packet_power_on(MCCmdPacket* packet) {
    return packet ? as_cmd_packet(packet)->powerOn() : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_power_off(MCCmdPacket* packet) {
    return packet ? as_cmd_packet(packet)->powerOff() : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_move_abs(MCCmdPacket* packet, double position) {
    return packet ? as_cmd_packet(packet)->moveAbs(1, &position) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_move_rel(MCCmdPacket* packet, double distance) {
    return packet ? as_cmd_packet(packet)->moveRel(1, &distance) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_move_add(MCCmdPacket* packet, int pos_len, const double* pos_arr) {
    if (!packet || !pos_arr || !valid_axis_count(pos_len)) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    return as_cmd_packet(packet)->moveAdd(pos_len, const_cast<double*>(pos_arr));
}

int axfb_cmd_packet_home(MCCmdPacket* packet, double position) {
    return packet ? as_cmd_packet(packet)->home(position) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_homing(MCCmdPacket* packet) {
    return packet ? as_cmd_packet(packet)->homing() : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_gear_in(MCCmdPacket* packet) {
    return packet ? as_cmd_packet(packet)->gearIn() : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_gear_out(MCCmdPacket* packet) {
    return packet ? as_cmd_packet(packet)->gearOut() : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_set_position(MCCmdPacket* packet, double position) {
    return packet ? as_cmd_packet(packet)->setPosition(position) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_datum(MCCmdPacket* packet, int mode) {
    return packet ? as_cmd_packet(packet)->datum(mode) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_move_velocity(MCCmdPacket* packet, int direction) {
    return packet ? as_cmd_packet(packet)->moveVelocity(direction) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_forward(MCCmdPacket* packet) {
    return packet ? as_cmd_packet(packet)->forward() : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_reverse(MCCmdPacket* packet) {
    return packet ? as_cmd_packet(packet)->reverse() : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_stop(MCCmdPacket* packet) {
    return packet ? as_cmd_packet(packet)->stop() : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_halt(MCCmdPacket* packet) {
    return packet ? as_cmd_packet(packet)->halt() : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_multi_move_abs(MCCmdPacket* packet, int pos_len, const double* pos_arr) {
    if (!packet || !pos_arr || !valid_axis_count(pos_len)) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    return as_cmd_packet(packet)->multiMoveAbs(pos_len, pos_arr);
}

int axfb_cmd_packet_multi_move_rel(MCCmdPacket* packet, int pos_len, const double* pos_arr) {
    if (!packet || !pos_arr || !valid_axis_count(pos_len)) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    return as_cmd_packet(packet)->multiMoveRel(pos_len, pos_arr);
}

int axfb_cmd_packet_group_select(MCCmdPacket* packet, int idx) {
    return packet ? as_cmd_packet(packet)->groupSelect(idx) : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_group_set_axes(MCCmdPacket* packet, int n, const int* axis_idx_arr) {
    if (!packet || !axis_idx_arr || !valid_axis_count(n)) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    return as_cmd_packet(packet)->groupSetAxes(n, axis_idx_arr);
}

int axfb_cmd_packet_group_enable(MCCmdPacket* packet) {
    return packet ? as_cmd_packet(packet)->groupEnable() : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_group_disable(MCCmdPacket* packet) {
    return packet ? as_cmd_packet(packet)->groupDisable() : ZMC_ERR_BRIDGE_INVALID_ARG;
}

int axfb_cmd_packet_group_reset(MCCmdPacket* packet) {
    return packet ? as_cmd_packet(packet)->groupReset() : ZMC_ERR_BRIDGE_INVALID_ARG;
}
