#include "EcatWrap.h"
#include "common/misc/compat_windows.h"
#include <string>
#include "string.h"
#include "ecat_mgr.h"
#include "mhelper/pdo_servo_addr.h"
#include "zecm/zecm_zh.h"
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

int servo_main(const char * eni, const char * ecat_name)
{
    return 0;
}

int ecatServo_demo(const char * eni){
    auto eni_file = _get_file_path("mhelper/data/pse_servo3.xml");
    if (eni) eni_file = eni;
    ECAT_RESULT ret;
    EcatServo m("zecm0");

    // Convert the current thread to a real-time thread with priority 50.
    // struct sched_param sched;
    // sched.sched_priority = 50;
    // pthread_setschedparam(pthread_self(), SCHED_FIFO, &sched);

    // Initialize the master environment.
    LOG_RET(ret, EcatMasterLibInit(ECAT_NULL));
    if (ECAT_FAILED(ret)) {
        printf("init failed %d\n", ret); 
        return -1;
    }

    if (m.load_eni_json(eni_file.c_str())){
        _ERR_("load ENI failed: ret=%s", eni_file.c_str());
        return -1;
    }
    ret = m.simple_start();
    if (ret) {
        _ERR_("Ecat Start failed: ret=%d", ret);
        return -1;
    }

    // m.bindPdoHandler(__pdo_print, nullptr);
    printf("press e to exit\n");
    while (getchar() != 'e')
        usleep(50000);
    
    // Exit.
    LOG_RET(ret, m.stop());   
    LOG_RET(ret, EcatMasterLibExit());
    return 0;
}
