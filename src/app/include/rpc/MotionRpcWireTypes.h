#ifndef _ZMC_MOTION_RPC_WIRE_TYPES_H_
#define _ZMC_MOTION_RPC_WIRE_TYPES_H_

#include <stddef.h>
#include <type_traits>

#include "rpc/MotionRpcTypes.h"
#include "rpc/msgpack.hpp"

struct RpcMoveWire {
    int32_t len;
    int32_t axis[ZMC_RPC_MAX_AXES];
    double distance[ZMC_RPC_MAX_AXES];
    MSGPACK_DEFINE_ARRAY(len, axis, distance)
};

struct RpcAxisStateWire {
    uint8_t axis_id;
    uint8_t axis_state;
    uint16_t error_id;
    uint32_t tick;
    double act_pos;
    double set_pos;
    MSGPACK_DEFINE_ARRAY(axis_id, axis_state, error_id, tick, act_pos, set_pos)
};

struct RpcAxisState2Wire {
    uint8_t axis_id;
    uint8_t axis_state;
    uint16_t error_id;
    uint32_t tick;
    double act_pos;
    double set_pos;
    double set_vel;
    double set_acc;
    int32_t set_position;
    int32_t act_position;
    MSGPACK_DEFINE_ARRAY(axis_id, axis_state, error_id, tick, act_pos, set_pos, set_vel, set_acc,
                         set_position, act_position)
};

struct RpcAxisListWire {
    int32_t len;
    int32_t axis[ZMC_RPC_MAX_AXES];
    MSGPACK_DEFINE_ARRAY(len, axis)
};

/* Fixed wire payloads for the two-axis circular command variants. */
struct RpcMoveCircWire {
    int32_t len;
    int32_t axis[ZMC_RPC_MAX_AXES];
    double end1;
    double end2;
    double centre1;
    double centre2;
    int32_t direction;
    MSGPACK_DEFINE_ARRAY(len, axis, end1, end2, centre1, centre2, direction)
};

struct RpcMoveCirc2Wire {
    int32_t len;
    int32_t axis[ZMC_RPC_MAX_AXES];
    double mid1;
    double mid2;
    double end1;
    double end2;
    MSGPACK_DEFINE_ARRAY(len, axis, mid1, mid2, end1, end2)
};

/* Serialize the public result layout while keeping native results internal. */
struct RpcCmdResultInfoWire : ZML_CmdResultInfo {
    MSGPACK_DEFINE_ARRAY(id, command_type, target_index, target_kind, status, io_count,
                         error_code, tick, data, data_ext)
};

/* Serializable snapshot of one configured IO alias. */
struct RpcIoMetadataWire {
    uint32_t mask;
    uint16_t io_number;
    uint16_t index;
    uint16_t bit_offset;
    uint8_t io_type;
    uint8_t source_kind;
    uint8_t slave_idx;
    uint8_t subindex;
    uint8_t bit_size;
    uint8_t is_input;
    MSGPACK_DEFINE_ARRAY(mask, io_number, index, bit_offset, io_type, source_kind, slave_idx,
                         subindex, bit_size, is_input)
};

struct RpcFlowCtrlConfigWire {
    uint16_t period;
    uint16_t batchSize;
    uint8_t verboseFlag;
    uint8_t reserved[3];
    MSGPACK_DEFINE_ARRAY(period, batchSize, verboseFlag, reserved)
};

struct RpcFlowCtrlStatusWire {
    uint16_t period;
    uint16_t batchSize;
    uint8_t verboseFlag;
    uint8_t reserved[3];
    uint16_t latency;
    uint16_t axis_num;
    uint32_t cycleTime;
    MSGPACK_DEFINE_ARRAY(period, batchSize, verboseFlag, reserved, latency, axis_num, cycleTime)
};

struct RpcHomeSdoConfigWire {
    int32_t mode;
    double offset;
    double velocity_fast;
    double velocity_slow;
    double acceleration;
    int32_t timeout_ms;
    MSGPACK_DEFINE_ARRAY(mode, offset, velocity_fast, velocity_slow, acceleration, timeout_ms)
};

struct RpcVersionInfoWire {
    char version[80];
    MSGPACK_DEFINE_ARRAY(version)
};

struct RpcAboutInfoWire {
    char about[255];
    MSGPACK_DEFINE_ARRAY(about)
};

/* Wire response for the independently exposed controller information RPC. */
struct RpcControllerInfoWire {
    char soft_type[80];
    char soft_version[80];
    char controller_id[80];
    MSGPACK_DEFINE_ARRAY(soft_type, soft_version, controller_id)
};

inline const RpcAxisState& rpc_from_wire(const RpcAxisStateWire& v) {
    return reinterpret_cast<const RpcAxisState&>(v);
}
inline RpcAxisState& rpc_from_wire(RpcAxisStateWire& v) {
    return reinterpret_cast<RpcAxisState&>(v);
}
inline const RpcAxisStateWire& rpc_to_wire(const RpcAxisState& v) {
    return reinterpret_cast<const RpcAxisStateWire&>(v);
}

inline const RpcAxisState2& rpc_from_wire(const RpcAxisState2Wire& v) {
    return reinterpret_cast<const RpcAxisState2&>(v);
}
inline RpcAxisState2& rpc_from_wire(RpcAxisState2Wire& v) {
    return reinterpret_cast<RpcAxisState2&>(v);
}
inline const RpcAxisState2Wire& rpc_to_wire(const RpcAxisState2& v) {
    return reinterpret_cast<const RpcAxisState2Wire&>(v);
}

inline ZML_CmdResultInfo rpc_from_wire(const RpcCmdResultInfoWire& v) {
    return static_cast<const ZML_CmdResultInfo&>(v);
}
inline RpcCmdResultInfoWire rpc_to_wire(const RpcCmdResultInfo& v) {
    RpcCmdResultInfoWire wire = {};
    ZML_CmdResultInfo& result = static_cast<ZML_CmdResultInfo&>(wire);
    result.id = v.id;
    result.command_type = v.command_type;
    result.target_index = v.target_index;
    result.target_kind = v.target_kind;
    result.status = v.status;
    result.io_count = v.io_count;
    result.error_code = v.error_code;
    result.tick = v.tick;
    result.data = v.data;
    result.data_ext = v.data_ext;
    return wire;
}

inline const RpcFlowCtrlConfig& rpc_from_wire(const RpcFlowCtrlConfigWire& v) {
    return reinterpret_cast<const RpcFlowCtrlConfig&>(v);
}
inline RpcFlowCtrlConfig& rpc_from_wire(RpcFlowCtrlConfigWire& v) {
    return reinterpret_cast<RpcFlowCtrlConfig&>(v);
}
inline const RpcFlowCtrlConfigWire& rpc_to_wire(const RpcFlowCtrlConfig& v) {
    return reinterpret_cast<const RpcFlowCtrlConfigWire&>(v);
}

inline const RpcHomeSdoConfig& rpc_from_wire(const RpcHomeSdoConfigWire& v) {
    return reinterpret_cast<const RpcHomeSdoConfig&>(v);
}
inline RpcHomeSdoConfig& rpc_from_wire(RpcHomeSdoConfigWire& v) {
    return reinterpret_cast<RpcHomeSdoConfig&>(v);
}
inline const RpcHomeSdoConfigWire& rpc_to_wire(const RpcHomeSdoConfig& v) {
    return reinterpret_cast<const RpcHomeSdoConfigWire&>(v);
}

inline const RpcFlowCtrlStatus& rpc_from_wire(const RpcFlowCtrlStatusWire& v) {
    return reinterpret_cast<const RpcFlowCtrlStatus&>(v);
}
inline RpcFlowCtrlStatus& rpc_from_wire(RpcFlowCtrlStatusWire& v) {
    return reinterpret_cast<RpcFlowCtrlStatus&>(v);
}
inline const RpcFlowCtrlStatusWire& rpc_to_wire(const RpcFlowCtrlStatus& v) {
    return reinterpret_cast<const RpcFlowCtrlStatusWire&>(v);
}

static_assert(sizeof(RpcAxisState) == sizeof(RpcAxisStateWire), "RpcAxisState layout mismatch");
static_assert(sizeof(RpcAxisState2) == sizeof(RpcAxisState2Wire), "RpcAxisState2 layout mismatch");
static_assert(offsetof(RpcAxisState, tick) == 4 && offsetof(RpcAxisState, act_pos) == 8 &&
                  offsetof(RpcAxisState, set_pos) == 16,
              "RpcAxisState public field order mismatch");
static_assert(offsetof(RpcAxisStateWire, tick) == 4 &&
                  offsetof(RpcAxisStateWire, act_pos) == 8 &&
                  offsetof(RpcAxisStateWire, set_pos) == 16,
              "RpcAxisState MsgPack field order mismatch");
static_assert(offsetof(RpcAxisState2, tick) == 4 && offsetof(RpcAxisState2, act_pos) == 8 &&
                  offsetof(RpcAxisState2, set_pos) == 16 &&
                  offsetof(RpcAxisState2, set_vel) == 24 &&
                  offsetof(RpcAxisState2, set_acc) == 32 &&
                  offsetof(RpcAxisState2, set_position) == 40 &&
                  offsetof(RpcAxisState2, act_position) == 44,
              "RpcAxisState2 public field order mismatch");
static_assert(offsetof(RpcAxisState2Wire, tick) == 4 &&
                  offsetof(RpcAxisState2Wire, act_pos) == 8 &&
                  offsetof(RpcAxisState2Wire, set_pos) == 16 &&
                  offsetof(RpcAxisState2Wire, set_vel) == 24 &&
                  offsetof(RpcAxisState2Wire, set_acc) == 32 &&
                  offsetof(RpcAxisState2Wire, set_position) == 40 &&
                  offsetof(RpcAxisState2Wire, act_position) == 44,
              "RpcAxisState2 MsgPack field order mismatch");
static_assert(sizeof(RpcCmdResultInfo) == sizeof(RpcCmdResultInfoWire), "RpcCmdResultInfo layout mismatch");
static_assert(std::is_standard_layout<RpcCmdResultInfoWire>::value,
              "CmdResult wire must be standard layout");
static_assert(sizeof(RpcFlowCtrlConfig) == sizeof(RpcFlowCtrlConfigWire), "RpcFlowCtrlConfig layout mismatch");
static_assert(sizeof(RpcFlowCtrlStatus) == sizeof(RpcFlowCtrlStatusWire), "RpcFlowCtrlStatus layout mismatch");

#endif
