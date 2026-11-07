/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-01
@ArtifactStatusAuthority GPT-5 Codex
*/

#ifndef _FBT_IMPL_ENUM_DEF_H_
#define _FBT_IMPL_ENUM_DEF_H_

typedef unsigned char uint8_t;

namespace zmc {
enum FbStatEnum : uint8_t {
    FB_STAT_IDLE,
    FB_STAT_INIT,
    FB_STAT_UNACTIVATED,  // push into buffer and waiting for execute
    FB_STAT_BUSY_ACTIVE,
    FB_STAT_DONE,
    FB_STAT_ERROR,
    FB_STAT_ABORTED,
};

// keep equal with FbStatEnum
enum FbStatEnum2 : uint8_t {
    FB_STAT_UNABLE,
    FB_STAT_PADDING,  // padding
    FB_STAT_WAITING,  // padding 2
    FB_STAT_BUSYING,
    FB_STAT_VALID,
    FB_STAT_EN_ERROR,
};

enum AxisCmdEnum : uint8_t {
    AXIS_CMD_NOP,
    AXIS_CMD_RESET,
    AXIS_CMD_POWEROFF,
    AXIS_CMD_QUICKSTOP,
    AXIS_CMD_POWERON,
    AXIS_CMD_HOME,
    AXIS_CMD_HALT,
    AXIS_CMD_MOTION,
};

enum FbMotStatEnum : uint8_t {
    FB_MOT_STAT_NOP,
    FB_MOT_STAT_TRAJ,
    FB_MOT_STAT_LOOKAHEAD,
    FB_MOT_STAT_VELPLAN,
    FB_MOT_STAT_UPDATE,
};

/**
 * Axis states
 */
enum MC_AXIS_STATES : uint8_t {
    mcDisabled = 0,

    /**
     * Power is on, there is no error in the axis, and there are no motion
     * commands active on the axis.
     */
    mcStandstill = 1,
    mcHoming = 2,              /// Homing
    mcDiscreteMotion = 3,      /// DiscreteMotion
    mcContinuousMotion = 4,    /// ContinuousMotion
    mcSynchronizedMotion = 5,  /// SynchronizedMotion
    mcStopping = 6,            /// Stopping
    mcErrorStop = 7,
};

// Identifies the producer-owned subtype of the current PLCopen axis state.
enum AxisStateDetail : uint8_t {
    AXIS_STATE_DETAIL_DEFAULT = 0,
    AXIS_STATE_DETAIL_HOME,
    AXIS_STATE_DETAIL_GEAR_OUT,
    AXIS_STATE_DETAIL_FOLLOW,
    AXIS_STATE_DETAIL_QUICK_STOP,
    AXIS_STATE_DETAIL_ERROR_POWER_OFF,
};

// Encoded axis-state values exposed through AxisState::axis_state.
enum MC_AXIS_STATES_EX : uint8_t {
    mcDisabledEx = 0x00,
    mcStandstillEx = 0x01,
    mcHomingEx = 0x02,  // software homing
    mcHomeEx = 0x12,    // drive home
    mcDiscreteMotionEx = 0x03,
    mcContinuousMotionEx = 0x04,
    mcGearOutEx = 0x14,
    mcSynchronizedMotionEx = 0x05,
    mcFollowEx = 0x15,
    mcStoppingEx = 0x06,
    mcQuickStoppingEx = 0x16,
    mcErrorStopPowerOnEx = 0x07,
    mcErrorStopPowerOffEx = 0x17,
};

static_assert(AXIS_STATE_DETAIL_DEFAULT == 0,
              "AxisImplCore::reset requires the default detail to be zero");

/**
 * ZMC axis group states
 * according to PLCopen for Motion Control part 4.
 */
enum ZMC_AXIS_GROUP_STATE : uint8_t {
    Disabled = 0,
    Standby = 1,
    Homing = 2,
    Moving,
    Stopping = 6,
    ErrorStop,
};

// see also AX_FACTORY_TYPE
enum AXIS_CTYPE {
    AXIS_TYPE_NOP,
    AXIS_TYPE_BASE,
    AXIS_TYPE_VIRTUAL,
    AXIS_TYPE_ENCODER,
    AXIS_TYPE_DRIVE_POS_CONTROL,
    AXIS_TYPE_LOGICAL,
    AXIS_TYPE_CIA402,
    AXIS_TYPE_CIA402_SIMULATED,
    AXIS_TYPE_PULSE,
    AXIS_TYPE_PULSE_SIMULATED,
};

enum {
    VEL_END_PT_REACHED = 2,
};

// Movement type enum
// use in FbSetMovementType
// depreciated, Use ZMC_AXIS_TYPE instead.
typedef enum MC_MOVEMENT_TYPE {
    MC_MOVEMENT_MODULO = 0,  // or rotary
    MC_MOVEMENT_LINEAR = 1
} MC_MOVEMENT_TYPE;

}  // namespace zmc
#endif
