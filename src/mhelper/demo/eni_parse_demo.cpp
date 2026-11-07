#include "stdio.h"
#include "string.h"
#include "ecat_json.h"
#include <iostream>
#include <string>
#include "assert.h"

using namespace zmc;
static std::string _get_file_path(const char *fname){
    std::string pth(__FILE__);
    return pth.substr(0, pth.size() - strlen("eni_parse_demo.cpp")) + fname;
}

void eni_json_parse_demo(){
    const char* fn= "../data/pse_servo3.xml";
    // eni_parse_wrap eni_info;
    // int load_eni_file(, eni_info);
    MasterInfo mast;
    SlaveInfo slaves;
    int ret = load_slaves_from_eni_xml_json(_get_file_path(fn).c_str(), slaves, &mast);;

    assert(ret == 0);
    assert(mast.cycle_time == 1000);
}

