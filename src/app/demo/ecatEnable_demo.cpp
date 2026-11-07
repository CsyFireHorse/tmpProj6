#include <string>
#include "string.h"
#include "stdio.h"
#include "ecat_mgr.h"
#include "EcatWrap.h"
// #include "common/ecat/Cia402Easy.hpp"
// #include "base/misc_utils.h"
#include <random>
#include "common/get_file_path.hpp"

using namespace zmc;

int pdoEnable_demo(const char *fname){
    return -1;
}

#if 0

struct ServoCtrler{
    SERVOS_t _servo[3];
    servo_stat_word s0[3];
};

static int __pdo_power(void* p, const void* inp, void* oup){
    ServoCtrler& servos = *(ServoCtrler*)p;
    proc_img_i_addr* txpdo_addr = (proc_img_i_addr*) inp;
    proc_img_o_addr* rxpdo_addr = (proc_img_o_addr*) oup;

    static bool firstFlag = true;
    if (firstFlag) {
        firstFlag = false;
        printf("firstFlag,%d,%d,%d\n", rxpdo_addr->len, txpdo_addr->len, txpdo_addr->axLen);
    }
    
    static int cnt = 0;
    if (cnt++ % 1000 == 1){  
        printf("cnt=%d\n", cnt);
    }

    int errCode = 0;
#if 0
    // errCode = !ecat_ready(servos._servo, txpdo_addr->axLen, txpdo_addr, rxpdo_addr);
#elif 1
    for (int i=0; i<txpdo_addr->axLen; i++){
        servo_stat_word& s1 = *txpdo_addr->servos[i].stat;
        servo_ctrl_word& c = *rxpdo_addr->servos[i].ctrl;
        if (!servo_control_enable(servos._servo[i], i, servos.s0[i], s1, c, false)){
            errCode |= 1;
        }
        *rxpdo_addr->servos[i].p = *txpdo_addr->servos[i].p;
        servos.s0[i] = s1;
    }
#else
    for (int i=0; i<txpdo_addr->axLen; i++){
        servo_stat_word& s1 = *txpdo_addr->servos[i].stat;
        servo_ctrl_word& c = *rxpdo_addr->servos[i].ctrl;
        if (!servo_control(servos._servo[i], 0, servos.s0[i], s1, c, false)){
            errCode |= 1;
        }
        servos.s0[i] = s1;
    }
#endif
    if ((std::rand() % 1024 >= 1022) && txpdo_addr->axLen){
        static const char* FAILED_STR [] = {"failed", "success"};
        printf("enable %s 0x%x,0x%x,%f\n", FAILED_STR[!errCode], *txpdo_addr->servos[0].stat, *rxpdo_addr->servos[0].ctrl, *txpdo_addr->servos[0].p);
    }
    if (errCode) return -1;
    

    for (int i=0; i<txpdo_addr->axLen; i++){
        // inp.actual_postion = *txpdo_addr->servos[i].p;
        // *rxpdo_addr->servos[i].p = oup.set_position;
    }
    return 0;
};


int pdoEnable_demo(const char *fname){
    auto eni_file = _get_file_path("mhelper/data/bf_el2809b.xml");
    // pse_doao_map
    if (fname) eni_file = fname;
    int st = EcatWrapMgrSg::init_ecat("zecm0");
    if (st) { printf("init failed %d\n", st); return -1;}
    else printf("init %d\n", st);
    int fd = EcatWrapMgrSg::find_netname("zecm0");
    ECAT_RESULT ret;
    EcatWrapMgrSg::get_ecat(fd)->set_eni(eni_file.c_str());
    if (EcatWrapMgrSg::get_ecat(fd)->load_eni_json(eni_file.c_str())){
        _ERR_("load ENI failed: ret=%s", eni_file.c_str());
        return -1;
    }
    // EcatWrapMgrSg::use_txrxpdo(ecat_idx);

    ret = EcatWrapMgrSg::start(fd);
    if (ret) {
        _ERR_("Ecat Start failed: ret=%d", ret);
        return -1;
    }
    int servo_num = EcatWrapMgrSg::get_ecat(fd)->servo_size();
    ServoCtrler servos;
    EcatWrapMgrSg::get_ecat(fd)->bindPdoHandler(__pdo_power, &servos);
    // EcatWrapMgrSg::get_ecat(fd)->bind(__pdo_marquee, EcatWrapMgrSg::get_ecat(fd));
    printf("Press e key to exit\n");
    while (getchar() != 'e');
    EcatWrapMgrSg::stop(fd);
    return 0;
}
#endif
