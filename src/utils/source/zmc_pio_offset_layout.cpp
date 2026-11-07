#include "utils/zmc_pio_offset_layout.h"

#include <algorithm>
#include <cstdio>
#include <string>

#include "common/zmc/zmc_predef.h"
#include "utils/eni_descriptor_pod.hpp"

namespace {

pi_offset_config default_slot_cfg() {
    // One-axis fallback keeps older demos usable before an ENI descriptor is selected.
    pi_offset_config cfg = {0, 0, -1, 0, 2, -1, -1, 4, 2, 0, 5, -1, -1};
    return cfg;
}

void layout_set_slots(ZmcPioOffsetLayout* layout, const pi_offset_config* slots, size_t count) {
    if (layout == nullptr) {
        return;
    }
    if (slots == nullptr || count == 0) {
        layout->slots[0] = default_slot_cfg();
        layout->slot_count = 1;
        return;
    }
    const size_t n = std::min(count, static_cast<size_t>(MAX_NUM_SERVOS));
    layout->slot_count = static_cast<uint16_t>(n);
    for (size_t i = 0; i < n; ++i) {
        layout->slots[i] = slots[i];
    }
}

const pi_offset_config* layout_slot_or_null(const ZmcPioOffsetLayout* layout, size_t axis_idx) {
    if (layout == nullptr || axis_idx >= layout->slot_count) {
        return nullptr;
    }
    return &layout->slots[axis_idx];
}

}  // namespace

extern "C" MC_ERROR_CODE zmc_pio_layout_from_descriptor_pod(const EniDescriptorPod* pod,
                                                            ZmcPioOffsetLayout* out) {
    if (out == nullptr || pod == nullptr) {
        return ZMC_ERR_DEV_NULL_ARG;
    }
    *out = ZmcPioOffsetLayout{};
    eni::EniDescriptorPodWithCap pod_with_cap{};
    static_cast<EniDescriptorPod&>(pod_with_cap) = *pod;
    pod_with_cap.updateCapsFromCells();
    out->descriptor_pod = static_cast<const EniDescriptorPod&>(pod_with_cap);

    for (uint16_t slave_idx = 0; slave_idx < pod->slave_count; ++slave_idx) {
        if (pod_with_cap.slaveIsAxis(static_cast<uint8_t>(slave_idx)) == 0u) {
            continue;
        }
        if (out->slot_count >= MAX_NUM_SERVOS) {
            break;
        }
        pi_offset_config cfg{};
        if (!pod_with_cap.deduceOffsets(static_cast<uint8_t>(slave_idx), &cfg)) {
            continue;
        }
        out->slots[out->slot_count++] = cfg;
    }
    if (out->slot_count == 0) {
        layout_set_slots(out, nullptr, 0);
    }
    return MC_COMM_S_OK;
}

extern "C" MC_ERROR_CODE zmc_pio_layout_try_load_from_eni_xml_blob(const char* xml_blob,
                                                                   size_t xml_len,
                                                                   ZmcPioOffsetLayout* out) {
    if (out == nullptr || xml_blob == nullptr || xml_len == 0) {
        return ZMC_ERR_DEV_NULL_ARG;
    }
    EniDescriptorPod pod{};
    const MC_ERROR_CODE rc =
        eni::LoadDescriptorPodFromEniXmlBlob(std::string(xml_blob, xml_len), &pod);
    if (rc != MC_COMM_S_OK) {
        return rc;
    }
    return zmc_pio_layout_from_descriptor_pod(&pod, out);
}

extern "C" size_t zmc_pio_layout_slot_count(const ZmcPioOffsetLayout* layout) {
    if (layout == nullptr) {
        return 0;
    }
    return layout->slot_count;
}

extern "C" const pi_offset_config* zmc_pio_layout_slot_at(const ZmcPioOffsetLayout* layout,
                                                          size_t axis_idx) {
    const pi_offset_config* slot = layout_slot_or_null(layout, axis_idx);
    if (slot != nullptr) {
        return slot;
    }
    static const pi_offset_config kDefault = default_slot_cfg();
    return &kDefault;
}

extern "C" const EniDescriptorPod* zmc_pio_layout_descriptor_pod(
    const ZmcPioOffsetLayout* layout) {
    if (layout == nullptr) {
        return nullptr;
    }
    return &layout->descriptor_pod;
}

extern "C" void zmc_pio_layout_print_brief(const ZmcPioOffsetLayout* layout) {
    if (layout == nullptr) {
        return;
    }
    std::printf("[ENI] servo_slots=%u tx_cap=%u rx_cap=%u\r\n",
                static_cast<unsigned>(layout->slot_count),
                static_cast<unsigned>(layout->descriptor_pod.tx_cap),
                static_cast<unsigned>(layout->descriptor_pod.rx_cap));
}

void zmc_pio_layout_reset(ZmcPioOffsetLayout* layout) {
    if (layout == nullptr) {
        return;
    }
    // eni = ZmcPioOffsetLayout{};
    auto& eni = layout->descriptor_pod;
    eni.rx_cap = 0;
    eni.tx_cap = 0;
    eni.slave_count = 0;
    eni.pdo_cell_count = 0;
    layout->slot_count = 0;
}