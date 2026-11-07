/* Independent Lua status-client userdata methods. */
#include "zmc_axfb_lua_status.hpp"

#include <vector>

#include "common/zmc/fbt_errorcode_def.h"

namespace zmc_lua {

namespace {

const char kStatusMt[] = "zmc.axfb.status";
const uint32_t kResultCap = 128u;

int status_gc(lua_State* L) {
    LuaStatus* status = static_cast<LuaStatus*>(luaL_checkudata(L, 1, kStatusMt));
    axfb_status_destroy(status->handle);
    status->handle = NULL;
    return 0;
}

int status_pop(lua_State* L) {
    LuaStatus* status = static_cast<LuaStatus*>(luaL_checkudata(L, 1, kStatusMt));
    LuaBridge* bridge = check_bridge(L, 2);
    if (bridge->handle == NULL) {
        return push_result(L, ZMC_ERR_BRIDGE_INVALID_HANDLE);
    }

    AxFbBridgePacket bridge_packet = {};
    const int pop_ret = axfb_bridge_pop_status_packet(bridge->handle, &bridge_packet);
    if (pop_ret != ZMC_NO_ERROR) {
        return push_result(L, pop_ret);
    }
    return push_result(L, axfb_status_apply_packet(status->handle, &bridge_packet));
}

void push_axis_state(lua_State* L, const AxisState& state) {
    lua_newtable(L);
    const int table_index = absolute_index(L, -1);
    set_integer_field(L, table_index, "axis_id", state.axis_id);
    set_integer_field(L, table_index, "axis_state", state.axis_state);
    set_integer_field(L, table_index, "error_id", state.error_id);
    set_integer_field(L, table_index, "tick", state.tick);
    set_number_field(L, table_index, "act_pos", state.act_pos);
    set_number_field(L, table_index, "set_pos", state.set_pos);
}

void push_axis_state2(lua_State* L, const AxisState2& state) {
    lua_newtable(L);
    const int table_index = absolute_index(L, -1);
    set_integer_field(L, table_index, "axis_id", state.axis_id);
    set_integer_field(L, table_index, "axis_state", state.axis_state);
    set_integer_field(L, table_index, "error_id", state.error_id);
    set_integer_field(L, table_index, "tick", state.tick);
    set_number_field(L, table_index, "act_pos", state.act_pos);
    set_number_field(L, table_index, "set_pos", state.set_pos);
    set_number_field(L, table_index, "set_vel", state.set_vel);
    set_number_field(L, table_index, "set_acc", state.set_acc);
    set_integer_field(L, table_index, "set_position", state.set_position);
    set_integer_field(L, table_index, "act_position", state.act_position);
}

void push_flow_status(lua_State* L, const FlowCtrlStatus& status) {
    lua_newtable(L);
    const int table_index = absolute_index(L, -1);
    set_integer_field(L, table_index, "period", status.period);
    set_integer_field(L, table_index, "batchSize", status.batchSize);
    set_integer_field(L, table_index, "verboseFlag", status.verboseFlag);
    set_integer_field(L, table_index, "latency", status.latency);
    set_integer_field(L, table_index, "axis_num", status.axis_num);
    set_integer_field(L, table_index, "cycleTime", status.cycleTime);
}

void push_cmd_result(lua_State* L, const CmdResult& result) {
    lua_newtable(L);
    const int table_index = absolute_index(L, -1);
    set_integer_field(L, table_index, "id", result.id);
    set_integer_field(L, table_index, "command_type", result.command_type);
    set_integer_field(L, table_index, "target_index", result.target_index);
    set_integer_field(L, table_index, "target_kind", result.target_kind);
    set_integer_field(L, table_index, "status", result.status);
    set_integer_field(L, table_index, "io_count", result.io_count);
    set_integer_field(L, table_index, "error_code", result.error_code);
    set_integer_field(L, table_index, "tick", result.tick);
    set_integer_field(L, table_index, "data", result.data);
    set_integer_field(L, table_index, "data_ext", result.data_ext);
}

// IO queries expose the same fields as axis-result queries.
void push_io_result(lua_State* L, const CmdResult& result) {
    push_cmd_result(L, result);
}

int status_get_axis_state(lua_State* L) {
    LuaStatus* status = static_cast<LuaStatus*>(luaL_checkudata(L, 1, kStatusMt));
    const int axis_id = static_cast<int>(luaL_checkinteger(L, 2));
    AxisState state = {};
    const int ret = axfb_status_get_axis_state(status->handle, axis_id, &state);
    lua_pushinteger(L, ret);
    if (ret != ZMC_NO_ERROR) {
        return 1;
    }
    push_axis_state(L, state);
    return 2;
}

int status_get_axis_state2(lua_State* L) {
    LuaStatus* status = static_cast<LuaStatus*>(luaL_checkudata(L, 1, kStatusMt));
    const int axis_id = static_cast<int>(luaL_checkinteger(L, 2));
    AxisState2 state = {};
    const int ret = axfb_status_get_axis_state2(status->handle, axis_id, &state);
    lua_pushinteger(L, ret);
    if (ret != ZMC_NO_ERROR) {
        return 1;
    }
    push_axis_state2(L, state);
    return 2;
}

int status_get_flow_status(lua_State* L) {
    LuaStatus* status = static_cast<LuaStatus*>(luaL_checkudata(L, 1, kStatusMt));
    FlowCtrlStatus flow = {};
    const int ret = axfb_status_get_flow_status(status->handle, &flow);
    lua_pushinteger(L, ret);
    if (ret != ZMC_NO_ERROR) {
        return 1;
    }
    push_flow_status(L, flow);
    return 2;
}

int status_get_cmd_results(lua_State* L) {
    LuaStatus* status = static_cast<LuaStatus*>(luaL_checkudata(L, 1, kStatusMt));
    std::vector<CmdResult> results(kResultCap);
    uint32_t len = kResultCap;
    const int ret = axfb_status_get_cmd_results(status->handle, &results[0], &len);
    lua_pushinteger(L, ret);
    if (ret != ZMC_NO_ERROR) {
        return 1;
    }
    lua_newtable(L);
    for (uint32_t i = 0; i < len; ++i) {
        push_cmd_result(L, results[i]);
        lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
    }
    return 2;
}

int status_get_io_results(lua_State* L) {
    LuaStatus* status = static_cast<LuaStatus*>(luaL_checkudata(L, 1, kStatusMt));
    std::vector<CmdResult> results(kResultCap);
    uint32_t len = kResultCap;
    const int ret = axfb_status_get_io_results(status->handle, &results[0], &len);
    lua_pushinteger(L, ret);
    if (ret != ZMC_NO_ERROR) {
        return 1;
    }
    lua_newtable(L);
    for (uint32_t i = 0; i < len; ++i) {
        push_io_result(L, results[i]);
        lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
    }
    return 2;
}

int status_current_tick_for_axis(lua_State* L) {
    LuaStatus* status = static_cast<LuaStatus*>(luaL_checkudata(L, 1, kStatusMt));
    const int axis = static_cast<int>(luaL_checkinteger(L, 2));
    uint32_t tick = 0;
    const int ret = axfb_status_current_tick_for_axis(status->handle, axis, &tick);
    lua_pushinteger(L, ret);
    if (ret != ZMC_NO_ERROR) {
        return 1;
    }
    lua_pushinteger(L, tick);
    return 2;
}

int status_current_axis_state(lua_State* L) {
    LuaStatus* status = static_cast<LuaStatus*>(luaL_checkudata(L, 1, kStatusMt));
    const int axis = static_cast<int>(luaL_checkinteger(L, 2));
    uint8_t state = 0;
    int has_state = 0;
    const int ret =
        axfb_status_current_axis_state(status->handle, axis, &state, &has_state);
    lua_pushinteger(L, ret);
    if (ret != ZMC_NO_ERROR) {
        return 1;
    }
    lua_pushinteger(L, state);
    lua_pushinteger(L, has_state);
    return 3;
}

int status_ack_seq(lua_State* L) {
    LuaStatus* status = static_cast<LuaStatus*>(luaL_checkudata(L, 1, kStatusMt));
    uint32_t seq = 0;
    const int ret = axfb_status_ack_seq(status->handle, &seq);
    lua_pushinteger(L, ret);
    if (ret != ZMC_NO_ERROR) {
        return 1;
    }
    lua_pushinteger(L, seq);
    return 2;
}

int status_ack_tick(lua_State* L) {
    LuaStatus* status = static_cast<LuaStatus*>(luaL_checkudata(L, 1, kStatusMt));
    uint32_t tick = 0;
    const int ret = axfb_status_ack_tick(status->handle, &tick);
    lua_pushinteger(L, ret);
    if (ret != ZMC_NO_ERROR) {
        return 1;
    }
    lua_pushinteger(L, tick);
    return 2;
}

int status_reset(lua_State* L) {
    LuaStatus* status = static_cast<LuaStatus*>(luaL_checkudata(L, 1, kStatusMt));
    return push_result(L, axfb_status_reset(status->handle));
}

const luaL_Reg kStatusMethods[] = {
    {"__gc", status_gc},
    {"pop", status_pop},
    {"get_axis_state", status_get_axis_state},
    {"get_axis_state2", status_get_axis_state2},
    {"get_flow_status", status_get_flow_status},
    {"get_cmd_results", status_get_cmd_results},
    {"get_io_results", status_get_io_results},
    {"current_tick_for_axis", status_current_tick_for_axis},
    {"current_axis_state", status_current_axis_state},
    {"ack_seq", status_ack_seq},
    {"ack_tick", status_ack_tick},
    {"reset", status_reset},
    {NULL, NULL},
};

}  // namespace

int lua_axfb_status_new(lua_State* L) {
    LuaStatus* status = static_cast<LuaStatus*>(lua_newuserdata(L, sizeof(LuaStatus)));
    status->handle = NULL;
    const int ret = axfb_status_create(&status->handle);
    if (ret != ZMC_NO_ERROR || status->handle == NULL) {
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_pushinteger(L, ret);
        return 2;
    }
    luaL_getmetatable(L, kStatusMt);
    lua_setmetatable(L, -2);
    lua_pushinteger(L, ret);
    return 2;
}

void register_status_metatable(lua_State* L) { register_metatable(L, kStatusMt, kStatusMethods); }

}  // namespace zmc_lua
