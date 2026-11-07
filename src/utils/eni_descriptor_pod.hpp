/*
 * @Description: C++ helpers for EniDescriptorPod (caps, offset deduction, descriptor conversion).
 */

#ifndef _ZMC_ENI_DESCRIPTOR_POD_HPP_
#define _ZMC_ENI_DESCRIPTOR_POD_HPP_

#include <cstdint>
#include <string>
#include <type_traits>

#include "common/zmc/mc_errno.h"
#include "common/zmc/pdo_offset_cfg.h"
#include "utils/eni_descriptor_pod.h"

namespace eni {

struct EniDescriptorPodWithCap : public EniDescriptorPod {
    void updateCapsFromCells();
    bool deduceOffsets(uint8_t slave_idx, pi_offset_config* out) const;
    uint8_t slaveIsAxis(uint8_t slave_idx) const;
};

static_assert(sizeof(RtBusPdoCell) == 8u, "RtBusPdoCell must be 8 bytes");
static_assert(std::is_standard_layout<EniDescriptorPod>::value,
              "EniDescriptorPod must be standard layout");
static_assert(std::is_trivially_copyable<EniDescriptorPod>::value,
              "EniDescriptorPod must be trivially copyable");

MCCommResult LoadDescriptorPodFromEniXmlBlob(const std::string& xml_blob, EniDescriptorPod* out);

}  // namespace eni

#endif
