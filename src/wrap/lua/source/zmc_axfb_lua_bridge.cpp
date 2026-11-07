/* Bridge Lua userdata methods. */
#include "zmc_axfb_lua_bridge.hpp"

#include <cstring>
#include <vector>

#include "common/zmc/fbt_errorcode_def.h"
#include "zmc_axfb_lua_command.hpp"

namespace zmc_lua {

int lua_axfb_new(lua_State* L) {
    LuaBridge* bridge = static_cast<LuaBridge*>(lua_newuserdata(L, sizeof(LuaBridge)));
    bridge->handle = NULL;
    luaL_getmetatable(L, kBridgeMt);
    lua_setmetatable(L, -2);

    const int ret = axfb_bridge_create(&bridge->handle);
    if (ret != 0 || bridge->handle == NULL) {
        close_bridge(bridge);
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_pushinteger(L, ret);
        return 2;
    }
    lua_pushinteger(L, 0);
    return 2;
}

namespace {

int bridge_gc(lua_State* L) {
    LuaBridge* bridge = check_bridge(L, 1);
    close_bridge(bridge);
    return 0;
}

int bridge_close(lua_State* L) {
    LuaBridge* bridge = check_bridge(L, 1);
    close_bridge(bridge);
    lua_pushinteger(L, 0);
    return 1;
}

int bridge_configure(lua_State* L) {
    LuaBridge* bridge = check_bridge(L, 1);
    if (bridge->handle == NULL) {
        return push_result(L, ZMC_ERR_BRIDGE_INVALID_HANDLE);
    }
    luaL_checktype(L, 2, LUA_TTABLE);
    const std::string ini_path = get_string_field(L, 2, "ini_path", "");
    const std::string ini_content = get_string_field(L, 2, "ini_content", "");
    const unsigned int axis_num = get_uint_field(L, 2, "axis_num", 0u);
    return push_result(L, axfb_bridge_configure(bridge->handle, ini_path.c_str(),
                                                ini_content.c_str(), axis_num));
}

int bridge_load_eni(lua_State* L) {
    LuaBridge* bridge = check_bridge(L, 1);
    const char* eni_path = luaL_checkstring(L, 2);
    if (bridge->handle == NULL) {
        return push_result(L, ZMC_ERR_BRIDGE_INVALID_HANDLE);
    }
    return push_result(L, axfb_bridge_load_eni(bridge->handle, eni_path));
}

int bridge_reset(lua_State* L) {
    LuaBridge* bridge = check_bridge(L, 1);
    if (bridge->handle == NULL) {
        return push_result(L, ZMC_ERR_BRIDGE_INVALID_HANDLE);
    }
    return push_result(L, axfb_bridge_reset(bridge->handle));
}

int bridge_run_cycle(lua_State* L) {
    LuaBridge* bridge = check_bridge(L, 1);
    if (bridge->handle == NULL) {
        lua_pushinteger(L, ZMC_ERR_BRIDGE_INVALID_HANDLE);
        return 1;
    }

    size_t inp_len = 0;
    const char* inp = luaL_checklstring(L, 2, &inp_len);
    luaL_argcheck(L, inp_len > 0, 2, "input bytes must not be empty");
    std::vector<uint8_t> inp_buf(inp_len);
    std::memcpy(&inp_buf[0], inp, inp_len);

    std::vector<uint8_t> oup_buf;
    if (lua_isnumber(L, 3)) {
        const int out_len = static_cast<int>(luaL_checkinteger(L, 3));
        luaL_argcheck(L, out_len > 0, 3, "output length must be positive");
        oup_buf.assign(static_cast<size_t>(out_len), 0u);
    } else {
        size_t oup_len = 0;
        const char* oup = luaL_checklstring(L, 3, &oup_len);
        luaL_argcheck(L, oup_len > 0, 3, "output bytes must not be empty");
        oup_buf.resize(oup_len);
        std::memcpy(&oup_buf[0], oup, oup_len);
    }

    const int ret = axfb_bridge_run_cycle(bridge->handle, &inp_buf[0], &oup_buf[0]);
    lua_pushinteger(L, ret);
    lua_pushlstring(L, reinterpret_cast<const char*>(&oup_buf[0]), oup_buf.size());
    return 2;
}

int bridge_push_packet(lua_State* L) {
    LuaBridge* bridge = check_bridge(L, 1);
    LuaCommandPacket* command = check_command_packet(L, 2);
    if (bridge->handle == NULL) {
        return push_result(L, ZMC_ERR_BRIDGE_INVALID_HANDLE);
    }
    return push_result(L, axfb_bridge_push_cmd_packet(bridge->handle, &command->packet));
}

const luaL_Reg kBridgeMethods[] = {
    {"__gc", bridge_gc},
    {"close", bridge_close},
    {"load_eni", bridge_load_eni},
    {"configure", bridge_configure},
    {"reset", bridge_reset},
    {"push_packet", bridge_push_packet},
    {"run_cycle", bridge_run_cycle},
    {NULL, NULL},
};

}  // namespace

void register_bridge_metatable(lua_State* L) { register_metatable(L, kBridgeMt, kBridgeMethods); }

}  // namespace zmc_lua
