#include "stdio.h"
#include "string.h"
#include "common/get_file_path.hpp"
#include "axfb/pou_memory_demo.hpp"

// void fbUnionDat_demo();
// void FbServoSgFaker_demo();
// void FbAxisCtrler_demo();
// void FbServoQuickStop_demo();
// void FbToyMove_demo();
// void velToyTrapz_demo(){};
// void axisGrp_toy_demo();
// void GearToy_demo();
// void ProfilePlanner_demo();
// void ScurvePlannerOffLine_demo();

void fbt_eager_demo();
void fbt_movev_demo();
void fbt_follow_demo();
void fbt_move_linear_zmot_demo();
void fbt_axisPlg_demo();
void fbt_axisPlg_move_demo();

void ax_movev_demo();
int ax_gear_demo(int argc, char** argv);
int ax_cam_demo(int argc, char** argv);
int ax_cam_demo2(int argc, char** argv);
int pou_demo(int argc, char** argv);
int pou_homing_demo(int argc, char** argv);
void morphPou_mini_demo();
int move_superimposed_demo(int argc, char** argv);
int pou_dual_limit_homing_demo(int argc, char** argv);
int pou_jog_demo(int argc, char** argv);
int pou_inch_demo(int argc, char** argv);
void pou_split_demo();
void pou_power_demo();
int pou_auto_demo(int argc, char** argv);
int pou_movev_demo(int argc, char** argv);
int pou_move_cont_abs_demo(int argc, char** argv);
int pou_move_cont_rel_demo(int argc, char** argv);
int pou_halt_demo(int argc, char** argv);
int pou_grp_linear_demo(int argc, char** argv);
int blending_demo(int argc, char** argv);
void blending_dynamic_demo();
void cam_tappet_serialize_demo();
int cam_write_read_demo(int argc, char** argv);

void FbAttrVisitorWrite2_demo();
void fbFactory_demo();
void axFbCmdMgr_demo();
void axFbCmdMgr_encoder_demo();
void axFbCmdMgr_rdwr_demo();
void axFbCmdMgr_rdwr_demo2();

void axFbCmdMgr_parse_demo(const char* fn);
void axFbCmdMgr_parse0_demo();
void VariableManager_demo();
void VariableVectManager_demo();

void cam_xyva_max_PosVelAcc_demo();
void cam_xyva_switch_segment_type_demo();
void cam_xyva_calc_pva_demo();
void CamTableFitter_demo();
void CamTableTappet_demo();
void fbt_cam_calc_demo();

int motNode_controlAxisByPos_demo(int argc, char** argv);
int motNode_mov_abs_demo(int argc, char** argv);
int motNode_mov_rel_demo(int argc, char** argv);
int motNode_mov_add_demo(int argc, char** argv);
int motNode_demo(int argc, char** argv);
void motNode_abs_cont_update_demo();
void motNode_add_cont_update_demo();

void motnodeSgrp_abs_blend_demo();
void motnodeSgrp_db_demo();
void motnodeSgrp_vel_abs_demo();
void motnodeSgrp_vel_buf_abs_demo();
void motnodeSgrp_abs_rel_demo();
void motnodeSgrp_abs_buf_rel_demo();
void motnodeSgrp_abs_add_demo();
void motnodeSgrp_vel_stop_demo();
void motnodeSgrp_vel_halt_demo();
void motnodeSgrp_abs_blend_rel_demo();
void motnodeSgrp_abs_blend_rel_demo2();
void motnodeSgrp_blend_3_demo();
void motnodeSgrp_blend_4_demo();

void motnodeSgrp_move_override_demo();
void motNode_move_cont_abs_demo();
void motNode_move_cont_rel_demo();
void motnodeSgrp_movecontabs_abs_demo();
void motnodeSgrp_movecontabs_buf_abs_demo();
void motnodeSgrp_abs_movecontabs_demo();
void motNode_follow_demo();

void lookahead_demo();
void motNode_gearIn_demo();

void fbMoveAbs_loop_demo();

void fbtmotbuf_demo();
void fbtmotbuf_demo2();

int axfb_cmd_simulator_demo(int argc, char* argv[]);
// void encoder_diEnPos2diEnPos_demo();
// void encoder_gear_demo();
int encoder_gear_demo(int argc, char** argv);

int printHelp(int argc, char** argv) {
    printf(
        " --version\n"
        "move\n"
        "movev\n"
        "follow\n"
        "zmot_linear\n"
        "axis_plg\n"
        "axis_plg_move\n"
        "ax_movev\n"
        "axfb_demo\n"
        "axfb_encoder\n"
        "axfb_load <filename>\n"
        "axfb_load\n"
        "cam_write \n"
        "ax_cam\n"
        "ax_cam2\n"
        "encoder_demo\n"
        "encoder_gear\n"
        "pou\n"
        "pou_homing\n"
        "pou_halt\n"
        "pou_auto\n"
        "pou_super\n"
        "pou_dual_limit_homing\n"
        "pou_morph\n"
        "pou_jog\n"
        "pou_inch\n"
        "pou_movev\n"
        "pou_move_cont_abs\n"
        "pou_move_cont_rel\n"
        "pou_memory [--memory.script \"power.Enable = true; delay 6; move.Execute = true\"] "
        "[--cycles 200]\n"
        "pou_memory_linear_circular [--memory.script \"...\"] [--cycles 200]\n"
        "pou_grp_linear\n"
        "pou_blending\n"
        "pou_blending_dynamic\n"
        "all\n"
        " sub\n"
        " node\n"
        " demo <demo_name>\n"
        " --help\n");
    return 0;
}

int printVersion() {
    printf("version 0.0.1\n");
    return 0;
}

int sub_main(int argc, char** argv);
int node_main(int argc, char** argv);
int demo_main(int argc, char** argv);
int _main(int argc, char** argv);

#define LEN(x) sizeof(x) / sizeof(x[0])
int main(int argc, char** argv) {
    if (argc == 1) {
#if 0
        char* argv2[] = {"", "node", "abs", "--velocity", "400", "--acceleration", "999", "--deceleration", "999"};
        //   --velocity 8000 --acceleration 999 --distance 999
        return _main(LEN(argv2), argv2);
#endif
    }
    return _main(argc, argv);
}

int _main(int argc, char** argv) {
    if (argc == 1) {
        return printHelp(argc, argv);
    }
    if (argc >= 2 && strcmp(argv[1], "--help") == 0) {
        return printHelp(argc, argv);
    } else if (argc >= 2 && strcmp(argv[1], "--version") == 0) {
        return printVersion();
    } else if (argc >= 2 && strcmp(argv[1], "move") == 0) {
        fbt_eager_demo();
        return 0;
    } else if (argc >= 2 && strcmp(argv[1], "movev") == 0) {
        fbt_movev_demo();
        return 0;
    } else if (argc >= 2 && strcmp(argv[1], "follow") == 0) {
        fbt_follow_demo();
        return 0;
    } else if (argc >= 2 && strcmp(argv[1], "zmot_linear") == 0) {
        fbt_move_linear_zmot_demo();
        return 0;
    } else if (argc >= 2 && strcmp(argv[1], "axis_plg") == 0) {
        fbt_axisPlg_demo();
        return 0;
    } else if (argc >= 2 && strcmp(argv[1], "axis_plg_move") == 0) {
        fbt_axisPlg_move_demo();
        return 0;
    } else if (argc >= 2 && strcmp(argv[1], "ax_movev") == 0) {
        ax_movev_demo();
        return 0;
    } else if (argc >= 2 && strcmp(argv[1], "pou") == 0) {
        return pou_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "pou_blending") == 0) {
        return blending_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "pou_blending_dynamic") == 0) {
        blending_dynamic_demo();
    } else if (argc >= 2 && strcmp(argv[1], "pou_auto") == 0) {
        return pou_auto_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "pou_movev") == 0) {
        return pou_movev_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "pou_move_cont_abs") == 0) {
        return pou_move_cont_abs_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "pou_move_cont_rel") == 0) {
        return pou_move_cont_rel_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "pou_memory") == 0) {
        return pou_memory_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "pou_memory_linear_circular") == 0) {
        return pou_memory_linear_circular_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "pou_grp_linear") == 0) {
        return pou_grp_linear_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "pou_halt") == 0) {
        return pou_halt_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "pou_homing") == 0) {
        return pou_homing_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "pou_homing_dual") == 0) {
        return pou_dual_limit_homing_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "morph_pou") == 0) {
        morphPou_mini_demo();
        return 0;
    } else if (argc >= 2 && strcmp(argv[1], "super_pou") == 0) {
        return move_superimposed_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "pou_jog") == 0) {
        return pou_jog_demo(argc - 1, argv + 1);
        return 0;
    } else if (argc >= 2 && strcmp(argv[1], "pou_inch") == 0) {
        return pou_inch_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "pou_split") == 0) {
        pou_split_demo();
        return 0;
    } else if (argc >= 2 && strcmp(argv[1], "pou_power") == 0) {
        pou_power_demo();
        return 0;
    } else if (argc >= 2 && strcmp(argv[1], "control_axis_by_pos") == 0) {
        return motNode_controlAxisByPos_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "ax_gear") == 0) {
        return ax_gear_demo(argc - 2, argv + 2);
    } else if (argc >= 2 && strcmp(argv[1], "cam_write") == 0) {
        return cam_write_read_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "ax_cam") == 0) {
        return ax_cam_demo(argc - 2, argv + 2);
    } else if (argc >= 2 && strcmp(argv[1], "ax_cam2") == 0) {
        return ax_cam_demo2(argc - 2, argv + 2);
    } else if (argc >= 2 && strcmp(argv[1], "encoder_gear") == 0) {
        return encoder_gear_demo(argc - 2, argv + 2);
    } else if (argc >= 2 && strcmp(argv[1], "axfb_demo") == 0) {
        axFbCmdMgr_demo();
    } else if (argc >= 2 && strcmp(argv[1], "axfb_encoder") == 0) {
        axFbCmdMgr_encoder_demo();
    } else if (argc >= 2 && strcmp(argv[1], "axfb_rdwr") == 0) {
        axFbCmdMgr_rdwr_demo();
    } else if (argc >= 2 && strcmp(argv[1], "axfb_rdwr2") == 0) {
        axFbCmdMgr_rdwr_demo2();
    } else if (argc >= 3 && strcmp(argv[1], "axfb_load") == 0) {
        axFbCmdMgr_parse_demo(argv[2]);
    } else if (argc >= 2 && strcmp(argv[1], "axfb_load") == 0) {
        axFbCmdMgr_parse0_demo();
    } else if (argc >= 2 && strcmp(argv[1], "axfb_simu") == 0) {
        axfb_cmd_simulator_demo(argc - 1, argv + 1);
        // }else if(argc >= 2 && strcmp(argv[1], "encoder_demo")==0){
        //     encoder_diEnPos2diEnPos_demo();
    } else if (argc >= 2 && strcmp(argv[1], "all") == 0) {
        fbt_eager_demo();
        fbt_movev_demo();
        fbt_follow_demo();
        fbt_axisPlg_demo();
        fbt_axisPlg_move_demo();
        return 0;
    } else if (argc >= 2 && strcmp(argv[1], "sub") == 0) {
        sub_main(argc - 1, argv + 1);
        return 0;
    } else if (argc >= 2 && strcmp(argv[1], "node") == 0) {
        node_main(argc - 1, argv + 1);
        return 0;
    } else if (argc >= 2 && strcmp(argv[1], "demo") == 0) {
        return demo_main(argc - 1, argv + 1);
    } else {
        return printHelp(argc, argv);
    }
    return 0;
}

int node_main(int argc, char** argv) {
    if (argc >= 2 && strcmp(argv[1], "dynamic") == 0) {
        motNode_abs_cont_update_demo();
    } else if (argc >= 2 && strcmp(argv[1], "add_update") == 0) {
        motNode_add_cont_update_demo();
    } else if (argc >= 2 && strcmp(argv[1], "rel") == 0) {
        return motNode_mov_rel_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "abs") == 0) {
        return motNode_mov_abs_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "add") == 0) {
        return motNode_mov_add_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "discrete") == 0) {
        return motNode_demo(argc - 1, argv + 1);
    } else if (argc >= 2 && strcmp(argv[1], "cont_abs") == 0) {
        motNode_move_cont_abs_demo();
    } else if (argc >= 2 && strcmp(argv[1], "cont_rel") == 0) {
        motNode_move_cont_rel_demo();
    } else if (argc >= 2 && strcmp(argv[1], "sgrp") == 0) {
        motnodeSgrp_db_demo();
    } else if (argc >= 2 && strcmp(argv[1], "abs_add") == 0) {
        motnodeSgrp_abs_add_demo();
    } else if (argc >= 2 && strcmp(argv[1], "abs_rel") == 0) {
        motnodeSgrp_abs_rel_demo();
    } else if (argc >= 2 && strcmp(argv[1], "abs_buf_rel") == 0) {
        motnodeSgrp_abs_buf_rel_demo();
    } else if (argc >= 2 && strcmp(argv[1], "vel_abs") == 0) {
        motnodeSgrp_vel_abs_demo();
    } else if (argc >= 2 && strcmp(argv[1], "vel_buf_abs") == 0) {
        motnodeSgrp_vel_buf_abs_demo();
    } else if (argc >= 2 && strcmp(argv[1], "movecontabs_abs") == 0) {
        motnodeSgrp_movecontabs_abs_demo();
    } else if (argc >= 2 && strcmp(argv[1], "abs_movecontabs") == 0) {
        motnodeSgrp_abs_movecontabs_demo();
    } else if (argc >= 2 && strcmp(argv[1], "movecontabs_buf_abs") == 0) {
        motnodeSgrp_movecontabs_buf_abs_demo();
    } else if (argc >= 2 && strcmp(argv[1], "abs_blend_rel") == 0) {
        motnodeSgrp_abs_blend_rel_demo();
    } else if (argc >= 2 && strcmp(argv[1], "abs_blend_rel2") == 0) {
        motnodeSgrp_abs_blend_rel_demo2();
    } else if (argc >= 2 && strcmp(argv[1], "blend_3") == 0) {
        motnodeSgrp_blend_3_demo();
    } else if (argc >= 2 && strcmp(argv[1], "blend_4") == 0) {
        motnodeSgrp_blend_4_demo();
    } else if (argc >= 2 && strcmp(argv[1], "vel_stop") == 0) {
        motnodeSgrp_vel_stop_demo();
    } else if (argc >= 2 && strcmp(argv[1], "vel_halt") == 0) {
        motnodeSgrp_vel_halt_demo();
    } else if (argc >= 2 && strcmp(argv[1], "move_loop") == 0) {
        fbMoveAbs_loop_demo();
    } else if (argc >= 2 && strcmp(argv[1], "follow") == 0) {
        motNode_follow_demo();
    } else if (argc >= 2 && strcmp(argv[1], "gear") == 0) {
        motNode_gearIn_demo();
    } else if (argc >= 2 && strcmp(argv[1], "cam_tappet") == 0) {
        CamTableTappet_demo();
    } else {
        printf(
            " node help\n"
            "dynamic\n"
            "abs\n"
            "rel\n"
            "sgrp\n"
            "abs_add\n"
            "abs_rel\n"
            "abs_buf_rel\n"
            "vel_abs\n"
            "vel_buf_abs\n"
            "abs_blend_rel\n"
            "abs_blend_rel2\n"
            "move_loop\n"
            "cam_tappet\n"
            "cont_abs\n"
            "cont_rel\n"
            "");
    }
    return 0;
}

int sub_main(int argc, char** argv) {
    if (argc >= 2 && strcmp(argv[1], "cam_tappet_serialize") == 0) {
        cam_tappet_serialize_demo();
    } else if (argc >= 2 && strcmp(argv[1], "cam_xyva_max") == 0) {
        cam_xyva_max_PosVelAcc_demo();
    } else if (argc >= 2 && strcmp(argv[1], "cam_xyva_switch") == 0) {
        cam_xyva_switch_segment_type_demo();
    } else if (argc >= 2 && strcmp(argv[1], "cam_xyva_calc") == 0) {
        cam_xyva_calc_pva_demo();
    } else if (argc >= 2 && strcmp(argv[1], "fbt_cam_calc") == 0) {
        fbt_cam_calc_demo();
    } else {
        printf(
            " help\n"
            "fifthpoly\n"
            "wrap\n"
            "wrap_xyva\n"
            "vel_follower\n"
            "vel_follower_low_acc\n"
            "vel_sin\n"
            "fbt_cam_calc\n"
            "");
    }

    // }else if(argc >= 2 && strcmp(argv[1], "fbt_servo")==0){
    //     FbServoSgFaker_demo();
    // }else if(argc >= 2 && strcmp(argv[1], "fbt_axis")==0){
    //     FbAxisCtrler_demo();
    // }else if(argc >= 2 && strcmp(argv[1], "fbt_move")==0){
    //     FbToyMove_demo();
    // }else if(argc >= 2 && strcmp(argv[1], "fbt_quickstop")==0){
    //     FbServoQuickStop_demo();
    // }else if(argc >= 2 && strcmp(argv[1], "axis_group")==0){
    //     axisGrp_toy_demo();
    // }else if(argc >= 2 && strcmp(argv[1], "gear")==0){
    //     GearToy_demo();
    // }else if(argc >= 2 && strcmp(argv[1], "planner")==0){
    //     ProfilePlanner_demo();
    // }else if(argc >= 2 && strcmp(argv[1], "scurve_planner")==0){
    //     ScurvePlannerOffLine_demo();
    // }else{
    //     return printHelp(argc, argv);
    // }
    return 0;
}

int demo_main(int argc, char** argv){
    if(argc < 2){
        printf("demo help:\n"
          "demo sw_limit\n"
          "demo <demo_name>\n"
          "demo move_cont_abs\n"
          "");
        return 0;
    }
    auto eni_file = zmc::_get_file_path("fb/demo/proj/zmc_motapp_demo_log.ini");
    if(strcmp(argv[1], "sw_limit")==0){
        char* demo_argv[] = {"", "pou", "--ini_path", (char*) eni_file.c_str(), "--cycles", "100000", "--move.Velocity", "1"};
        return _main(LEN(demo_argv), demo_argv);
    }else if(strcmp(argv[1], "move_cont_abs")==0){
        char* demo_argv[] = {"", "pou_move_cont_abs", "--ini_path", (char*) eni_file.c_str(), "--cycles", "1000", "--move.Jerk", "50", "--move.Position", "20"};
        return _main(LEN(demo_argv), demo_argv);
    }else{
        printf("Unknown demo: %s\n", argv[1]);
        printf("Available demos:\n"
          "  sw_limit\n"
          "");
        return 1;
    }
}
