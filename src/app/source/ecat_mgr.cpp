#include "stdio.h"
#include "ecat_mgr.h"
// #include "ecat_pdoioWrap.h"
// #include <atomic>
using namespace std;

namespace zmc{



// int EcatManager::ecat_start_pdo(const std::string& net_name){
//     rt_bus_run(0);
//     static const rt_bus_event bus_evt[] = { { nullptr, _ether_cat_pdo_inout } };
//     rt_bus_event_set(0, bus_evt);
//     return 0;
// }

// int __proc(void* p, uint8_t* inp, uint8_t* oup);
// static void _ether_cat_pdo_inout(void *context, uint32_t, int32_t is_output)
// {
//     // return;
//     static int cnt = 0;
//     if (is_output)
//     { 
//         // _task_forward();
//         if (cnt++ % 10000 == 5000)
//             printf("count:%d\n", cnt);
//     }
// }

// int EcatManagerSg::use_txrxpdo(uint16_t idx){
//     EcatManagerSg::get_ecat(idx)->init_txrxpdo(idx);
//     get_ecat(idx)->configTaskHandler(__proc, (void*) get_txrxpdo(idx));
//     return 0;
// }
}