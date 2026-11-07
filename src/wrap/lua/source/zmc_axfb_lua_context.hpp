/* Shared Lua wrapper state and stack helpers. */
#ifndef ZMC_AXFB_LUA_CONTEXT_HPP
#define ZMC_AXFB_LUA_CONTEXT_HPP

#include <stddef.h>
#include <stdint.h>

#include <string>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

#include "axfb_bridge_api.h"

namespace zmc_lua {

extern const char kBridgeMt[];
extern const char kCmdMt[];

struct LuaBridge {
    axfb_bridge_handle_t handle;
};

size_t raw_length(lua_State* L, int index);
int absolute_index(lua_State* L, int index);
void set_integer_field(lua_State* L, int index, const char* name, lua_Integer value);
void set_number_field(lua_State* L, int index, const char* name, lua_Number value);
void set_string_field(lua_State* L, int index, const char* name, const char* value);
int get_int_field(lua_State* L, int index, const char* name, int default_value);
unsigned int get_uint_field(lua_State* L, int index, const char* name,
                            unsigned int default_value);
std::string get_string_field(lua_State* L, int index, const char* name,
                             const char* default_value);
LuaBridge* check_bridge(lua_State* L, int index);
int push_result(lua_State* L, int ret);
void close_bridge(LuaBridge* bridge);
void register_metatable(lua_State* L, const char* name, const luaL_Reg* funcs);

}  // namespace zmc_lua

#endif  // ZMC_AXFB_LUA_CONTEXT_HPP
