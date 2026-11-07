#include <string>
#include <vector>

#include "EcatWrap.h"
#include "ecat_mgr.h"
#include "stdio.h"
#include "string.h"

// #include "AxisCia402.h"
// #include "FbAll.h"
// #include "Axis.h"
// #include "common/ecat/Cia402Easy.hpp"
// #include "base/misc_utils.h"
#include "AxisFbGraph.hpp"
#include "FbWrapCmd.h"
#include "common/ecat/ecat_pdo_key.h"
#include "common/get_file_path.hpp"
#include "mhelper/zspdlog_init.h"

// #include "common/ecat/ServoSgFaker.hpp"

using namespace zmc;

static void fbMoveTrigger(FbMoveWrap& fbGrp) {
    int cnt = fbGrp.cnt;
    if (cnt == 1001) {
        // fbGrp.sendReset();
    }
    if (cnt == 3001) {
        fbGrp.sendExecute(1);
    }
    if (cnt == 5001) {
        fbGrp.setPowerOn();
    }
    if (cnt == 6001) {
        // fbGrp.sendExecute(1);
    }
}

int __pdo_power(void* p, const void* inp, void* oup) {
    AxisFbGraph& fbAxGrp = *(AxisFbGraph*)p;
    FbMoveWrap& fbGrp = fbAxGrp.fbGrp;
    const proc_img_i_addr* txpdo_addr = (const proc_img_i_addr*)inp;
    proc_img_o_addr* rxpdo_addr = (proc_img_o_addr*)oup;

    static bool firstFlag = true;
    if (firstFlag) {
        firstFlag = false;
        printf("firstFlag,%d,%d,%d\n", rxpdo_addr->len, txpdo_addr->len, txpdo_addr->axLen);
    }

    static int cnt = 0;
    if (cnt++ % 1000 == 1) {
        printf("cnt=%d\n", cnt);
    }
    fbMoveTrigger(fbGrp);
    for (int i = 0; i < txpdo_addr->axLen; i++) {
        AxisCiaInput& inp = fbAxGrp.inpArr[i];
        inp.status_word = *(uint16_t*)txpdo_addr->servos[i].stat;
        inp.actual_position = *txpdo_addr->servos[i].p;
        if (txpdo_addr->servos[i].mode)
            inp.mode_of_operation_display = *txpdo_addr->servos[i].mode;
    }
    // fbGrp.runCycle();
    fbAxGrp.run_once();
    char buf[250] = {0};
    for (int i = 0; i < txpdo_addr->axLen; i++) {
        AxisCiaOutput& oup = fbAxGrp.oupArr[i];
        if (rxpdo_addr->servos[i].mode) *rxpdo_addr->servos[i].mode = oup.mode_of_operation;
        *rxpdo_addr->servos[i].p = oup.set_position;
        rxpdo_addr->servos[i].ctrl->v = oup.control_word;
        // REAL vel = ax[i]->getVelCmd();
        // printf("%d,%d,%d,0x%x,%d,", cnt, i, fbAxGrp.axCiaArr[i].axis()->axis_state(),
        // rxpdo_addr->servos[i].ctrl->v, *rxpdo_addr->servos[i].p);
    }
    // printf("\n");
    return 0;
};

// args: [eniPath, args...]
int pdoPower_demo(const std::vector<std::string>& args) {
    zmc_logger_init();
    if (args.size() < 1) {
        printf("Error: ENI file path required\n");
        return -1;
    }
    std::string eni_file = args[0];
    int st = EcatWrapMgrSg::init_ecat("zecm0");
    if (st) {
        printf("init failed %d\n", st);
        return -1;
    } else
        printf("init %d\n", st);
    int fd = EcatWrapMgrSg::find_netname("zecm0");
    ECAT_RESULT ret;

    // ServoSgFakerEager::Instance(0).init_pos(55);

    EcatWrapMgrSg::get_ecat(fd)->set_eni(eni_file.c_str());
    if (EcatWrapMgrSg::get_ecat(fd)->load_eni_json(eni_file.c_str())) {
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
    AxisFbGraph fbAxGrp;
    fbAxGrp.configure("", "", servo_num);
    EcatWrapMgrSg::get_ecat(fd)->config_servo_op_mode(OP_MODE_CSP);
    EcatWrapMgrSg::get_ecat(fd)->bindPdoHandler(__pdo_power, &fbAxGrp);
    // EcatWrapMgrSg::get_ecat(fd)->bind(__pdo_marquee, EcatWrapMgrSg::get_ecat(fd));
    runCmdLoop(fbAxGrp.fbGrp);
    printf("Press e key to exit\n");
    while (getchar() != 'e');
    EcatWrapMgrSg::stop(fd);
    return 0;
}

int _pdoPower_demo_with_eni(const std::vector<std::string>& args, const std::string eniPath) {
    std::vector<std::string> new_args = args;
    new_args.insert(new_args.begin(), eniPath);
    return pdoPower_demo(new_args);
}

int pdoPower_demo_fak(const std::vector<std::string>& args) {
    auto eni_file = _get_file_path("mhelper/data/pse_servo1.xml");
    return _pdoPower_demo_with_eni(args, eni_file);
}
int pdoPower_demo_fak2(const std::vector<std::string>& args) {
    auto eni_file = _get_file_path("mhelper/data/pse_servo2.xml");
    return _pdoPower_demo_with_eni(args, eni_file);
}
int pdoPower_demo_fak3(const std::vector<std::string>& args) {
    auto eni_file = _get_file_path("mhelper/data/pse_servo3.xml");
    return _pdoPower_demo_with_eni(args, eni_file);
}
