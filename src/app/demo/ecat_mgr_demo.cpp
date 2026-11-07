#include "stdio.h"
#include <string>
#include "string.h"
#include "ecat_mgr.h"
// #include "../source/ecat_pdoio.h"
#include "common/get_file_path.hpp"
using namespace zmc;

int ecat_mgr_demo(const char *fname){
    auto eni_file = _get_file_path("mhelper/data/pse_servo3.xml");
    if (fname) eni_file = fname;
    int st = EcatManagerSg::init_ecat("zecm0");
    printf("init %d\n", st);
    EcatManagerSg::get_ecat("zecm0")->set_eni(eni_file.c_str());
    EcatManagerSg::get_ecat("zecm0")->use_default_cfg();
    EcatManagerSg::get_ecat("zecm0")->start();
    printf("Press e key to exit\n");
    while (getchar() != 'e');
    EcatManagerSg::get_ecat("zecm0")->stop();
    return 0;
}

static int __pdo_print_count(void*, uint8_t*,  uint8_t*){
    static int cnt = 0;
    if (cnt++ % 1024 == 1){
        printf("cnt=%d\n", cnt);
    }
    return 0;
}

int ecat_mgr_demo2(const char *fname){
    auto eni_file = _get_file_path("mhelper/data/pse_servo3.xml");
    if (fname) eni_file = fname;
    int st = EcatManagerSg::init_ecat("zecm0");
    printf("init %d\n", st);
    if (st) return -1;
    int idx = EcatManagerSg::find_netname("zecm0");
    printf("idx=%d, ptr=%d\n", idx, EcatManagerSg::get_ecat(idx));
    EcatManagerSg::get_ecat("zecm0")->set_eni(eni_file.c_str());
    EcatManagerSg::get_ecat("zecm0")->use_default_cfg();
    bool is_running = false;
    while(1){
        printf("Press r key to run\n"
            "Press c key to cancel run\n"
            "Press w key to switch eni\n"
            "Press e key to exit\n");
        if (is_running)
            printf("Press b key to bind null\n"
                "Press p key to bind printer\n");
        auto key = getchar();
        switch(key){
            case 'r':
                st = EcatManagerSg::start(idx);
                if (st)
                    printf("start ret = %d\n", st);
                else
                    is_running = true;
                break;
            case 'c':
                EcatManagerSg::stop(idx);
                is_running = false;
                break;
            case 'w':
                EcatManagerSg::get_ecat(idx)->set_eni(eni_file.c_str());
                break;
            case 'b':
                EcatManagerSg::get_ecat(idx)->unbind();
                break;
            case 'p':
                EcatManagerSg::get_ecat(idx)->bind(__pdo_print_count, EcatManagerSg::get_ecat(idx));
                break;
            case 'e':
                EcatManagerSg::stop(idx);
                return 0;
            default:
                break;
        }
    }
    return 0;
}

// int ecat_mgr_rtbus_demo(const char *fname){
//     auto eni_file = _get_file_path("mhelper/data/pse_doao_map.xml");
//     if (fname) eni_file = fname;
//     int st = EcatManagerSg::init_ecat("zecm0", eni_file.c_str());
//     printf("init %d\n", st);
//     EcatManagerSg::Instance().ecat_start_pdo("zecm0");
//     printf("Press e key to exit\n");
//     while (getchar() != 'e');
//     EcatManagerSg::get_ecat("zecm0")->stop_ecat();
//     return 0;
// }