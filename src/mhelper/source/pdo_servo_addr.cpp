#include "eni_servo_parser.h"
#include "pdo_servo_addr.h"

namespace zmc{

void pdo_addr::bind_servo(const EniServoMapper& servo){
    pServo = (EniServoMapper*) &servo;
    txpdo_addr.len = servo.slvInfo.get_tx_end_addr();
    rxpdo_addr.len = servo.slvInfo.get_rx_end_addr();
    txpdo_addr.axLen = servo.servo_pdo.size();
    rxpdo_addr.axLen = servo.servo_pdo.size();

    apply_input(nullptr);
    apply_output(nullptr);
}

int pdo_addr::apply_input(const void* inp) {
    if (!pServo) {
        return -1;
    }
    return pdo_get_input(txpdo_addr, *pServo, inp);
}

int pdo_addr::apply_output(void* oup) {
    if (!pServo) {
        return -1;
    }
    return pdo_get_output(rxpdo_addr, *pServo, oup);
}

bool pdo_addr::contain_key(int servo_idx, int idx)const{
    return pServo->contain_key(servo_idx, idx);
}
int pdo_addr::get_servo_slave_idx(int servo_idx)const{
    return pServo->get_servo_slave_idx(servo_idx);
}
int pdo_addr::get_servo_offset(int servo_idx)const{
    return pServo->get_servo_offset(servo_idx);
}
int pdo_addr::get_slave_num()const{
    return pServo->size();
}

int pi_get_output(proc_img_o_addr& _pi_o, const std::vector<pi_offset_config> &config, void *output)
{
    if(_pi_o.pBuf == output && _pi_o.axLen == config.size()) return 0; 
    _pi_o.pBuf = (uint32_t *)output ;
    auto sz = config.size();
    _pi_o.axLen = sz;
    for (uint32_t i = 0; i < sz; i++)
    {
        servo_ctrl_addr *ctrl = &_pi_o.servos[i];
        const pi_offset_config *curr = &config.at(i);

        ctrl->slave_idx = curr->slave_idx;
        ctrl->slave_channel_idx = curr->slave_channel_idx;
        ctrl->mode = (uint8_t *)((curr->po_mode_offset != -1) ? (char *)output + curr->po_mode_offset : 0);
        ctrl->ctrl = (servo_ctrl_word *)((curr->po_ctrl_word_offset != -1) ? (char *)output + curr->po_ctrl_word_offset : 0);

        ctrl->p = (I32 *)((curr->po_p_offset != -1) ? (char *)output + curr->po_p_offset : 0);
        ctrl->v = (I32 *)((curr->po_v_offset != -1) ? (char *)output + curr->po_v_offset : 0);        
        ctrl->t = (I16 *)((curr->po_t_offset != -1) ? (char *)output + curr->po_t_offset : 0);    
    }
    return 0;
}

int pi_get_input(proc_img_i_addr& _pi_i, const std::vector<pi_offset_config> &config, const void *output)
{
    auto sz = config.size();
    _pi_i.pBuf = (void*) output;
    _pi_i.axLen = sz;
    for (uint32_t i = 0; i < sz; i++)
    {
        servo_stat_addr *stat = &_pi_i.servos[i];
        const pi_offset_config *curr = &config.at(i);
        stat->slave_idx = curr->slave_idx;
        stat->slave_channel_idx = curr->slave_channel_idx;
        stat->mode = (uint8_t *)((curr->pi_mode_offset != -1) ? (char *)output + curr->pi_mode_offset : 0);
        stat->stat = (servo_stat_word *)((curr->pi_stat_offset != -1) ? (char *)output + curr->pi_stat_offset : 0);
        stat->error_code = (U16 *)((curr->pi_error_offset != -1) ? (char *)output + curr->pi_error_offset : 0); 

        stat->p = (I32 *)((curr->pi_p_offset != -1) ? (char *)output + curr->pi_p_offset : 0);
        stat->v = (I32 *)((curr->pi_v_offset != -1) ? (char *)output + curr->pi_v_offset : 0);
        stat->t = (I16 *)((curr->pi_t_offset != -1) ? (char *)output + curr->pi_t_offset : 0);      

    }
    return 0;
}

int pdo_get_input(proc_img_i_addr& _pi_i, const EniServoMapper& config, const void *input){
    return pi_get_input(_pi_i, config.servo_pdo, input);
}
int pdo_get_output(proc_img_o_addr& _pi_o, const EniServoMapper& config, void *output){
    return pi_get_output(_pi_o, config.servo_pdo, output);
}
}
