// #include "ecat_pdoioWrap.h"
#include "ecat_mgr.h"
#include "EcatWrap.h"
#include <string>
#include "string.h"
#include "common/get_file_path.hpp"
using namespace zmc;


static int __pdo_marquee(void* p, uint8_t* inp, uint8_t* oup){
    EcatServo* pEcat = (EcatServo*)p;
    static bool firstFlag = true;
    if (firstFlag) {
        firstFlag = false;
        printf("firstFlag,%d,%d\n", pEcat->pdoAddr.rxpdo_addr.len, pEcat->pdoAddr.txpdo_addr.len);
    }
    
    static int cnt = 0;
    static uint16_t mask = 0x8001;

    #define CCROT_LEFT(v, n)  (v >> (sizeof(v)*8 - n)) | (v << n)
    #define CROT_RIGHT(v, n)  (v << (sizeof(v)*8 - n)) | (v >> n)
    if (cnt++ % 500 != 1){  
        // return 0;
    }else{
        mask = CCROT_LEFT(mask, 1);
        printf("cnt=%d\n", cnt);
    }
    // return 0;
    uint8_t* inp2 = (uint8_t*) inp;
    uint8_t* oup2 = (uint8_t*) oup;

    uint16_t rval = 0; // inp2[0] | (inp2[1]<<8);
    // const int sz = 16;     
    oup2[0] = mask & 0xff;
    oup2[1] = mask >> 8;
    if (cnt % 500 == 1)
        printf("%x,%x\n", rval, mask);
    return 0;
};

int pdoMarquee_demo(const char *fname){
    auto eni_file = _get_file_path("mhelper/data/bf_el2809b.xml");
    // pse_doao_map
    if (fname) eni_file = fname;
    int st = EcatWrapMgrSg::init_ecat("zecm0");
    if (st) { printf("init failed %d\n", st); return -1;}
    else printf("init %d\n", st);
    int fd = EcatWrapMgrSg::find_netname("zecm0");
    printf("init fd=%d\n", fd);
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
    // EcatWrapMgrSg::get_ecat(fd)->bindPdoHandler(__pdo_print, nullptr);
    EcatWrapMgrSg::get_ecat(fd)->bind(__pdo_marquee, EcatWrapMgrSg::get_ecat(fd));
    printf("Press e key to exit\n");
    while (getchar() != 'e');
    EcatWrapMgrSg::stop(fd);
    return 0;
}
