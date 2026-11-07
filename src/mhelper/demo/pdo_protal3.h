#ifndef __PDO_PROTAL3_H_
#define __PDO_PROTAL3_H_

#include "stdint.h"

// PDO 过程映像
#pragma pack(push, 1)
struct PdoRxProtal3{
    struct{
        uint16_t ctrl;
        int32_t pos;
        uint16_t touch;
    }servos_0;
    struct{
        uint16_t ctrl;
        int32_t pos;
        int32_t vel;
        uint16_t touch;
    }servos_1;
    struct{
        uint16_t ctrl;
        int32_t pos;
        uint16_t touch;
    }servos_2;

    inline int* get_pos_ptr(unsigned int i){
        if (i == 0){
            return &servos_0.pos;
        }else if (i == 1){
            return &servos_1.pos;
        }else{
            return &servos_2.pos;
        }
    }
    inline uint16_t* get_ctrl_ptr(unsigned int i){
        if (i == 0){
            return &servos_0.ctrl;
        }else if (i == 1){
            return &servos_1.ctrl;
        }else{
            return &servos_2.ctrl;
        }
    }
};


struct PdoTxProtal3{
    struct{
        uint16_t error_code;
        uint16_t stat;
        uint8_t modes;
        int32_t pos;
        uint16_t touch_status;
        int32_t touch_val;
        uint32_t digital;
    }servos_0;
    struct{
        uint16_t error_code;
        uint16_t stat;
        int32_t pos;
        uint32_t digital;
        uint16_t touch_status;
        int32_t touch_val;
        uint8_t modes;
    }servos_1;
    struct{
        uint16_t error_code;
        uint16_t stat;
        uint8_t modes;
        int32_t pos;
        uint16_t touch_status;
        int32_t touch_val;
        uint32_t digital;
    }servos_2;

    inline int* get_pos_ptr(unsigned int i){
        if (i == 0){
            return &servos_0.pos;
        }else if (i == 1){
            return &servos_1.pos;
        }else{
            return &servos_2.pos;
        }
    }
    inline uint16_t* get_stat_ptr(unsigned int i){
        if (i == 0){
            return &servos_0.stat;
        }else if (i == 1){
            return &servos_1.stat;
        }else{
            return &servos_2.stat;
        }
    }
};

#pragma pack(pop)

#endif