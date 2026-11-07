#ifndef _ZMC_AXIS_CIA_CONFIG_HPP_
#define _ZMC_AXIS_CIA_CONFIG_HPP_

#include "stdint.h"
#include "PdoItemFlag.hpp"

#ifdef __cplusplus
extern "C"{
#endif


typedef struct AxisDynamicLimits_t {
    double velocity;
    double acceleration;
    double deceleration;
    double jerk;
} AxisDynamicLimits;

typedef struct AxisSoftwareErrorReaction_t {
    double deceleration;
    double max_distance;
} AxisSoftwareErrorReaction;

typedef struct AxisLinearParam_t {
    uint32_t increments_units_in_application_num;
    uint32_t increments_units_in_application_den;
} AxisLinearParam;

typedef union UnSwHwLimitFlag{
    // default is 0x00
    uint8_t value;
    struct{
        // SWLimit, PosDirection, NegDirection is default enabled
        // others is default disabled
        bool bSWLimitDisable:1;
        bool bHWLimitEnable:1;
        bool bDynamicLimitEnable:1;
        bool bDiscontinuityCheckEnable:1; 
        bool bPosDirectionDisable:1;
        bool bNegDirectionDisable:1;
    };
}UnSwHwLimitFlag;


typedef struct DebugParam_t {
    bool enable_servoAda_param;
    uint8_t eAxisMtype;  // see AXIS_CTYPE
    UnSwHwLimitFlag limit_flag;
    uint8_t mode;
    uint8_t level;
    uint8_t axis_dtype; // deaf or dummy
    uint8_t arg_value_flag;
    uint8_t log_type;
    uint16_t batch_size;
    uint32_t modulo;
    uint16_t dbg_input_delay;
    uint16_t dbg_output_delay;
    uint32_t exit_when_count;
    uint8_t servo_fak_statw_sm;
    int32_t servo_fak_pos;
    double servo_fak_rcfilter_scale;
    int axisAddr;
    int32_t arg_value[3];
    double arg_value_f[4];
} DebugParam;

// TODO
typedef struct FiniteLimit_t {
    bool is_activate;
    double negative;
    double positive;
} FiniteLimit;


typedef struct PositionLagSupervision_t {
    bool is_changeable;
    double value;
} PositionLagSupervision;


typedef struct RotaryParam_t {
    uint32_t gear_application_num;
    uint32_t gear_application_den;
    uint32_t increments_motor_turns_num;
    uint32_t increments_motor_turns_den;
    uint32_t motor_gear_num;
    uint32_t motor_gear_den;
} RotaryParam;

typedef struct AxisCiaConfig{
    bool is_axis_virtual;

    uint32_t axis_index;
    uint32_t slave_node_id;
    uint32_t id;
    uint32_t cycle_time; // cycle time (us)
    // axis type:  true for finite, false for modulo
    uint8_t axis_type;
    struct{
        double modulo_value;
    }modulo;
    struct{
        // finite axis max & min range
        struct{
            bool is_activate;
            double negative;
            double positive;
        }software_limits;
    }finite;

    AxisDynamicLimits dynamic_limits;

    AxisSoftwareErrorReaction software_error_reaction;

    // @see also Velocity_Ramp_Type
    uint8_t velocity_ramp_option;
    // see also Position_Lag_Supervision
    uint8_t position_lag_option;
    struct{
        bool is_changeable;
        double value;
    }position_lag_supervision;

    // motion type : false for linear, true for rotary
    uint8_t motor_type;
    bool is_invert_direction;
    AxisLinearParam linear;
    struct{
        uint32_t gear_application_num;
        uint32_t gear_application_den;
        uint32_t increments_motor_turns_num;
        uint32_t increments_motor_turns_den;
        uint32_t motor_gear_num;
        uint32_t motor_gear_den;
    }rotary;

    char* task_name;
    bool is_automatic_mapping;
    uint32_t channels_stat_len;
    uint32_t channels_ctrl_len;
    char* channels_stat_name;
    char* channels_ctrl_name;
    uint32_t mapping_id;
    uint8_t bit_width;
    PdoEnableFlag pdo_flag;
    DebugParam debug;
}AxisCiaConfig;


void AxisCiaConfig_default_init(AxisCiaConfig* cfg);
int AxisCiaConfig_check(const AxisCiaConfig* pCfg);

inline bool AxisCiaConfig_is_axis_deaf(const AxisCiaConfig* cfg){
    return cfg->debug.axis_dtype & 1; 
}

inline bool AxisCiaConfig_is_axis_dummy(const AxisCiaConfig* cfg){
    return cfg->debug.axis_dtype & 2; 
}
inline uint32_t AxisCiaConfig_get_pdo_item_flag(const AxisCiaConfig* cfg){
    return cfg->pdo_flag.enable_value;
}

#ifdef __cplusplus
}
#endif


#endif