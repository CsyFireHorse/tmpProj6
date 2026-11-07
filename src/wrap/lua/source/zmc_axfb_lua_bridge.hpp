/* Bridge userdata registration. */
#ifndef ZMC_AXFB_LUA_BRIDGE_HPP
#define ZMC_AXFB_LUA_BRIDGE_HPP

#include "zmc_axfb_lua_context.hpp"

namespace zmc_lua {

int lua_axfb_new(lua_State* L);
void register_bridge_metatable(lua_State* L);

}  // namespace zmc_lua

#endif  // ZMC_AXFB_LUA_BRIDGE_HPP
