
#include "AxisCiaConfig.hpp"
#include "mc_enum_def_spec.h"
#include "fbt_errorcode_def.h"

void DebugParam_default_init(DebugParam* cfg){
    cfg->enable_servoAda_param = true;
    cfg->eAxisMtype = 0;
    cfg->mode = 0;
    cfg->limit_flag.value = 0;
    cfg->batch_size = 1;
    cfg->axis_dtype = 0;
    cfg->level = 0;
    cfg->log_type = 0;
    cfg->modulo = 2024;
    cfg->exit_when_count = 0;
    cfg->dbg_input_delay = 0;
    cfg->dbg_output_delay = 0;

    cfg->servo_fak_statw_sm = 0;
    cfg->servo_fak_pos = 0;
    cfg->servo_fak_rcfilter_scale = 0;

    cfg->arg_value_flag = 0;

    cfg->axisAddr = 0;

    cfg->arg_value[0] = 0;
    cfg->arg_value[1] = 0;
    cfg->arg_value[2] = 0;
    
    cfg->arg_value_f[0] = 0;
    cfg->arg_value_f[1] = 0;
    cfg->arg_value_f[2] = 0;
    cfg->arg_value_f[3] = 0;
}

void AxisCiaConfig_default_init(AxisCiaConfig* cfg){
    cfg->axis_index = 0;
    cfg->slave_node_id = 0;
    cfg->id = 0;
    cfg->is_axis_virtual = false;
    cfg->cycle_time = 1000;
    cfg->axis_type = AXIS_TYPE_FINITE;
    cfg->modulo.modulo_value = 1e11;
    cfg->finite.software_limits.is_activate = false;
    cfg->finite.software_limits.negative = -1e30;
    cfg->finite.software_limits.positive = 1e30;
    cfg->dynamic_limits.velocity = 1000;
    cfg->dynamic_limits.acceleration = 5000;
    cfg->dynamic_limits.deceleration = 5000;
    cfg->dynamic_limits.jerk = 0;

    cfg->is_invert_direction = false;
    cfg->motor_type = MOTOR_TYPE_LINEAR;
    cfg->linear.increments_units_in_application_num = 1;
    cfg->linear.increments_units_in_application_den = 1;
    
    cfg->rotary.gear_application_num = 1;
    cfg->rotary.gear_application_den = 1;
    cfg->rotary.increments_motor_turns_num = 1;
    cfg->rotary.increments_motor_turns_den = 1;
    cfg->rotary.motor_gear_num = 1;
    cfg->rotary.motor_gear_den = 1;

    cfg->software_error_reaction.deceleration = 1e8;
    cfg->software_error_reaction.max_distance = 0;
    cfg->velocity_ramp_option = VEL_RAMP_TYPE_TRAPEZOID;

    cfg->position_lag_option = POS_LAG_SUPER_DEACTIVATED;
    cfg->position_lag_supervision.is_changeable = false;
    cfg->position_lag_supervision.value = 1e8;

    cfg->is_automatic_mapping = false;
    cfg->bit_width = 32;
    cfg->mapping_id = 0;
    PdoEnableFlag_init(&cfg->pdo_flag);

    cfg->channels_stat_name = 0;
    cfg->channels_ctrl_name = 0;
    cfg->task_name = 0;
    DebugParam_default_init(&cfg->debug);
}

int AxisCiaConfig_check(const AxisCiaConfig* pCfg){
    auto& cfg = *pCfg;
    if (cfg.axis_type != AXIS_TYPE_FINITE && cfg.axis_type!= AXIS_TYPE_MODULO){
        return ZMC_INVALID_AXIS_TYPE;
    }
    if (cfg.axis_type == AXIS_TYPE_MODULO && cfg.modulo.modulo_value <= 0){
        return ZMC_NEGATIVE_MODULO_PERIOD;
    }  
    if (cfg.cycle_time <= 0){
        return ZMC_P_FTASKCYCLE_EMPTY;
    } 
    if (cfg.motor_type == MOTOR_TYPE_LINEAR && 
        (cfg.linear.increments_units_in_application_den == 0 || cfg.linear.increments_units_in_application_num == 0)){
        return ZMC_CGR_ZERO_VALUES;
    }
    if (cfg.motor_type == MOTOR_TYPE_ROTARY && 
        (cfg.rotary.gear_application_den == 0 || cfg.rotary.gear_application_num == 0 ||
        cfg.rotary.increments_motor_turns_den == 0 || cfg.rotary.increments_motor_turns_num == 0 ||
        cfg.rotary.motor_gear_den == 0 || cfg.rotary.motor_gear_num == 0)){
        return ZMC_CGR_ZERO_VALUES;
    }
    
    if (cfg.debug.limit_flag.bDynamicLimitEnable){
        if (cfg.dynamic_limits.velocity <= 0 || cfg.dynamic_limits.acceleration <= 0 || cfg.dynamic_limits.deceleration <= 0 )
            return ZMC_SDL_INVALID_VELACC_VALUES;
    }
    if (cfg.axis_type == AXIS_TYPE_FINITE && cfg.finite.software_limits.is_activate){
        if (cfg.finite.software_limits.negative >= cfg.finite.software_limits.positive)
            return ZMC_DI_SWLIMITS_EXCEEDED;
    }
    if (cfg.debug.limit_flag.bDynamicLimitEnable || (cfg.axis_type == AXIS_TYPE_FINITE && cfg.finite.software_limits.is_activate))
    if (cfg.software_error_reaction.deceleration < 0 || cfg.software_error_reaction.max_distance < 0 )
        return ZMC_ERR_INVALID_SW_ERROR_REACTION; 
    // if (cfg.debug.limit_flag.bHWLimitEnable){
        // if (fHWMaxVelocity <= 0 || fHWMaxAcceleration <= 0 || fHWMaxDeceleration <= 0)
        //     return ZMC_ERR_CFG_HARDWARE_PARAM_INVALID;
    // }
    return ZMC_NO_ERROR;
}