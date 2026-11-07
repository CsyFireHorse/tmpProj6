/* Memory-pou engine, live image buffer, axis_virtual, and Mc* create/exec/destroy. */
#include "zmc_axfb_lua_memory_pou.hpp"

#include <cstring>
#include <new>
#include <stdint.h>
#include <vector>

#include "axis_config_api.h"
#include "axis_virtual_api.h"
#include "axfb_memory_pou_api.h"
#include "fbt_axis_base.h"
#include "mc_fb_api.h"
#include "zmc_axfb_lua_context.hpp"

namespace zmc_lua {
namespace {

const char kMemoryPouMt[] = "zmc.axfb.memory_pou";
const char kImageMt[] = "zmc.axfb.image";
const char kAxisVirtualMt[] = "zmc.axfb.axis_virtual";

struct LuaMemoryPou {
    axfb_memory_pou_t handle;
};

struct LuaImage {
    unsigned char* bytes;
    size_t size;
    size_t align;
};

struct LuaAxisVirtual {
    axis_virtual_handle_t handle;
};

LuaMemoryPou* check_memory_pou(lua_State* L, int index) {
    return static_cast<LuaMemoryPou*>(luaL_checkudata(L, index, kMemoryPouMt));
}

LuaImage* check_image(lua_State* L, int index) {
    return static_cast<LuaImage*>(luaL_checkudata(L, index, kImageMt));
}

LuaAxisVirtual* check_axis_virtual(lua_State* L, int index) {
    return static_cast<LuaAxisVirtual*>(luaL_checkudata(L, index, kAxisVirtualMt));
}

unsigned char* image_ptr_at(LuaImage* image, size_t offset, size_t need) {
    if (!image || !image->bytes || offset > image->size || need > image->size - offset) {
        return NULL;
    }
    return image->bytes + offset;
}

int memory_pou_gc(lua_State* L) {
    LuaMemoryPou* pou = check_memory_pou(L, 1);
    if (pou->handle) {
        axfb_memory_pou_destroy(pou->handle);
        pou->handle = NULL;
    }
    return 0;
}

int memory_pou_load_layout(lua_State* L) {
    LuaMemoryPou* pou = check_memory_pou(L, 1);
    const char* json = NULL;
    if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
        json = luaL_checkstring(L, 2);
    } else {
        json = axfb_memory_pou_default_layout();
    }
    return push_result(L, axfb_memory_pou_load_layout(pou->handle, json));
}

int memory_pou_configure(lua_State* L) {
    LuaMemoryPou* pou = check_memory_pou(L, 1);
    const char* fields = luaL_checkstring(L, 2);
    return push_result(L, axfb_memory_pou_configure(pou->handle, fields));
}

int memory_pou_root_size(lua_State* L) {
    LuaMemoryPou* pou = check_memory_pou(L, 1);
    lua_pushinteger(L, static_cast<lua_Integer>(axfb_memory_pou_root_size(pou->handle)));
    return 1;
}

int memory_pou_max_align(lua_State* L) {
    LuaMemoryPou* pou = check_memory_pou(L, 1);
    lua_pushinteger(L, static_cast<lua_Integer>(axfb_memory_pou_max_align(pou->handle)));
    return 1;
}

int memory_pou_decl_count(lua_State* L) {
    LuaMemoryPou* pou = check_memory_pou(L, 1);
    lua_pushinteger(L, static_cast<lua_Integer>(axfb_memory_pou_decl_count(pou->handle)));
    return 1;
}

int memory_pou_decl_at(lua_State* L) {
    LuaMemoryPou* pou = check_memory_pou(L, 1);
    const uint32_t index = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    char type[128];
    char name[128];
    size_t offset = 0;
    size_t size = 0;
    const int ret =
        axfb_memory_pou_decl_at(pou->handle, index, type, sizeof(type), name, sizeof(name),
                                &offset, &size);
    if (ret != MEMOP_OK) {
        lua_pushnil(L);
        lua_pushinteger(L, ret);
        return 2;
    }
    lua_pushstring(L, type);
    lua_pushstring(L, name);
    lua_pushinteger(L, static_cast<lua_Integer>(offset));
    lua_pushinteger(L, static_cast<lua_Integer>(size));
    return 4;
}

int memory_pou_symbol_offset(lua_State* L) {
    LuaMemoryPou* pou = check_memory_pou(L, 1);
    const char* name = luaL_checkstring(L, 2);
    size_t offset = 0;
    size_t size = 0;
    const int ret = axfb_memory_pou_symbol_offset(pou->handle, name, &offset, &size);
    if (ret != MEMOP_OK) {
        lua_pushnil(L);
        lua_pushinteger(L, ret);
        return 2;
    }
    lua_pushinteger(L, static_cast<lua_Integer>(offset));
    lua_pushinteger(L, static_cast<lua_Integer>(size));
    return 2;
}

int memory_pou_bind(lua_State* L) {
    LuaMemoryPou* pou = check_memory_pou(L, 1);
    LuaImage* image = check_image(L, 2);
    return push_result(L, axfb_memory_pou_bind(pou->handle, image->bytes, image->size));
}

int memory_pou_parse_script(lua_State* L) {
    LuaMemoryPou* pou = check_memory_pou(L, 1);
    const char* script = luaL_checkstring(L, 2);
    return push_result(L, axfb_memory_pou_parse_script(pou->handle, script));
}

int memory_pou_cmd_advance(lua_State* L) {
    LuaMemoryPou* pou = check_memory_pou(L, 1);
    return push_result(L, axfb_memory_pou_cmd_advance(pou->handle));
}

int memory_pou_error(lua_State* L) {
    LuaMemoryPou* pou = check_memory_pou(L, 1);
    return push_result(L, axfb_memory_pou_error(pou->handle));
}

int memory_pou_capture_count(lua_State* L) {
    LuaMemoryPou* pou = check_memory_pou(L, 1);
    lua_pushinteger(L, static_cast<lua_Integer>(axfb_memory_pou_capture_count(pou->handle)));
    return 1;
}

int memory_pou_capture_at(lua_State* L) {
    LuaMemoryPou* pou = check_memory_pou(L, 1);
    const uint32_t index = static_cast<uint32_t>(luaL_checkinteger(L, 2));
    AxFbMemoryPouCaptureInfo info;
    const int ret = axfb_memory_pou_capture_info_at(pou->handle, index, &info);
    if (ret != MEMOP_OK) {
        lua_pushnil(L);
        lua_pushinteger(L, ret);
        return 2;
    }
    size_t value_len = 0;
    const int vret =
        axfb_memory_pou_capture_value_at(pou->handle, index, NULL, 0, &value_len);
    if (vret != MEMOP_OK) {
        lua_pushnil(L);
        lua_pushinteger(L, vret);
        return 2;
    }
    std::vector<unsigned char> bytes(value_len);
    if (value_len > 0) {
        const int copy_ret = axfb_memory_pou_capture_value_at(pou->handle, index, &bytes[0],
                                                               value_len, &value_len);
        if (copy_ret != MEMOP_OK) {
            lua_pushnil(L);
            lua_pushinteger(L, copy_ret);
            return 2;
        }
    }
    lua_newtable(L);
    const int tbl = lua_gettop(L);
    lua_pushinteger(L, static_cast<lua_Integer>(info.tick));
    lua_setfield(L, tbl, "tick");
    lua_pushinteger(L, static_cast<lua_Integer>(info.line));
    lua_setfield(L, tbl, "line");
    lua_pushinteger(L, static_cast<lua_Integer>(info.stmtOff));
    lua_setfield(L, tbl, "stmtOff");
    lua_pushinteger(L, static_cast<lua_Integer>(info.src));
    lua_setfield(L, tbl, "src");
    lua_pushinteger(L, static_cast<lua_Integer>(info.bufOff));
    lua_setfield(L, tbl, "bufOff");
    lua_pushinteger(L, static_cast<lua_Integer>(info.len));
    lua_setfield(L, tbl, "len");
    lua_pushinteger(L, static_cast<lua_Integer>(info.kind));
    lua_setfield(L, tbl, "kind");
    if (value_len > 0) {
        lua_pushlstring(L, reinterpret_cast<const char*>(&bytes[0]), value_len);
    } else {
        lua_pushlstring(L, "", 0);
    }
    lua_setfield(L, tbl, "value");
    return 1;
}

const luaL_Reg kMemoryPouMethods[] = {
    {"__gc", memory_pou_gc},
    {"load_layout", memory_pou_load_layout},
    {"configure", memory_pou_configure},
    {"root_size", memory_pou_root_size},
    {"max_align", memory_pou_max_align},
    {"decl_count", memory_pou_decl_count},
    {"decl_at", memory_pou_decl_at},
    {"symbol_offset", memory_pou_symbol_offset},
    {"bind", memory_pou_bind},
    {"parse_script", memory_pou_parse_script},
    {"cmd_advance", memory_pou_cmd_advance},
    {"error", memory_pou_error},
    {"capture_count", memory_pou_capture_count},
    {"capture_at", memory_pou_capture_at},
    {NULL, NULL},
};

int image_gc(lua_State* L) {
    LuaImage* image = check_image(L, 1);
    if (image->bytes) {
        ::operator delete(image->bytes);
        image->bytes = NULL;
        image->size = 0;
    }
    return 0;
}

int image_size(lua_State* L) {
    LuaImage* image = check_image(L, 1);
    lua_pushinteger(L, static_cast<lua_Integer>(image->size));
    return 1;
}

int image_at(lua_State* L) {
    LuaImage* image = check_image(L, 1);
    const size_t offset = static_cast<size_t>(luaL_checkinteger(L, 2));
    size_t need = 1;
    if (lua_gettop(L) >= 3) {
        need = static_cast<size_t>(luaL_checkinteger(L, 3));
    }
    unsigned char* p = image_ptr_at(image, offset, need);
    if (!p) {
        return luaL_error(L, "image:at out of range");
    }
    lua_pushlightuserdata(L, p);
    return 1;
}

int image_write_ptr(lua_State* L) {
    LuaImage* image = check_image(L, 1);
    const size_t offset = static_cast<size_t>(luaL_checkinteger(L, 2));
    void* value = lua_touserdata(L, 3);
    unsigned char* p = image_ptr_at(image, offset, sizeof(void*));
    if (!p) {
        return push_result(L, MEMOP_ERR_RANGE);
    }
    std::memcpy(p, &value, sizeof(void*));
    return push_result(L, MEMOP_OK);
}

const luaL_Reg kImageMethods[] = {
    {"__gc", image_gc},
    {"size", image_size},
    {"at", image_at},
    {"write_ptr", image_write_ptr},
    {NULL, NULL},
};

int axis_virtual_gc(lua_State* L) {
    LuaAxisVirtual* axis = check_axis_virtual(L, 1);
    if (axis->handle) {
        axis_virtual_delete(axis->handle);
        axis->handle = NULL;
    }
    return 0;
}

int axis_virtual_ref(lua_State* L) {
    LuaAxisVirtual* axis = check_axis_virtual(L, 1);
    AXIS_REF ref = axis_virtual_get_ref(axis->handle);
    lua_pushlightuserdata(L, ref);
    return 1;
}

int axis_virtual_run_cycle(lua_State* L) {
    LuaAxisVirtual* axis = check_axis_virtual(L, 1);
    axis_virtual_runcycle(axis->handle);
    return push_result(L, 0);
}

int axis_virtual_act_position(lua_State* L) {
    LuaAxisVirtual* axis = check_axis_virtual(L, 1);
    AXIS_REF ref = axis_virtual_get_ref(axis->handle);
    if (!ref) {
        lua_pushnumber(L, 0);
        return 1;
    }
    zmc::AxisImplBase* impl = reinterpret_cast<zmc::AxisImplBase*>(ref);
    lua_pushnumber(L, impl->fActPosition);
    return 1;
}

int axis_virtual_set_position(lua_State* L) {
    LuaAxisVirtual* axis = check_axis_virtual(L, 1);
    AXIS_REF ref = axis_virtual_get_ref(axis->handle);
    if (!ref) {
        return push_result(L, MEMOP_ERR_INIT);
    }
    const double pos = luaL_checknumber(L, 2);
    zmc::AxisImplBase* impl = reinterpret_cast<zmc::AxisImplBase*>(ref);
    impl->fActPosition = pos;
    impl->fSetPosition = pos;
    return push_result(L, 0);
}

const luaL_Reg kAxisVirtualMethods[] = {
    {"__gc", axis_virtual_gc},
    {"ref", axis_virtual_ref},
    {"run_cycle", axis_virtual_run_cycle},
    {"act_position", axis_virtual_act_position},
    {"set_position", axis_virtual_set_position},
    {NULL, NULL},
};

template <typename McT>
McT* check_mc_slot(lua_State* L, int index) {
    void* p = lua_touserdata(L, index);
    luaL_argcheck(L, p != NULL, index, "expected lightuserdata Mc* slot");
    return static_cast<McT*>(p);
}

#define ZMC_LUA_MC_BIND(Name, McT, create_fn, exec_fn, destroy_fn)        \
    int lua_mc_##Name##_create(lua_State* L) {                            \
        McT* slot = check_mc_slot<McT>(L, 1);                             \
        return push_result(L, create_fn(slot));                           \
    }                                                                     \
    int lua_mc_##Name##_exec(lua_State* L) {                              \
        McT* slot = check_mc_slot<McT>(L, 1);                             \
        return push_result(L, exec_fn(slot));                             \
    }                                                                     \
    int lua_mc_##Name##_destroy(lua_State* L) {                           \
        McT* slot = check_mc_slot<McT>(L, 1);                             \
        return push_result(L, destroy_fn(slot));                          \
    }

ZMC_LUA_MC_BIND(Power, McPower, MC_Power_create, MC_Power_exec, MC_Power_destroy)
ZMC_LUA_MC_BIND(Halt, McHalt, MC_Halt_create, MC_Halt_exec, MC_Halt_destroy)
ZMC_LUA_MC_BIND(MoveRelative, McMoveRelative, MC_MoveRelative_create, MC_MoveRelative_exec,
                MC_MoveRelative_destroy)
ZMC_LUA_MC_BIND(MoveAdditive, McMoveAdditive, MC_MoveAdditive_create, MC_MoveAdditive_exec,
                MC_MoveAdditive_destroy)
ZMC_LUA_MC_BIND(MoveAbsolute, McMoveAbsolute, MC_MoveAbsolute_create, MC_MoveAbsolute_exec,
                MC_MoveAbsolute_destroy)

#undef ZMC_LUA_MC_BIND

}  // namespace

int lua_axfb_memory_pou_new(lua_State* L) {
    LuaMemoryPou* pou = static_cast<LuaMemoryPou*>(lua_newuserdata(L, sizeof(LuaMemoryPou)));
    pou->handle = NULL;
    luaL_getmetatable(L, kMemoryPouMt);
    lua_setmetatable(L, -2);
    const int ret = axfb_memory_pou_create(&pou->handle);
    if (ret != MEMOP_OK || !pou->handle) {
        if (pou->handle) {
            axfb_memory_pou_destroy(pou->handle);
            pou->handle = NULL;
        }
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_pushinteger(L, ret);
        return 2;
    }
    lua_pushinteger(L, MEMOP_OK);
    return 2;
}

int lua_axfb_image_new(lua_State* L) {
    const size_t size = static_cast<size_t>(luaL_checkinteger(L, 1));
    size_t align = 8;
    if (lua_gettop(L) >= 2) {
        align = static_cast<size_t>(luaL_checkinteger(L, 2));
    }
    if (size == 0 || align == 0) {
        lua_pushnil(L);
        lua_pushinteger(L, MEMOP_ERR_INIT);
        return 2;
    }
    LuaImage* image = static_cast<LuaImage*>(lua_newuserdata(L, sizeof(LuaImage)));
    image->bytes = NULL;
    image->size = size;
    image->align = align;
    luaL_getmetatable(L, kImageMt);
    lua_setmetatable(L, -2);
    image->bytes = static_cast<unsigned char*>(::operator new(size));
    std::memset(image->bytes, 0, size);
    lua_pushinteger(L, MEMOP_OK);
    return 2;
}

int lua_axfb_axis_virtual_new(lua_State* L) {
    LuaAxisVirtual* axis =
        static_cast<LuaAxisVirtual*>(lua_newuserdata(L, sizeof(LuaAxisVirtual)));
    axis->handle = NULL;
    luaL_getmetatable(L, kAxisVirtualMt);
    lua_setmetatable(L, -2);

    axis->handle = axis_virtual_new();
    if (!axis->handle) {
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_pushinteger(L, MEMOP_ERR_INIT);
        return 2;
    }

    AxisCiaConfig cfg;
    axis_config_default_init(&cfg);
    cfg.cycle_time = 10000;
    cfg.is_axis_virtual = true;
    cfg.debug.servo_fak_pos = 1;
    if (lua_istable(L, 1)) {
        lua_getfield(L, 1, "cycle_time");
        if (lua_isnumber(L, -1)) {
            cfg.cycle_time = static_cast<int>(lua_tointeger(L, -1));
        }
        lua_pop(L, 1);
    }
    axis_virtual_set_config(axis->handle, &cfg);
    lua_pushinteger(L, MEMOP_OK);
    return 2;
}

void register_memory_pou_metatables(lua_State* L) {
    register_metatable(L, kMemoryPouMt, kMemoryPouMethods);
    register_metatable(L, kImageMt, kImageMethods);
    register_metatable(L, kAxisVirtualMt, kAxisVirtualMethods);
}

void register_mc_fb_table(lua_State* L, int module_index) {
    module_index = absolute_index(L, module_index);
    lua_newtable(L);
    const int mc_index = absolute_index(L, -1);

    lua_pushcfunction(L, lua_mc_Power_create);
    lua_setfield(L, mc_index, "Power_create");
    lua_pushcfunction(L, lua_mc_Power_exec);
    lua_setfield(L, mc_index, "Power_exec");
    lua_pushcfunction(L, lua_mc_Power_destroy);
    lua_setfield(L, mc_index, "Power_destroy");

    lua_pushcfunction(L, lua_mc_Halt_create);
    lua_setfield(L, mc_index, "Halt_create");
    lua_pushcfunction(L, lua_mc_Halt_exec);
    lua_setfield(L, mc_index, "Halt_exec");
    lua_pushcfunction(L, lua_mc_Halt_destroy);
    lua_setfield(L, mc_index, "Halt_destroy");

    lua_pushcfunction(L, lua_mc_MoveRelative_create);
    lua_setfield(L, mc_index, "MoveRelative_create");
    lua_pushcfunction(L, lua_mc_MoveRelative_exec);
    lua_setfield(L, mc_index, "MoveRelative_exec");
    lua_pushcfunction(L, lua_mc_MoveRelative_destroy);
    lua_setfield(L, mc_index, "MoveRelative_destroy");

    lua_pushcfunction(L, lua_mc_MoveAdditive_create);
    lua_setfield(L, mc_index, "MoveAdditive_create");
    lua_pushcfunction(L, lua_mc_MoveAdditive_exec);
    lua_setfield(L, mc_index, "MoveAdditive_exec");
    lua_pushcfunction(L, lua_mc_MoveAdditive_destroy);
    lua_setfield(L, mc_index, "MoveAdditive_destroy");

    lua_pushcfunction(L, lua_mc_MoveAbsolute_create);
    lua_setfield(L, mc_index, "MoveAbsolute_create");
    lua_pushcfunction(L, lua_mc_MoveAbsolute_exec);
    lua_setfield(L, mc_index, "MoveAbsolute_exec");
    lua_pushcfunction(L, lua_mc_MoveAbsolute_destroy);
    lua_setfield(L, mc_index, "MoveAbsolute_destroy");

    lua_setfield(L, module_index, "mc");
}

}  // namespace zmc_lua
