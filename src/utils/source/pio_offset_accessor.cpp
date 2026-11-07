#include "utils/pio_offset_accessor.hpp"

namespace eni {

PioOffsetAccessor::PioOffsetAccessor(const ZmcPioOffsetLayout& layout)
    : layout_(layout)
{
}

void PioOffsetAccessor::bind(const uint8_t* tx, uint8_t* rx)
{
    tx_ = tx;
    rx_ = rx;
}

void PioOffsetAccessor::unbind()
{
    tx_ = nullptr;
    rx_ = nullptr;
}

const pi_offset_config* PioOffsetAccessor::slotOrNull(size_t axis_idx) const
{
    if (axis_idx >= layout_.slot_count) {
        return nullptr;
    }
    return &layout_.slots[axis_idx];
}

bool PioOffsetAccessor::ofsOkTx(int32_t ofs, uint32_t sz) const
{
    return ofs >= 0 && static_cast<uint32_t>(ofs) + sz <= layout_.descriptor_pod.tx_cap;
}

bool PioOffsetAccessor::ofsOkRx(int32_t ofs, uint32_t sz) const
{
    return ofs >= 0 && static_cast<uint32_t>(ofs) + sz <= layout_.descriptor_pod.rx_cap;
}

uint16_t PioOffsetAccessor::rdU16Tx(int32_t ofs, uint16_t fallback) const
{
    if (tx_ == nullptr || !ofsOkTx(ofs, 2)) {
        return fallback;
    }
    return static_cast<uint16_t>(tx_[ofs] | (static_cast<uint16_t>(tx_[ofs + 1]) << 8));
}

uint16_t PioOffsetAccessor::rdU16Rx(int32_t ofs, uint16_t fallback) const
{
    if (rx_ == nullptr || !ofsOkRx(ofs, 2)) {
        return fallback;
    }
    return static_cast<uint16_t>(rx_[ofs] | (static_cast<uint16_t>(rx_[ofs + 1]) << 8));
}

int32_t PioOffsetAccessor::rdI32Tx(int32_t ofs, int32_t fallback) const
{
    if (tx_ == nullptr || !ofsOkTx(ofs, 4)) {
        return fallback;
    }
    uint32_t u = 0;
    u |= static_cast<uint32_t>(tx_[ofs]);
    u |= static_cast<uint32_t>(tx_[ofs + 1]) << 8;
    u |= static_cast<uint32_t>(tx_[ofs + 2]) << 16;
    u |= static_cast<uint32_t>(tx_[ofs + 3]) << 24;
    return static_cast<int32_t>(u);
}

int32_t PioOffsetAccessor::rdI32Rx(int32_t ofs, int32_t fallback) const
{
    if (rx_ == nullptr || !ofsOkRx(ofs, 4)) {
        return fallback;
    }
    uint32_t u = 0;
    u |= static_cast<uint32_t>(rx_[ofs]);
    u |= static_cast<uint32_t>(rx_[ofs + 1]) << 8;
    u |= static_cast<uint32_t>(rx_[ofs + 2]) << 16;
    u |= static_cast<uint32_t>(rx_[ofs + 3]) << 24;
    return static_cast<int32_t>(u);
}

void PioOffsetAccessor::wrU16Rx(int32_t ofs, uint16_t v)
{
    if (rx_ == nullptr || !ofsOkRx(ofs, 2)) {
        return;
    }
    rx_[ofs] = static_cast<uint8_t>(v & 0xFFu);
    rx_[ofs + 1] = static_cast<uint8_t>((v >> 8) & 0xFFu);
}

void PioOffsetAccessor::wrI32Rx(int32_t ofs, int32_t v)
{
    if (rx_ == nullptr || !ofsOkRx(ofs, 4)) {
        return;
    }
    const uint32_t u = static_cast<uint32_t>(v);
    rx_[ofs] = static_cast<uint8_t>(u & 0xFFu);
    rx_[ofs + 1] = static_cast<uint8_t>((u >> 8) & 0xFFu);
    rx_[ofs + 2] = static_cast<uint8_t>((u >> 16) & 0xFFu);
    rx_[ofs + 3] = static_cast<uint8_t>((u >> 24) & 0xFFu);
}

uint16_t PioOffsetAccessor::getTxControlWord(size_t axis_idx, uint16_t fallback) const
{
    const pi_offset_config* slot = slotOrNull(axis_idx);
    if (slot == nullptr) {
        return fallback;
    }
    return rdU16Tx(slot->po_ctrl_word_offset, fallback);
}

int32_t PioOffsetAccessor::getTxTargetPos(size_t axis_idx, int32_t fallback) const
{
    const pi_offset_config* slot = slotOrNull(axis_idx);
    if (slot == nullptr) {
        return fallback;
    }
    return rdI32Tx(slot->po_p_offset, fallback);
}

uint16_t PioOffsetAccessor::getRxStatusWord(size_t axis_idx, uint16_t fallback) const
{
    const pi_offset_config* slot = slotOrNull(axis_idx);
    if (slot == nullptr) {
        return fallback;
    }
    return rdU16Rx(slot->pi_stat_offset, fallback);
}

int32_t PioOffsetAccessor::getRxActualPos(size_t axis_idx, int32_t fallback) const
{
    const pi_offset_config* slot = slotOrNull(axis_idx);
    if (slot == nullptr) {
        return fallback;
    }
    return rdI32Rx(slot->pi_p_offset, fallback);
}

void PioOffsetAccessor::setRxStatusWord(size_t axis_idx, uint16_t value)
{
    const pi_offset_config* slot = slotOrNull(axis_idx);
    if (slot == nullptr) {
        return;
    }
    wrU16Rx(slot->pi_stat_offset, value);
}

void PioOffsetAccessor::setRxActualPos(size_t axis_idx, int32_t value)
{
    const pi_offset_config* slot = slotOrNull(axis_idx);
    if (slot == nullptr) {
        return;
    }
    wrI32Rx(slot->pi_p_offset, value);
}

void PioOffsetAccessor::setRxModeFromTx(size_t axis_idx)
{
    const pi_offset_config* slot = slotOrNull(axis_idx);
    if (slot == nullptr) {
        return;
    }
    const pi_offset_config& cfg = *slot;
    if (tx_ == nullptr || rx_ == nullptr || cfg.po_mode_offset < 0 || cfg.pi_mode_offset < 0 || !ofsOkTx(cfg.po_mode_offset, 1)
        || !ofsOkRx(cfg.pi_mode_offset, 1)) {
        return;
    }
    rx_[cfg.pi_mode_offset] = tx_[cfg.po_mode_offset];
}

void PioOffsetAccessor::setRxErrorWord(size_t axis_idx, uint16_t value)
{
    const pi_offset_config* slot = slotOrNull(axis_idx);
    if (slot == nullptr) {
        return;
    }
    wrU16Rx(slot->pi_error_offset, value);
}

} // namespace eni
