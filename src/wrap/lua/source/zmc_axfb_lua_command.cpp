/* mcapi command-packet Lua userdata methods over MCCmdPacket. */
#include "zmc_axfb_lua_command.hpp"

#include <cstring>

#include "common/zmc/fbt_errorcode_def.h"

namespace zmc_lua {

namespace {

int read_double_array(lua_State* L, int index, double* out, int max_n, int* out_n) {
    luaL_checktype(L, index, LUA_TTABLE);
    const int n = static_cast<int>(raw_length(L, index));
    if (n <= 0 || n > max_n) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    for (int i = 0; i < n; ++i) {
        lua_rawgeti(L, index, i + 1);
        if (!lua_isnumber(L, -1)) {
            lua_pop(L, 1);
            return ZMC_ERR_BRIDGE_INVALID_ARG;
        }
        out[i] = lua_tonumber(L, -1);
        lua_pop(L, 1);
    }
    *out_n = n;
    return ZMC_NO_ERROR;
}

int read_int_array(lua_State* L, int index, int* out, int max_n, int* out_n) {
    luaL_checktype(L, index, LUA_TTABLE);
    const int n = static_cast<int>(raw_length(L, index));
    if (n <= 0 || n > max_n) {
        return ZMC_ERR_BRIDGE_INVALID_ARG;
    }
    for (int i = 0; i < n; ++i) {
        lua_rawgeti(L, index, i + 1);
        if (!lua_isinteger(L, -1) && !lua_isnumber(L, -1)) {
            lua_pop(L, 1);
            return ZMC_ERR_BRIDGE_INVALID_ARG;
        }
        out[i] = static_cast<int>(luaL_checkinteger(L, -1));
        lua_pop(L, 1);
    }
    *out_n = n;
    return ZMC_NO_ERROR;
}

int command_init(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_init(&command->packet));
}

int command_reset(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_reset(&command->packet));
}

int command_can_append(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    const uint16_t param_len = static_cast<uint16_t>(luaL_checkinteger(L, 2));
    int can = 0;
    const int ret = axfb_cmd_packet_can_append(&command->packet, param_len, &can);
    lua_pushinteger(L, ret);
    if (ret != ZMC_NO_ERROR) {
        return 1;
    }
    lua_pushinteger(L, can);
    return 2;
}

int command_set_axis(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_set_axis(&command->packet,
                                                     static_cast<int>(luaL_checkinteger(L, 2))));
}

int command_set_base_axis(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_set_base_axis(
                              &command->packet, static_cast<int>(luaL_checkinteger(L, 2))));
}

int command_set_master_axis(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_set_master_axis(
                              &command->packet, static_cast<int>(luaL_checkinteger(L, 2))));
}

int command_set_units(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_set_units(&command->packet, luaL_checknumber(L, 2)));
}

int command_set_speed(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_set_speed(&command->packet, luaL_checknumber(L, 2)));
}

int command_set_accel(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_set_accel(&command->packet, luaL_checknumber(L, 2)));
}

int command_set_decel(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_set_decel(&command->packet, luaL_checknumber(L, 2)));
}

int command_set_jerk(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_set_jerk(&command->packet, luaL_checknumber(L, 2)));
}

int command_set_fast_decel(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L,
                       axfb_cmd_packet_set_fast_decel(&command->packet, luaL_checknumber(L, 2)));
}

int command_set_speed_ratio(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L,
                       axfb_cmd_packet_set_speed_ratio(&command->packet, luaL_checknumber(L, 2)));
}

int command_set_direction(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_set_direction(
                              &command->packet, static_cast<int>(luaL_checkinteger(L, 2))));
}

int command_set_buffer_mode(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_set_buffer_mode(
                              &command->packet, static_cast<int>(luaL_checkinteger(L, 2))));
}

int command_set_flow_config(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_set_flow_config(
                              &command->packet, static_cast<uint32_t>(luaL_checkinteger(L, 2)),
                              static_cast<uint32_t>(luaL_checkinteger(L, 3)),
                              static_cast<uint8_t>(luaL_optinteger(L, 4, 1))));
}

int command_power_on(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_power_on(&command->packet));
}

int command_power_off(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_power_off(&command->packet));
}

int command_move_abs(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_move_abs(&command->packet, luaL_checknumber(L, 2)));
}

int command_move_rel(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_move_rel(&command->packet, luaL_checknumber(L, 2)));
}

int command_move_add(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    double pos[AXFB_CMD_MAX_AXES];
    int n = 0;
    const int parse_ret = read_double_array(L, 2, pos, static_cast<int>(AXFB_CMD_MAX_AXES), &n);
    if (parse_ret != ZMC_NO_ERROR) {
        return push_result(L, parse_ret);
    }
    return push_result(L, axfb_cmd_packet_move_add(&command->packet, n, pos));
}

int command_home(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    const double position = luaL_optnumber(L, 2, 0.0);
    return push_result(L, axfb_cmd_packet_home(&command->packet, position));
}

int command_homing(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_homing(&command->packet));
}

int command_gear_in(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_gear_in(&command->packet));
}

int command_gear_out(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_gear_out(&command->packet));
}

int command_set_position(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_set_position(&command->packet, luaL_checknumber(L, 2)));
}

int command_datum(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_datum(&command->packet,
                                                 static_cast<int>(luaL_checkinteger(L, 2))));
}

int command_move_velocity(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_move_velocity(
                              &command->packet, static_cast<int>(luaL_checkinteger(L, 2))));
}

int command_forward(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_forward(&command->packet));
}

int command_reverse(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_reverse(&command->packet));
}

int command_stop(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_stop(&command->packet));
}

int command_halt(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_halt(&command->packet));
}

int command_multi_move_abs(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    double pos[AXFB_CMD_MAX_AXES];
    int n = 0;
    const int parse_ret = read_double_array(L, 2, pos, static_cast<int>(AXFB_CMD_MAX_AXES), &n);
    if (parse_ret != ZMC_NO_ERROR) {
        return push_result(L, parse_ret);
    }
    return push_result(L, axfb_cmd_packet_multi_move_abs(&command->packet, n, pos));
}

int command_multi_move_rel(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    double pos[AXFB_CMD_MAX_AXES];
    int n = 0;
    const int parse_ret = read_double_array(L, 2, pos, static_cast<int>(AXFB_CMD_MAX_AXES), &n);
    if (parse_ret != ZMC_NO_ERROR) {
        return push_result(L, parse_ret);
    }
    return push_result(L, axfb_cmd_packet_multi_move_rel(&command->packet, n, pos));
}

int command_group_select(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_group_select(
                              &command->packet, static_cast<int>(luaL_checkinteger(L, 2))));
}

int command_group_set_axes(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    int axes[AXFB_CMD_MAX_AXES];
    int n = 0;
    const int parse_ret = read_int_array(L, 2, axes, static_cast<int>(AXFB_CMD_MAX_AXES), &n);
    if (parse_ret != ZMC_NO_ERROR) {
        return push_result(L, parse_ret);
    }
    return push_result(L, axfb_cmd_packet_group_set_axes(&command->packet, n, axes));
}

int command_group_enable(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_group_enable(&command->packet));
}

int command_group_disable(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_group_disable(&command->packet));
}

int command_group_reset(lua_State* L) {
    LuaCommandPacket* command = check_command_packet(L, 1);
    return push_result(L, axfb_cmd_packet_group_reset(&command->packet));
}

const luaL_Reg kCommandMethods[] = {
    {"init", command_init},
    {"reset", command_reset},
    {"can_append", command_can_append},
    {"set_axis", command_set_axis},
    {"set_base_axis", command_set_base_axis},
    {"set_master_axis", command_set_master_axis},
    {"set_units", command_set_units},
    {"set_speed", command_set_speed},
    {"set_accel", command_set_accel},
    {"set_decel", command_set_decel},
    {"set_jerk", command_set_jerk},
    {"set_fast_decel", command_set_fast_decel},
    {"set_speed_ratio", command_set_speed_ratio},
    {"set_direction", command_set_direction},
    {"set_buffer_mode", command_set_buffer_mode},
    {"set_flow_config", command_set_flow_config},
    {"power_on", command_power_on},
    {"power_off", command_power_off},
    {"move_abs", command_move_abs},
    {"move_rel", command_move_rel},
    {"move_add", command_move_add},
    {"home", command_home},
    {"homing", command_homing},
    {"gear_in", command_gear_in},
    {"gear_out", command_gear_out},
    {"set_position", command_set_position},
    {"datum", command_datum},
    {"move_velocity", command_move_velocity},
    {"forward", command_forward},
    {"reverse", command_reverse},
    {"stop", command_stop},
    {"halt", command_halt},
    {"multi_move_abs", command_multi_move_abs},
    {"multi_move_rel", command_multi_move_rel},
    {"group_select", command_group_select},
    {"group_set_axes", command_group_set_axes},
    {"group_enable", command_group_enable},
    {"group_disable", command_group_disable},
    {"group_reset", command_group_reset},
    {NULL, NULL},
};

}  // namespace

LuaCommandPacket* check_command_packet(lua_State* L, int index) {
    return static_cast<LuaCommandPacket*>(luaL_checkudata(L, index, kCmdMt));
}

int lua_axfb_packet_new(lua_State* L) {
    LuaCommandPacket* command =
        static_cast<LuaCommandPacket*>(lua_newuserdata(L, sizeof(LuaCommandPacket)));
    std::memset(command, 0, sizeof(*command));
    const int ret = axfb_cmd_packet_init(&command->packet);
    if (ret != ZMC_NO_ERROR) {
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_pushinteger(L, ret);
        return 2;
    }
    luaL_getmetatable(L, kCmdMt);
    lua_setmetatable(L, -2);
    lua_pushinteger(L, ZMC_NO_ERROR);
    return 2;
}

void register_command_metatable(lua_State* L) { register_metatable(L, kCmdMt, kCommandMethods); }

}  // namespace zmc_lua
