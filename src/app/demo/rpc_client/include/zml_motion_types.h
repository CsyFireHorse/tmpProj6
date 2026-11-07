#ifndef ZML_MOTION_TYPES_H
#define ZML_MOTION_TYPES_H

#include <stdint.h>
#include "zml_types_def.h"

/* Compact axis state returned by the RPC API. */
typedef struct ZML_AxisState {
    uint8_t axis_id;
    uint8_t axis_state;
    uint16_t error_id;
    uint32_t tick;
    double act_pos;
    double set_pos;
} ZML_AxisState;

/* Extended axis state returned by the RPC API. */
typedef struct ZML_AxisStateEx {
    uint8_t axis_id;
    uint8_t axis_state;
    uint16_t error_id;
    uint32_t tick;
    double act_pos;
    double set_pos;
    double set_vel;
    double set_acc;
    int32_t set_position;
    int32_t act_position;
} ZML_AxisStateEx;

typedef struct ZML_CmdResultInfo {
    uint32_t id;
    uint16_t command_type;
    uint8_t target_index;
    uint8_t target_kind;
    uint8_t status;
    uint8_t io_count;
    uint16_t error_code;
    uint32_t tick;
    int32_t data;
    int32_t data_ext;
} ZML_CmdResultInfo;

typedef struct ZML_FlowCtrlConfig {
    uint16_t period;
    uint16_t batch_size;
    uint8_t verbose_flag;
    uint8_t reserved[3];
} ZML_FlowCtrlConfig;

typedef struct ZML_FlowCtrlStatus {
    uint16_t period;
    uint16_t batch_size;
    uint8_t verbose_flag;
    uint8_t reserved[3];
    uint16_t latency;
    uint16_t axis_num;
    uint32_t cycle_time;
} ZML_FlowCtrlStatus;

typedef struct ZML_SetupStatus {
    uint32_t id;
    uint16_t type;
    uint16_t param;
    uint32_t value;
    uint32_t tick;
    uint16_t status;
    uint16_t padding;
} ZML_SetupStatus;

#endif /* ZML_MOTION_TYPES_H */
