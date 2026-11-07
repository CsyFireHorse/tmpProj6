/*
 * @Description: PDO image accessor bound to a ZmcPioOffsetLayout (CiA402 field read/write).
 */

#ifndef PIO_OFFSET_ACCESSOR_HPP
#define PIO_OFFSET_ACCESSOR_HPP

#include <cstddef>
#include <cstdint>

#include "utils/zmc_pio_offset_layout.h"

namespace eni {

class PioOffsetAccessor {
public:
    explicit PioOffsetAccessor(const ZmcPioOffsetLayout& layout);

    void bind(const uint8_t* tx, uint8_t* rx);
    void unbind();

    uint16_t getTxControlWord(size_t axis_idx, uint16_t fallback = 0u) const;
    int32_t getTxTargetPos(size_t axis_idx, int32_t fallback = 0) const;
    uint16_t getRxStatusWord(size_t axis_idx, uint16_t fallback = 0u) const;
    int32_t getRxActualPos(size_t axis_idx, int32_t fallback = 0) const;

    void setRxStatusWord(size_t axis_idx, uint16_t value);
    void setRxActualPos(size_t axis_idx, int32_t value);
    void setRxModeFromTx(size_t axis_idx);
    void setRxErrorWord(size_t axis_idx, uint16_t value);

private:
    const pi_offset_config* slotOrNull(size_t axis_idx) const;

    bool ofsOkTx(int32_t ofs, uint32_t sz) const;
    bool ofsOkRx(int32_t ofs, uint32_t sz) const;
    uint16_t rdU16Tx(int32_t ofs, uint16_t fallback) const;
    uint16_t rdU16Rx(int32_t ofs, uint16_t fallback) const;
    int32_t rdI32Tx(int32_t ofs, int32_t fallback) const;
    int32_t rdI32Rx(int32_t ofs, int32_t fallback) const;
    void wrU16Rx(int32_t ofs, uint16_t v);
    void wrI32Rx(int32_t ofs, int32_t v);

    ZmcPioOffsetLayout layout_ {};
    const uint8_t* tx_ = nullptr;
    uint8_t* rx_ = nullptr;
};

} // namespace eni

#endif /* PIO_OFFSET_ACCESSOR_HPP */
