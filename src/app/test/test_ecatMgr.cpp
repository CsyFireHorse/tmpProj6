#include "stdio.h"
#include "string.h"
#include <iostream>
#include <string>
#include "assert.h"
#include <gtest/gtest.h>
#include "ecat_mgr.h"
// #include "common/zal_cpp_common.h"
#include "common/get_file_path.hpp"
#define LEN(x) sizeof(x)/sizeof(*(x))

#include <fstream>
#include <iostream>

inline bool isFileExist(const std::string& filename){
    std::ifstream file(filename);
    return file.good();
}

// using namespace zmc;
using namespace zmc;



TEST(ecatMgr, sg){
    // const char *fname;
    auto eni_file = _get_file_path("mhelper/data/pse_servo3.xml");
    // auto eni_file = get_file_path<constLength("")>("/../../../mhelper/data/pse_servo3.xml");
    // auto eni_file = std::string(__FILE__) + "/../../../mhelper/data/pse_servo1.xml";
    // if (fname) eni_file = fname;
    EXPECT_EQ(isFileExist(eni_file), true);
    int st = EcatManagerSg::init_ecat("zecm0");
    EXPECT_EQ(st, 0);
    EcatManagerSg::get_ecat("zecm0")->set_eni(eni_file.c_str());

    int ret = EcatManagerSg::get_ecat("zecm0")->start();
    EXPECT_EQ(ret, 0);
    EcatManagerSg::get_ecat("zecm0")->stop();
}


TEST(ecatMgr, mult){
    auto eni_file = _get_file_path("mhelper/data/pse_servo1.xml");
    // auto eni_file = std::string(__FILE__) + "/../../../mhelper/data/pse_servo1.xml";
    EXPECT_EQ(isFileExist(eni_file), true);
    const char* net_name_arr[] = {"zecm0", "zecm1", "zecm2"};
    for (int i=0; i<3; i++){
        int st = EcatManagerSg::init_ecat(net_name_arr[i]);
        EXPECT_EQ(st, 0);
        int idx = EcatManagerSg::find_netname(net_name_arr[i]);
        EcatManagerSg::get_ecat(idx)->set_eni(eni_file.c_str());

        int ret = EcatManagerSg::get_ecat(idx)->start();
        EXPECT_EQ(ret, 0);
    }

    for (int i=0; i<3; i++){
        EcatManagerSg::get_ecat(net_name_arr[i])->stop();
    }
}
