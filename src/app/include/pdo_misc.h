#ifndef _PDO_MISC_H_
#define _PDO_MISC_H_

#include "stdio.h"
#include "EcatBase.h"
#include "mhelper/pdo_servo_addr.h"


typedef struct {
    servo_ctrl_word sc;    // 新控制字
    servo_stat_word ss;    // 旧状态字
    int run_delay  = 2000;         // 延时启动
    int fault_reset_delay = 2000; // 延时再错误恢复
    float zero_p;          // 零点位置
    float offset_p;        // 偏移输出位置
    int out_p;             // 输出位置
    int out_v;             // 输出转速
    int16_t out_t;             // 输出扭矩
    bool reverse;          // 当前方向
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