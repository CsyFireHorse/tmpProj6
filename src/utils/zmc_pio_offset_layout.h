/*
 * @Description: ENI-derived per-axis PDO offset layout (POD + extern "C" builders/queries).
 */

#ifndef ZMC_PIO_OFFSET_LAYOUT_H
#define ZMC_PIO_OFFSET_LAYOUT_H

#include <stddef.h>
#include <stdint.h>

#include "common/zmc/fbt_errorcode_def.h"
#include "common/zmc/pdo_offset_cfg.h"
#include "common/zmc/zmc_predef.h"
#include "utils/eni_descriptor_pod.h"

typedef struct ZmcPioOffsetLayout {
    EniDescriptorPod descriptor_pod;
    pi_offset_config slots[MAX_NUM_SERVOS];
    uint16_t slot_count;
} ZmcPioOffsetLayout;

#ifdef __cplusplus
extern "C" {
#endif

MC_ERROR_CODE zmc_pio_layout_from_descriptor_pod(const EniDescriptorPod* pod,
                                                 ZmcPioOffsetLayout* out);

MC_ERROR_CODE zmc_pio_layout_try_load_from_eni_xml_blob(const char* xml_blob, size_t xml_len,
                                                        ZmcPioOffsetLayout* out);

size_t zmc_pio_layout_slot_count(const ZmcPioOffsetLayout* layout);

const pi_offset_config* zmc_pio_layout_slot_at(const ZmcPioOffsetLayout* layout, size_t axis_idx);

const EniDescriptorPod* zmc_pio_layout_descriptor_pod(const ZmcPioOffsetLayout* layout);

void zmc_pio_layout_print_brief(const ZmcPioOffsetLayout* layout);

void zmc_pio_layout_reset(ZmcPioOffsetLayout* layout);

#ifdef __cplusplus
}
#endif

#endif /* ZMC_PIO_OFFSET_LAYOUT_H */
