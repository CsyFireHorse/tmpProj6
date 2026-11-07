#include "utils/eni_descriptor_pod.hpp"

#include "utils/eni_motor_map.hpp"

namespace eni {

MCCommResult FromDescriptor(const EniDescriptor& desc, EniDescriptorPod* out);

namespace {

constexpr uint32_t kDefaultMailboxCycleUs = 5000u;
constexpr uint32_t kDefaultAutoRecoveryTimeoutUs = 1000u;

// Keep absent PDO objects as -1 so downstream offset checks can reject them cheaply.
void InitOffset(pi_offset_config* cfg) {
    cfg->slave_idx = 0;
    cfg->slave_channel_idx = 0;
    cfg->po_mode_offset = -1;
    cfg->po_ctrl_word_offset = -1;
    cfg->po_p_offset = -1;
    cfg->po_v_offset = -1;
    cfg->po_t_offset = -1;
    cfg->pi_mode_offset = -1;
    cfg->pi_stat_offset = -1;
    cfg->pi_error_offset = -1;
    cfg->pi_p_offset = -1;
    cfg->pi_v_offset = -1;
    cfg->pi_t_offset = -1;
}

bool TryMakeCell(const PdoEntry& entry, uint8_t slave_idx, uint8_t is_input, RtBusPdoCell* cell) {
    if (cell == nullptr) {
        return false;
    }
    // Compact POD stores absolute bit addressing; reject empty or out-of-range entries.
    if (entry.bit_size == 0u || entry.bit_size > 0xffu || entry.bit_offset > 0xffffu) {
        return false;
    }

    cell->index = static_cast<uint16_t>(entry.index);
    cell->bit_offset = static_cast<uint16_t>(entry.bit_offset);
    cell->sub_index = static_cast<uint8_t>(entry.subindex);
    cell->bit_size = static_cast<uint8_t>(entry.bit_size);
    cell->is_input = is_input;
    cell->slave_idx = slave_idx;
    return true;
}

bool AppendCell(EniDescriptorPod* pod, const RtBusPdoCell& cell) {
    if (pod->pdo_cell_count >= MC_ENI_POD_MAX_PDO_CELLS) {
        return false;
    }
    pod->pdo_cells[pod->pdo_cell_count] = cell;
    ++pod->pdo_cell_count;
    return true;
}

bool HasPdoIndex(const std::vector<PdoEntry>& entries, uint32_t index) {
    for (const PdoEntry& entry : entries) {
        if (entry.index == index) {
            return true;
        }
    }
    return false;
}

bool IsServoSlave(const SlaveInfo& slave) {
    // Keep the axis limit aligned with the ENI parser: only CiA402-capable slaves count as axes.
    return HasPdoIndex(slave.outputs, 0x6040u) && HasPdoIndex(slave.inputs, 0x6041u);
}

void ApplyCellToOffsets(const RtBusPdoCell& cell, pi_offset_config* cfg) {
    // Legacy servo offsets are byte-based; skip non-byte-aligned CiA402 objects.
    if ((cell.bit_offset % 8u) != 0u) {
        return;
    }
    const int32_t byte_ofs = static_cast<int32_t>(cell.bit_offset / 8u);
    // Map the CiA402 objects used by the motion bridge into the legacy offset structure.
    if (cell.is_input == 0u) {
        switch (cell.index) {
            case 0x6040:
                cfg->po_ctrl_word_offset = byte_ofs;
                break;
            case 0x607A:
                cfg->po_p_offset = byte_ofs;
                break;
            case 0x60FF:
                cfg->po_v_offset = byte_ofs;
                break;
            case 0x6071:
                cfg->po_t_offset = byte_ofs;
                break;
            case 0x6060:
                cfg->po_mode_offset = byte_ofs;
                break;
            default:
                break;
        }
    } else {
        switch (cell.index) {
            case 0x6041:
                cfg->pi_stat_offset = byte_ofs;
                break;
            case 0x6064:
                cfg->pi_p_offset = byte_ofs;
                break;
            case 0x606C:
                cfg->pi_v_offset = byte_ofs;
                break;
            case 0x6077:
                cfg->pi_t_offset = byte_ofs;
                break;
            case 0x6061:
                cfg->pi_mode_offset = byte_ofs;
                break;
            case 0x603F:
                cfg->pi_error_offset = byte_ofs;
                break;
            default:
                break;
        }
    }
}

MCCommResult LoadDescriptorPodFromXmlBlobImpl(const std::string& xml_blob, EniDescriptorPod* out) {
    if (out == nullptr) {
        return ZMC_ERR_DEV_NULL_ARG;
    }
    EniDescriptor desc{};
    const MCCommResult rc = LoadDescriptorFromEniXmlBlob(xml_blob, &desc);
    if (rc != MC_COMM_S_OK) {
        return rc;
    }
    return FromDescriptor(desc, out);
}

}  // namespace

void EniDescriptorPodWithCap::updateCapsFromCells() {
    uint32_t tx_end = 0;
    uint32_t rx_end = 0;
    // Capacities remain byte counts: ceil of the highest bit touched by each direction.
    for (uint16_t i = 0; i < pdo_cell_count; ++i) {
        const RtBusPdoCell& cell = pdo_cells[i];
        const uint32_t end_bits =
            static_cast<uint32_t>(cell.bit_offset) + static_cast<uint32_t>(cell.bit_size);
        const uint32_t end = (end_bits + 7u) / 8u;
        if (cell.is_input == 0u) {
            if (end > tx_end) {
                tx_end = end;
            }
        } else if (end > rx_end) {
            rx_end = end;
        }
    }
    tx_cap = tx_end;
    rx_cap = rx_end;
}

bool EniDescriptorPodWithCap::deduceOffsets(uint8_t slave_idx, pi_offset_config* out) const {
    if (out == nullptr || slave_idx >= slave_count) {
        return false;
    }

    InitOffset(out);
    out->slave_idx = slave_idx;

    const SlaveAddrInfo& slave = slaves[slave_idx];
    const uint32_t end = static_cast<uint32_t>(slave.pdo_cell_begin) + slave.pdo_cell_count;
    // Each slave owns a contiguous slice in pdo_cells, recorded by pdo_cell_begin/count.
    for (uint32_t i = slave.pdo_cell_begin; i < end; ++i) {
        ApplyCellToOffsets(pdo_cells[i], out);
    }
    return true;
}

uint8_t EniDescriptorPodWithCap::slaveIsAxis(uint8_t slave_idx) const {
    if (slave_idx >= slave_count) {
        return 0u;
    }

    const SlaveAddrInfo& slave = slaves[slave_idx];
    const uint32_t end = static_cast<uint32_t>(slave.pdo_cell_begin) + slave.pdo_cell_count;
    bool has_ctrl = false;
    bool has_stat = false;
    // Treat a slave as a motion axis only when both controlword and statusword are present.
    for (uint32_t i = slave.pdo_cell_begin; i < end; ++i) {
        const RtBusPdoCell& cell = pdo_cells[i];
        if (cell.is_input == 0u && cell.index == 0x6040u) {
            has_ctrl = true;
        }
        if (cell.is_input == 1u && cell.index == 0x6041u) {
            has_stat = true;
        }
    }
    return (has_ctrl && has_stat) ? 1u : 0u;
}

MCCommResult FromDescriptor(const EniDescriptor& desc, EniDescriptorPod* out) {
    if (out == nullptr) {
        return ZMC_ERR_DEV_NULL_ARG;
    }

    *out = EniDescriptorPod{};
    out->master.cycle_time_us = desc.master.cycle_time_us;
    out->master.mailbox_cycle_time_us = kDefaultMailboxCycleUs;
    out->master.auto_recovery_timeout_us = kDefaultAutoRecoveryTimeoutUs;

    if (desc.slaves.size() > MAX_NUM_SLAVES) {
        return ZMC_ERR_ENI_SLAVE_OVERFLOW;
    }

    uint8_t slave_idx = 0;
    uint32_t axis_count = 0;
    for (const SlaveInfo& slave : desc.slaves) {
        if (IsServoSlave(slave)) {
            ++axis_count;
            if (axis_count > MAX_NUM_SERVOS) {
                return ZMC_ERR_ENI_AXIS_OVERFLOW;
            }
        }
        SlaveAddrInfo& slot = out->slaves[slave_idx];
        slot.phys_addr = slave.phys_addr;
        slot.auto_inc_addr = slave.auto_inc_addr;
        slot.pdo_cell_begin = out->pdo_cell_count;

        // Flatten output/input PDO vectors into one fixed-size POD array for C/shared-memory
        // users.
        const auto append_entries = [&](const std::vector<PdoEntry>& entries,
                                        uint8_t is_input) -> MCCommResult {
            for (const PdoEntry& entry : entries) {
                RtBusPdoCell cell{};
                if (!TryMakeCell(entry, slave_idx, is_input, &cell)) {
                    continue;
                }
                if (!AppendCell(out, cell)) {
                    return ZMC_ERR_ENI_POD_OVERFLOW;
                }
            }
            return MC_COMM_S_OK;
        };

        MCCommResult rc = append_entries(slave.outputs, 0u);
        if (rc != MC_COMM_S_OK) {
            return rc;
        }
        rc = append_entries(slave.inputs, 1u);
        if (rc != MC_COMM_S_OK) {
            return rc;
        }

        slot.pdo_cell_count = static_cast<uint16_t>(out->pdo_cell_count - slot.pdo_cell_begin);
        ++slave_idx;
    }

    out->slave_count = slave_idx;
    return MC_COMM_S_OK;
}

MCCommResult LoadDescriptorPodFromEniXmlBlob(const std::string& xml_blob, EniDescriptorPod* out) {
    return LoadDescriptorPodFromXmlBlobImpl(xml_blob, out);
}

}  // namespace eni
