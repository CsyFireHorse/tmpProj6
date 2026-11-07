#if 1
#include "stdio.h"
#include "string.h"
#include <iostream>
#include <string>
#include "assert.h"
#include <gtest/gtest.h>
#include "EcatWrap.h"
#include "common/zmc/fbt_errorcode_def.h"
#include "zecm/zecm_enum_def.h"
// #include "common/ecat/ServoSgFaker.hpp"
// #include "common/zal_cpp_common.h"
#include "common/get_file_path.hpp"

#define LEN(x) sizeof(x)/sizeof(*(x))

#include <fstream>
#include <iostream>

inline bool isFileExist(const std::string& filename){
    std::ifstream file(filename);
    return file.good();
}


// ecatFakCountReset function removed - using bitmap allocator now

using namespace zmc;



struct EcatChecker{
    volatile int cnt;
    int pos;
    unsigned int rxpdo_len;
    unsigned int txpdo_len;
};
template<int N=3>
struct EcatCheckerArr{
    EcatChecker arr[N];
};


static int __pdo_check_init_pos(void* p, const void* inp, void* oup){
    proc_img_i_addr* txpdo_addr = (proc_img_i_addr*) inp;
    proc_img_o_addr* rxpdo_addr = (proc_img_o_addr*) oup;
    EcatChecker& chk = *(EcatChecker*) p;
    // static bool firstFlag = true;
    chk.cnt ++;
    if (chk.cnt==1) {
        // firstFlag = false;
        // printf("firstFlag,%d,%d\n", rxpdo_addr->len, txpdo_addr->len);
        // EXPECT_EQ(chk.pos, *txpdo_addr->servos[0].p);
        // for (int i=0; i<txpdo_addr->axLen; i++){
            // inp.status_word = *(uint16_t*) txpdo_addr->servos[i].stat;
            // inp.actual_position = *txpdo_addr->servos[i].p;
        // }
    }
    return 0;
};


TEST(ecatWrap, sg){
    // const char *fname;
    auto eni_file = _get_file_path("mhelper/data/pse_servo1.xml");
    // auto eni_file = get_file_path<constLength("")>("/../../../mhelper/data/pse_servo3.xml");
    // auto eni_file = std::string(__FILE__) + "/../../../mhelper/data/pse_servo1.xml";
    // if (fname) eni_file = fname;
    EXPECT_EQ(isFileExist(eni_file), true);
    // No need to reset counter - using bitmap allocator now
    // ServoSgFakerEager::Instance(0).init_pos(45);
    int st = EcatWrapMgrSg::init_ecat("zecm0");
    EXPECT_EQ(st, 0);
    // Verify local errors before startup and successful queries afterwards. GPT-6, 2026-10-21.
    EcatServo* servo = EcatWrapMgrSg::get_ecat("zecm0");
    bool running = false;
    EXPECT_EQ(ZMC_ERR_ECAT_NULL_ARG, servo->bus_is_running(nullptr));
    EXPECT_EQ(ZMC_ERR_ECAT_INVALID_HANDLE, servo->bus_is_running(&running));
    EXPECT_FALSE(running);
    EcatWrapMgrSg::get_ecat("zecm0")->set_eni(eni_file.c_str());
    if (EcatWrapMgrSg::get_ecat(0)->load_eni_json(eni_file.c_str())){
        EXPECT_TRUE(false);
        return;
    }
    EXPECT_EQ(64, EcatWrapMgrSg::get_ecat(0)->pdoAddr.rxpdo_addr.len);
    EXPECT_EQ(152, EcatWrapMgrSg::get_ecat(0)->pdoAddr.txpdo_addr.len);
    
    int ret = EcatWrapMgrSg::get_ecat("zecm0")->start();
    EXPECT_EQ(ret, 0);
    ASSERT_EQ(0, servo->bus_is_running(&running));
    EXPECT_TRUE(running);
    uint8_t requested_state = 0;
    ASSERT_EQ(0, servo->get_master_req_state(&requested_state));
    EXPECT_EQ(EcatStateO, requested_state);

    EcatChecker chk = {0, 45, 152, 64};
    EcatWrapMgrSg::get_ecat(0)->bindPdoHandler(__pdo_check_init_pos, &chk);
    while(1){
        if (chk.cnt)
            break;
    }
    EcatWrapMgrSg::get_ecat("zecm0")->stop();
}

TEST(ecatWrap, mult){
    auto eni_file = _get_file_path("mhelper/data/pse_servo1.xml");
    // auto eni_file = std::string(__FILE__) + "/../../../mhelper/data/pse_servo1.xml";
    // EXPECT_EQ(isFileExist(eni_file), true);
    const char* net_name_arr[] = {"zecm0", "zecm1", "zecm2"};
    const int posArr[] = {55, -55, 20};
    // No need to reset counter - using bitmap allocator now
    for (int i=0; i<3; i++){
        // ServoSgFakerEager::Instance(i).init_pos(posArr[i]);
        int st = EcatWrapMgrSg::init_ecat(net_name_arr[i]);
        EXPECT_EQ(st, 0);
        int idx = EcatWrapMgrSg::find_netname(net_name_arr[i]);
        EcatWrapMgrSg::get_ecat(idx)->set_eni(eni_file.c_str());
        if (EcatWrapMgrSg::get_ecat(i)->load_eni_json(eni_file.c_str())){
            EXPECT_TRUE(false);
            return;
        }
        EXPECT_EQ(64, EcatWrapMgrSg::get_ecat(idx)->pdoAddr.rxpdo_addr.len);
        EXPECT_EQ(152, EcatWrapMgrSg::get_ecat(idx)->pdoAddr.txpdo_addr.len);
        int ret = EcatWrapMgrSg::get_ecat(idx)->start();
        EXPECT_EQ(ret, 0);
    }
    EcatCheckerArr<3> chk = { {{0, posArr[0], 152, 64}, {0, posArr[1]}, {0, posArr[2]}}};
    for (int i=0; i<3; i++){
        EcatWrapMgrSg::get_ecat(i)->bindPdoHandler(__pdo_check_init_pos, &chk.arr[i]);
    }
    while(1){
        if (chk.arr[0].cnt && chk.arr[1].cnt && chk.arr[2].cnt)
            break;
    }
    for (int i=0; i<3; i++){
        EcatWrapMgrSg::get_ecat(net_name_arr[i])->stop();
    }
}
#endif
