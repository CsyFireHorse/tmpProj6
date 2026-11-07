#ifndef _ZMC_SDO_RT_ECAT_HPP_
#define _ZMC_SDO_RT_ECAT_HPP_

#include "sdo_rt_thread.hpp"

namespace zmc {

class EcatSdoRt final : public SdoRtThread {
   public:
    EcatSdoRt();
    ~EcatSdoRt();

    int bindEcat(void* ecat_handle);

    EcatSdoRt(const EcatSdoRt&) = delete;
    EcatSdoRt& operator=(const EcatSdoRt&) = delete;
};

}  // namespace zmc

#endif  // _ZMC_SDO_RT_ECAT_HPP_
