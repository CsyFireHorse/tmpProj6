/*
 * @Author: ChenShengyong 
 * @Date: 2025-01-23 10:20:55 
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-01-23 13:41:28
 */
#ifndef _SERVO_WORD_H_
#define _SERVO_WORD_H_

#include "stdint.h"

typedef int16_t I16;
typedef int32_t I32;
typedef uint16_t U16;

// #pragma pack(push, 1)

typedef union {
    struct {
        U16 ready_to_switch_on: 1;  // bit 0
        U16 switched_on: 1;
        U16 operation_enabled: 1;
        U16 fault: 1;
        U16 voltage_enabled: 1;  // bit 4
        U16 quick_stop: 1;
        U16 switch_on_disabled: 1;
        U16 warning: 1;     // bit 7

        U16 pad0: 1;        // bit 8  Manufacturer specific
        U16 remote: 1;
        U16 target_reached: 1;      // bit 10
        U16 internal_limit_active: 1;
        U16 home_attained: 1;   // bit 12  Set-point acknowledge, Operation mode specific
        U16 following_error: 1; // bit 13  Operation mode specific
        U16 pad1: 1;        // Manufacturer specific
        U16 pad2: 1;        // Manufacturer specific
    } b;
    U16 v;
} servo_stat_word;

typedef union {
    struct {
        U16 switch_on: 1;   // bit 0
        U16 enable_voltage: 1;
        U16 quick_stop: 1;
        U16 enable_operation: 1; // bit 3
        U16 homing_start: 1; // bit 4, Operation mode specific
        // bit 4, HOME mode. Homing operation start
        // bit 4, PPM. New 0 Does not assume target position set-point; 1 Assume target position
        U16 abs_or_rel: 1 ; // bit 5, Operation mode specific
        U16 change_set_immed: 1; // bit 6, Change set immediately Operation mode specific
        U16 fault_reset: 1; // bit 7

        U16 halt: 1;
        U16 pa0: 2;    // bit9, bit 10  .  reserved
        U16 pad1: 5;    // manufacturer specific
    } b;
    U16 v;
} servo_ctrl_word;
// #pragma pack(pop)

#endif