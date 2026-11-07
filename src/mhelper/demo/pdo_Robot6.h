/*
 * @Author: ChenShengyong 
 * @Date: 2024-07-30 11:07:23 
 * @Last Modified by:   ChenShengyong 
 * @Last Modified time: 2024-07-30 11:07:23 
 */
#ifndef __PDO_ROBOT6_H_
#define __PDO_ROBOT6_H_

#include "stdint.h"

// PDO
#pragma pack(push, 1)
struct PdoRxRobot6{
    struct{
        uint16_t ctrl;
        int32_t pos;
        uint8_t mode;
    }servos_0;
    struct{
        uint16_t ctrl;
        int32_t pos;
        uint8_t mode;
    }servos_1;
    struct{
        uint16_t ctrl;
        int32_t pos;
        uint8_t mode;
    }servos_2;
    struct{
        uint16_t ctrl;
        int32_t pos;
        uint8_t mode;
    }servos_3;
    struct{
        uint16_t ctrl;
        int32_t pos;
        uint8_t mode;
    }servos_4;
    struct{
        uint16_t ctrl;
        int32_t pos;
        uint8_t mode;
    }servos_5;

    inline int* get_pos_ptr(unsigned int i){
        if (i == 0){
            return &servos_0.pos;
        }else if (i == 1){
            return &servos_1.pos;
        }else if (i == 2){
            return &servos_2.pos;
        }else if (i == 3){
            return &servos_3.pos;
        }else if (i == 4){
            return &servos_4.pos;
        }else {
            return &servos_5.pos;
        }
    }
    inline uint16_t* get_ctrl_ptr(unsigned int i){
        if (i == 0){
            return &servos_0.ctrl;
        }else if (i == 1){
            return &servos_1.ctrl;
        }else if (i == 2){
            return &servos_2.ctrl;
        }else if (i == 3){
            return &servos_3.ctrl;
        }else if (i == 4){
            return &servos_4.ctrl;
        }else {
            return &servos_5.ctrl;
        }
    }
};


struct PdoTxRobot6{
    struct{
        uint16_t stat;
        int32_t pos;
    }servos_0;
    struct{
        uint16_t stat;
        int32_t pos;
    }servos_1;
    struct{
        uint16_t stat;
        int32_t pos;
    }servos_2;
    struct{
        uint16_t stat;
        int32_t pos;
    }servos_3;
    struct{
        uint16_t stat;
        int32_t pos;
    }servos_4;
    struct{
        uint16_t stat;
        int32_t pos;
    }servos_5;
    inline int* get_pos_ptr(unsigned int i){
        if (i == 0){
            return &servos_0.pos;
        }else if (i == 1){
            return &servos_1.pos;
        }else if (i == 2){
            return &servos_2.pos;
        }else if (i == 3){
            return &servos_3.pos;
        }else if (i == 4){
            return &servos_4.pos;
        }else {
            return &servos_5.pos;
        }
    }
    inline uint16_t* get_stat_ptr(unsigned int i){
        if (i == 0){
            return &servos_0.stat;
        }else if (i == 1){
            return &servos_1.stat;
        }else if (i == 2){
            return &servos_2.stat;
        }else if (i == 3){
            return &servos_3.stat;
        }else if (i == 4){
            return &servos_4.stat;
        }else {
            return &servos_5.stat;
        }
    }
};

#pragma pack(pop)

#endif