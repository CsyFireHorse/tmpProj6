/*
 * @Description: ENI descriptor compact POD layout (C-compatible, no C++ namespace).
 */

#ifndef ZMC_ENI_DESCRIPTOR_POD_H
#define ZMC_ENI_DESCRIPTOR_POD_H

#include <stdint.h>

#include "common/zmc/zmc_predef.h"

/* is_input = 0: master Tx (output to slave); 1: master Rx (input from slave) */
typedef struct RtBusPdoCell {
    uint16_t index;
    uint16_t bit_offset; /* absolute bit offset in process image */
    uint8_t sub_index;
    uint8_t bit_size; /* bit length; 0 = empty/unused */
    uint8_t is_input;
    uint8_t slave_idx;
} RtBusPdoCell;

typedef struct SlaveAddrInfo {
    uint32_t phys_addr;
    uint32_t auto_inc_addr;
    uint16_t pdo_cell_begin;
    uint16_t pdo_cell_count;
} SlaveAddrInfo;

typedef struct EniMasterTiming {
    uint32_t cycle_time_us;
    uint32_t mailbox_cycle_time_us;
    uint32_t auto_recovery_timeout_us;
} EniMasterTiming;

typedef struct EniDescriptorPod {
    EniMasterTiming master;
    uint16_t slave_count;
    uint16_t pdo_cell_count;
    uint32_t tx_cap;
    uint32_t rx_cap;
    SlaveAddrInfo slaves[MAX_NUM_SLAVES];
    RtBusPdoCell pdo_cells[MC_ENI_POD_MAX_PDO_CELLS];
} EniDescriptorPod;

#endif /* ZMC_ENI_DESCRIPTOR_POD_H */
