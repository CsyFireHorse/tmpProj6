/* Shared Lua wrapper state and stack helpers. */
#include "zmc_axfb_lua_context.hpp"

namespace zmc_lua {

const char kBridgeMt[] = "zmc.axfb.bridge";
const char kCmdMt[] = "zmc.axfb.cmd";

size_t raw_length(lua_State* L, int index) { return lua_rawlen(L, index); }

int absolute_index(lua_State* L, int index) {
    if (index > 0 || index <= LUA_REGISTRYINDEX) {
        return index;
    }
    return lua_gettop(L) + index + 1;
}

void set_integer_field(lua_State* L, int index, const char* name, lua_Integer value) {
    index = absolute_index(L, index);
    lua_pushinteger(L, value);
    lua_setfield(L, index, name);
}

void set_number_field(lua_State* L, int index, const char* name, lua_Number value) {
    index = absolute_index(L, index);
    lua_pushnumber(L, value);
    lua_setfield(L, index, name);
}

void set_string_field(lua_State* L, int index, const char* name, const char* value) {
    index = absolute_index(L, index);
    lua_pushstring(L, value);
    lua_setfield(L, index, name);
}

int get_int_field(lua_State* L, int index, const char* name, int default_value) {
    index = absolute_index(L, index);
    lua_getfield(L, index, name);
    int value = default_value;
    if (!lua_isnil(L, -1)) {
        value = static_cast<int>(luaL_checkinteger(L, -1));
    }
    lua_pop(L, 1);
    return value;
}

unsigned int get_uint_field(lua_State* L, int index, const char* name,
                            unsigned int default_value) {
    index = absolute_index(L, index);
    lua_getfield(L, index, name);
    unsigned int value = default_value;
    if (!lua_isnil(L, -1)) {
        value = static_cast<unsigned int>(luaL_checkinteger(L, -1));
    }
    lua_pop(L, 1);
    return value;
}

std::string get_string_field(lua_State* L, int index, const char* name,
                             const char* default_value) {
    index = absolute_index(L, index);
    lua_getfield(L, index, name);
    std::string value(default_value ? default_value : "");
    if (!lua_isnil(L, -1)) {
        const char* raw = luaL_checkstring(L, -1);
        value = raw ? raw : "";
    }
    lua_pop(L, 1);
    return value;
}

LuaBridge* check_bridge(lua_State* L, int index) {
    return static_cast<LuaBridge*>(luaL_checkudata(L, index, kBridgeMt));
}

int push_result(lua_State* L, int ret) {
    lua_pushinteger(L, ret);
    return 1;
}

void close_bridge(LuaBridge* bridge) {
    if (bridge != NULL && bridge->handle != NULL) {
        axfb_bridge_destroy(bridge->handle);
        bridge->handle = NULL;
    }
}

void register_metatable(lua_State* L, const char* name, const luaL_Reg* funcs) {
    luaL_newmetatable(L, name);
    luaL_setfuncs(L, funcs, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);
}

}  // namespace zmc_lua
