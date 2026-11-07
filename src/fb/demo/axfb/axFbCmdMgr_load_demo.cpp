// #include "FbAttrVisitor.h"
#include "AxFbCmdManager.h"
#include "AxisCiaConfig.hpp"
#include "stdio.h"
#include "AxFbCmdParser.hpp"
#include "str2axFbAttr.hpp"
#include "common/get_file_path.hpp"

using namespace zmc;

void axFbCmdMgr_parse_demo(const char* fn) {
    AxFbCmdConfig cfg;
    int ret = load_AxFbCmdConfig_from_json(cfg, fn);
    if (ret) {
        printf("load failed\n");
        return;
    }
    AxFbCmdManager axFbCmdMgr;
    axFbCmdMgr.set_config(cfg);
    axFbCmdMgr.run();
}

void axFbCmdMgr_parse0_demo() {     
    auto eni_file = zmc::_get_file_path("fb/demo/axfb/motbuf_cmds.json");
    axFbCmdMgr_parse_demo(eni_file.c_str()); 
}
