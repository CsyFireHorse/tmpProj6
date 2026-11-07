#ifndef ZMC_PIO_LAYOUT_ADDR_H
#define ZMC_PIO_LAYOUT_ADDR_H

#include "pdo_servo_addr.h"
#include "utils/zmc_pio_offset_layout.h"

namespace zmc {

int pio_layout_get_input(proc_img_i_addr& pi_i, const ZmcPioOffsetLayout& layout,
                         const void* input);
int pio_layout_get_output(proc_img_o_addr& pi_o, const ZmcPioOffsetLayout& layout, void* output);

struct pio_layout_addr {
    ZmcPioOffsetLayout layout;
    proc_img_i_addr txpdo_addr;
    proc_img_o_addr rxpdo_addr;

    pio_layout_addr();
    void bind_layout(const ZmcPioOffsetLayout& src);
    void reset();
    bool is_empty() const;
    int apply_input(const void* inp);
    int apply_output(void* oup);
    int servo_size() const;
    int get_slave_num() const;
    int get_servo_slave_idx(int servo_idx) const;
    int get_servo_offset(int servo_idx) const;
    bool contain_key(int servo_idx, int pdo_key) const;
};

}  // namespace zmc

#endif /* ZMC_PIO_LAYOUT_ADDR_H */
