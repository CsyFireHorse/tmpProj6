#ifndef _ZMC_IO_METADATA_HPP_
#define _ZMC_IO_METADATA_HPP_

#include <stdint.h>

namespace zmc {

// Identifies one configured IO alias group.
enum IoAliasKind : uint8_t {
    IO_ALIAS_KIND_DI = 0,
    IO_ALIAS_KIND_DO,
    IO_ALIAS_KIND_AI,
    IO_ALIAS_KIND_AO
};

// Identifies the storage source of one configured IO alias.
enum IoSourceKind : uint8_t {
    IO_SOURCE_KIND_NOP = 0,
    IO_SOURCE_KIND_PDO,
    IO_SOURCE_KIND_VIRTUAL,
    IO_SOURCE_KIND_GPIO,
    IO_SOURCE_KIND_MODBUS,
    IO_SOURCE_KIND_END,
};

// Describes one configured IO alias.
struct IoMetadataEntry {
    uint32_t mask;  // Zero for virtual, analog, and whole-object aliases.
    IoAliasKind kind;
    IoSourceKind source_kind;
    uint16_t io_number;
    uint16_t index;
    uint8_t slave_idx;
    uint8_t subindex;
    uint16_t bit_offset;
    uint8_t bit_size;
    bool is_input;
};

}  // namespace zmc

#endif  // _ZMC_IO_METADATA_HPP_
