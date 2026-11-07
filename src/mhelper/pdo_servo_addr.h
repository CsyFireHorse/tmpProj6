#ifndef _PDO_SERVO_ADDR_H_
#define _PDO_SERVO_ADDR_H_

#include "stdint.h"
#include "common/ecat/servo_word.h"
#include "common/zmc/pdo_offset_cfg.h"
// #include <vector>
// #include "eni_servo_parser.h"
#include "common/zmc/zmc_predef.h"


namespace zmc{
typedef struct {
    uint8_t             slave_idx;
    uint8_t             slave_channel_idx;
    uint8_t             *mode;
    servo_ctrl_word     *ctrl;
    I32                 *p;
    I32                 *v;
    I16                 *t;    
} servo_ctrl_addr;

typedef struct {
    uint8_t             slave_idx;
    uint8_t             slave_channel_idx;
    uint8_t             *mode;
    servo_stat_word     *stat;
    U16                 *error_code;
    I32                 *p;
    I32                 *v;
    I16                 *t;
} servo_stat_addr;

typedef struct {
    uint16_t len;
    uint16_t axLen;
    void*   pBuf;
    servo_ctrl_addr servos[MAX_NUM_SERVOS];
} proc_img_o_addr;

typedef struct {
    uint16_t len;
    uint16_t axLen;
    void*   pBuf;
    servo_stat_addr servos[MAX_NUM_SERVOS];
} proc_img_i_addr;

struct EniServoMapper;
struct pdo_addr{
    proc_img_i_addr txpdo_addr;
    proc_img_o_addr rxpdo_addr;
    EniServoMapper* pServo = nullptr;
    void bind_servo(const EniServoMapper& servo);
    bool bound() const { return pServo != nullptr; }
    int apply_input(const void* inp);
    int apply_output(void* oup);
    int servo_size()const;
    int get_slave_num()const;
    int get_servo_slave_idx(int servo_idx)const;
    int get_servo_offset(int servo_idx)const;
    bool contain_key(int servo_idx, int idx)const;
};


// int pi_get_input(proc_img_i_addr& _pi_i, const std::vector<pi_offset_config>& config, const void *input);
// int pi_get_output(proc_img_o_addr& _pi_o, const std::vector<pi_offset_config>& config, void *output);

/**
 * parse servo pdo input from input buffer 
 * 
 * \param[out]  _pi_i       servo field infomation
 * \param[in]   config      some servo field offset infomation
 * \param[in]   input       input buffer
 */
int pdo_get_input(proc_img_i_addr& _pi_i, const EniServoMapper& config, const void *input);

/**
 * dump servo pdo output to output buffer 
 * 
 * \param[inout]  _pi_o       servo field information to dump
 * \param[in]   config      some servo field offset infomation
 * \param[out]   output      output buffer
 */
int pdo_get_output(proc_img_o_addr& _pi_o, const EniServoMapper& config, void *output);

}
#endif