/* mcapi command-packet Lua userdata registration. */
#ifndef ZMC_AXFB_LUA_COMMAND_HPP
#define ZMC_AXFB_LUA_COMMAND_HPP

#include "zmc_axfb_lua_context.hpp"

#include "axfb_command_packet_api.h"

namespace zmc_lua {

struct LuaCommandPacket {
    AxFbBridgePacket packet;
};

LuaCommandPacket* check_command_packet(lua_State* L, int index);
int lua_axfb_packet_new(lua_State* L);
void register_command_metatable(lua_State* L);

}  // namespace zmc_lua

#endif  // ZMC_AXFB_LUA_COMMAND_HPP
