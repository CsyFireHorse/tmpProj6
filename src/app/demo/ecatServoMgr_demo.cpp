#include "EcatWrap.h"
#include "common/misc/compat_windows.h"
#include <string>
#include "string.h"
#include "common/get_file_path.hpp"

using namespace zmc;


static int __pdo_print(void* obj, const void* inp, void* out){
    proc_img_i_addr* pIn = (proc_img_i_addr*) inp;
    proc_img_o_addr* pOut = (proc_img_o_addr*) out;
    static int cnt=0; 
    if ( cnt++ % 1000 == 1)
        printf("%d,%d,%d\n", cnt, pIn->len, pOut->len);
    // pIn->servos[0].p;
    return 0;
}

int ecatServoMgr_demo(const char * eni){
    auto eni_file = _get_file_path("mhelper/data/pse_servo3.xml");
    if (eni) eni_file = eni;
    ECAT_RESULT ret;

    int st = EcatWrapMgrSg::init_ecat(std::string("zecm0"));
    if (st) { 
        printf("init failed %d\n", st); 
        return -1;
    }else 
    printf("init %d\n", st);
    int fd = EcatWrapMgrSg::find_netname("zecm0");
    EcatWrapMgrSg::get_ecat(fd)->set_eni(eni_file.c_str());

    if (EcatWrapMgrSg::get_ecat(fd)->load_eni_json(eni_file.c_str())){
        _ERR_("load ENI failed: ret=%s", eni_file.c_str());
        return -1;
    }
    ret = EcatWrapMgrSg::get_ecat(fd)->simple_start();
    if (ret) {
        _ERR_("Ecat Start failed: ret=%d", ret);
        return -1;
    }
    // EcatWrapMgrSg::get_ecat(fd)->bindPdoHandler(__pdo_print, nullptr);

    printf("Press e key to exit\n");
    while (getchar() != 'e');
    EcatWrapMgrSg::get_ecat(fd)->stop();

    return 0;
}
