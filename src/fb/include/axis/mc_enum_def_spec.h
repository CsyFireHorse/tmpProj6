#ifndef _MC_ENUM_DEF_SPEC_H_
#define _MC_ENUM_DEF_SPEC_H_


// The type (finite or modulo) of SoftMotion axes.
typedef enum _Axis_Type_t {
    AXIS_TYPE_MODULO,
    AXIS_TYPE_FINITE,
} Axis_Type;


typedef enum _Motor_Type_t {
    MOTOR_TYPE_ROTARY,
    MOTOR_TYPE_LINEAR,
} Motor_Type;

// see also MC_RAMPTYPE
typedef enum _Velocity_Ramp_Type_t {
    VEL_RAMP_TYPE_TRAPEZOID,
    VEL_RAMP_TYPE_SIN2,
    VEL_RAMP_TYPE_QUADRATIC,
    VEL_RAMP_TYPE_QUADRATIC_SMOOTH,
} Velocity_Ramp_Type;

// SMC3_CheckPositionLagMode 
typedef enum _Position_Lag_Supervision_t {
    POS_LAG_SUPER_DEACTIVATED,    // No check
    POS_LAG_SUPER_DISABLE_DRIVE,  // Disables (bRegulatorOn => FALSE) after position lag error has
                                  // occured
    POS_LAG_SUPER_DO_QUICK_STOP,  // Halts (bDriveStart => FALSE) after position lag error has
                                  // occured.
    POS_LAG_SUPER_STAY_ENABLED,   // Generates error but stay enabled/started.
} Position_Lag_Supervision;

#endif