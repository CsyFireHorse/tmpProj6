#include "stdio.h"
#include "string.h"
#include "eni_iomap_parser.h"
#include <iostream>
#include <string>
#include "assert.h"
#include "pdo_bfdoao.h"
#include "pdo_bfdoaoai.h"

using namespace zmc;

static std::string _get_file_path(const char *fname){
    std::string pth(__FILE__);
    return pth.substr(0, pth.size() - strlen("eni_io_map_demo.cpp")) + fname;
}

void eni_parse_map_demo3(){
    printf("sz = %d,%d\n", sizeof(PdoRxBfdoao), sizeof(PdoTxBfdoao));

    const char* fn= "../data/pse_doao_map.xml";
    EniIomapper eni_info;
    int errCode = load_pdo_iomap_from_file(_get_file_path(fn).c_str(), eni_info);
    assert(errCode == 0);
    assert(eni_info.master_pdo.cycle_time == 1000);
}

void eni_parse_map_demo2(){
    const char* fn= "../data/notfound.xml";
    EniIomapper eni_info;
    int errCode = load_pdo_iomap_from_file(_get_file_path(fn).c_str(), eni_info);
    assert(errCode);
}

void eni_parse_map_demo4(){
    printf("sz = %d,%d\n", sizeof(PdoRxBfdoao), sizeof(PdoTxBfdoao));

    const char* fn= "../data/pse_doao_map.xml";
    EniIomapper eni_info;
    int errCode = load_pdo_iomap_from_file(_get_file_path(fn).c_str(), eni_info);
    assert(errCode == 0);
    assert(eni_info.master_pdo.cycle_time == 1000);
    assert(eni_info.master_pdo.do_end == 4);
    PdoTxBfdoao tx;
    PdoRxBfdoao rx;
    PdoIoMapper iomap(&eni_info);
    iomap.bind_in_out((uint8_t*)&rx, (uint8_t*)&tx);
    memset(&tx, 0, sizeof(PdoTxBfdoao));
    const uint8_t do_arr[4] = {1, 0, 1, 0};
    for (int i=0; i<4; i++){
        iomap.set_do(i, do_arr[i]);
        assert(do_arr[i] == tx.get_do(i));
    }
    const uint16_t ao_arr[4] = {11, 22, 333, 444};
    for (int i=0; i<4; i++){
        iomap.set_ao(i, ao_arr[i]);
        assert(ao_arr[i] == *tx.get_ao_ptr(i));
    }
    printf("ok\n");
    // assert(0);
}

#define LEN(x) sizeof(x)/sizeof(*(x))
void eni_parse_map_demo(){
    printf("sz = %d,%d\n", sizeof(PdoRxBfdoaoai), sizeof(PdoTxBfdoaoai));

    const char* fn= "../data/pse_doaoai_map.xml";
    EniIomapper eni_info;
    int errCode = load_pdo_iomap_from_file(_get_file_path(fn).c_str(), eni_info);
    assert(errCode == 0);
    assert(eni_info.master_pdo.cycle_time == 1000);
    assert(eni_info.master_pdo.do_end == 4);
    assert(eni_info.master_pdo.ao_end == 12);
    PdoTxBfdoaoai tx;
    PdoRxBfdoaoai rx;
    PdoIoMapper iomap(&eni_info);
    iomap.bind_in_out((uint8_t*)&rx, (uint8_t*)&tx);
    memset(&tx, 0, sizeof(PdoTxBfdoaoai));
    const uint8_t do_arr[4] = {1, 0, 1, 0};
    for (int i=0; i<4; i++){
        iomap.set_do(i, do_arr[i]);
        assert(do_arr[i] == tx.get_do(i));
    }
    const uint16_t ao_arr[4] = {11, 22, 333, 444};
    for (int i=0; i<4; i++){
        iomap.set_ao(i, ao_arr[i]);
        assert(ao_arr[i] == *tx.get_ao_ptr(i));
    }
    const uint8_t di_arr[8] = {1, 0, 1, 0, 1, 1, 0, 0};
    for (int i=0; i<LEN(di_arr); i++){
        uint8_t v;
        rx.set_di(i, di_arr[i]);
        iomap.get_di(i, &v);
        assert(di_arr[i] == v);
    }
    const uint16_t ai_arr[8] = {11, 22, 333, 444, 555, 666, 777, 888};
    for (int i=0; i<LEN(ai_arr); i++){
        uint16_t v;
        rx.set_ai(i, ai_arr[i]);
        iomap.get_ai(i, &v);
        assert(ai_arr[i] == v);
    }
    printf("ok\n");
    // assert(0);
}