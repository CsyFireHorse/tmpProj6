/* Independent Lua status-client userdata registration. */
#ifndef ZMC_AXFB_LUA_STATUS_HPP
#define ZMC_AXFB_LUA_STATUS_HPP

#include "zmc_axfb_lua_context.hpp"

#include "axfb_status_api.h"

namespace zmc_lua {

struct LuaStatus {
    axfb_status_handle_t handle;
};

int lua_axfb_status_new(lua_State* L);
void register_status_metatable(lua_State* L);

}  // namespace zmc_lua

#endif  // ZMC_AXFB_LUA_STATUS_HPP
