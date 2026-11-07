#ifndef ZMC_ECAT_MOTION_AXIS_INI_HPP_
#define ZMC_ECAT_MOTION_AXIS_INI_HPP_

#include <stdio.h>

#include "EcatMotion.h"
#include "EcatWrap.h"

// Adds PDO diagnostics to the motion runtime used by axis INI demos.
class EcatMotionAxisIni : public zmc::EcatMotion {
   public:
    int bind_pdo_handler();

   private:
    static int pdoCallback(void* p, const void* inp, void* oup);
    void cycle_tick_debug(const zmc::proc_img_i_addr* txpdo, zmc::proc_img_o_addr* rxpdo);

    bool first_flag_ = true;
    int cycle_cnt_ = 0;
};

inline void EcatMotionAxisIni::cycle_tick_debug(const zmc::proc_img_i_addr* txpdo,
                                               zmc::proc_img_o_addr* rxpdo) {
    if (first_flag_) {
        first_flag_ = false;
        printf("firstFlag,%d,%d,%d\n", rxpdo->len, txpdo->len, txpdo->axLen);
    }

    if (cycle_cnt_++ % 10000 == 1) {
        printf("cnt=%d\n", cycle_cnt_);
    }

    int axisNum = axis_count();
    if (cycle_cnt_ == 999) {
        for (int i = 0; i < axisNum; ++i) {
            printf("axis[%d].addr =%d, inp.actual_position=%d\n", i, get_axis_address(i),
                   get_axis_actual_position(i));
        }
    }
}

inline int EcatMotionAxisIni::pdoCallback(void* p, const void* inp, void* oup) {
    auto* self = static_cast<EcatMotionAxisIni*>(p);
    const zmc::proc_img_i_addr* txpdo = static_cast<const zmc::proc_img_i_addr*>(inp);
    zmc::proc_img_o_addr* rxpdo = static_cast<zmc::proc_img_o_addr*>(oup);
    self->cycle_tick_debug(txpdo, rxpdo);
    return self->run_pdo_cycle_parsed(txpdo, rxpdo);
}

inline int EcatMotionAxisIni::bind_pdo_handler() {
    if (!ecat()) {
        return -1;
    }
    return ecat()->bindPdoHandler(&EcatMotionAxisIni::pdoCallback, this);
}

#endif  // ZMC_ECAT_MOTION_AXIS_INI_HPP_
