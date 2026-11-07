/*
 * @Author: ChenShengyong
 * @Date: 2026-05-13 17:48:36
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2026-05-13 17:48:36
 */
#ifndef _ZMC_ENI_MOTOR_MAP_HPP_
#define _ZMC_ENI_MOTOR_MAP_HPP_

#include <cstdint>
#include <string>
#include <vector>

#include "common/zmc/pdo_offset_cfg.h"
#include "common/zmc/mc_errno.h"

namespace eni {

struct MasterInfo {
    uint32_t cycle_time_us = 1000;
};

struct PdoEntry {
    std::string channel;
    std::string name;
    uint32_t index = 0;
    uint16_t subindex = 0;
    std::string type;
    uint32_t bit_size = 0;
    uint32_t bit_offset = 0;
};

struct SlaveInfo {
    int index = 0;
    std::string name;
    uint32_t vendor_id = 0;
    std::string vendor_name;
    std::string type;
    uint32_t product_code = 0;
    uint32_t revision_no = 0;
    std::string product_revision;
    uint16_t phys_addr = 0;
    uint16_t auto_inc_addr = 0;
    std::vector<PdoEntry> outputs;
    std::vector<PdoEntry> inputs;
    bool is_servo = false;
    pi_offset_config offsets {};
};

struct EniDescriptor {
    MasterInfo master;
    std::vector<SlaveInfo> slaves;
};

// Single pipeline: embedded JSON in XML blob -> EniDescriptor. Errors are MCCommResult / ZMC_ERR_ENI_*.
MCCommResult LoadDescriptorFromEniXmlBlob(const std::string& xml_blob, EniDescriptor* out);

} // namespace eni
#endif
