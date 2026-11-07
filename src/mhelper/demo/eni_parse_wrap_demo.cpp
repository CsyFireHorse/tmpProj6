#include "stdio.h"
#include "string.h"
// #include "eni_parse_wrap.h"
#include <iostream>
#include <string>
#include "assert.h"

// using namespace zmc;

static std::string _get_file_path(const char *fname){
    std::string pth(__FILE__);
    return pth.substr(0, pth.size() - strlen("eni_parse_wrap_demo.cpp")) + fname;
}

// void eni_parse_demo(){
//     const char* fn= "../data/pse_servo3.xml";
//     eni_parse_wrap eni_info;
//     int ret = load_eni_file(_get_file_path(fn).c_str(), eni_info);
//     assert(ret == 0);
//     assert(eni_info.master_pdo.cycle_time == 1000);
// }

