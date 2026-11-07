/*
 * @Author: ChenShengyong
 * @Date: 2024-06-19 17:13:17
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-05-16 18:06:49
 */
#ifndef _MC_ENUM_DEF_H_
#define _MC_ENUM_DEF_H_

#include "stdint.h"

namespace zmc {

typedef enum MC_BUFFER_MODE : uint16_t {
    /**
     * @brief Start FB immediately (default mode).
     * The next FB aborts an ongoing motion and the command
     * affects the axis immediately. The buffer is cleared.
     */
    mcAborting = 0,
    /**
     * @brief Start FB after current motion has finished.
     * The next FB affects the axis as soon as the previous movement is 'Done'.
     * There is no blending.
     */
    mcBuffered = 1,
    /**
     * @brief The velocity is blended with the lowest velocity of both FBs.
     *
     */
    mcBlendingLow = 2,       // The velocity is blended with the lowest velocity of both FBs
    mcBlendingPrevious = 3,  // The velocity is blended with the velocity of the first FB
    mcBlendingNext = 4,      // The velocity is blended with velocity of the second FB
    mcBlendingHigh = 5       // The velocity is blended with highest velocity of both FBs
} MC_BUFFER_MODE;


typedef enum MC_DIRECTION : int16_t {
    mcNegativeDirection = -1,
    mcShortestWay = 0,  // only modulo axes
    mcPositiveDirection,
    mcCurrentDirection,  // only modulo axes
    mcFastestWay,        // only modulo axes (codesys only)
    mcFiniteDirection,   // MoveAbsolute uses the target as a finite-axis position
} MC_DIRECTION;

// use in MC_CamIn
typedef enum MC_STARTMODE : uint16_t {
    mcAbsolute,
    mcRelative,
    mcRampIn,
    mcRampInPos,  // codesys only
    mcRampInNeg,  // codesys only
} MC_STARTMODE;

// Defines the homing sequence. Used in function block SMC_Homing.
typedef enum MC_HOMING_MODE : uint16_t {
    FAST_BSLOW_S_STOP =
        0,  // Move with fast velocity onto the reference switch; invert and move with slow
            // velocity out of the reference switch; execute "set position"; stop
    FAST_BSLOW_STOP_S =
        1,  // Move with fast velocity onto the reference switch; invert and move with slow
            // velocity out of the reference switch; stop; execute "set position"
    FAST_BSLOW_I_S_STOP = 2,  // Move with fast velocity onto the reference switch; invert and move
                              // with slow velocity out of the reference switch; wait for the index
                              // pulse; execute "set position"; stop
    FAST_SLOW_S_STOP = 4,     // Move with fast velocity onto the reference switch; move with slow
                              // velocity out of the reference switch; execute "set position"; stop
    FAST_SLOW_STOP_S = 5,     // Move with fast velocity onto the reference switch; move with slow
                              // velocity out of the reference switch; stop; execute "set position"
    FAST_SLOW_I_S_STOP =
        6,  // Move with fast velocity onto the reference switch; move with slow velocity out of
            // the reference switch; wait for the index pulse; execute "set position"; stop
    // FAST_BSLOW_STOP
    // SLOW_STOP_S,
    // SLOW_S_STOP,
    // DIRECT_S,
} MC_HOMING_MODE;

// use in MC_DigitalCAMSwitch
// This global variable is part of the library SM3_Basic. It is used at the function block  and
// specifies whether the tappet values refer to the set position of the master axis (1) or the
// actual position (2). In case of auto detection (0), the function block decides depending on the
// control state of the drive if it uses the set values (bRegulatorOn = TRUE) or the actual values
// (bRegulatorOn = FALSE).
typedef enum MC_TAPPETMODE : uint16_t {
    tp_mode_auto = 0,            //  0 Auto mode
    tp_mode_demandposition = 1,  // Use set values
    tp_mode_actualposition = 2,  // Use actual values
} MC_TAPPETMODE;

// Ramp type enum
// use in FbSetRampType
typedef enum MC_RAMPTYPE_t : uint16_t {
    trapez = 0,
    sinsquare = 1,
    quadratic_ramp = 2,
    quadratic_smooth_ramp = 3
} MC_RAMPTYPE;

// use in SMC_SetControllerMode
typedef enum MC_CONTROLLER_MODE_t : uint16_t {
    ZMC_no_control = 0,
    ZMC_torque = 1,    // Torque control mode (CST)
    ZMC_velocity = 2,  // Velocity control mode (CSV)
    ZMC_position = 3,  // Position control mode (CSP)
    ZMC_current = 4,
} MC_CONTROLLER_MODE;

// use in MC_ReadMotionState
typedef enum { mcCommandedValue, mcSetValue, mcActualValue } MC_SOURCE;

// use in MC_SetPosition
typedef enum MC_EXECUTION_MODE_t { mcImmediately, mcQueued } MC_EXECUTION_MODE;

// use in MC_GearInPos
typedef enum MC_SYNC_MODE_t { mcShortest, mcCatchUp, mcSlowDown } MC_SYNC_MODE;

// use in  MC_CombineAxes
typedef enum MC_COMBINE_MODE_t { mcAddAxes, mcSubAxes } MC_COMBINE_MODE;

// Custom ramp type for SMC_SetCustomRampType
typedef int16_t MC_TG_IRAMPTYPE;

// Logger mode for SMC_AxisDiagnosticLog
typedef enum MC_LOGGERMODE_t : uint16_t { LOG_CONTINUOUS = 0, LOG_AT_CLOSE = 1 } MC_LOGGERMODE;

// Communication state for SMC_CheckAxisCommunication
typedef enum MC_CommunicationState_t : uint16_t {
    MC_COMSTATE_NOT_STARTED,
    MC_COMSTATE_VARIABLE_INITIALIZATION,
    MC_COMSTATE_BASE_COM_INITIALIZATION,
    MC_COMSTATE_DRIVE_INITIALIZATION,
    MC_COMSTATE_DRIVE_WAITING_FOR_SYNC,
    MC_COMSTATE_INITIALIZATION_DONE,
    MC_COMSTATE_OPERATIONAL,
    MC_COMSTATE_REINITIALIZATION,
    MC_COMSTATE_ERROR,
    MC_COMSTATE_UNKNOWN
} MC_CommunicationState;

// Backlash compensation mode for SMC_BacklashCompensation
typedef enum MC_BACKLASH_MODE_t : int16_t {
    MC_BL_NEGATIVE =
        -1,  // Compensation in negative direction, independent of the movement of the master axis
    MC_BL_OFF = 0,  // No backlash compensation
    MC_BL_POSITIVE =
        1,  // Compensation in positive direction, independent of the movement of the master axis
    MC_BL_AUTO =
        2,  // The direction in which the master axis moves determines the compensation direction
} MC_BACKLASH_MODE;

// Backlash start state for SMC_BacklashCompensation
typedef enum MC_BACKLASH_STARTSTATE_t : int16_t {
    MC_BL_START_NONE =
        0,  // The slave axis is not under traction; a movement in positive or negative direction
            // will cause a corresponding compensating movement of distance fBacklash
    MC_BL_START_POSITIVE =
        1,  // The slave axis is under traction in positive direction, that is: a movement in
            // positive direction needs no compensation; in case if a movement in negative
            // direction the double fBacklash distance will be made up
    MC_BL_START_NEGATIVE =
        -1  // The slave axis is under traction in negative direction, that is: a movement in
            // negative direction needs no compensation; in case of a movement in positive
            // direction the double fBacklash distance will be made up
} MC_BACKLASH_STARTSTATE;

// Brake set state for SMC3_BrakeControl
typedef enum MC_BrakeSetState_t : int16_t {
    MC_BRAKE_AUTO,  // The opening and closing of the brake is handled by the drive itself
    MC_BRAKE_OPEN,  // The brake is open
    MC_BRAKE_CLOSE  // The brake is closed
} MC_BrakeSetState;

// Value select for SMC_Logical_Axis
typedef enum MC_ValueSelect_t : int16_t {
    ZMC_VALUESELECT_SET = 0,     // set
    ZMC_VALUESELECT_ACTUAL = 1,  // actual

    // add by ChenSy
    ZMC_VALUESELECT_SAVE = 2  // fSavePosition / fSaveVelocity / fSaveAcceleration
} MC_ValueSelect;

}  // namespace zmc

#endif
