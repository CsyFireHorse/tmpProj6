/* Lua module registration and exported loader entry point. */
#include "zmc_axfb_lua_module.h"

#include "axfb_memory_pou_api.h"
#include "common/zmc/fbt_errorcode_def.h"
#include "zmc_axfb_lua_bridge.hpp"
#include "zmc_axfb_lua_command.hpp"
#include "zmc_axfb_lua_context.hpp"
#include "zmc_axfb_lua_memory_pou.hpp"
#include "zmc_axfb_lua_status.hpp"

namespace {

const int kMcFiniteDirection = 4;

void set_constants(lua_State* L, int module_index) {
    module_index = zmc_lua::absolute_index(L, module_index);
    lua_newtable(L);
    int err_index = zmc_lua::absolute_index(L, -1);
    zmc_lua::set_integer_field(L, err_index, "ZMC_NO_ERROR", ZMC_NO_ERROR);
    zmc_lua::set_integer_field(L, err_index, "ZMC_ERR_BRIDGE_INVALID_ARG",
                               ZMC_ERR_BRIDGE_INVALID_ARG);
    zmc_lua::set_integer_field(L, err_index, "ZMC_ERR_BRIDGE_INVALID_HANDLE",
                               ZMC_ERR_BRIDGE_INVALID_HANDLE);
    zmc_lua::set_integer_field(L, err_index, "ZMC_ERR_MOTCMD_PACKET_APPEND_FAILED",
                               ZMC_ERR_MOTCMD_PACKET_APPEND_FAILED);
    zmc_lua::set_integer_field(L, err_index, "MEMOP_OK", MEMOP_OK);
    zmc_lua::set_integer_field(L, err_index, "MEMOP_ERR_INIT", MEMOP_ERR_INIT);
    zmc_lua::set_integer_field(L, err_index, "MEMOP_ERR_LAYOUT", MEMOP_ERR_LAYOUT);
    zmc_lua::set_integer_field(L, err_index, "MEMOP_ERR_SYMBOL", MEMOP_ERR_SYMBOL);
    zmc_lua::set_integer_field(L, err_index, "MEMOP_ERR_EXPECT", MEMOP_ERR_EXPECT);
    lua_setfield(L, module_index, "err");

    lua_newtable(L);
    int dir_index = zmc_lua::absolute_index(L, -1);
    zmc_lua::set_integer_field(L, dir_index, "negative", -1);
    zmc_lua::set_integer_field(L, dir_index, "shortest", 0);
    zmc_lua::set_integer_field(L, dir_index, "positive", 1);
    zmc_lua::set_integer_field(L, dir_index, "current", 2);
    zmc_lua::set_integer_field(L, dir_index, "fastest", 3);
    zmc_lua::set_integer_field(L, dir_index, "finite", kMcFiniteDirection);
    lua_setfield(L, module_index, "direction");
}

const luaL_Reg kModuleFuncs[] = {
    {"new", zmc_lua::lua_axfb_new},
    {"packet", zmc_lua::lua_axfb_packet_new},
    {"status", zmc_lua::lua_axfb_status_new},
    {"memory_pou", zmc_lua::lua_axfb_memory_pou_new},
    {"image", zmc_lua::lua_axfb_image_new},
    {"axis_virtual", zmc_lua::lua_axfb_axis_virtual_new},
    {NULL, NULL},
};

}  // namespace

extern "C" ZMC_AXFB_LUA_EXPORT int luaopen_zmc_axfb(lua_State* L) {
    zmc_lua::register_bridge_metatable(L);
    zmc_lua::register_command_metatable(L);
    zmc_lua::register_status_metatable(L);
    zmc_lua::register_memory_pou_metatables(L);

    lua_newtable(L);
    const int module_index = zmc_lua::absolute_index(L, -1);
    luaL_setfuncs(L, kModuleFuncs, 0);

    set_constants(L, module_index);
    zmc_lua::register_mc_fb_table(L, module_index);
    return 1;
}
