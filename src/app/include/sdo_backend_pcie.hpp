#ifndef _ZMC_SDO_BACKEND_PCIE_HPP_
#define _ZMC_SDO_BACKEND_PCIE_HPP_

#include "pci_zecm.h"
#include <cstdint>

namespace zmc {

struct PcieSdoBackend {
    static int32_t Upload(void* h, uint16_t slave_id, uint16_t index, uint8_t subindex,
                          uint32_t* len, void* data, uint32_t timeout_ms) {
        return EcatCoeSDOUploadEx(
            static_cast<ECAT_HANDLE>(h),
            slave_id, index, subindex,
            0,  // bCompleteAccess
            len, data, timeout_ms);
    }
    static int32_t Download(void* h, uint16_t slave_id, uint16_t index, uint8_t subindex,
                            uint32_t len, const void* data, uint32_t timeout_ms) {
        return EcatCoeSDODownloadEx(
            static_cast<ECAT_HANDLE>(h),
            slave_id, index, subindex,
            0,  // bCompleteAccess
            len, data, timeout_ms);
    }
};

} // namespace zmc

#endif // _ZMC_SDO_BACKEND_PCIE_HPP_
