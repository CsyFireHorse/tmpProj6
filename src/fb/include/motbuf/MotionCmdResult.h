#ifndef ZMC_MOTION_CMD_RESULT_H
#define ZMC_MOTION_CMD_RESULT_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* AI modification: GPT-6 Codex, 2026-10-21. Shared C/C++ result protocol. */
/* Non-IO target values start at one; each IO base is followed by four group values. */
typedef enum CmdTargetKind {
    CMD_TARGET_AXIS = 0x01,
    CMD_TARGET_GROUP = 0x02,
    CMD_TARGET_CONTROLLER = 0x03,
    CMD_TARGET_DI = 0x10,
    CMD_TARGET_DO = 0x14,
    CMD_TARGET_AI = 0x18,
    CMD_TARGET_AO = 0x1c,
    CMD_TARGET_INVALID = 0xff
} CmdTargetKind;

/* Shared command confirmation and execution-result payload. */
typedef struct CmdResult {
    uint32_t id;
    uint16_t command_type; /* Original command type from EnumFbm. */
    uint8_t target_index;  /* Axis/group index, or the IO starting index within a group. */
    uint8_t target_kind;   /* Target category, including the IO kind and group. */
    uint8_t status;        /* FbStatEnum. */
    uint8_t io_count;      /* IO point count; zero for non-IO or an invalid IO range. */
    uint16_t error_code;
    uint32_t tick; /* Cycle when this record is produced. */
    int32_t data;  /* IO value bits, or start-cycle bits for axis terminal results. */
    int32_t data_ext; /* Extension data. */
} CmdResult;

/* Decode only IO targets; an invalid target has no recoverable IO number. */
static inline int cmd_result_is_io(const CmdResult* result) {
    return result->target_kind >= CMD_TARGET_DI && result->target_kind < CMD_TARGET_AO + 4;
}

/* Recover the starting number within one IO kind. */
static inline uint16_t cmd_result_io_start(const CmdResult* result) {
    return (uint16_t)(((result->target_kind - CMD_TARGET_DI) & 3) * 256 + result->target_index);
}

/* A caller must require a nonzero io_count before decoding the ending number. */
static inline uint16_t cmd_result_io_end(const CmdResult* result) {
    return (uint16_t)(cmd_result_io_start(result) + result->io_count - 1);
}

/* Preserve unsigned IO values and start cycles without signed numeric conversion. */
static inline uint32_t cmd_result_data_u32(const CmdResult* result) {
    uint32_t value;
    memcpy(&value, &result->data, sizeof(value));
    return value;
}

#endif /* ZMC_MOTION_CMD_RESULT_H */
