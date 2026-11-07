#ifndef _ZMC_SDO_BACKEND_ECAT_HPP_
#define _ZMC_SDO_BACKEND_ECAT_HPP_

#include "zecm/zecm_ex.h"
#include <cstdint>

// EcatCoeSDODownload / EcatCoeSDOUpload: ZECM API (same semantics as zecm_fak.cpp).
namespace zmc {

struct EcatSdoBackend {
    static int32_t Upload(void* h, uint16_t slave_id, uint16_t index, uint8_t subindex,
                          uint32_t* len, void* data, uint32_t timeout_ms) {
        return EcatCoeSDOUpload(
            static_cast<ECAT_HANDLE>(h),
            slave_id, index, subindex,
            0,  // bCompleteAccess
            len, static_cast<ECAT_BYTE*>(data), *len, timeout_ms);
    }
    static int32_t Download(void* h, uint16_t slave_id, uint16_t index, uint8_t subindex,
                            uint32_t len, const void* data, uint32_t timeout_ms) {
        ECAT_DWORD dwLen = len;
        return EcatCoeSDODownload(
            static_cast<ECAT_HANDLE>(h),
            slave_id, index, subindex,
            0,  // bCompleteAccess
            &dwLen, const_cast<ECAT_BYTE*>(static_cast<const ECAT_BYTE*>(data)), len, timeout_ms);
    }
};

} // namespace zmc

#endif // _ZMC_SDO_BACKEND_ECAT_HPP_
