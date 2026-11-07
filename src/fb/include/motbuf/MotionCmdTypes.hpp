#ifndef _ZMC_MOTION_CMD_TYPES_HPP_
#define _ZMC_MOTION_CMD_TYPES_HPP_

// Wire payload layouts for CmdPacket/StatusPacket items (MC and Server share).

#include <stdint.h>
#include <stddef.h>

#include "motbuf/MotionCmdResult.h"
#include "common/zmc/mc_motion_packet.h"
#include "common/zmc/zmc_predef.h"

struct AxisState {
    uint8_t axis_id;
    uint8_t axis_state;  // Low nibble is MC_AXIS_STATES; high nibble is its explicit detail.
    uint16_t error_id;
    uint32_t tick;
    double act_pos;
    double set_pos;
};

struct AxisState2 {
    uint8_t axis_id;
    uint8_t axis_state;  // Low nibble is MC_AXIS_STATES; high nibble is its explicit detail.
    uint16_t error_id;
    uint32_t tick;
    double act_pos;
    double set_pos;
    double set_vel;
    double set_acc;
    int32_t set_position;
    int32_t act_position;
};

struct FlowCtrlStatus {
    uint16_t period;
    uint16_t batchSize;
    uint8_t verboseFlag;
    uint8_t reserved;        // Former reserved[0]; unused.
    // Last fully accepted packet seq; zero means none. Reuses former reserved[1..2].
    uint16_t processed_seq;
    uint16_t latency;
    uint16_t axis_num;
    uint32_t cycleTime;  // us
};

static_assert(sizeof(FlowCtrlStatus) == 16, "FlowCtrlStatus wire layout must stay 16 bytes");

struct FlowCtrlConfig {
    uint16_t period;
    uint16_t batchSize;
    uint8_t verboseFlag;
    uint8_t reserved[3];
};

// Drive-home SDO configuration carried by FBM_HOME_SDO.
struct HomeSdoConfig {
    int32_t mode;
    double offset;
    double velocity_fast;
    double velocity_slow;
    double acceleration;
    int32_t timeout_ms;
};

static_assert(sizeof(HomeSdoConfig) <= 48,
              "HomeSdoConfig must fit the inline motion-command payload");

struct HeartbeatAck {
    uint32_t ackSeq;
    uint32_t ackTick;
};

// GPT-6 Codex, 2026-11-04. Consecutive fragments carry one logical item's parameters.
struct MotionCmdFragmentHeader {
    uint16_t command_type;
    uint16_t total_len;
    uint16_t offset;
};
static_assert(sizeof(MotionCmdFragmentHeader) == 6, "Fragment header must stay six bytes");
static constexpr size_t MOTION_CMD_PARAM_MAX = MC_MOTION_MAX_AXES * sizeof(double);
static constexpr size_t MOTION_CMD_ITEM_HEADER_SIZE = 2 * sizeof(uint16_t);
static constexpr size_t MOTION_CMD_HEARTBEAT_SIZE =
    MOTION_CMD_ITEM_HEADER_SIZE + sizeof(HeartbeatAck);
static constexpr size_t MOTION_CMD_SINGLE_PARAM_MAX =
    MC_CMD_PACKET_PAYLOAD_MAX - MOTION_CMD_HEARTBEAT_SIZE - MOTION_CMD_ITEM_HEADER_SIZE;
static constexpr size_t MOTION_CMD_FRAGMENT_DATA_MAX =
    MOTION_CMD_SINGLE_PARAM_MAX - sizeof(MotionCmdFragmentHeader);
static constexpr size_t MOTION_CMD_FRAGMENT_PACKETS_MAX =
    (MOTION_CMD_PARAM_MAX + MOTION_CMD_FRAGMENT_DATA_MAX - 1) / MOTION_CMD_FRAGMENT_DATA_MAX;
// Bound a fragmented command group to eight transport packets.
static constexpr size_t MOTION_CMD_FRAGMENT_GROUP_MAX = 8;
static constexpr size_t MOTION_CMD_FRAGMENT_PARAM_MAX =
    MOTION_CMD_FRAGMENT_GROUP_MAX * MOTION_CMD_FRAGMENT_DATA_MAX;
static_assert(MOTION_CMD_PARAM_MAX <= UINT16_MAX, "Logical parameters use a 16-bit length");
static_assert(MOTION_CMD_FRAGMENT_PARAM_MAX <= UINT16_MAX,
              "Fragmented parameters use a 16-bit length");

// Explicit ID for a synchronous no-op; automatic command IDs reserve the high bit.
struct SyncPingParam {
    uint32_t request_id;
};

// Explicit target and ID for one numeric axis read. Model signature: GPT-6 Codex.
struct ReadParameterParam {
    uint32_t request_id;
    int32_t axis_id;
    int32_t parameter_number;
};

// Numeric write payload retains the read request's target and ID prefix. GPT-6 Codex.
struct WriteParameterParam {
    uint32_t request_id;
    int32_t axis_id;
    int32_t parameter_number;
    double value;
};

// Wire payload shared by all alias-based IO commands.
struct IoCommandParam {
    uint16_t start_io_number;
    uint16_t end_io_number;
    uint32_t value;  // Write value, or nonzero synchronous request ID for single-point reads.
};

// Command confirmations, axis terminal results and IO results share MotionCmdResult.h.

// Wire request for one AxisWithFb motion-parameter snapshot.
struct AxisInfoQuery {
    uint32_t axis_id;
};

// Wire response containing AxisWithFb parameters and the active pulse scaling ratio.
struct AxisInfo {
    uint32_t axis_id;
    int32_t error_code;
    double velocity;
    double acceleration;
    double deceleration;
    double jerk;
    int32_t pulses_per_unit_num;
    uint32_t pulses_per_unit_den;
};

// Wire payload for a two-axis circular group move.
struct MotionCmdCircularParam {
    double aux[2];
    double end[2];
    uint16_t pathChoice;
    uint16_t reserved[3];
};


#endif
