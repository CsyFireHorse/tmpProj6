#ifndef _PDO_ITEM_FLAG_HPP_
#define _PDO_ITEM_FLAG_HPP_

#include "stdint.h"

typedef union _PdoEnableFlag {
    uint32_t enable_value;
    struct {
        uint8_t enable_status_word : 1;
        uint8_t enable_actual_postion : 1;
        uint8_t enable_actual_velocity : 1;
        uint8_t enable_actual_torque : 1;

        uint8_t enable_digital_inputs : 1;
        uint8_t enable_following_error : 1;
        uint8_t enable_modes_of_operation_display : 1;
        uint8_t enable_touch_probe_1_falling_edge : 1;

        uint8_t enable_touch_probe_1_rising_edge : 1;
        uint8_t enable_touch_probe_2_falling_edge : 1;
        uint8_t enable_touch_probe_2_rising_edge : 1;
        uint8_t enable_touch_probe_status : 1;
        // control word
        uint8_t enable_control_word : 1;
        uint8_t enable_set_position : 1;
        uint8_t enable_set_velocity : 1;
        uint8_t enable_set_torque : 1;
        uint8_t enable_add_torque_value : 1;
        uint8_t enable_add_velocity_value : 1;
        uint8_t enable_digital_output : 1;
        uint8_t enable_modes_of_operation : 1;

        uint8_t enable_touch_probe_function : 1;
    };
    uint8_t enable_arr[4];
    
}PdoEnableFlag;

inline void PdoEnableFlag_init(PdoEnableFlag* pdoFlg) {
    pdoFlg->enable_value = 0;

    pdoFlg->enable_status_word = 1;
    pdoFlg->enable_actual_postion = 1;
    pdoFlg->enable_control_word = 1;
    pdoFlg->enable_set_position = 1;
}

inline uint32_t PdoEnableFlag_init_value() {
    PdoEnableFlag pdoFlg;
    PdoEnableFlag_init(&pdoFlg);
    return pdoFlg.enable_value;
}

#endif