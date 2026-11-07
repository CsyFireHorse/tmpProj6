/* Memory image engine + live-image buffer Lua bindings (no FB type policy). */
#ifndef ZMC_AXFB_LUA_MEMORY_POU_HPP
#define ZMC_AXFB_LUA_MEMORY_POU_HPP

struct lua_State;

namespace zmc_lua {

int lua_axfb_memory_pou_new(lua_State* L);
int lua_axfb_image_new(lua_State* L);
int lua_axfb_axis_virtual_new(lua_State* L);
void register_memory_pou_metatables(lua_State* L);
void register_mc_fb_table(lua_State* L, int module_index);

}  // namespace zmc_lua

#endif
