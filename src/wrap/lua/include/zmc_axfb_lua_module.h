/* Lua module loader entry point. */
#ifndef ZMC_AXFB_LUA_MODULE_H
#define ZMC_AXFB_LUA_MODULE_H

struct lua_State;

/* Exported Lua loader entry point for require("zmc.axfb"). */
#ifdef _WIN32
#define ZMC_AXFB_LUA_EXPORT __declspec(dllexport)
#else
#define ZMC_AXFB_LUA_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

ZMC_AXFB_LUA_EXPORT int luaopen_zmc_axfb(struct lua_State* L);

#ifdef __cplusplus
}
#endif

#endif  /* ZMC_AXFB_LUA_MODULE_H */
