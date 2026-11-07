#ifndef _ZMC_FBM_ENUM_H_
#define _ZMC_FBM_ENUM_H_

#include "stdint.h"

namespace zmc {

enum EnumFbm {
    FBM_NOP,
    FBM_BASE,
    FBM_AXIS,
    FBM_SYNC_PING,

    FBM_SET_PARAMETER = 0x20,
    FBM_SET_UNIT,
    FBM_SET_NUM_DEN,
    FBM_SET_SPEED,
    FBM_SET_ACCEL,
    FBM_SET_DECEL,
    FBM_SET_JERK,
    FBM_SET_FAST_DECEL,
    FBM_SET_CREEP,
    FBM_SET_BUFFER_MODE,
    FBM_SET_DIRECTION,
    FBM_SET_MASTER,
    FBM_SET_SPEED_RATIO,

    FBM_POWER = 0x0030,
    FBM_RESET,
    FBM_SET_POSITION,  // CMD_DEFPOS
    FBM_HOME,
    FBM_HOMING,
    FBM_DATUM,
    FBM_HOME_SDO,

    FBM_STOP = 0x0040,
    FBM_HALT,
    FBM_MOVE_RELATIVE,
    FBM_MOVE_ABSOLUTE,
    FBM_MOVE_ADDITIVE,
    FBM_MOVE_VELOCITY,

    FBM_AXIS_GROUP = 0x0050,  // axis group selection
    FBM_GROUP_SELECT,         // which group receives pushCommand
    FBM_MULTI_MOVE,           // multi-axis linear move
    FBM_MULTI_MOVE_REL,       // multi-axis linear move (relative)
    FBM_MULTI_CIRCULAR,       // relative center-defined circular group move
    FBM_MULTI_CIRCULAR_ABS,   // absolute center-defined circular group move
    FBM_MULTI_CIRCULAR2,      // relative three-point circular group move
    FBM_MULTI_CIRCULAR2_ABS,  // absolute three-point circular group move
    FBM_GROUP_ENABLE,         // enable selected group
    FBM_GROUP_DISABLE,        // disable selected group
    FBM_GROUP_RESET,          // reset selected group from error
    FBM_GROUP_SET_VEL,        // set selected group velocity
    FBM_GROUP_SET_ACC,        // set selected group acceleration
    FBM_GROUP_SET_DEC,        // set selected group deceleration
    FBM_GROUP_SET_JERK,       // set selected group jerk

    FBM_GEAR_IN = 0x0060,
    FBM_GEAR_OUT,
    FBM_CAM_IN,
    FBM_CAM_OUT,

    FBM_IO_READ_DI = 0x0070,
    FBM_IO_READ_DI_MULTI,
    FBM_IO_READ_DO,
    FBM_IO_READ_DO_MULTI,
    FBM_IO_WRITE_DO,
    FBM_IO_WRITE_DO_MULTI,
    FBM_IO_READ_AI,
    FBM_IO_WRITE_AO,
    FBM_IO_READ_AO,
    FBM_IO_WRITE_VIRTUAL_DI,
    FBM_IO_WRITE_VIRTUAL_AI,

    FBM_SIMU_MANUAL = 0x0E0,
    FBM_SIMU_PAUSE,
    FBM_SIMU_RESUME,
    FBM_STEPS,
    FBM_TICK_TO,
    FBM_TIME_TO,
    FBM_DELAY,
    FBM_BLOCK_TICKS,  // block cmd queue for n ticks (exclusive packet)
    FBM_EXIT,

    FBM_HEARTBEAT = 0x00F0,
    FBM_FLOWCTRL_CFG,  // Flow control configuration
    FBM_FRAG,
    FBM_AXIS_INFO_QUERY,
    FBM_READ_PARAMETER,  // Numeric axis read with a synchronous request ID. GPT-6 Codex.
    FBM_TEST_LONG_PACKET,  // Reserved for long-packet tests.

    FBM_ROBOT_BEGIN = 0x100,
    FBM_ROBOT_END = 0x1FF,
    // FBM_Number
};

enum StatType {
    STAT_SHOW = 0x8001,
    STAT_STATE,
    STAT_CIA402,
    STAT_MOTBUF,
    STAT_CMDRESULT,
    // Extended status types
    STAT_STATE2,           // Rich axis state (AxisState2)
    STAT_FLOWCTRL_STATUS,  // Flow control status (FlowCtrlStatus)
    STAT_IO_RESULT,       // Retired wire slot; IO results use STAT_CMD_CONFIRM.
    STAT_AXIS_INFO,
    STAT_CMD_CONFIRM,
    STAT_HEARTBEAT_ACK = 0x80FF
};

// These axis commands produce a terminal CmdResult after their acceptance confirmation.
inline bool isAsyncAxisCommand(uint16_t type) {
    switch (type) {
        case FBM_SET_NUM_DEN:
        case FBM_RESET:
        case FBM_STOP:
        case FBM_HALT:
        case FBM_GEAR_IN:
        case FBM_GEAR_OUT:
        case FBM_HOME:
        case FBM_HOME_SDO:
        case FBM_SET_POSITION:
        case FBM_MOVE_RELATIVE:
        case FBM_MOVE_ABSOLUTE:
        case FBM_MOVE_ADDITIVE:
        case FBM_MOVE_VELOCITY:
            return true;
        default:
            return false;
    }
}

}  // namespace zmc
#endif
