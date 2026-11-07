/* Thin memory-image engine C API. No FB type policy. */
#ifndef AXFB_MEMORY_POU_API_H
#define AXFB_MEMORY_POU_API_H

#include "zmc_common.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Error codes match MemoryOperator::MemOpCode.
 * Skip redefinition when MemoryOperator.hpp is already included.
 */
#ifndef ZMC_MEMORY_OPERATOR_HPP
enum {
    MEMOP_OK = 0,
    MEMOP_ERR_IO = 1,
    MEMOP_ERR_JSON = 2,
    MEMOP_ERR_LAYOUT = 3,
    MEMOP_ERR_INIT = 4,
    MEMOP_ERR_SYMBOL = 5,
    MEMOP_ERR_ASSIGN = 6,
    MEMOP_ERR_RANGE = 7,
    MEMOP_ERR_EXPECT = 8
};
#endif

/* Capture entry kind for out/expect samples. */
enum {
    AXFB_MEMORY_POU_CAPTURE_OUT = 0,
    AXFB_MEMORY_POU_CAPTURE_EXPECT_OK = 1,
    AXFB_MEMORY_POU_CAPTURE_EXPECT_FAIL = 2
};

typedef struct AxFbMemoryPouCaptureInfo {
    uint32_t tick;
    int32_t line;
    int32_t stmtOff;
    uint32_t src;
    uint32_t bufOff;
    uint32_t len;
    uint32_t kind;
} AxFbMemoryPouCaptureInfo;

typedef struct AxFbMemoryPou AxFbMemoryPou;
typedef AxFbMemoryPou* axfb_memory_pou_t;

ZMC_DLLPORT int axfb_memory_pou_create(axfb_memory_pou_t* out);
ZMC_DLLPORT void axfb_memory_pou_destroy(axfb_memory_pou_t handle);

/* Built-in Fbt* type catalog JSON (same as pou_memory demo layout). */
ZMC_DLLPORT const char* axfb_memory_pou_default_layout(void);

ZMC_DLLPORT int axfb_memory_pou_load_layout(axfb_memory_pou_t handle, const char* json);
/* fields: MEMORY_POU_DEMO_FIELDS body; braces added only when missing. */
ZMC_DLLPORT int axfb_memory_pou_configure(axfb_memory_pou_t handle, const char* fields);

ZMC_DLLPORT size_t axfb_memory_pou_root_size(axfb_memory_pou_t handle);
ZMC_DLLPORT size_t axfb_memory_pou_max_align(axfb_memory_pou_t handle);
ZMC_DLLPORT uint32_t axfb_memory_pou_decl_count(axfb_memory_pou_t handle);
ZMC_DLLPORT int axfb_memory_pou_decl_at(axfb_memory_pou_t handle, uint32_t index, char* type,
                                       size_t type_cap, char* name, size_t name_cap,
                                       size_t* offset, size_t* size);
ZMC_DLLPORT int axfb_memory_pou_symbol_offset(axfb_memory_pou_t handle, const char* name,
                                              size_t* offset, size_t* size);

ZMC_DLLPORT int axfb_memory_pou_bind(axfb_memory_pou_t handle, void* base, size_t bytes);
ZMC_DLLPORT int axfb_memory_pou_parse_script(axfb_memory_pou_t handle, const char* script);
ZMC_DLLPORT int axfb_memory_pou_cmd_advance(axfb_memory_pou_t handle);
ZMC_DLLPORT int axfb_memory_pou_error(axfb_memory_pou_t handle);

ZMC_DLLPORT uint32_t axfb_memory_pou_capture_count(axfb_memory_pou_t handle);
ZMC_DLLPORT int axfb_memory_pou_capture_info_at(axfb_memory_pou_t handle, uint32_t index,
                                                AxFbMemoryPouCaptureInfo* info);
ZMC_DLLPORT int axfb_memory_pou_capture_value_at(axfb_memory_pou_t handle, uint32_t index,
                                                 void* dst, size_t cap, size_t* out_len);

#ifdef __cplusplus
}
#endif

#endif
