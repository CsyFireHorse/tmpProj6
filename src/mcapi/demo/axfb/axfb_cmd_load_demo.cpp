#include "axfb_cmd_api.h"
#include <stdio.h>
#include "common/get_file_path.hpp"

#define LEN(x) sizeof(x)/sizeof(x[0])

void axfb_cmd_load_demo(const char* fn) {
    axfb_cmd_manager_t mgr = axfb_cmd_manager_create();
    AxFbCmdConfig* cfg = axfb_cmd_config_create();
    int ret = load_axfb_cmd_config_from_json(cfg, fn);
    if (ret) {
        printf("load failed\n");
        return;
    }
    axfb_set_config(mgr, cfg);
    axfb_run(mgr);
    axfb_cmd_config_destroy(cfg);
    axfb_cmd_manager_destroy(mgr);
}

void axfb_cmd_load_default_demo() {     
    auto eni_file = zmc::_get_file_path("fb/demo/axfb/motbuf_cmds.json");
    axfb_cmd_load_demo(eni_file.c_str()); 
}
