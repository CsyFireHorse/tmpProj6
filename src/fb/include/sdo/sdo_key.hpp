#ifndef _ZMC_SDO_KEY_HPP_
#define _ZMC_SDO_KEY_HPP_

#include <cstdint>

namespace zmc {

// Identify one SDO object (slave + index + subindex)
struct SdoKey {
    uint16_t slave_id;
    uint16_t index;
    uint8_t subindex;

    bool operator==(const SdoKey& other) const noexcept {
        return slave_id == other.slave_id && index == other.index && subindex == other.subindex;
    }
};

}  // namespace zmc

#endif  // _ZMC_SDO_KEY_HPP_
