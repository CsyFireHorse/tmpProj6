#include "zmc_pio_layout_addr.h"

#include "common/ecat/ecat_pdo_key.h"

namespace zmc {

namespace {

#define OFFSET_UNIT 8

// Resolve a typed PDO field pointer from a mutable process-image base; -1 offset yields nullptr.
template <typename T>
T* pio_field_at(void* base, int offset) {
    return (offset != -1) ? reinterpret_cast<T*>(static_cast<char*>(base) + offset) : nullptr;
}

// Same for const input buffer (proc_img_i_addr keeps writable field pointers).
template <typename T>
T* pio_field_at(const void* base, int offset) {
    return (offset != -1)
               ? reinterpret_cast<T*>(const_cast<char*>(static_cast<const char*>(base)) + offset)
               : nullptr;
}

// Reuse the same slot query rules as eni_servo_parser.cpp (EniServoMapper::contain_key).
bool layout_slot_contain_key(const pi_offset_config& servo, int idx) {
    switch (idx) {
        case PDO_PO_MODE:
            return servo.po_mode_offset != -1;
        case PDO_PO_CTRL:
            return servo.po_ctrl_word_offset != -1;
        case PDO_PO_POS:
            return servo.po_p_offset != -1;
        case PDO_PO_VEL:
            return servo.po_v_offset != -1;
        case PDO_PO_TORCH:
            return servo.po_t_offset != -1;
        case PDO_PI_MODE:
            return servo.pi_mode_offset != -1;
        case PDO_PI_STAT:
            return servo.pi_stat_offset != -1;
        case PDO_PI_ERROR:
            return servo.pi_error_offset != -1;
        case PDO_PI_POS:
            return servo.pi_p_offset != -1;
        case PDO_PI_VEL:
            return servo.pi_v_offset != -1;
        case PDO_PI_TORCH:
            return servo.pi_t_offset != -1;
        default:
            return false;
    }
}

int pio_layout_fill_input(proc_img_i_addr& pi_i, const ZmcPioOffsetLayout& layout,
                          const void* input) {
    const size_t slot_count = zmc_pio_layout_slot_count(&layout);
    pi_i.pBuf = const_cast<void*>(input);
    pi_i.axLen = static_cast<uint16_t>(slot_count);
    for (size_t i = 0; i < slot_count; ++i) {
        const pi_offset_config* curr = zmc_pio_layout_slot_at(&layout, i);
        if (curr == nullptr) {
            return -1;
        }
        servo_stat_addr* stat = &pi_i.servos[i];
        stat->slave_idx = curr->slave_idx;
        stat->slave_channel_idx = curr->slave_channel_idx;
        stat->mode = pio_field_at<uint8_t>(input, curr->pi_mode_offset);
        stat->stat = pio_field_at<servo_stat_word>(input, curr->pi_stat_offset);
        stat->error_code = pio_field_at<U16>(input, curr->pi_error_offset);
        stat->p = pio_field_at<I32>(input, curr->pi_p_offset);
        stat->v = pio_field_at<I32>(input, curr->pi_v_offset);
        stat->t = pio_field_at<I16>(input, curr->pi_t_offset);
    }
    return 0;
}

int pio_layout_fill_output(proc_img_o_addr& pi_o, const ZmcPioOffsetLayout& layout, void* output) {
    const size_t slot_count = zmc_pio_layout_slot_count(&layout);
    if (pi_o.pBuf == output && pi_o.axLen == slot_count) {
        return 0;
    }
    pi_o.pBuf = output;
    pi_o.axLen = static_cast<uint16_t>(slot_count);
    for (size_t i = 0; i < slot_count; ++i) {
        const pi_offset_config* curr = zmc_pio_layout_slot_at(&layout, i);
        if (curr == nullptr) {
            return -1;
        }
        servo_ctrl_addr* ctrl = &pi_o.servos[i];
        ctrl->slave_idx = curr->slave_idx;
        ctrl->slave_channel_idx = curr->slave_channel_idx;
        ctrl->mode = pio_field_at<uint8_t>(output, curr->po_mode_offset);
        ctrl->ctrl = pio_field_at<servo_ctrl_word>(output, curr->po_ctrl_word_offset);
        ctrl->p = pio_field_at<I32>(output, curr->po_p_offset);
        ctrl->v = pio_field_at<I32>(output, curr->po_v_offset);
        ctrl->t = pio_field_at<I16>(output, curr->po_t_offset);
    }
    return 0;
}

}  // namespace

int pio_layout_get_input(proc_img_i_addr& pi_i, const ZmcPioOffsetLayout& layout,
                         const void* input) {
    pi_i.len = layout.descriptor_pod.tx_cap * OFFSET_UNIT;
    return pio_layout_fill_input(pi_i, layout, input);
}

int pio_layout_get_output(proc_img_o_addr& pi_o, const ZmcPioOffsetLayout& layout, void* output) {
    pi_o.len = layout.descriptor_pod.rx_cap * OFFSET_UNIT;
    return pio_layout_fill_output(pi_o, layout, output);
}

pio_layout_addr::pio_layout_addr() : layout(), txpdo_addr(), rxpdo_addr() {}

void pio_layout_addr::reset() {
    zmc_pio_layout_reset(&layout);
    txpdo_addr = proc_img_i_addr();
    rxpdo_addr = proc_img_o_addr();
}

void pio_layout_addr::bind_layout(const ZmcPioOffsetLayout& src) {
    layout = src;
    txpdo_addr.len = layout.descriptor_pod.tx_cap * OFFSET_UNIT;
    rxpdo_addr.len = layout.descriptor_pod.rx_cap * OFFSET_UNIT;
    txpdo_addr.axLen = static_cast<uint16_t>(zmc_pio_layout_slot_count(&layout));
    rxpdo_addr.axLen = static_cast<uint16_t>(zmc_pio_layout_slot_count(&layout));
    apply_input(nullptr);
    apply_output(nullptr);
}

bool pio_layout_addr::is_empty() const {
    return layout.descriptor_pod.slave_count == 0 && layout.descriptor_pod.pdo_cell_count == 0;
}

int pio_layout_addr::apply_input(const void* inp) {
    return pio_layout_get_input(txpdo_addr, layout, inp);
}

int pio_layout_addr::apply_output(void* oup) {
    return pio_layout_get_output(rxpdo_addr, layout, oup);
}

int pio_layout_addr::servo_size() const {
    return static_cast<int>(zmc_pio_layout_slot_count(&layout));
}

int pio_layout_addr::get_slave_num() const { return layout.descriptor_pod.slave_count; }

int pio_layout_addr::get_servo_slave_idx(int servo_idx) const {
    const pi_offset_config* slot = zmc_pio_layout_slot_at(&layout, static_cast<size_t>(servo_idx));
    if (slot == nullptr) {
        return -1;
    }
    return slot->slave_idx;
}

int pio_layout_addr::get_servo_offset(int servo_idx) const {
    const pi_offset_config* slot = zmc_pio_layout_slot_at(&layout, static_cast<size_t>(servo_idx));
    if (slot == nullptr) {
        return 0;
    }
    return slot->slave_channel_idx * PDO_MULT_SERVO_OFFSET;
}

bool pio_layout_addr::contain_key(int servo_idx, int pdo_key) const {
    const pi_offset_config* slot = zmc_pio_layout_slot_at(&layout, static_cast<size_t>(servo_idx));
    if (slot == nullptr) {
        return false;
    }
    return layout_slot_contain_key(*slot, pdo_key);
}

}  // namespace zmc
