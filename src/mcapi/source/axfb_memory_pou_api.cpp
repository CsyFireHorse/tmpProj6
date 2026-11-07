#include "memory/MemoryPouEngine.hpp"
#include "memory/MemoryPouTypeLayout.hpp"

#include "axfb_memory_pou_api.h"

#include <cstring>
#include <string>

struct AxFbMemoryPou {
    zmc::MemoryPouEngine engine;
};

namespace {

std::string wrap_fields_if_needed(const char* fields) {
    if (!fields) {
        return std::string();
    }
    std::string text(fields);
    std::size_t i = 0;
    while (i < text.size() &&
           (text[i] == ' ' || text[i] == '\t' || text[i] == '\n' || text[i] == '\r')) {
        ++i;
    }
    if (i < text.size() && text[i] == '{') {
        return text;
    }
    return std::string("{\n") + text + "\n};";
}

int copy_cstr(char* dst, size_t cap, const std::string& src) {
    if (!dst || cap == 0) {
        return MEMOP_ERR_INIT;
    }
    if (src.size() + 1 > cap) {
        return MEMOP_ERR_RANGE;
    }
    std::memcpy(dst, src.c_str(), src.size() + 1);
    return MEMOP_OK;
}

}  // namespace

int axfb_memory_pou_create(axfb_memory_pou_t* out) {
    if (!out) {
        return MEMOP_ERR_INIT;
    }
    *out = new AxFbMemoryPou();
    return MEMOP_OK;
}

void axfb_memory_pou_destroy(axfb_memory_pou_t handle) {
    delete handle;
}

const char* axfb_memory_pou_default_layout(void) {
    return kMemoryPouTypeLayout;
}

int axfb_memory_pou_load_layout(axfb_memory_pou_t handle, const char* json) {
    if (!handle || !json) {
        return MEMOP_ERR_INIT;
    }
    return handle->engine.loadTypeLayout(json);
}

int axfb_memory_pou_configure(axfb_memory_pou_t handle, const char* fields) {
    if (!handle || !fields) {
        return MEMOP_ERR_INIT;
    }
    return handle->engine.configure(wrap_fields_if_needed(fields));
}

size_t axfb_memory_pou_root_size(axfb_memory_pou_t handle) {
    return handle ? handle->engine.memoryOperator().rootSize() : 0;
}

size_t axfb_memory_pou_max_align(axfb_memory_pou_t handle) {
    return handle ? handle->engine.memoryOperator().maxAlign() : 1;
}

uint32_t axfb_memory_pou_decl_count(axfb_memory_pou_t handle) {
    if (!handle) {
        return 0;
    }
    return static_cast<uint32_t>(handle->engine.memoryOperator().declCount());
}

int axfb_memory_pou_decl_at(axfb_memory_pou_t handle, uint32_t index, char* type, size_t type_cap,
                            char* name, size_t name_cap, size_t* offset, size_t* size) {
    if (!handle || !offset || !size) {
        return MEMOP_ERR_INIT;
    }
    const MemoryRootDecl* decl = handle->engine.memoryOperator().declAt(index);
    if (!decl) {
        return MEMOP_ERR_RANGE;
    }
    const int tr = copy_cstr(type, type_cap, decl->type);
    if (tr != MEMOP_OK) {
        return tr;
    }
    const int nr = copy_cstr(name, name_cap, decl->name);
    if (nr != MEMOP_OK) {
        return nr;
    }
    *offset = decl->offset;
    *size = decl->size;
    return MEMOP_OK;
}

int axfb_memory_pou_symbol_offset(axfb_memory_pou_t handle, const char* name, size_t* offset,
                                  size_t* size) {
    if (!handle || !name || !offset || !size) {
        return MEMOP_ERR_INIT;
    }
    const SymbolInfo* sym = handle->engine.memoryOperator().findSymbol(name);
    if (!sym) {
        return MEMOP_ERR_SYMBOL;
    }
    *offset = sym->offset;
    *size = sym->size;
    return MEMOP_OK;
}

int axfb_memory_pou_bind(axfb_memory_pou_t handle, void* base, size_t bytes) {
    if (!handle || !base || bytes == 0) {
        return MEMOP_ERR_INIT;
    }
    if (bytes < handle->engine.memoryOperator().rootSize()) {
        return MEMOP_ERR_RANGE;
    }
    handle->engine.bindMemory(base, bytes);
    return MEMOP_OK;
}

int axfb_memory_pou_parse_script(axfb_memory_pou_t handle, const char* script) {
    if (!handle || !script) {
        return MEMOP_ERR_INIT;
    }
    return handle->engine.parseScheduledScript(script);
}

int axfb_memory_pou_cmd_advance(axfb_memory_pou_t handle) {
    if (!handle) {
        return MEMOP_ERR_INIT;
    }
    handle->engine.cmdAdvance();
    return handle->engine.errorCode();
}

int axfb_memory_pou_error(axfb_memory_pou_t handle) {
    return handle ? handle->engine.errorCode() : MEMOP_ERR_INIT;
}

uint32_t axfb_memory_pou_capture_count(axfb_memory_pou_t handle) {
    if (!handle) {
        return 0;
    }
    return static_cast<uint32_t>(handle->engine.captureCount());
}

int axfb_memory_pou_capture_info_at(axfb_memory_pou_t handle, uint32_t index,
                                    AxFbMemoryPouCaptureInfo* info) {
    if (!handle || !info) {
        return MEMOP_ERR_INIT;
    }
    const zmc::MemoryPouCaptureInfo* cap = handle->engine.captureAt(index);
    if (!cap) {
        return MEMOP_ERR_RANGE;
    }
    info->tick = cap->tick;
    info->line = cap->line;
    info->stmtOff = cap->stmtOff;
    info->src = cap->src;
    info->bufOff = cap->bufOff;
    info->len = cap->len;
    info->kind = cap->kind;
    return MEMOP_OK;
}

int axfb_memory_pou_capture_value_at(axfb_memory_pou_t handle, uint32_t index, void* dst,
                                     size_t cap, size_t* out_len) {
    if (!handle || !out_len) {
        return MEMOP_ERR_INIT;
    }
    const zmc::MemoryPouCaptureInfo* entry = handle->engine.captureAt(index);
    if (!entry) {
        return MEMOP_ERR_RANGE;
    }
    *out_len = entry->len;
    if (!dst) {
        return MEMOP_OK;
    }
    if (entry->len > cap) {
        return MEMOP_ERR_RANGE;
    }
    const unsigned char* buf = handle->engine.valueBufData();
    if (!buf && entry->len > 0) {
        return MEMOP_ERR_ASSIGN;
    }
    if (entry->len > 0) {
        std::memcpy(dst, buf + entry->bufOff, entry->len);
    }
    return MEMOP_OK;
}
