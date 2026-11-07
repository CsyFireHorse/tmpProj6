#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>
#include "stdio.h"
#include "string.h"
#include "AxisIniLoader.hpp"
#include "AxisIniLoaderInternal.hpp"
#include "common/misc/PathAbout.hpp"
#include "common/misc/TagUnion.hpp"
#include "common/zmc/zmc_predef.h"
#include "impl/fbt_impl_enum_def.h"
#include "axLogFactory.h"
#include "vel/include/vel_ramp_type.hpp"
#include "AxisCiaConfig.hpp"

using namespace zmc;

int str_to_enum_to_int(const char* str) {
    if (!str) return -1;
    std::string toupper = str;
    std::transform(toupper.begin(), toupper.end(), toupper.begin(), ::toupper);
    const unsigned int hash = strhash(toupper.c_str());
    int val = (int)VelRampTypeFromString(toupper.c_str());
    if (val < 0) {
        auto hash = strhash(toupper.c_str());
        switch (hash) {
            case strhash("FINITE"):
                val = 1;
                break;  // AXIS_TYPE_FINITE
            case strhash("MODULO"):
                val = 0;
                break;  // AXIS_TYPE_MODULO
            case strhash("ROTARY"):
                val = 0;
                break;  // MOTOR_TYPE_ROTARY
            case strhash("LINEAR"):
                val = 1;
                break;  // MOTOR_TYPE_LINEAR

            case strhash("LOG_BUFFER"):
                val = 1;
                break;  //
            case strhash("LOG_CONSOLE"):
                val = 2;
                break;  // LOG_CONSOLE
            case strhash("LOG_FILE"):
                val = 3;
                break;  // LOG_FILE
            case strhash("LOG_SPDLOG"):
                val = 4;
                break;  // LOG_SPDLOG

            case strhash("AXIS_TYPE_VIRTUAL"):
                val = AXIS_TYPE_VIRTUAL;
                break;  //
            case strhash("AXIS_TYPE_ENCODER"):
                val = AXIS_TYPE_ENCODER;
                break;  //
            case strhash("AXIS_TYPE_DRIVE_POS_CONTROL"):
                val = AXIS_TYPE_DRIVE_POS_CONTROL;
                break;  //
            case strhash("AXIS_TYPE_CIA402"):
                val = AXIS_TYPE_CIA402;
                break;  //
            case strhash("AXIS_TYPE_CIA402_SIMULATED"):
                val = AXIS_TYPE_CIA402_SIMULATED;
                break;  //
            case strhash("TRAPZ"):
            case strhash("TRAPEZ"):
            case strhash("TRAPEZOID"):
                val = 0;
                break;
            case strhash("SIN2"):
            case strhash("SINSQUARE"):
                val = 1;
                break;
            case strhash("QUADRATIC"):
                val = 2;
                break;
            case strhash("QUADRATIC(SMOOTH)"):
                val = 3;
                break;
            case strhash("DEACTIVATED"):
                val = 0;
                break;
            case strhash("DISABLE_DRIVE"):
                val = 1;
                break;
            case strhash("DO_QUICK_STOP"):
                val = 2;
                break;
            case strhash("STAY_ENABLED"):
                val = 3;
                break;
            case strhash("0"):
                val = 0;
                break;
            case strhash("1"):
                val = 1;
                break;
            case strhash("2"):
                val = 2;
                break;
            case strhash("3"):
                val = 3;
                break;
            case strhash("4"):
                val = 4;
                break;
            default:
                val = -1;
                break;
        }
    }
    return val;
}

void axisComcfg_default_init(AxisCommonConfig* pCfg) {
    if (!pCfg) return;
    pCfg->ini_dir_path = nullptr;
    pCfg->eni_path = nullptr;
    pCfg->net_name = nullptr;
    pCfg->device_name = nullptr;
    pCfg->axis_num = 0;
    pCfg->count_to_exit = 0;
    pCfg->input_delay = 0;
    pCfg->output_delay = 0;
    pCfg->axis_file_num = 0;
    pCfg->axis_file_array = nullptr;
    pCfg->axis_file_dir = nullptr;
    pCfg->verbose_flag = 0;
    pCfg->verbose_modulo = 1;
    pCfg->config_mask = 0;
}

void axisComcfg_merge_by_mask(AxisCommonConfig* dst, const AxisCommonConfig* src) {
    if (!dst || !src) return;
    if (src->config_mask & AX_COM_MASK_AXIS_NUM) dst->axis_num = src->axis_num;
    if (src->config_mask & AX_COM_MASK_ENI_PATH) dst->eni_path = src->eni_path;
    if (src->config_mask & AX_COM_MASK_NET_NAME) dst->net_name = src->net_name;
    if (src->config_mask & AX_COM_MASK_DEVICE_NAME) dst->device_name = src->device_name;
    if (src->config_mask & AX_COM_MASK_VERBOSE_FLAG) dst->verbose_flag = src->verbose_flag;
    if (src->config_mask & AX_COM_MASK_VERBOSE_MODULO) dst->verbose_modulo = src->verbose_modulo;
    if (src->config_mask & AX_COM_MASK_COUNT_TO_EXIT) dst->count_to_exit = src->count_to_exit;
    if (src->config_mask & AX_COM_MASK_INPUT_DELAY) dst->input_delay = src->input_delay;
    if (src->config_mask & AX_COM_MASK_OUTPUT_DELAY) dst->output_delay = src->output_delay;
    if (src->config_mask & AX_COM_MASK_AXIS_FILE_NUM) {
        dst->axis_file_num = src->axis_file_num;
        dst->axis_file_array = src->axis_file_array;
    }
    dst->config_mask |= src->config_mask;
}

// Parse INI string to dictionary for specified section
ConfigMap parse_axis_from_ini_string(const std::string& ini_content, const std::string& section);
// Configure AxisCiaConfig from dictionary
int configure_axis_from_dict(AxisCiaConfig& cfg, const ConfigMap& section_dict);

// Helper macros for configuration setting
#define SET_INT_CONFIG(cfg, member, key)                \
    if (section_dict.find(key) != section_dict.end()) { \
        cfg.member = section_dict.at(key).getInt();     \
    }

#define SET_ENUM_CONFIG(cfg, member, key)                                    \
    if (section_dict.find(key) != section_dict.end()) {                      \
        if (section_dict.at(key).isInt())                                    \
            cfg.member = section_dict.at(key).getInt();                      \
        else {                                                               \
            int val = str_to_enum_to_int(section_dict.at(key).getCharPtr()); \
            if (val != -1) cfg.member = val;                                 \
        }                                                                    \
    }

#define SET_BOOL_CONFIG(cfg, member, key)               \
    if (section_dict.find(key) != section_dict.end()) { \
        cfg.member = section_dict.at(key).getBool();    \
    }

#define SET_DOUBLE_CONFIG(cfg, member, key)             \
    if (section_dict.find(key) != section_dict.end()) { \
        cfg.member = section_dict.at(key).getDouble();  \
    }

#define SET_STRING_CONFIG(cfg, member, key)                \
    if (section_dict.find(key) != section_dict.end()) {    \
        char* str_val = section_dict.at(key).getCharPtr(); \
        if (str_val) {                                     \
            cfg.member = new char[strlen(str_val) + 1];    \
            strcpy(cfg.member, str_val);                   \
        }                                                  \
    }

// Common axis configuration keys
static const std::vector<std::string> COMMON_AXIS_KEYS = {"cycle_time",
                                                          "axis_index",
                                                          "id",
                                                          "slave_node_id",
                                                          "is_axis_virtual",
                                                          "axis_type",
                                                          "modulo_value",
                                                          "mapping_id",
                                                          "bit_width",
                                                          "software_limits.is_activate",
                                                          "software_limits.positive",
                                                          "software_limits.negative",
                                                          "position_lag_option",
                                                          "position_lag_supervision.is_changeable",
                                                          "position_lag_supervision.value",
                                                          "dynamic_limits.velocity",
                                                          "dynamic_limits.acceleration",
                                                          "dynamic_limits.deceleration",
                                                          "dynamic_limits.jerk",
                                                          "velocity_ramp_option",
                                                          "software_error_reaction.deceleration",
                                                          "software_error_reaction.max_distance",
                                                          "motor_type",
                                                          "is_invert_direction",
                                                          "increments_units_in_application_num",
                                                          "increments_units_in_application_den",
                                                          "gear_application_num",
                                                          "gear_application_den",
                                                          "increments_motor_turns_num",
                                                          "increments_motor_turns_den",
                                                          "motor_gear_num",
                                                          "motor_gear_den",
                                                          "enable_value",
                                                          "debug.level",
                                                          "debug.modulo",
                                                          "debug.mode",
                                                          "debug.log_type",
                                                          "debug.axis_dtype",
                                                          "debug.batch_size",
                                                          "debug.servo_fak_pos",
                                                          "debug.servo_fak_rcfilter_scale",
                                                          "debug.servo_fak_statw_sm",
                                                          "debug.eAxisMtype",
                                                          "debug.arg_value_flag",
                                                          "debug.enable_servoAda_param",
                                                          "debug.limit_flag",
                                                          "task_name",
                                                          "is_automatic_mapping",
                                                          "channels_stat_len",
                                                          "channels_ctrl_len",
                                                          "channels_stat_name",
                                                          "channels_ctrl_name"};

bool axis_ini_has_section(const INIReader& reader, const std::string& section) {
    const std::set<std::string>& sections = reader.Sections();
    return sections.find(section) != sections.end();
}

int axis_ini_load_axis_from_reader(const INIReader& reader, const char* inifile,
                                   AxisCiaConfig& cfg, const std::string& key) {
    if (reader.ParseError() != 0) {
        printf("Can't load '%s'\n", inifile);
        return ZMC_ERR_INI_PARSE;
    }
    auto secSet = reader.Sections();
    auto search = secSet.find(key);
    if (search == secSet.end()) {
        printf("Can't find '%s'\n", key.c_str());
        return ZMC_ERR_INI_MISSING_SECTION;
    }

    // Build ConfigMap from INI file using common keys
    ConfigMap config_map;
    for (const auto& config_key : COMMON_AXIS_KEYS) {
        if (reader.HasKey(key, config_key)) {
            std::string value = reader.Get(key, config_key, "");
            tagUnion tag_val = parse_value_to_tagUnion(value);
            if (tag_val.isCharPtr()) {
                auto newVal = new std::string(value);
                tag_val = tagUnion(newVal->c_str());
            }
            config_map[config_key] = tag_val;
        }
    }
    if (config_map.empty()) {
        printf("No configuration found for '%s'\n", key.c_str());
        return -3;
    }
    // Use configure_axis_from_dict to set the configuration
    int result = configure_axis_from_dict(cfg, config_map);
    if (result != 0) {
        printf("Failed to configure axis from dictionary\n");
        return -3;
    }

    // printf("cycle_time=%d\n", cfg.cycle_time);
    return 0;
}

template <int N>
struct StrArray {
    char* ptr[N];
    char content[100 * N];
};

int axis_ini_load_base_from_reader(const INIReader& reader, const char* inifile, bool isContent,
                                   AxisCommonConfig* pCfg) {
    if (reader.ParseError() != 0) {
        // INSERT_YOUR_CODE
        if (!isContent) {
            std::string fileName = inifile ? inifile : "";
            auto found = fileName.rfind(".ini");
            if (found == std::string::npos || found != fileName.length() - 4) {
                printf("Warning: the file '%s' does not have a .ini extension\n",
                       inifile ? inifile : "");
            }
        }
        printf("Can't load '%s'\n", inifile);
        return ZMC_ERR_INI_PARSE;
    }
    auto secSet = reader.Sections();
    auto search = secSet.find("base");
    if (search == secSet.end()) {
        printf("Can't find [base]\n");
        return ZMC_ERR_INI_MISSING_SECTION;
    }
    if (!pCfg) {
        printf("pCfg is nullptr\n");
        return -3;
    }
    auto& cfg = *pCfg;

    std::string* pIni_dir_path = nullptr;
    if (!isContent) {
        pIni_dir_path = new std::string(getFolderPath(inifile));
        cfg.ini_dir_path = (char*)pIni_dir_path->c_str();
    }
    auto eni_path = reader.Get("base", "eni_path", "");
    if (!eni_path.size()) {
        if (!isContent) printf("eni_path not found\n");
    } else {
        if (pIni_dir_path) str_replace(eni_path, "{{dirname}}", *pIni_dir_path);
    }
    auto pStr = new std::string(eni_path);
    cfg.eni_path = (char*)pStr->c_str();
    if (reader.HasKey("base", "eni_path")) cfg.config_mask |= AX_COM_MASK_ENI_PATH;

    pStr = new std::string(reader.Get("base", "net_name", ZMC_DEFAULT_NET_NAME));
    cfg.net_name = (char*)pStr->c_str();
    if (reader.HasKey("base", "net_name")) cfg.config_mask |= AX_COM_MASK_NET_NAME;

    pStr = new std::string(reader.Get("base", "device_name", ZMC_DEFAULT_DEVICE_NAME));
    cfg.device_name = (char*)pStr->c_str();
    if (reader.HasKey("base", "device_name")) cfg.config_mask |= AX_COM_MASK_DEVICE_NAME;

#define SET_BASE_CFG_INT(var, sec, name, msk)        \
    if (reader.HasKey(sec, #name)) {                 \
        var.name = reader.GetInteger(sec, #name, 0); \
        var.config_mask |= msk;                      \
    }
    //
    SET_BASE_CFG_INT(cfg, "base", axis_num, AX_COM_MASK_AXIS_NUM);
    SET_BASE_CFG_INT(cfg, "base", input_delay, AX_COM_MASK_INPUT_DELAY);
    SET_BASE_CFG_INT(cfg, "base", output_delay, AX_COM_MASK_OUTPUT_DELAY);
    SET_BASE_CFG_INT(cfg, "base", count_to_exit, AX_COM_MASK_COUNT_TO_EXIT);
    SET_BASE_CFG_INT(cfg, "base", verbose_flag, AX_COM_MASK_VERBOSE_FLAG);
    SET_BASE_CFG_INT(cfg, "base", verbose_modulo, AX_COM_MASK_VERBOSE_MODULO);

    auto axisFileDir = reader.Get("base", "axis_file_dir", "");
    if (pIni_dir_path) str_replace(axisFileDir, "{{dirname}}", *pIni_dir_path);
    pStr = new std::string(axisFileDir);
    cfg.axis_file_dir = (char*)pStr->c_str();

    cfg.axis_file_num = reader.GetInteger("base", "axis_file_num", 0);
    if (reader.HasKey("base", "axis_file_num")) cfg.config_mask |= AX_COM_MASK_AXIS_FILE_NUM;
    if (cfg.axis_file_num) {
        auto strArr = new StrArray<32>;
        for (int i = 0; i < cfg.axis_file_num; i++) {
            if (i >= 32) break;
            std::string key = "axis_file_" + std::to_string(i);
            auto pth = reader.Get("base", key, "");
            strcpy(&strArr->content[i * 100], pth.c_str());
            strArr->ptr[i] = &strArr->content[i * 100];
        }
        cfg.axis_file_array = strArr->ptr;
    }
    return 0;
}

int load_axis_ini_base(const char* inifile, AxisCommonConfig* pCfg) {
    AxisIniLoader loader(inifile, false);
    return loader.load_base(pCfg);
}
int load_axis_ini_base_from_content(const char* iniContent, AxisCommonConfig* pCfg) {
    AxisIniLoader loader(iniContent, true);
    return loader.load_base(pCfg);
}

int load_axcfg_batch(const char* inifile, int len, AxisCiaConfig* cfgArr,
                     AxisCommonConfig* pIniCfg) {
    AxisIniLoader loader(inifile, false);
    int ret = loader.load_base(pIniCfg);
    if (ret) {
        return ret;
    }
    for (int i = 0; i < len; i++) {
        AxisCiaConfig& cfg = cfgArr[i];
        AxisCiaConfig_default_init(&cfg);
        loader.load_axis_by_index(i, pIniCfg, &cfg);
    }
    return 0;
}

int load_axcfg_by_index(const char* inifile, int idx, const AxisCommonConfig* pIniCfg,
                        AxisCiaConfig* pCfg) {
    AxisIniLoader loader(inifile, false);
    return loader.load_axis_by_index(idx, pIniCfg, pCfg);
}

// Parse INI string to dictionary for specified section using INIReader
ConfigMap parse_axis_from_ini_string(const std::string& ini_content, const std::string& section) {
    ConfigMap result;

    // Use INIReader to parse the string content
    INIReader reader = INIReader::FromString(ini_content);

    if (reader.ParseError() != 0) {
        printf("Error parsing INI content\n");
        return result;
    }

    // Check if the specified section exists
    auto sections = reader.Sections();
    if (sections.find(section) == sections.end()) {
        printf("Section '%s' not found in INI content\n", section.c_str());
        return result;
    }

    for (const auto& key : COMMON_AXIS_KEYS) {
        if (reader.HasKey(section, key)) {
            std::string value = reader.Get(section, key, "");
            tagUnion tag_val = parse_value_to_tagUnion(value);
            result[key] = tag_val;
        }
    }

    return result;
}

// Configure AxisCiaConfig from dictionary
int configure_axis_from_dict(AxisCiaConfig& cfg, const ConfigMap& section_dict) {
    // Basic configuration
    SET_INT_CONFIG(cfg, cycle_time, "cycle_time");
    SET_INT_CONFIG(cfg, axis_index, "axis_index");
    SET_INT_CONFIG(cfg, id, "id");
    SET_INT_CONFIG(cfg, slave_node_id, "slave_node_id");
    SET_BOOL_CONFIG(cfg, is_axis_virtual, "is_axis_virtual");

    SET_ENUM_CONFIG(cfg, axis_type, "axis_type");

    SET_DOUBLE_CONFIG(cfg, modulo.modulo_value, "modulo_value");
    SET_INT_CONFIG(cfg, mapping_id, "mapping_id");
    SET_INT_CONFIG(cfg, bit_width, "bit_width");

    // Software limits
    SET_BOOL_CONFIG(cfg, finite.software_limits.is_activate, "software_limits.is_activate");
    SET_DOUBLE_CONFIG(cfg, finite.software_limits.positive, "software_limits.positive");
    SET_DOUBLE_CONFIG(cfg, finite.software_limits.negative, "software_limits.negative");

    // Position lag supervision
    SET_INT_CONFIG(cfg, position_lag_option, "position_lag_option");
    SET_BOOL_CONFIG(cfg, position_lag_supervision.is_changeable,
                    "position_lag_supervision.is_changeable");
    SET_DOUBLE_CONFIG(cfg, position_lag_supervision.value, "position_lag_supervision.value");

    // Dynamic limits
    SET_DOUBLE_CONFIG(cfg, dynamic_limits.velocity, "dynamic_limits.velocity");
    SET_DOUBLE_CONFIG(cfg, dynamic_limits.acceleration, "dynamic_limits.acceleration");
    SET_DOUBLE_CONFIG(cfg, dynamic_limits.deceleration, "dynamic_limits.deceleration");
    SET_DOUBLE_CONFIG(cfg, dynamic_limits.jerk, "dynamic_limits.jerk");

    // Velocity ramp option
    SET_ENUM_CONFIG(cfg, velocity_ramp_option, "velocity_ramp_option");

    // Software error reaction
    SET_DOUBLE_CONFIG(cfg, software_error_reaction.deceleration,
                      "software_error_reaction.deceleration");
    SET_DOUBLE_CONFIG(cfg, software_error_reaction.max_distance,
                      "software_error_reaction.max_distance");

    // Motor configuration
    SET_ENUM_CONFIG(cfg, motor_type, "motor_type");
    SET_BOOL_CONFIG(cfg, is_invert_direction, "is_invert_direction");

    // Linear parameters
    SET_INT_CONFIG(cfg, linear.increments_units_in_application_num,
                   "increments_units_in_application_num");
    SET_INT_CONFIG(cfg, linear.increments_units_in_application_den,
                   "increments_units_in_application_den");

    // Rotary parameters
    SET_INT_CONFIG(cfg, rotary.gear_application_num, "gear_application_num");
    SET_INT_CONFIG(cfg, rotary.gear_application_den, "gear_application_den");
    SET_INT_CONFIG(cfg, rotary.increments_motor_turns_num, "increments_motor_turns_num");
    SET_INT_CONFIG(cfg, rotary.increments_motor_turns_den, "increments_motor_turns_den");
    SET_INT_CONFIG(cfg, rotary.motor_gear_num, "motor_gear_num");
    SET_INT_CONFIG(cfg, rotary.motor_gear_den, "motor_gear_den");

    // PDO flag
    if (section_dict.find("enable_value") != section_dict.end()) {
        int val = section_dict.at("enable_value").getInt();
        if (val) cfg.pdo_flag.enable_value = val;
    }

    // Debug parameters
    SET_INT_CONFIG(cfg, debug.level, "debug.level");
    SET_INT_CONFIG(cfg, debug.modulo, "debug.modulo");
    SET_INT_CONFIG(cfg, debug.mode, "debug.mode");
    SET_ENUM_CONFIG(cfg, debug.log_type, "debug.log_type");
    SET_INT_CONFIG(cfg, debug.axis_dtype, "debug.axis_dtype");
    SET_INT_CONFIG(cfg, debug.batch_size, "debug.batch_size");
    SET_INT_CONFIG(cfg, debug.servo_fak_pos, "debug.servo_fak_pos");
    SET_DOUBLE_CONFIG(cfg, debug.servo_fak_rcfilter_scale, "debug.servo_fak_rcfilter_scale");
    SET_INT_CONFIG(cfg, debug.servo_fak_statw_sm, "debug.servo_fak_statw_sm");
    SET_INT_CONFIG(cfg, debug.eAxisMtype, "debug.eAxisMtype");
    if (section_dict.find("debug.eAxisMtype") != section_dict.end()) {
        std::string str = section_dict.at("debug.eAxisMtype").getCharPtr();
        auto val = str_to_enum_to_int(str.c_str());
        if (val != -1) {
            cfg.debug.eAxisMtype = val;
        } else {
            cfg.debug.eAxisMtype = AXIS_TYPE_VIRTUAL;
        }
    }
    SET_INT_CONFIG(cfg, debug.arg_value_flag, "debug.arg_value_flag");
    SET_BOOL_CONFIG(cfg, debug.enable_servoAda_param, "debug.enable_servoAda_param");
    SET_INT_CONFIG(cfg, debug.limit_flag.value, "debug.limit_flag");

    // String fields
    SET_STRING_CONFIG(cfg, task_name, "task_name");

    SET_BOOL_CONFIG(cfg, is_automatic_mapping, "is_automatic_mapping");
    SET_INT_CONFIG(cfg, channels_stat_len, "channels_stat_len");
    SET_INT_CONFIG(cfg, channels_ctrl_len, "channels_ctrl_len");

    SET_STRING_CONFIG(cfg, channels_stat_name, "channels_stat_name");
    SET_STRING_CONFIG(cfg, channels_ctrl_name, "channels_ctrl_name");

    return 0;
}

// Clean up macros to avoid pollution
#undef SET_INT_CONFIG
#undef SET_BOOL_CONFIG
#undef SET_DOUBLE_CONFIG
#undef SET_STRING_CONFIG

int load_axcfg_from_ini(const char* inifile, AxisCiaConfig* cfg, const char* key) {
    AxisIniLoader loader(inifile, false);
    return loader.load_axis(cfg, key);
}
int load_axcfg_from_content(const char* inifile, AxisCiaConfig* cfg, const char* key) {
    AxisIniLoader loader(inifile, true);
    return loader.load_axis(cfg, key);
}

// AxisCiaConfig load_axis_ini_axcfg_pstr(const char* inifile, const char* pstr) {
//     std::string key(pstr);
//     AxisCiaConfig cfg;
//     AxisCiaConfig_default_init(&cfg);
//     zmc::load_axcfg_from_ini(inifile, cfg, key);
//     return cfg;
// }
