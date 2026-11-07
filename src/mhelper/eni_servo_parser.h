#ifndef _ENI_SERVO_PARSER_H_
#define _ENI_SERVO_PARSER_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "common/zmc/pdo_offset_cfg.h"
#include "ecat_json.h"
#include "utils/zmc_pio_offset_layout.h"


namespace zmc {

struct EniServoMapper {
    MasterInfo master_pdo;
    SlaveInfo slvInfo;
    std::vector<pi_offset_config> servo_pdo;

    uint32_t size() const { return slvInfo.size(); }
    uint32_t servo_size() const { return servo_pdo.size(); }
    bool contain_key(int servo_idx, int idx) const;
    int get_servo_offset(int servo_idx) const;
    int get_servo_slave_idx(int servo_idx) const;

    // Load offsets from ENI XML/JSON file path (legacy mhelper path).
    int load_from_eni_file(const char* fname);
};

int load_pdo_offset_from_file(const char* fname, EniServoMapper& mapper);

// Print an already parsed ENI configuration.
void print_eni_servo_mapper(const EniServoMapper& mapper);

// Fill servo_pdo + minimal slvInfo from shared ZmcPioOffsetLayout (caller binds pdo_addr).
int eni_servo_mapper_from_pio_layout(EniServoMapper* out, const ZmcPioOffsetLayout* layout);
// Fill shared ZmcPioOffsetLayout from an ENI-derived servo mapper.
int pio_layout_from_eni_servo_mapper(const EniServoMapper* mapper, ZmcPioOffsetLayout* out);

}  // namespace zmc

#endif
