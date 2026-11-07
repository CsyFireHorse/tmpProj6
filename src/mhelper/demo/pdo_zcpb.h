#ifndef __PDO_ZCPB_H_
#define __PDO_ZCPB_H_

#include "stdint.h"

// PDO 过程映像
#pragma pack(push, 1)
struct PdoRxZcpb1{
    struct{
        uint16_t ctrl;
        int32_t pos;
    }servos_0;
    inline int* get_pos_ptr(unsigned int ){
        return &servos_0.pos;
    }
    inline uint16_t* get_ctrl_ptr(unsigned int ){
        return &servos_0.ctrl;
    }
};


struct PdoTxZcpb1{
    struct{
        uint16_t stat;
        int32_t pos;
        uint16_t error_code;
        uint32_t digital;
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