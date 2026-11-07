#include "stdio.h"
#include "string.h"
#include <iostream>
#include <string>
#include "assert.h"
#include "pdo_protal3.h"
#include "eni_servo_parser.h"
#include "pdo_servo_addr.h"

using namespace zmc;
static std::string _get_file_path(const char *fname){
    std::string pth(__FILE__);
    return pth.substr(0, pth.size() - strlen("eni_servo_map_demo.cpp")) + fname;
}

void _eni_servo_map_demo(const char* fn);
void eni_servo_map_demo(){
    const char* fn= "../data/OL3_fak.xml";
    _eni_servo_map_demo(fn);
}
void eni_servo_map_demo0(){
    const char* fn= "../data/pse_servo3.xml";
    _eni_servo_map_demo(fn);
}

void _eni_servo_map_demo(const char* fn){
    EniServoMapper svMap;
    int errCode = load_pdo_offset_from_file(_get_file_path(fn).c_str(),  svMap);
    assert(errCode == 0);

    proc_img_i_addr _pi_i;
    proc_img_o_addr _pi_o;

    PdoTxProtal3 tx;
    PdoRxProtal3 rx;
    pdo_get_input(_pi_i, svMap, (const void *)&tx);
    for (int i=0; i<3; i++){
        assert(*_pi_i.servos[i].p == *tx.get_pos_ptr(i));
    }
    pdo_get_output(_pi_o, svMap, (void *)&rx);
    for (int i=0; i<3; i++){
        assert(*_pi_o.servos[i].p == *rx.get_pos_ptr(i));
    }
}

