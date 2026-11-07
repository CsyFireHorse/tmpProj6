#ifndef ZML_IO_METADATA_H
#define ZML_IO_METADATA_H

#include <stdint.h>

/* Configured IO alias metadata returned by zml_get_io_metadata. */
enum {
    ZML_IO_TYPE_DI = 0,
    ZML_IO_TYPE_DO = 1,
    ZML_IO_TYPE_AI = 2,
    ZML_IO_TYPE_AO = 3,
    ZML_IO_TYPE_UNKNOWN = 255
};

enum {
    ZML_IO_SOURCE_KIND_NOP = 0,
    ZML_IO_SOURCE_KIND_PDO = 1,
    ZML_IO_SOURCE_KIND_VIRTUAL = 2,
    ZML_IO_SOURCE_KIND_GPIO = 3,
    ZML_IO_SOURCE_KIND_MODBUS = 4,
    ZML_IO_SOURCE_KIND_UNKNOWN = 255
};

typedef struct ZML_IoMetadataEntry {
    uint32_t mask;
    uint16_t io_number;
    uint16_t index;
    uint16_t bit_offset;
    uint8_t io_type;
    uint8_t source_kind;
    uint8_t slave_idx;
    uint8_t subindex;
    uint8_t bit_size;
    uint8_t is_input; /* 1 for input; 0 for output. */
} ZML_IoMetadataEntry;

#endif /* ZML_IO_METADATA_H */
