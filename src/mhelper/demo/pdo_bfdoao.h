#ifndef __PDO_BFDOAO_H_
#define __PDO_BFDOAO_H_

#include "stdint.h"

// PDO process image.
#pragma pack(push, 1)
struct PdoRxBfdoao{
    // struct{}servos_0;
    // struct{}servos_1;
    // struct{}servos_2;
};

struct PdoTxBfdoao{
    // struct{}servos_0;
    struct{
        uint8_t po_DO_0:1;
        uint8_t po_DO_1:1;
        uint8_t po_DO_2:1;
        uint8_t po_DO_3:1;
        uint8_t _:4;
    }servos_1;
    struct{
        uint16_t po_AO_0;
        uint16_t po_AO_1;
        uint16_t po_AO_2;
        uint16_t po_AO_3;
    }servos_2;
    inline int get_do(unsigned int i)const{
        if (i == 0){
            return servos_1.po_DO_0;
        }else if (i == 1){
            return servos_1.po_DO_1;
        }else if (i == 2){
            return servos_1.po_DO_2;
        }else{
            return servos_1.po_DO_3;
        }
    }
    inline uint16_t* get_ao_ptr(unsigned int i){
        if (i == 0){
            return &servos_2.po_AO_0;
        }else if (i == 1){
            return &servos_2.po_AO_1;
        }else if (i == 2){
            return &servos_2.po_AO_2;
        }else{
            return &servos_2.po_AO_3;
        }
    }
};

#pragma pack(pop)

#endif
