#ifndef _PDO_MISC_H_
#define _PDO_MISC_H_

#include "stdio.h"
#include "EcatBase.h"
#include "mhelper/pdo_servo_addr.h"


typedef struct {
    servo_ctrl_word sc;    // New control word.
    servo_stat_word ss;    // Previous status word.
    int run_delay  = 2000;         // Startup delay.
    int fault_reset_delay = 2000; // Delay before fault recovery.
    float zero_p;          // Zero position.
    float offset_p;        // Output position offset.
    int out_p;             // Output position.
    int out_v;             // Output velocity.
    int16_t out_t;             // Output torque.
    bool reverse;          // Current direction.
    void reset(double pos){
        out_p = pos;
        zero_p = out_p;
        offset_p = 0;
        out_v = 0;
        out_t = 0;
        reverse = false;
    }
    void init(){
        run_delay = 2000; 
        fault_reset_delay = 2000;
    }
} SERVOS_t;

namespace zmc{
bool ecat_ready(SERVOS_t* servos, int len, proc_img_i_addr *pi, proc_img_o_addr *po);

bool servo_control_enable(SERVOS_t& servo, int idx, servo_stat_word &s0, servo_stat_word &s1, servo_ctrl_word &c, bool show);

bool servo_control(SERVOS_t& _servo, int axis, servo_stat_word &s0, servo_stat_word &s1, servo_ctrl_word &c, bool show);

}


#endif
