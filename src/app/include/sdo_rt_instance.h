#ifndef _ZMC_SDO_RT_INSTANCE_H_
#define _ZMC_SDO_RT_INSTANCE_H_

#include "sdo_rt_ecat.hpp"
#include "sdo_rt_pcie.hpp"

namespace zmc {

struct SdoRtInstance {
    static EcatSdoRt& instance();
    static int bindEcat(void* h);
    static void start();
    static void stop();
};

struct SdoPcieRtInstance {
    static PcieSdoRt& instance();
    static int bindEcat(void* h);
    static void start();
    static void stop();
};

}  // namespace zmc

#endif  // _ZMC_SDO_RT_INSTANCE_H_
