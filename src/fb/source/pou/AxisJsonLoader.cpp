#include "AxisJsonLoader.h"

#include <fstream>
#include <string>

#include "common/thirdlib/nlohmann/json.hpp"
#include "mc_enum_def_spec.h"
#include "stdio.h"
#include "string.h"

using nlohmann::json;

namespace zmc {

#define JSON_CONTAIN(DICT, T, KEY, VAL) ((DICT).contains(KEY) ? (DICT)[KEY].get<T>() : (VAL))

int load_axis_json(const char* jsonFile, AxisCiaConfig& cfg) {
    std::ifstream f_new(jsonFile);
    if (!f_new) {
        perror("json file not found\n");
        return -1;
    }
    json jdata = json::parse(f_new);

    cfg.cycle_time = JSON_CONTAIN(jdata, int, "task_cycle_time", 10000);
    cfg.axis_index = JSON_CONTAIN(jdata, int, "axis_index", 0);
    cfg.id = jdata["identification"]["id"].get<int>();
    cfg.slave_node_id = JSON_CONTAIN(jdata, int, "slave_node_id", 0);

    const auto axis_type_str =
        jdata["axis_type_and_limits"]["axis_type"]["select_value"].get<std::string>();
    cfg.axis_type = axis_type_str == "modulo";
    auto axis_type_option = jdata["axis_type_and_limits"]["axis_type"]["select_option"];

    cfg.finite.software_limits.is_activate =
        axis_type_option["finite"]["software_limits"]["activate"].get<bool>();
    cfg.finite.software_limits.positive =
        axis_type_option["finite"]["software_limits"]["positive"].get<double>();
    cfg.finite.software_limits.negative =
        axis_type_option["finite"]["software_limits"]["negative"].get<double>();

    cfg.modulo.modulo_value =
        axis_type_option["modulo"]["modulo_settings"]["modulo_value"].get<int>();

    cfg.software_error_reaction.deceleration =
        jdata["axis_type_and_limits"]["software_error_reaction"]["deceleration"].get<double>();
    cfg.software_error_reaction.max_distance =
        jdata["axis_type_and_limits"]["software_error_reaction"]["max_distance"].get<double>();

    cfg.is_axis_virtual = jdata["axis_type_and_limits"]["virtual_mode"].get<bool>();

    cfg.dynamic_limits.velocity = jdata["dynamic_limits"]["velocity"].get<double>();
    cfg.dynamic_limits.acceleration = jdata["dynamic_limits"]["acceleration"].get<double>();
    cfg.dynamic_limits.deceleration = jdata["dynamic_limits"]["deceleration"].get<double>();
    cfg.dynamic_limits.jerk = jdata["dynamic_limits"]["jerk"].get<double>();

    const auto velocity_ramp_str = jdata["velocity_ramp_type"]["select_value"].get<std::string>();
    if (velocity_ramp_str == "sin2")
        cfg.velocity_ramp_option = VEL_RAMP_TYPE_SIN2;
    else if (velocity_ramp_str == "quadratic")
        cfg.velocity_ramp_option = VEL_RAMP_TYPE_QUADRATIC;
    else if (velocity_ramp_str == "quadratic(smooth)")
        cfg.velocity_ramp_option = VEL_RAMP_TYPE_QUADRATIC_SMOOTH;
    else
        cfg.velocity_ramp_option = VEL_RAMP_TYPE_TRAPEZOID;

    const auto position_lag_str =
        jdata["position_lag_supervision"]["select_value"].get<std::string>();
    if (position_lag_str == "disable_drive")
        cfg.position_lag_option = POS_LAG_SUPER_DISABLE_DRIVE;
    else if (position_lag_str == "do_quick_stop")
        cfg.position_lag_option = POS_LAG_SUPER_DO_QUICK_STOP;
    else if (position_lag_str == "stay_enabled")
        cfg.position_lag_option = POS_LAG_SUPER_STAY_ENABLED;
    else
        cfg.position_lag_option = POS_LAG_SUPER_DEACTIVATED;

    cfg.position_lag_supervision.is_changeable = jdata["position_lag_supervision"]["select_option"]
                                                      [position_lag_str]["lag_limit"]["changeable"]
                                                          .get<bool>();
    cfg.position_lag_supervision.value =
        jdata["position_lag_supervision"]["select_option"][position_lag_str]["lag_limit"]["value"]
            .get<double>();

    const auto motor_type_str = jdata["motor_type"]["select_value"].get<std::string>();
    cfg.motor_type = motor_type_str == "rotary";
    cfg.is_invert_direction =
        jdata["motor_type"]["select_option"][motor_type_str]["scaling"]["invert_direction"]
            .get<bool>();
    cfg.linear.increments_units_in_application_num =
        jdata["motor_type"]["select_option"]["linear"]["scaling"]
             ["increments_units_in_application"][0]
                 .get<int>();
    cfg.linear.increments_units_in_application_den =
        jdata["motor_type"]["select_option"]["linear"]["scaling"]
             ["increments_units_in_application"][1]
                 .get<int>();
    cfg.rotary.gear_application_num = jdata["motor_type"]["select_option"]["rotary"]["scaling"]
                                           ["gear_output_turns_units_in_application"][0]
                                               .get<int>();
    cfg.rotary.gear_application_den = jdata["motor_type"]["select_option"]["rotary"]["scaling"]
                                           ["gear_output_turns_units_in_application"][1]
                                               .get<int>();
    cfg.rotary.increments_motor_turns_num =
        jdata["motor_type"]["select_option"]["rotary"]["scaling"]["increments_motor_turns"][0]
            .get<int>();
    cfg.rotary.increments_motor_turns_den =
        jdata["motor_type"]["select_option"]["rotary"]["scaling"]["increments_motor_turns"][1]
            .get<int>();
    cfg.rotary.motor_gear_num = jdata["motor_type"]["select_option"]["rotary"]["scaling"]
                                     ["motor_turns_gear_output_turns"][0]
                                         .get<int>();
    cfg.rotary.motor_gear_den = jdata["motor_type"]["select_option"]["rotary"]["scaling"]
                                     ["motor_turns_gear_output_turns"][1]
                                         .get<int>();

    cfg.task_name = (char*)jdata["task_name"].get<std::string>().c_str();
    if (jdata["mapping"]["automatic_mapping"].get<bool>()) {
        const auto& servo_ctrl = jdata["mapping"]["servo_ctrl"];
        cfg.pdo_flag.enable_control_word = servo_ctrl["control_word"]["enable"].get<bool>();
        cfg.pdo_flag.enable_set_position = servo_ctrl["set_position"]["enable"].get<bool>();
        cfg.pdo_flag.enable_set_velocity = servo_ctrl["set_velocity"]["enable"].get<bool>();
        cfg.pdo_flag.enable_set_torque = servo_ctrl["set_torque"]["enable"].get<bool>();
        cfg.pdo_flag.enable_modes_of_operation =
            servo_ctrl["modes_of_operation"]["enable"].get<bool>();

        const auto& servo_stat = jdata["mapping"]["servo_stat"];
        cfg.pdo_flag.enable_status_word = servo_stat["status_word"]["enable"].get<bool>();
        cfg.pdo_flag.enable_actual_postion = servo_stat["actual_position"]["enable"].get<bool>();
        cfg.pdo_flag.enable_actual_velocity = servo_stat["actual_velocity"]["enable"].get<bool>();
        cfg.pdo_flag.enable_actual_torque = servo_stat["actual_torque"]["enable"].get<bool>();
        cfg.pdo_flag.enable_modes_of_operation_display =
            servo_stat["modes_of_operation_display"]["enable"].get<bool>();
    }

    if (jdata.contains("debug")) {
        auto dbg = jdata["debug"];
        cfg.debug.level = JSON_CONTAIN(dbg, int, "level", 0);
        cfg.debug.eAxisMtype = JSON_CONTAIN(dbg, int, "eAxisMtype", 0);
        cfg.debug.modulo = JSON_CONTAIN(dbg, int, "modulo", 1000);
        cfg.debug.mode = JSON_CONTAIN(dbg, int, "mode", 0);
        cfg.debug.log_type = JSON_CONTAIN(dbg, int, "log_type", 0);
        cfg.debug.axis_dtype = JSON_CONTAIN(dbg, int, "axis_dtype", 0);
        cfg.debug.batch_size = JSON_CONTAIN(dbg, int, "batch_size", 1);
        cfg.debug.servo_fak_pos = JSON_CONTAIN(dbg, int, "servo_fak_pos", 0);
        cfg.debug.servo_fak_statw_sm = JSON_CONTAIN(dbg, int, "servo_fak_statw_sm", 0);
        cfg.debug.servo_fak_rcfilter_scale =
            JSON_CONTAIN(dbg, double, "servo_fak_rcfilter_scale", 0);
        // cfg.debug.arg_value_flag = reader.GetInteger(key, "debug.arg_value_flag", 0);
        // cfg.debug.arg_value[0] = reader.GetInteger(key, "debug.arg_value[0]", 0);
    }
    return 0;
}

}  // namespace zmc