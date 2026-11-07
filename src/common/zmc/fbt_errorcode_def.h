/*
 * @Author: ChenShengyong
 * @Date: 2024-06-19 17:13:17
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2026-09-23 14:43:01
 */
#ifndef _FBT_ERRORCODE_DEF_H_
#define _FBT_ERRORCODE_DEF_H_

#include "stdint.h"

/* Fixed offset from utils/device base codes to host-facing codes (mst_utils return boundary). */
#ifndef ZMC_ERR_UTILS_HOST_OFFSET
#define ZMC_ERR_UTILS_HOST_OFFSET 1000
#endif

//  : uint16_t
typedef enum MC_ERROR_CODE {
    ZMC_NO_ERROR = 0,  // No error

    ZMC_FB_WASNT_CALLED_DURING_MOTION =
        30,  // Motion creating module has not been called again before end of the motion
    ZMC_AXIS_IS_NO_AXIS_REF = 31,  // Type of given AXIS_REF variable is not AXIS_REF
    ZMC_AXIS_REF_CHANGED_DURING_OPERATION =
        32,  // AXIS_REF variable has been exchanged while the module was active.
    ZMC_FB_ACTIVE_AXIS_DISABLED = 33,  // Axis disabled while being moved. MC_Power.bRegulatorOn
    ZMC_AXIS_NOT_READY_FOR_MOTION =
        34,  // Axis in its current state cannot execute a motion command, because the axis doesn't
             // signal currently that it follows the target values.
    ZMC_AXIS_ERROR_DURING_MOTION = 35,  // The drive reported an error during an ongoing movement.

    // virtual drive
    ZMC_VD_MAX_VELOCITY_EXCEEDED = 40,      // Maximum velocity fMaxVelocity exceeded
    ZMC_VD_MAX_ACCELERATION_EXCEEDED = 41,  // Maximum acceleration fMaxAcceleration exceeded
    ZMC_VD_MAX_DECELERATION_EXCEEDED = 42,  // Maximum deceleration fMaxDeceleration exceeded
    // SMC_Homing
    ZMC_3SH_INVALID_VELACC_VALUES = 50,
    // SMC_SetControllerMode errors
    ZMC_SCM_NOT_SUPPORTED = 70,        // Mode not supported
    ZMC_SCM_AXIS_IN_WRONG_STATE = 71,  // In current mode, controller mode cannot be changed.
    SMC_SCM_INTERRUPTED =
        72,  // SMC_SetControllerMode has been interrupted by MC_Stop or errorstop.
    ZMC_ERR_SCM_MODE_SWITCH_TIMEOUT,  // zmc specific, Controller mode switch timeout
    // SMC_SetTorque
    SMC_ST_WRONG_CONTROLLER_MODE = 75,  // Axis not in correct controller mode. Deprecated, no
                                        // longer returned by SMC_SetTorque.

    // SMC3_PersistPosition (FB)
    SMC_PP_WRONG_AXIS_TYPE = 85,  // The function block does not support virtual or logical axes.
    SMC_PP_NUMBER_OF_ABSOLUTE_BITS_INVALID =
        86,  // The number of absolute bits is invalid, must be in the range 8 .. 32.
    ZMC_NEGATIVE_MODULO_PERIOD = 87,  // The modulo period is negative, this is not supported.
    // SMC_ChangeGearingRatio FB
    ZMC_CGR_ZERO_VALUES = 90,  // Invalid values
    SMC_CGR_DRIVE_POWERED =
        91,  // Gearing parameters must not be changed as long as the drive is under control.
    SMC_CGR_INVALID_POSPERIOD =
        92,  // Invalid modulo period (<= 0 or greater than half the bus bandwidth)
    SMC_CGR_POSPERIOD_NOT_INTEGRAL = 93,  // The modulo period in increments is not an integer, but
                                          // the modulo-handling is done by the drive.
    SMC_FRACTION_TECH_UNITS_INVALID =
        94,  // The fraction describing the conversion between increments and technical units is
             // not initialized properly. (Zero numerator or denumerator.)
    SMC_SCALING_PARAMETERS_NOT_POSITIVE =
        95,  // The scaling parameters (increments per turn, gear settings, units in application)
             // must be all positive.
    SMC_SCALING_SETTINGS_TOO_LARGE =
        96,  // The scaling parameters (increments per turn, gear settings, units in application)
             // are too large to fit into a fraction with 32 bit numerator and denominator.
    SMC_MODULO_PERIOD_TOO_MANY_DIGITS =
        97,  // The modulo period has too many digits. The modulo period times the denominator of
             // the fraction from user units to increments must be an integer.

    // misc
    ZMC_DI_GENERAL_COMMUNICATION_ERROR =
        101,  // Communication error. The fieldbus slave is no longer in state operational.
    ZMC_DI_AXIS_ERROR = 102,            // Axis error
    ZMC_AXIS_IS_NOT_OPERATIONAL = 103,  // Axis is not operational wCommunicationState <> 100
    ZMC_DI_SWLIMITS_EXCEEDED = 106,     // Position outside of permissible range of SWLimit
    ZMC_DI_HWLIMITS_EXCEEDED = 107,     // Hardware end switch is active

    // MC_Power
    ZMC_P_FTASKCYCLE_EMPTY = 110,  // Axis contain no information on cycle time (fTaskCycle = 0).
    ZMC_ERR_P_WHEN_DRIVE_ERROR,    // zmc specific, power on when axis has error
    ZMC_ERR_P_TIMEOUT,             // zmc specific

    // misc
    // SMC_REGULATOR_OR_START_NOT_SET = 115,
    ZMC_DI_POSITIONLAGERROR = 116,    // Position lag error. Difference between set and current
                                      // position exceeds the given limit
    ZMC_DI_HOMING_ERROR = 117,        // Homing error reported by axis
    ZMC_WRONG_CONTROLLER_MODE = 118,  // Axis in wrong controller mode

    // MC_Reset
    ZMC_R_NO_ERROR_TO_RESET = 120,
    SMC_R_DRIVE_DOESNT_ANSWER = 121,          // Axis does not perform error-reset.
    SMC_R_ERROR_NOT_RESETTABLE = 122,         // Error could not be reset.
    SMC_R_DRIVE_DOESNT_ANSWER_IN_TIME = 123,  // Communication with the axis did not work.
    SMC_R_CANNOT_RESET_COMMUNICATION_ERROR =
        124,                  // If there is communication error, it cannot be reset
    ZMC_ERR_R_TIMEOUT = 125,  // zmc specific

    // MC_ReadParameter
    ZMC_RP_PARAM_UNKNOWN = 130,     // Parameter number unknown
    SMC_RP_REQUESTING_ERROR = 131,  // Error during transmission to the drives. See error number in
                                    // function block instance ReadDriveParameter.
    SMC_RP_DRIVE_PARAMETER_NOT_MAPPED = 132,  // No assignment for drive parameters available
    SMC_RP_PARAM_CONVERSION_ERROR =
        133,  // Conversion of the value to / from the drive parameters failed.

    // MC_WriteParameter
    ZMC_WP_PARAM_INVALID = 140,  // Parameter number unknown or writing not allowed
    SMC_WP_SENDING_ERROR = 141,  // See error number in function block instance WriteDriveParameter
    SMC_WP_DRIVE_PARAMETER_NOT_MAPPED = 142,  // No assignment for drive parameters available
    SMC_WP_PARAM_CONVERSION_ERROR =
        143,  // Conversion of the value to / from the drive parameters failed
    ZMC_ERR_WP_VALUE_INVALID = 144,

    // MC_WriteCoEParameter (one-shot SDO download through the axis CoE channel)
    ZMC_ERR_WP_SDO_UNBOUND = 150,  // Axis has no bound EtherCAT slave for SDO access
    ZMC_ERR_WP_SDO_ABORTED,        // Slave or transport rejected the SDO download
    ZMC_ERR_WP_SDO_TIMEOUT,        // No SDO write result reached the axis in time
    ZMC_ERR_WP_SDO_QUEUE_FULL,     // Axis CoE request FIFO is full
    // ZMC_ERR_WP_SDO_WATCH_KEY,      // Target object is owned by a periodic SDO watch

    // MC_Home
    ZMC_H_AXIS_WASNT_STANDSTILL = 170,  // Axis was not in standstill state.
    // ZMC_H_AXIS_DIDNT_START_HOMING = 171,  // Drive did not start homing.
    ZMC_ERR_H_TIMEOUT = 172,  // Drive did not complete homing before the configured timeout.
    // ZMC_H_ERROR_WHEN_STOPPING = 173,  // Error at stop after homing. Deceleration may not be
    // set.
    ZMC_H_AXIS_IN_ERRORSTOP = 174,  // Drive is in errorstop status. Homing cannot be executed.
    ZMC_H_SET_OP_MODE_HOMING_FAILED = 175,  // Drive could not be switched to homing mode.
    SMC_H_CHECK_SET_OP_MODE_HOMING_FAILED =
        176,                               // Drive did not report that it is in homing mode.
    SMC_H_PRE_HOMING_CHECKS_FAILED = 177,  // Pre homing checks failed.
    ZMC_ERR_H_AXIS_IN_MOVING,  // zmc specific, AxisStateViolation, General invalid state
                               // transition
    // ZMC_ERR_H_ERROR = 175,  // zmc specific, home error from controlword
    // ZMC_ERR_H_OPERATION_MODE_TIMEOUT = 176, // zmc specific

    // MC_Stop FB
    // SMC_MS_UNKNOWN_STOPPING_ERROR = 180,  // Unknown error at stop
    SMC_MS_INVALID_ACCDEC_VALUES =
        181,  // Invalid velocity, acceleration, deceleration, or jerk values
    SMC_MS_AXIS_IN_ERRORSTOP = 183,  // Drive is in errorstop status. Stop cannot be executed
    SMC_BLOCKING_MC_STOP_WASNT_CALLED =
        184,  // Instance of MC_Stop blocking the axis by Execute = TRUE has not been called yet.
              // MC_Stop (Execute = FALSE) has to be called.
    SMC_MS_AXIS_ALREADY_STOPPING = 185,  // Cannot interrupt an ongoing MC_Stop

    // Task interval error
    SMC_UNKNOWN_TASK_INTERVAL = 200,  // The task interval of the bus task could not be determined.

    // move absolute FB
    SMC_MA_INVALID_VELACC_VALUES = 201,  // Invalid velocity or acceleration values
    SMC_MA_INVALID_DIRECTION = 202,      // Direction error
    // moveRelative
    SMC_MR_INVALID_VELACC_VALUES = 226,  // Invalid velocity or acceleration values
    SMC_OVR_INVALID_OVERRIDE_FACTORS =
        231,  // Invalid velocity, acceleration or jerk override factor
    // moveAdditive
    SMC_MAD_INVALID_VELACC_VALUES = 251,  // Invalid velocity or acceleration values

    // SMC_MoveSuperimposed fb
    SMC_MSI_INVALID_VELACC_VALUES =
        276,  // Invalid velocity, acceleration, deceleration, or jerk values
    SMC_MSI_INVALID_DIRECTION = 277,  //  Direction error
    ZMC_MSI_INVALID_EXECUTION_ORDER =
        278,  // Except from aborting, dont call a major mvt-FB after SMC_MoveSuperimposed
    ZMC_ERR_MSI_SUPERIMPOSED_NOT_SUPPORTED,

    // MC_MoveVelocity fb
    SMC_MV_INVALID_ACCDEC_VALUES =
        301,  // Invalid velocity, acceleration, deceleration, or jerk values
    ZMC_MV_DIRECTION_NOT_APPLICABLE = 302,  // Direction = shortest/fastest not applicable

    // MC_PositionProfile fb
    SMC_PP_ARRAYSIZE = 325,  // Erroneous array size
    SMC_PP_STEP0MS = 326,    // Step time = t#0s
    // MC_VelocityProfile
    SMC_VP_ARRAYSIZE = 350,  // Erroneous array size
    SMC_VP_STEP0MS = 351,    // Step time = t#0s
    // MC_AccelerationProfile
    SMC_AP_ARRAYSIZE = 375,  // Erroneous array size
    SMC_AP_STEP0MS = 376,    // Step time = t#0s

    // MC_MoveContinuousRelative fb
    SMC_MCR_INVALID_VELACC_VALUES =
        426,  // Invalid velocity, acceleration, deceleration, or jerk values
    SMC_MCR_INVALID_DIRECTION = 427,  // Invalid direction
    // MC_MoveContinuousAbsolute
    SMC_MCA_INVALID_VELACC_VALUES =
        451,  // Invalid velocity, acceleration, deceleration, or jerk values
    SMC_MCA_INVALID_DIRECTION = 452,  // Invalid direction
    // SMC_ChangeDynamicLimits
    ZMC_SDL_INVALID_AXIS_STATE =
        475,  // SMC_ChangeDynamicLimits may only be called in state standstill. or power_off.
    ZMC_SDL_INVALID_VELACC_VALUES =
        476,  // Invalid velocity, acceleration, deceleration, or jerk values
    ZMC_INVALID_VELACCJERK_VALUES =
        501,                      // Invalid Velocity, Acceleration, Deceleration, or Jerk values
    SMC_INVALID_DIRECTION = 502,  // Invalid Direction

    // SMC_CamRegister (FB)
    SMC_CR_NO_TAPPETS_IN_CAM = 600,      // Cam does not contain any tappets.
    SMC_CR_TOO_MANY_TAPPETS = 601,       // Tappet group ID exceeds MAX_NUM_TAPPETS
    SMC_CR_MORE_THAN_32_ACCESSES = 602,  // More than 32 accesses on one CAM_REF

    // SMC_CamIn (FB)
    ZMC_CI_NO_CAM_SELECTED = 625,      // No cam selected
    SMC_CI_MASTER_OUT_OF_SCALE = 626,  // Master axis out of valid range
    SMC_CI_RAMPIN_NEEDS_VELACC_VALUES =
        627,  // Velocity and acceleration values must be specified for ramp_in function
    SMC_CI_SCALING_INCORRECT =
        628,  // Scaling variables fEditor/TableMasterMin/Max are not correct
    SMC_CI_TOO_MANY_TAPPETS_PER_CYCLE = 629,  // Too many tappets became active during one cycle
    SMC_CI_MORE_THAN_32_ACCESSES = 630,       // More than 32 accesses on one CAM_REF
    SMC_CI_MASTER_SYNC_POSITION_OUT_OF_SCALE = 631,  // MasterSyncPosition out of valid range
    SMC_CI_AVOID_REVERSAL_FOR_FINITE_AXIS =
        632,  // Input AvoidReversal must not be set for finite slave axes.
    SMC_CI_MASTERSTARTDISTANCE_MUST_BE_ZERO_BUFFERED =
        633,  // If BufferMode is not Aborting, then MasterStartDistance must not be positive.
    SMC_CI_CAM_CHANGED_DURING_RAMP_IN_DIST =
        634,  // The input CamTableId changed while ramping in with start mode ramp_in_dist
    // SMC_CAMBounds
    SMC_CB_NOT_IMPLEMENTED = 640,
    // MC_GearIn fb
    SMC_GI_RATIO_DENOM = 675,               // RatioDenominator = 0
    SMC_GI_INVALID_ACC = 676,               // Acceleration invalid
    SMC_GI_INVALID_DEC = 677,               // Deceleration invalid
    SMC_GI_MASTER_REGULATOR_CHANGED = 678,  // Master changed regulator state (on -> off or off ->
                                            // on) while coupling with slave was active.
    SMC_GI_INVALID_JERK = 679,              // Jerk invalid
    // MC_Phasing FB
    SMC_PH_INVALID_VELACCDEC = 725,  // Velocity and acceleration/deceleration values invalid
    // MC_CamTableSelect (FB)
    ZMC_NO_CAM_REF_TYPE = 750,  // Type of given cam is not MC_CAM_REF.
    SMC_CAM_TABLE_DOES_NOT_COVER_MASTER_SCALE =
        751,  // Master area, xStart and xEnd, from CamTable is not covered by curve data.
    SMC_CAM_TABLE_EMPTY_MASTER_RANGE = 752,     // Cam data table has empty master range.
    SMC_CAM_TABLE_INVALID_MASTER_MINMAX = 753,  // Cam data master has invalid max-, min-values.
    SMC_CAM_TABLE_INVALID_SLAVE_MINMAX = 754,   // Cam data slave has invalid max-, min-values.
    ZMC_CAM_INVALID_ELEMENT_COUNT = 755,        // The element count of the cam must be > 0.
    SMC_CAM_UNKNOWN_SEGMENT_TYPE = 756,         // Unknown cam segment type encountered.
    SMC_SAFE_CAM_REF_ONLY_ONE_ACCESS_ALLOWED_AT_A_TIME =
        757,  // CAM_REF_MULTICORE_SAFE can only be accessed by 1 read or 1 write at a time.
    SMC_CAM_REF_INSUFFICIENT_MEMORY =
        758,  // The data array of MC_CAM_REF is too small to fit all cam elements.
    ZMC_CAM_TABLE_INVALID_DATA,     // zmc spec
    ZMC_ERR_WRC_DUMP_FAILED = 765,  // zmc spec

    // SMC_GearInPos (FB)
    SMC_GIP_MASTER_DIRECTION_CHANGE = 775,  // Obsolete: During coupling of slave axis, master axis
                                            // has changed direction of rotation.
    SMC_GIP_SLAVE_REVERSAL_CANNOT_BE_AVOIDED =
        776,  // Obsolete: Input AvoidReversal is set, but slave reversal cannot be avoided.
    SMC_GIP_AVOID_REVERSAL_FOR_FINITE_AXIS =
        777,  // Input AvoidReversal must not be set for finite slave axes.
    SMC_GIP_MASTERSTARTDISTANCE_MUST_BE_ZERO_BUFFERED =
        778,  // If BufferMode is not Aborting, then MasterStartDistance must not be positive.
    SMC_GIP_CANNOT_START_SYNC =
        779,  // The synchronisation cannot be started. This can happen when GearInPos is commanded
              // as a buffered or blending movement and the master is in standstill when it becomes
              // active.
    SMC_RAMP_IN_MASTER_DIRECTION_CHANGED =
        780,  // During coupling of slave axis, master axis has changed direction of rotation.
    SMC_RAMP_IN_SLAVE_REVERSAL_CANNOT_BE_AVOIDED =
        781,  // Input AvoidReversal is set, but slave reversal cannot be avoided.

    // SMC_DigitalCamSwitch (FB)
    SMC_DCS_ON_OFF_COMPENSATION_REQUIRES_FORECAST =
        790,  // Using a negative On or OffCompensation requires setting a forecast.
    SMC_DCS_ON_OFF_COMPENSATION_EXCEEDS_FORECAST =
        791,  // The forecast must not be less than the maximum absolute value of all negative
              // On/OffCompensations.
    SMC_DCS_SWITCHES_MUST_BE_ORDERED_BY_POSITION =
        792,  // Switches on the same track must be ordered by position.
    SMC_DCS_SWITCHES_MUST_NOT_OVERLAP = 793,  // Switches on the same track must not overlap.
    SMC_DCS_INVALID_TRACK_NUMBER = 794,       // Switch has an invalid track number.
    SMC_DCS_MAX_SWITCHES_PER_TRACK_EXCEEDED =
        795,  // Maximum number of switches for a track exceeded.
    SMC_DCS_SWITCH_POSITION_OUTSIDE_MODULO_PERIOD =
        796,  // At least one of the switches lies outside the modulo period of the axis.
    SMC_DCS_CAM_SWITCH_POINTER_NOT_SET = 797,  // The Switches.CamSwitchPtr is not set.

    // Quadratic trajectory computation errors
    SMC_QPROF_DIVERGES = 825,  // Internal error: computation of quadratic trajectory failed
    SMC_QPROF_INVALID_PARAMETER =
        826,                          // Internal error: computation of quadratic trajectory failed
    SMC_QPROF_NO_RESULT = 827,        // Internal error: computation of quadratic trajectory failed
    SMC_QPROF_INVALID_NEW_LBD = 828,  // Internal error: computation of quadratic trajectory failed
    SMC_QPROF_BAD_NEGOTIATION = 829,  // Internal error: computation of quadratic trajectory failed
    SMC_QPROF_INVALID_INTERVAL =
        830,  // Internal error: computation of quadratic trajectory failed
    SMC_QPROF_NOT_ENOUGH_PHASES =
        831,                      // Internal error: computation of quadratic trajectory failed
    SMC_TG_INTERNAL_ERROR = 832,  // Internal error: computation of quadratic trajectory failed

    // SMC_SetRampType (FB)
    SMC_SRT_NOT_STANDSTILL_OR_POWEROFF = 850,  // Action only permitted in standstill or power_off.
    SMC_SRT_INVALID_RAMPTYPE = 851,            // Invalid ramp type

    // SMC_SetMovementType (FB)
    SMC_SMT_NOT_STANDSTILL_OR_POWEROFF = 852,  // Action only permitted in standstill or power_off.
    SMC_SMT_INVALID_MOVEMENTTYPE_OR_POSITIONPERIOD =
        853,                         // Invalid motion type or position period
    SMC_SMT_AXIS_NOT_VIRTUAL = 854,  // Function block only applicable to virtual axis.

    SMC_NO_LICENSE = 1000,
    // SMC_Interpolator
    SMC_INT_VEL_ZERO = 1001,

    //  SMC_CheckVelocities
    SMC_CV_ACC_DEC_VEL_NONPOSITIVE = 1100,
    //  SMC_Controlaxisbypos
    SMC_CA_INVALID_ACCDEC_VALUES = 1120,

    // Movement blending and buffering errors
    ZMC_BLENDING_NOT_SUPPORTED_BY_PREVIOUS_MOVEMENT =
        1951,  // The previous movement does not support blending.
    ZMC_BUFFERED_NOT_SUPPORTED_BY_PREVIOUS_MOVEMENT =
        1952,  // The previous movement does not support buffered movements.
    SMC_INHERIT_OWNER_ACTIVE_MOVEMENT_NOT_CALLED =
        1953,  // Cannot inherit owner because the active movement has not been not called yet in
               // this cycle. Internal error.
    SMC_MOVING_WITHOUT_ACTIVE_MOVEMENT =
        1954,  // The axis is moving but there is no active movement function block. Missing
               // follow-up movement after movement that does not end in standstill?
    ZMC_BUFFER_MODE_NOT_SUPPORTED =
        1955,  // The configured BufferMode is not supported by the commanded movement.
    SMC_ERROR_IN_A_PREVIOUS_MOVEMENT = 1956,  // An error occured in a previous movement.
    SMC_MORE_THAN_ONE_MOVEMENT_PER_INSTANCE =
        1957,  // Only one busy buffered/blending movement per FB instance is allowed.
    SMC_SUPERIMPOSED_FB_WASNT_CALLED =
        1958,  // A superimposed FB (MC_MoveSuperimposed or MC_HaltSuperimposed) that controlled
               // the superimposed movement has not been called in this task cycle.

    // SMC_AxisDiagnosticLog
    SMC_ADL_FILE_CANNOT_BE_OPENED = 2100,  // File could not be opened
    SMC_ADL_BUFFER_OVERRUN = 2101,  // Buffer overrun. WriteToFile must be called more frequently.

    // Cam file operations errors
    SMC_RCAM_FILE_DOESNT_EXIST = 2200,   // File could not be opened.
    SMC_RCAM_TOO_MUCH_DATA = 2201,       // Saved cam too big
    SMC_RCAM_WRONG_COMPILE_TYPE = 2202,  // Wrong compilation mode
    SMC_RCAM_WRONG_VERSION = 2203,       // File has wrong version
    SMC_RCAM_UNEXPECTED_EOF = 2204,      // Unexpected end of file
    SMC_RCAM_WRITE_ERROR = 2205,         // Error writing to the cam file
    SMC_RCAM_READ_ERROR = 2206,          // Error reading from the cam file

    // encoder
    SMC_ENC_DENOM_ZERO = 5000,  // Nominator of the conversion factor dwRatioTechUnitsDenom of the
                                // Encoder reference is 0
    ZMC_INVALID_AXIS_TYPE = 6001,  // Invalid axis type, not finite or modulo (internal error)

    SMC_MOVE_INVALID_BUFFER_MODE = 11006,  // The given buffer mode is not supported
    // setOverride
    ZMC_MOVE_INVALID_DYNAMIC_FACTOR = 11007,  // The dynamic factor is not in the range [0 .. 1]
    SMC_MAC_INTERNAL_ERROR = 13041,

    ZMC_ERR_SERVO_ERROR = 0x4001,  // 16385U, drive has error, depreciated
    ZMC_ERR_UNKNOWN_SW_ERROR,      // axis has software error

    // ZMC_ERR_RDWR_PARAMETER_NOT_RIGHT, // depreciated
    ZMC_ERR_SLAVE_SAME_AXIS_REF,

    ZMC_ERR_FB_ACTIVATE_AXIS_ERRORSTOP,
    ZMC_ERR_POWEROFF_WHEN_MOVING,
    ZMC_ERR_ERRORSTOP_WHEN_MOVING,
    ZMC_ERR_AXIS_STATE_VIOLATION,
    ZMC_ERR_INVALID_SW_ERROR_REACTION,  // software_error_reaction deceration or max_distance is
                                        // invalid, or both are 0.
    ZMC_ERR_FB_ACTIVATE_AXIS_STOPPING,
    ZMC_ERR_VEL_VEL_ACC_DEC_INVALID = 16420,  // depreciated
    ZMC_ERR_VEL_VEL_ACC_INVALID,              // depreciated
    ZMC_ERR_VEL_BUILD_FAILED,
    ZMC_ERR_VEL_POS_JUMP,

    ZMC_ERR_AXIS_NOT_IN_GEAR = 16430,
    ZMC_ERR_AXIS_NOT_IN_CAM,
    ZMC_ERR_AXIS_NOT_IN_SYNC_MOTION,

    ZMC_ERR_POSITION_NOT_CONVERGENCE = 16440,  // depreciated
    ZMC_ERR_SETPOSITION_WHILE_MOVING,

    // SMC3_ReinitDrive FB
    ZMC_RD_AXIS_IN_WRONG_STATE = 16450,  // Axis is in wrong state for reinitialization
    ZMC_RD_VIRTUAL_MODE_ALREADY_SET,     // Virtual mode is already set as requested  // The modulo
                                         // period has too many digits. The modulo period times the
    // denominator of the fraction from user units to increments
    // must be an integer.

    ZMC_ERR_REPDBG_TRIG_ERROR = 16480,
    ZMC_ERR_REPDBG_OVERFLOW_ERROR,

    // mcErrorCode_PowerOnOffFromErrorStop,     // Try to power on at ErrorStop
    ZMC_ERR_INVALIDSTATE_FROMSTOPPING = 16500,  // Invalid state transition at Stopping
    ZMC_ERR_INVALIDSTATE_FROMERRORSTOP,         // Invalid state transition at ErrorStop
    ZMC_ERR_INVALIDSTATE_FROMDISABLED,          // Invalid state transition at Disabled

    // mcErrorCode_Motion_limit_Error = 0x4400,    // Velocity, Acceleration or Position over Limit
    ZMC_ERR_INVALID_DIRECTION_POSITIVE,    // Velocity in forbiden direction
    ZMC_ERR_INVALID_DIRECTION_NEGATIVE,    // Velocity in forbiden direction
    ZMC_ERR_POSITION_OVER_POSITIVE_LIMIT,  // Position over positive limit
    ZMC_ERR_POSITION_OVER_NEGATIVE_LIMIT,  // Position over negative limit

    ZMC_ERR_CFG_HARDWARE_PARAM_INVALID,

    ZMC_ERR_NO_AXISGRP_REF = 17000,
    ZMC_ERR_AXISGRP_EMPTY,
    ZMC_ERR_AXISGRP_FULL,
    ZMC_ERR_AXISGRP_INVALID_STATE,
    ZMC_ERR_AXISGRP_NO_ERROR_TO_RESET,
    ZMC_ERR_AXISGRP_AXIS_NOT_FOUND,
    ZMC_ERR_AXISGRP_ERRORSTOP,

    // MotionCmdPack status cache query
    ZMC_ERR_MOTCMD_AXIS_STATE_INVALID_AXIS = 17100,
    ZMC_ERR_MOTCMD_AXIS_STATE2_INVALID_AXIS,
    ZMC_ERR_MOTCMD_CMD_RESULTS_EMPTY,
    ZMC_ERR_MOTCMD_WAIT_INVALID_ARG,
    ZMC_ERR_MOTCMD_SCRIPT_BUSY_DISCARD,
    ZMC_ERR_MOTCMD_ABORT_PUSH_FAILED,
    ZMC_ERR_MOTCMD_PACKET_APPEND_FAILED,
    ZMC_ERR_MOTCMD_QUEUE_PUSH_FAILED,
    ZMC_ERR_MOTCMD_INVALID_ARG,
    ZMC_ERR_MOTCMD_SCRIPT_EMPTY,
    ZMC_ERR_MOTCMD_SCRIPT_LINE_EMPTY,
    ZMC_ERR_MOTCMD_SCRIPT_MISSING_ARG,
    ZMC_ERR_MOTCMD_SCRIPT_PARSE_FAILED,
    ZMC_ERR_MOTCMD_UNKNOWN_COMMAND,
    ZMC_ERR_MOTCMD_IO_RESULTS_EMPTY,
    ZMC_ERR_MOTCMD_AXIS_INFO_UNAVAILABLE,
    ZMC_ERR_MOTCMD_WAIT_TIMEOUT,           // Blocking wait did not reach its target state in time
    ZMC_ERR_MOTCMD_WAIT_AXIS_ERROR,        // Blocking wait ended because the axis entered ErrorStop
    ZMC_ERR_MOTCMD_FLOW_PAUSED,           // Heartbeat flow control is awaiting acknowledgement

    // Host ENI descriptor parsing (pcie_mst)
    ZMC_ERR_ENI_FILE_OPEN_FAILED = 17301,
    ZMC_ERR_ENI_JSON_NOT_FOUND,
    ZMC_ERR_ENI_JSON_INVALID,
    ZMC_ERR_ENI_SLAVE_INVALID,
    ZMC_ERR_ENI_SLAVE_OVERFLOW,
    ZMC_ERR_ENI_AXIS_OVERFLOW,
    ZMC_ERR_ENI_POD_OVERFLOW,
    ZMC_ERR_ENI_FILE_TOO_LARGE,
    // Host INI configuration parsing (pcie_mst user.ini)
    ZMC_ERR_INI_FILE_OPEN = 17351,  // fopen/fseek/fread failed before content parse
    ZMC_ERR_INI_PARSE,              // file exceeds size limit or buffer parse failed
    ZMC_ERR_INI_MISSING_ENI_PATH,   // [base] section has no eni_path key
    ZMC_ERR_INI_MISSING_SECTION,
    ZMC_ERR_INI_MISSING_KEY,
    ZMC_ERR_INI_KEY,
    ZMC_ERR_INI_VALUE_FORMAT,
    ZMC_ERR_INI_VALUE_RANGE,
    ZMC_ERR_INI_DUPLICATE,
    ZMC_ERR_INI_AXIS_BINDING_CONFIG,
    ZMC_ERR_INI_IO_BINDING_CONFIG,
    ZMC_ERR_INI_BINARY_FORMAT,
    ZMC_ERR_INI_BINARY_VERSION,
    // AI modification signature: GPT-5 Codex

    ZMC_ERR_AX_BIND_ADDR_FAILED = 17371,
    ZMC_ERR_AX_BIND_OVER_RANGE,
    ZMC_ERR_AX_BIND_OVERLAP,

    // Device MC API / mailbox / PCIe transport (pcie_mst MCCommResult)
    ZMC_ERR_DEV_INVALID_HANDLE = 17401,
    ZMC_ERR_DEV_RECV_BUFF_TOO_SMALL,
    ZMC_ERR_DEV_RECV_TIMEOUT,
    ZMC_ERR_DEV_SEND_TIMEOUT,
    ZMC_ERR_DEV_INVALID_ARG,
    ZMC_ERR_DEV_NO_DATA,
    ZMC_ERR_DEV_NOT_OPEN,
    ZMC_ERR_DEV_MALLOC_FAILED,
    ZMC_ERR_DEV_BOARD_OPEN_FAILED,
    ZMC_ERR_DEV_LOCAL_FILE_OPEN_FAILED,
    ZMC_ERR_DEV_FILE_TRANSFER_FAILED,
    ZMC_ERR_DEV_IPC_PIPE_FAILED,
    ZMC_ERR_DEV_NOT_IMPLEMENTED,
    ZMC_ERR_DEV_INTERNAL,
    ZMC_ERR_DEV_FILE_STAT_FAILED,
    ZMC_ERR_DEV_NULL_ARG,    // Required device-side argument pointer is null.
    ZMC_ERR_DEV_OVER_RANGE,  // Device-side argument value exceeds the accepted range.

    ZMC_ERR_DEV_MASTER_STATE_SWITCH_TIMEOUT,  // EtherCAT master did not reach requested state.
    ZMC_ERR_DEV_QUEUE_FULL,
    ZMC_ERR_DEV_SDO_ADAPTER_NULL_ARG = 17431,  // SDO adapter received a required null argument.
    ZMC_ERR_DEV_UNKNOWN = 17445,

    // AxisFbBridge local API errors
    ZMC_ERR_BRIDGE_INVALID_ARG = 17700,
    ZMC_ERR_BRIDGE_INVALID_HANDLE,
    ZMC_ERR_BRIDGE_PDO_UNCONFIGURED,
    ZMC_ERR_BRIDGE_PDO_EMPTY_LAYOUT,
    ZMC_ERR_BRIDGE_ECAT_STATE_ADAPTER_INVALID,

    // Bridge IO manager errors
    ZMC_ERR_IO_INVALID_ARG = 17720,
    ZMC_ERR_IO_OBJECT_NOT_FOUND,
    ZMC_ERR_IO_ALIAS_DUPLICATE,
    ZMC_ERR_IO_ALIAS_CAPACITY,
    ZMC_ERR_IO_PROCESS_IMAGE_UNBOUND,
    ZMC_ERR_IO_DATA_SIZE_UNSUPPORTED,
    ZMC_ERR_IO_PROCESS_IMAGE_OVER_RANGE,
    ZMC_ERR_IO_MASK_INVALID,

    // SDO request, cache, and worker-local errors.
    ZMC_ERR_SDO_INVALID_ARG = 17740,
    ZMC_ERR_SDO_INVALID_LENGTH,
    ZMC_ERR_SDO_QUEUE_FULL,
    ZMC_ERR_SDO_NO_RESULT,
    ZMC_ERR_SDO_WATCH_NOT_FOUND,
    ZMC_ERR_SDO_CAPACITY,
    ZMC_ERR_SDO_ADAPTER_INVALID,
    ZMC_ERR_BRIDGE_SDO_ADAPTER_INVALID,
    ZMC_ERR_BRIDGE_SDO_NOT_STARTED,
    ZMC_ERR_BRIDGE_SDO_THREAD_START_FAILED,

    // Trajectory and spline errors
    ZMC_ERR_TRAJ_BUILD_FAILED = 17800,
    ZMC_ERR_TRAJ_NOT_IMPLEMENTED,
    ZMC_ERR_TRAJ_INVALID_MODE,
    ZMC_ERR_TRAJ_INVALID_AXIS_NUM,
    ZMC_ERR_TRAJ_CACHE_INVALID,
    ZMC_ERR_TRAJ_ALLOC_FAILED,
    ZMC_ERR_TRAJ_INVALID_SPLINE_DATA,
    ZMC_ERR_TRAJ_INVALID_ARG,

    // Motion RPC demo client (MotionRpcApi)
    ZMC_ERR_RPC_CLIENT_CONNECT_FAILED = 18200,
    ZMC_ERR_RPC_CLIENT_NOT_OPEN,
    ZMC_ERR_RPC_CLIENT_DISCONNECTED,
    ZMC_ERR_RPC_CLIENT_CONNECT_TIMEOUT,
    ZMC_ERR_RPC_CLIENT_CALL_FAILED,

    ZMC_ERR_RPC_JSON_INVALID = 18220,
    ZMC_ERR_RPC_JSON_MISSING_FIELD,
    ZMC_ERR_RPC_CLIENT_INVALID_ARG,
    ZMC_ERR_RPC_RECV_BUFF_TOO_SMALL,
    ZMC_ERR_RPC_INVALID_ARG,

    // HTTP remote file service (/zmot/api/files); do not reuse HTTP status numbers as codes
    ZMC_ERR_RPC_FILE_INVALID_NAME = 18240,  // Remote name grammar rejected
    ZMC_ERR_RPC_FILE_NOT_FOUND,             // Missing or non-regular target
    ZMC_ERR_RPC_FILE_ALREADY_EXISTS,        // Create would overwrite an existing name
    ZMC_ERR_RPC_FILE_TOO_LARGE,             // Upload exceeds the 16 MiB limit
    ZMC_ERR_RPC_FILE_INVALID_REQUEST,       // Multipart/body shape is invalid
    ZMC_ERR_RPC_FILE_IO_FAILED,             // Root/list/read/write/publish I/O failure

    // EtherCAT synchronous slave state query
    ZMC_ERR_ECAT_SLAVE_NOT_PRESENT = 19006,
    ZMC_ERR_ECAT_SLAVE_NOT_OP = 19007,
    ZMC_ERR_ECAT_INVALID_SLAVE_INDEX = 19008,
    ZMC_ERR_ECAT_GET_SLAVE_STATE_FAILED = 19009,
    ZMC_ERR_ECAT_STATE_ADAPTER_INVALID,
    ZMC_ERR_ECAT_GET_SLAVE_COUNT_FAILED,
    ZMC_ERR_ECAT_GET_MASTER_STATE_FAILED,
    ZMC_ERR_ECAT_MASTER_NOT_OP,
    ZMC_ERR_ECAT_SLAVE_COUNT_OVERFLOW,

    // EtherCAT local API errors. AI modification signature: GPT-6, 2026-10-21.
    ZMC_ERR_ECAT_NULL_ARG,  // A required argument pointer is null.
    ZMC_ERR_ECAT_INVALID_HANDLE,  // The master handle is not available.
    ZMC_ERR_ECAT_MASTER_REQ_STATE_UNAVAILABLE,  // No successful state request is recorded.
    ZMC_ERR_ECAT_ALREADY_RUNNING,  // Configuration cannot change while the bus is started.
    ZMC_ERR_ECAT_SERVO_OP_MODE_MISMATCH,  // SDO readback differs from the requested mode.

    // Host-prefixed utils ENI parsing errors (= ZMC_ERR_ENI_* + ZMC_ERR_UTILS_HOST_OFFSET)
    ZMC_ERR_HOST_ENI_FILE_OPEN_FAILED = ZMC_ERR_ENI_FILE_OPEN_FAILED + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_ENI_JSON_NOT_FOUND,
    ZMC_ERR_HOST_ENI_JSON_INVALID,
    ZMC_ERR_HOST_ENI_SLAVE_INVALID,
    ZMC_ERR_HOST_ENI_SLAVE_OVERFLOW,
    ZMC_ERR_HOST_ENI_AXIS_OVERFLOW,
    ZMC_ERR_HOST_ENI_POD_OVERFLOW,
    ZMC_ERR_HOST_ENI_FILE_TOO_LARGE,

    // Host-prefixed utils INI parsing errors (= ZMC_ERR_INI_* + ZMC_ERR_UTILS_HOST_OFFSET)
    ZMC_ERR_HOST_INI_FILE_OPEN = ZMC_ERR_INI_FILE_OPEN + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_INI_PARSE,
    ZMC_ERR_HOST_INI_MISSING_ENI_PATH,
    ZMC_ERR_HOST_INI_AXIS_NUM_OVER,

    // Host-side local API errors (= matching ZMC_ERR_DEV_* + ZMC_ERR_UTILS_HOST_OFFSET)
    ZMC_ERR_HOST_INVALID_HANDLE = ZMC_ERR_DEV_INVALID_HANDLE + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_RECV_BUFF_TOO_SMALL = ZMC_ERR_DEV_RECV_BUFF_TOO_SMALL + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_RECV_TIMEOUT = ZMC_ERR_DEV_RECV_TIMEOUT + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_SEND_TIMEOUT = ZMC_ERR_DEV_SEND_TIMEOUT + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_INVALID_ARG = ZMC_ERR_DEV_INVALID_ARG + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_NO_DATA = ZMC_ERR_DEV_NO_DATA + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_NOT_OPEN = ZMC_ERR_DEV_NOT_OPEN + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_MALLOC_FAILED = ZMC_ERR_DEV_MALLOC_FAILED + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_BOARD_OPEN_FAILED = ZMC_ERR_DEV_BOARD_OPEN_FAILED + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_LOCAL_FILE_OPEN_FAILED = ZMC_ERR_DEV_LOCAL_FILE_OPEN_FAILED +
        ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_FILE_TRANSFER_FAILED = ZMC_ERR_DEV_FILE_TRANSFER_FAILED +
        ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_IPC_PIPE_FAILED = ZMC_ERR_DEV_IPC_PIPE_FAILED + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_NOT_IMPLEMENTED = ZMC_ERR_DEV_NOT_IMPLEMENTED + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_INTERNAL = ZMC_ERR_DEV_INTERNAL + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_FILE_STAT_FAILED = ZMC_ERR_DEV_FILE_STAT_FAILED + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_NULL_ARG = ZMC_ERR_DEV_NULL_ARG + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_OVER_RANGE = ZMC_ERR_DEV_OVER_RANGE + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_MASTER_STATE_SWITCH_TIMEOUT = ZMC_ERR_DEV_MASTER_STATE_SWITCH_TIMEOUT +
        ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_SDO_ADAPTER_NULL_ARG = ZMC_ERR_DEV_SDO_ADAPTER_NULL_ARG +
        ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_UNKNOWN = ZMC_ERR_DEV_UNKNOWN + ZMC_ERR_UTILS_HOST_OFFSET,
    ZMC_ERR_HOST_HANDLE_ALREADY_OPEN, /* host-only; no device twin */
    ZMC_ERR_HOST_FILE_OPEN_FAILED,    /* host-only; generic local file open */
    ZMC_ERR_HOST_SESSION_CAPACITY,    /* host-only; all eight service session slots are occupied */

    // Deprecated MCCommResult names; use ZMC_ERR_DEV_* above
    MC_COMM_S_OK = ZMC_NO_ERROR,

    // depreciated
    // mcErrorCode_AxisErrorStop = 0x4500,         // Axis error stopped
    // mcErrorCode_Servo_No_Error = 0x4600,               // Servo has no error
    // mcErrorCode_power_timeout2,

    // mcErrorCode_push_motion_to_buffer_while_error = 20000,
    // mcErrorCode_push_motion_to_buffer_while_stopping,
    // mcErrorCode_motion_pool_is_exhausted,
    // mcErrorCode_push_too_many_motion_to_buffer,
    // mcErrorCode_motion_buffer_is_full,
    // mcErrorCode_motnode_error,
    // mcErrorCode_Vel_plan_failed,
    // mcErrorCode_fb_read_failed,
    // mcErrorCode_fb_write_failed,
} MC_ERROR_CODE;

// typedef MC_ERROR_CODE ZMC_ERROR_CODE;
// typedef MC_ERROR_CODE MC_ERROR;
typedef uint16_t MC_ERROR;
#endif
