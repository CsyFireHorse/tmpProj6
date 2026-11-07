#ifndef _ZMC_SDO_RT_PCIE_HPP_
#define _ZMC_SDO_RT_PCIE_HPP_

#include "sdo_rt_thread.hpp"

namespace zmc {

class PcieSdoRt final : public SdoRtThread {
   public:
    PcieSdoRt();
    ~PcieSdoRt();

    int bindEcat(void* ecat_handle);

    PcieSdoRt(const PcieSdoRt&) = delete;
    PcieSdoRt& operator=(const PcieSdoRt&) = delete;
};

}  // namespace zmc

#endif  // _ZMC_SDO_RT_PCIE_HPP_
