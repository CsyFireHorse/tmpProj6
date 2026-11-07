#ifndef _ECAT_JSON_HPP_
#define _ECAT_JSON_HPP_

#include <stdint.h>
#include <string>
#include <vector>
// #include "common/zmc_errno.h"
#include "common/ecat/ecat_pdo_key.h"

using std::string;
using std::vector;

namespace zmc {
typedef struct {
    string channel;
    string name;
    string type;

    uint16_t index;       // json is string
    uint16_t subindex;    // json is string
    uint16_t bit_size;    // json is string
    uint16_t bit_offset;  // json is string
    inline uint16_t get_end_addr() const { return bit_size + bit_offset; }
} pdo_info_t;

// single slave
typedef struct {
    string name;
    string vendor_id;
    string vendor_name;
    string type;
    string product_code;
    string revision_no;
    string product_revision;
    uint32_t phys_addr;
    uint32_t auto_inc_addr;
    string slot_name;

    vector<pdo_info_t> outputs;
    vector<pdo_info_t> inputs;
    inline uint16_t get_rx_end_addr() const {
        return outputs.size() ? outputs.back().get_end_addr() : 0;
    }
    inline uint16_t get_tx_end_addr() const {
        return inputs.size() ? inputs.back().get_end_addr() : 0;
    }
    typedef struct {
        uint16_t index;
        uint8_t sub_index;
        uint8_t byte_len;
    } rt_bus_pdo_cell;

    bool is_axis(uint16_t subIndex = 0) const {
        // CiA402：control word & status word {0x6040:0:2, 0x6041:0:2}
        return find_pdo(outputs,
                        {(uint16_t)(PDO_PO_CTRL + subIndex * PDO_MULT_SERVO_OFFSET), 0, 2}) &&
               find_pdo(inputs,
                        {(uint16_t)(PDO_PI_STAT + subIndex * PDO_MULT_SERVO_OFFSET), 0, 2});
    }
    bool find_pdo(const std::vector<pdo_info_t>& pdo_list, const rt_bus_pdo_cell& pdo) const {
        for (const auto& data : pdo_list) {
            if (data.index == pdo.index && data.subindex == pdo.sub_index &&
                data.bit_size == pdo.byte_len * 8) {
                return true;
            }
        }
        return false;
    }
} SLAVES_INFO_t;

// all slaves
struct SlaveInfo {
    vector<SLAVES_INFO_t> slaves;
    uint16_t get_rx_end_addr() const;
    uint16_t get_tx_end_addr() const;
    inline uint32_t size() const { return slaves.size(); }
};

struct MasterInfo {
    uint32_t cycle_time = 1000;
    uint32_t mailbox_cycle_time = 5000;
    uint32_t auto_recovery_timeout = 1000;

    uint32_t di_begin_index = 0;
    uint32_t do_begin_index = 0;
    uint32_t ai_begin_index = 0;
    uint32_t ao_begin_index = 0;
    uint32_t di_end = 0;
    uint32_t do_end = 0;
    uint32_t ai_end = 0;
    uint32_t ao_end = 0;
    vector<uint32_t> di_keys;
    vector<uint32_t> do_keys;
    vector<uint32_t> ai_keys;
    vector<uint32_t> ao_keys;
};

/** 从ENI文件中 解析 从站信息，主站信息，杂类信息
 *
 * xml_file \param[in]      ENI 文件
 * slaves   \param[out]     从站信息
 * mast     \param[out]     主站信息和杂类信息
 *
 */
int load_slaves_from_eni_xml_json(const char* xml_file, SlaveInfo& slaves,
                                  MasterInfo* mast = nullptr);

/**
 * Extract embedded ENI JSON text from an ENI XML file.
 * Scans for markers: ENI_PDO_LIST / ENI_LIST / ENI_INFO.
 * out_json receives the JSON substring suitable for json::parse.
 */
int extract_eni_json_text(const char* eni_file, std::string& out_json);

}  // namespace zmc
#endif