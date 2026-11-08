#ifndef __PDO_OL3_H_
#define __PDO_OL3_H_

#include "stdint.h"

// PDO process image.
#pragma pack(push, 1)
struct PdoRxOL{
    struct{
        uint16_t ctrl;
        int32_t pos;
        int32_t vel;
        uint16_t touch;
    }servos_0;
    inline int* get_pos_ptr(unsigned int ){
        return &servos_0.pos;
    }
    inline uint16_t* get_ctrl_ptr(unsigned int ){
        return &servos_0.ctrl;
    }
};


struct PdoTxOL{
    struct{
        uint16_t error_code;
        uint16_t stat;
        int32_t pos;
        uint32_t digital;
        uint16_t touch_status;
        int32_t touch_val;
        uint8_t modes;
    }servos_0;
    inline int* get_pos_ptr(unsigned int ){
        return &servos_0.pos;
    }
    inline uint16_t* get_stat_ptr(unsigned int ){
        return &servos_0.stat;
    }
};

#pragma pack(pop)

#endif
