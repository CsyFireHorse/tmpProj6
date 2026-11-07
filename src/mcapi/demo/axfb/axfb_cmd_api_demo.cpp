#include "axfb_cmd_api.h"
#include <stdio.h>

#define LEN(x) sizeof(x)/sizeof(x[0])

void axfb_cmd_api_demo() {

    axfb_cmd_manager_t mgr = axfb_cmd_manager_create();
    

    AxFbRunConfig rCfg = {0.01, 100, true, 0};
    axfb_set_run_config(mgr, &rCfg);
    axfb_create_axis_array(mgr, 1, AX_FACTORY_VIRTUAL); 
    axfb_create_fb(mgr, FB_TYPE_POWER);                 
    axfb_create_fb(mgr, FB_TYPE_MOVEABSOLUTE);          


    CommandStruct4 cmds4[] = {
        {1, FB_ATTR_AUTORUN, 0, 1},
        {2, FB_ATTR_AUTORUN, 0, 1},
        {1, FB_ATTR_AXIS, 0, 1},
        {1, FB_ATTR_ENABLE, 0, true},
        {1, FB_ATTR_BREGULATORON, 0, true},
        {1, FB_ATTR_BDRIVESTART, 0, true}, 
        {2, FB_ATTR_AXIS, 0, 1},
        {2, FB_ATTR_POSITION, 0, 15},
        {2, FB_ATTR_VELOCITY, 0, 20},
        {2, FB_ATTR_ACCELERATION, 0,  200},
        {2, FB_ATTR_DECELERATION, 0, 200},
        {2, FB_ATTR_EXECUTE, 6, true},
    };
    CommandStruct* cmds = (CommandStruct*) cmds4;

    for (int i = 0; i < LEN(cmds); i++) {
        axfb_push_command(mgr, &cmds[i]);
    }


    axfb_run(mgr);


    axfb_cmd_manager_destroy(mgr);
}


void axfb_cmd_api_encoder_demo() {
    axfb_cmd_manager_t mgr = axfb_cmd_manager_create();
    

    axfb_create_axis_array(mgr, 1, AX_FACTORY_ENCODER);
    axfb_create_fb(mgr, FB_TYPE_POWER);


    CommandStruct4 base_cmds4[] = {
        {1, FB_ATTR_AUTORUN, 0, 1},
        {1, FB_ATTR_AXIS, 0, 1},
        {1, FB_ATTR_ENABLE, 0, true},
        {1, FB_ATTR_BREGULATORON, 0, true},
        {1, FB_ATTR_BDRIVESTART, 0, true}
    };
    CommandStruct* base_cmds = (CommandStruct*) base_cmds4;
    for (int i = 0; i < LEN(base_cmds); i++) {
        
        axfb_push_command(mgr, &base_cmds[i]);
    }


    for (int i = 0; i < 100; i++) {
        if (i >= 5) {
            CommandStruct pos_cmd = {1, (uint8_t) AX_ATTR_DIENCODERPOSITION, AX_ATTR_SET, i, (i-5)*2.45};
            axfb_push_command(mgr, &pos_cmd);
        }
        axfb_run_cycle(mgr);
    }

    axfb_cmd_manager_destroy(mgr);
}

