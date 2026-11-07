#include "stdio.h"
#include "string.h"

void mc_api_axis_moveabs_demo();
void mc_api_axis_move_demo();

int cam_file_to_csv_demo(int argc, char** argv);
void cam_file_parse_demo();
// void cam_file_to_csv_demo(const char* filename);
void cam_tappet_update_demo() ;
void tappet_show_demo();
void calc_poly_max_PosVelAcc_demo();
void cam_calc_poly5_pva();

void zmc_get_version_major_demo();

void axfb_cmd_api_demo();
void axfb_cmd_api_encoder_demo();
void axfb_cmd_load_demo(const char*);
void axfb_cmd_load_default_demo();
void mc_camIn_demo();

int axfb_pou_runner_demo(int argc, char** argv);

void print_help(){
        printf("mcapi demo\n");
        printf("  cam_file_parse\n");
        printf("  cam_file_parse <filename>\n");
        printf("  axfb_load\n");
        printf("  axfb_load <filename>\n");
}
// copy mcapi.so or dll file to build folder and run this demo
int main(int argc, char** argv){
    if (argc == 1){
        // mc_api_axis_move_demo();
        print_help();
    }else if(argc == 2 && strcmp(argv[1], "--help") == 0){
        print_help();
    }else if(argc == 2 && strcmp(argv[1], "mc_api_axis_move_demo")==0){
        mc_api_axis_move_demo();
    }else if(argc == 2 && strcmp(argv[1], "mc_api_axis_moveabs_demo")==0){
        mc_api_axis_moveabs_demo();
    }else if(argc >= 3 && strcmp(argv[1], "cam_file_parse")==0){
        return cam_file_to_csv_demo(argc-1, argv+1);
    }else if(argc == 2 && strcmp(argv[1], "cam_file_parse")==0){
        cam_file_parse_demo();
    }else if(argc >= 2 && strcmp(argv[1], "cam_calc_poly5")==0){
        cam_calc_poly5_pva();
    }else if (argc >= 2 && strcmp(argv[1], "cam_max_posvelacc")==0){
        calc_poly_max_PosVelAcc_demo();
    }else if (argc >= 2 && strcmp(argv[1], "cam_tappet_update")==0){
        cam_tappet_update_demo();
    }else if (argc >= 2 && strcmp(argv[1], "cam_tappet_show")==0){
        tappet_show_demo();
    }else if (argc >= 2 && strcmp(argv[1], "camIn")==0){
        mc_camIn_demo();
    }else if (argc >= 2 && strcmp(argv[1], "axfb_demo")==0){
        axfb_cmd_api_demo();
    }else if (argc >= 2 && strcmp(argv[1], "axfb_encoder_demo")==0){
        axfb_cmd_api_encoder_demo();
    }else if (argc >= 3 && strcmp(argv[1], "axfb_load")==0){
        axfb_cmd_load_demo(argv[2]);
    }else if (argc >= 2 && strcmp(argv[1], "axfb_load")==0){
        axfb_cmd_load_default_demo();
    }else if(argc >= 2 && strcmp(argv[1], "version")==0){
        zmc_get_version_major_demo();
    }else if (argc >= 2 && strcmp(argv[1], "axfb_pou_runner_demo")==0){
        return axfb_pou_runner_demo(argc-1, argv+1);
    }else{
        print_help();
    }
    // printf("register done\n");
    return 0;
}