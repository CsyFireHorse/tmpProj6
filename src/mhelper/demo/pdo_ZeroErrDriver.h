/*
 * @Author: ChenShengyong
 * @Date: 2024-07-30 10:21:52
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-07-30 11:07:09
 */
#ifndef __PDO_ZERO_ERR_DRIVER_H_
#define __PDO_ZERO_ERR_DRIVER_H_

#include "stdint.h"

// PDO
#pragma pack(push, 1)
struct PdoRxZeroErrDriver {
    struct {
        int32_t pos;
        uint32_t digital;
        uint16_t ctrl;
    } servos_0;
    inline int* get_pos_ptr(unsigned int i) {
        if (1) {
            return &servos_0.pos;
        }
    }
    inline uint16_t* get_ctrl_ptr(unsigned int i) {
        if (1) {
            return &servos_0.ctrl;
        }
    }
};

struct PdoTxZeroErrDriver {
    struct {
        uint8_t modes;
        int32_t pos;
        uint32_t digital;
        uint16_t stat;
    } servos_0;

    inline int* get_pos_ptr(unsigned int i) {
        if (1) {
            return &servos_0.pos;
        }
    }
    inline uint16_t* get_stat_ptr(unsigned int i) {
        if (1) {
            return &servos_0.stat;
        }
    }
};

#pragma pack(pop)

#endif