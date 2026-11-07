/*
 * @ArtifactStatus: garbage
 * @ArtifactStatusDate: 2026-08-25
 * @ArtifactStatusAuthority: user
 */
#include "io/IoManager.hpp"

#include <cstddef>
#include <cstring>
#include <memory>
#include <new>
#include <utility>

#include "common/zmc/fbt_errorcode_def.h"

namespace zmc {

namespace {

// Each IO kind keeps the existing fixed alias-number capacity.
constexpr std::size_t kIoAliasCapacity = MC_IO_MAX_COUNT;
// Digital values use one packed bank bit per IO number.
constexpr std::size_t kDigitalBankByteCapacity = (kIoAliasCapacity + 7u) / 8u;
// Digital bank windows use one fixed 64-bit load or store, including at the logical end.
constexpr std::size_t kDigitalBankStorageCapacity =
    kDigitalBankByteCapacity + sizeof(uint64_t) - 1u;
// One PDO DI run can contribute one aligned copy or one operation per Bank byte.
constexpr std::size_t kInputAliasCapacity = kIoAliasCapacity;
// One PDO DI run can contribute a leading and a trailing masked range.
constexpr std::size_t kInputMaskRangeCapacity = kInputAliasCapacity * 2u;
// DO and AO can each contribute one aligned copy range.
constexpr std::size_t kOutputAliasCapacity = kIoAliasCapacity * 2u;
// One output alias can contribute a leading and a trailing masked range.
constexpr std::size_t kOutputMaskRangeCapacity = kOutputAliasCapacity * 2u;

bool same_key(const RtBusPdoCell& cell, const IoObjectKey& key) {
    return cell.slave_idx == key.slave_idx && cell.index == key.index &&
           cell.sub_index == key.subindex;
}

bool is_valid_multi_io_range(uint32_t start_io_number, uint32_t end_io_number) {
    return start_io_number <= end_io_number && end_io_number - start_io_number < 32u;
}

/* Read up to 4 contiguous process-image bytes (byte API helper). */
int read_process_image(const uint8_t* image, uint32_t capacity, uint16_t byte_offset,
                       uint8_t byte_len, uint32_t* value) {
    if (!value) {
        return ZMC_ERR_IO_INVALID_ARG;
    }
    if (byte_len < 1u || byte_len > 4u) {
        return ZMC_ERR_IO_DATA_SIZE_UNSUPPORTED;
    }
    if (!image) {
        return ZMC_ERR_IO_PROCESS_IMAGE_UNBOUND;
    }
    if (static_cast<uint32_t>(byte_offset) + byte_len > capacity) {
        return ZMC_ERR_IO_PROCESS_IMAGE_OVER_RANGE;
    }

    uint32_t result = 0;
    std::memcpy(&result, image + byte_offset, byte_len);
    *value = result;
    return 0;
}

/* Write up to 4 contiguous process-image bytes (byte API helper). */
int write_process_image(uint8_t* image, uint32_t capacity, uint16_t byte_offset, uint8_t byte_len,
                        uint32_t value) {
    if (byte_len < 1u || byte_len > 4u) {
        return ZMC_ERR_IO_DATA_SIZE_UNSUPPORTED;
    }
    if (!image) {
        return ZMC_ERR_IO_PROCESS_IMAGE_UNBOUND;
    }
    if (static_cast<uint32_t>(byte_offset) + byte_len > capacity) {
        return ZMC_ERR_IO_PROCESS_IMAGE_OVER_RANGE;
    }

    std::memcpy(image + byte_offset, &value, byte_len);
    return 0;
}

/* True when [bit_offset, bit_offset + bit_size) fits inside capacity bytes. */
bool bits_in_range(uint32_t capacity_bytes, uint16_t bit_offset, uint8_t bit_size) {
    if (bit_size == 0u) {
        return false;
    }
    const uint32_t end_bits = static_cast<uint32_t>(bit_offset) + static_cast<uint32_t>(bit_size);
    const uint32_t end_bytes = (end_bits + 7u) / 8u;
    return end_bytes <= capacity_bytes;
}

/*
 * Extract bit_size bits starting at bit_offset (little-endian bit packing within the image).
 * Supports 1..32 bits, including non-byte-aligned fields.
 */
int read_bits(const uint8_t* image, uint32_t capacity_bytes, uint16_t bit_offset, uint8_t bit_size,
              uint32_t* value) {
    if (!value) {
        return ZMC_ERR_IO_INVALID_ARG;
    }
    if (bit_size < 1u || bit_size > 32u) {
        return ZMC_ERR_IO_DATA_SIZE_UNSUPPORTED;
    }
    if (!image) {
        return ZMC_ERR_IO_PROCESS_IMAGE_UNBOUND;
    }
    if (!bits_in_range(capacity_bytes, bit_offset, bit_size)) {
        return ZMC_ERR_IO_PROCESS_IMAGE_OVER_RANGE;
    }

    uint32_t result = 0;
    for (uint8_t i = 0; i < bit_size; ++i) {
        const uint32_t abs_bit = static_cast<uint32_t>(bit_offset) + i;
        const uint32_t byte_idx = abs_bit / 8u;
        const uint8_t bit_in_byte = static_cast<uint8_t>(abs_bit % 8u);
        if ((image[byte_idx] & static_cast<uint8_t>(1u << bit_in_byte)) != 0u) {
            result |= (1u << i);
        }
    }
    *value = result;
    return 0;
}

/*
 * Write bit_size bits starting at bit_offset with RMW (only touched bits change).
 * Supports 1..32 bits, including non-byte-aligned fields.
 */
int write_bits(uint8_t* image, uint32_t capacity_bytes, uint16_t bit_offset, uint8_t bit_size,
               uint32_t value) {
    if (bit_size < 1u || bit_size > 32u) {
        return ZMC_ERR_IO_DATA_SIZE_UNSUPPORTED;
    }
    if (!image) {
        return ZMC_ERR_IO_PROCESS_IMAGE_UNBOUND;
    }
    if (!bits_in_range(capacity_bytes, bit_offset, bit_size)) {
        return ZMC_ERR_IO_PROCESS_IMAGE_OVER_RANGE;
    }

    for (uint8_t i = 0; i < bit_size; ++i) {
        const uint32_t abs_bit = static_cast<uint32_t>(bit_offset) + i;
        const uint32_t byte_idx = abs_bit / 8u;
        const uint8_t bit_in_byte = static_cast<uint8_t>(abs_bit % 8u);
        const uint8_t mask = static_cast<uint8_t>(1u << bit_in_byte);
        if ((value & (1u << i)) != 0u) {
            image[byte_idx] = static_cast<uint8_t>(image[byte_idx] | mask);
        } else {
            image[byte_idx] = static_cast<uint8_t>(image[byte_idx] & static_cast<uint8_t>(~mask));
        }
    }
    return 0;
}

/* Read one IO-number bit from a packed digital bank. */
bool read_bank_bit(const uint8_t* bank, uint32_t io_number) {
    const uint32_t byte_offset = io_number / 8u;
    const uint8_t mask = static_cast<uint8_t>(1u << (io_number % 8u));
    return (bank[byte_offset] & mask) != 0u;
}

/* Write one IO-number bit without changing neighboring packed values. */
void write_bank_bit(uint8_t* bank, uint32_t io_number, bool value) {
    const uint32_t byte_offset = io_number / 8u;
    const uint8_t mask = static_cast<uint8_t>(1u << (io_number % 8u));
    if (value) {
        bank[byte_offset] = static_cast<uint8_t>(bank[byte_offset] | mask);
    } else {
        bank[byte_offset] = static_cast<uint8_t>(bank[byte_offset] & static_cast<uint8_t>(~mask));
    }
}

/* Extract 1..32 packed digital bits with one fixed-width little-endian load. */
uint32_t read_digital_bank_bits(const uint8_t* bank, uint32_t bit_offset, uint32_t bit_count) {
    uint64_t window = 0u;
    std::memcpy(&window, bank + bit_offset / 8u, sizeof(window));
    const uint32_t mask = 0xffffffffu >> (32u - bit_count);
    return static_cast<uint32_t>(window >> (bit_offset % 8u)) & mask;
}

/* Write 1..32 packed digital bits with one fixed-width little-endian RMW window. */
void write_digital_bank_bits(uint8_t* bank, uint32_t bit_offset, uint32_t bit_count,
                             uint32_t value) {
    const uint32_t bit_shift = bit_offset % 8u;
    const uint64_t value_mask = (static_cast<uint64_t>(1u) << bit_count) - 1u;
    const uint64_t write_mask = value_mask << bit_shift;
    uint64_t window = 0u;
    std::memcpy(&window, bank + bit_offset / 8u, sizeof(window));
    window = (window & ~write_mask) | ((static_cast<uint64_t>(value) << bit_shift) & write_mask);
    std::memcpy(bank + bit_offset / 8u, &window, sizeof(window));
}

}  // namespace

struct IoManager::Impl {
    // Alias records contain dispatch and PDO mapping data, never virtual values.
    struct Alias {
        uint32_t mask;
        uint16_t bit_offset;
        uint8_t bit_size; /* 0 = empty/unused */
        uint16_t cell_index;
        IoSourceKind source_kind;
    };

    // ByteRange is one compiled memcpy or byte-mask operation span.
    struct ByteRange {
        uint16_t byte_offset;
        uint16_t byte_len;
        uint8_t mask;
    };

    // InputCopyRange copies aligned PDO bytes directly into the logical DI Bank.
    struct InputCopyRange {
        uint16_t source_byte_offset;
        uint16_t bank_byte_offset;
        uint16_t byte_len;
    };

    // InputMaskRange copies selected same-phase bits while preserving neighboring bank bits.
    struct InputMaskRange {
        uint16_t source_byte_offset;
        uint16_t bank_byte_offset;
        uint8_t mask;
        uint8_t preserve_mask;
    };

    // InputShiftOperation applies one precompiled net shift to one Bank byte.
    struct InputShiftOperation {
        uint16_t source_byte_offset;
        uint16_t bank_byte_offset;
        uint8_t shift;
        uint8_t write_mask;
        uint8_t preserve_mask;
    };

    // Shift policies keep the hot path direction branch at compile time.
    struct InputShiftLeft {
        static uint16_t apply(uint16_t value, uint8_t shift) {
            return static_cast<uint16_t>(value << shift);
        }
    };

    struct InputShiftRight {
        static uint16_t apply(uint16_t value, uint8_t shift) {
            return static_cast<uint16_t>(value >> shift);
        }
    };

    // PDO DI snapshots, DO cache values, and virtual values share one fixed logical IO Bank.
    struct IoValueBank {
        uint8_t di_values[kDigitalBankStorageCapacity];
        uint8_t do_values[kDigitalBankStorageCapacity];
        uint32_t ai_values[kIoAliasCapacity];
        uint32_t ao_values[kIoAliasCapacity];
    };

    // The layout copy owns the PDO identity and bit-layout information.
    std::vector<RtBusPdoCell> cells;
    // Four alias tables provide unified numbering and source dispatch per IO kind.
    Alias di_aliases[kIoAliasCapacity];
    Alias do_aliases[kIoAliasCapacity];
    Alias ai_aliases[kIoAliasCapacity];
    Alias ao_aliases[kIoAliasCapacity];
    IoValueBank value_bank;
    // PDO input plans are allocated and compiled before the cycle path.
    std::vector<InputCopyRange> input_copy_ranges;
    std::vector<InputMaskRange> input_mask_ranges;
    std::vector<InputShiftOperation> input_shift_left_byte_operations;
    std::vector<InputShiftOperation> input_shift_right_byte_operations;
    std::vector<InputShiftOperation> input_shift_right_word_operations;
    // PDO output state is allocated from rx_cap.
    std::unique_ptr<uint8_t[]> output_shadow;
    // Registration compiles output aliases into preallocated copy and mask plans.
    std::vector<ByteRange> copy_ranges;
    std::vector<ByteRange> mask_ranges;
    uint32_t input_capacity;
    uint32_t output_capacity;
    const uint8_t* input;
    uint8_t* output;

    Impl()
        : cells(),
          di_aliases(),
          do_aliases(),
          ai_aliases(),
          ao_aliases(),
          value_bank(),
          input_copy_ranges(),
          input_mask_ranges(),
          input_shift_left_byte_operations(),
          input_shift_right_byte_operations(),
          input_shift_right_word_operations(),
          output_shadow(),
          copy_ranges(),
          mask_ranges(),
          input_capacity(0),
          output_capacity(0),
          input(nullptr),
          output(nullptr) {}

    const RtBusPdoCell* find_cell(const IoObjectKey& key, uint8_t is_input) const {
        for (std::size_t i = 0; i < cells.size(); ++i) {
            const RtBusPdoCell& cell = cells[i];
            if (cell.is_input == is_input && same_key(cell, key)) {
                return &cell;
            }
        }
        return nullptr;
    }

    const Alias* alias_at(const Alias* aliases, uint32_t io_number) const {
        if (io_number >= kIoAliasCapacity || aliases[io_number].bit_size == 0u) {
            return nullptr;
        }
        return &aliases[io_number];
    }

    int validate_alias(uint32_t io_number, const RtBusPdoCell* cell, const Alias* aliases,
                       uint32_t capacity) const {
        if (io_number >= kIoAliasCapacity) {
            return ZMC_ERR_IO_ALIAS_CAPACITY;
        }
        if (aliases[io_number].bit_size != 0u) {
            return ZMC_ERR_IO_ALIAS_DUPLICATE;
        }
        if (cell->bit_size < 1u || cell->bit_size > 32u) {
            return ZMC_ERR_IO_DATA_SIZE_UNSUPPORTED;
        }
        if (!bits_in_range(capacity, cell->bit_offset, cell->bit_size)) {
            return ZMC_ERR_IO_PROCESS_IMAGE_OVER_RANGE;
        }
        return 0;
    }

    int prepare_alias(uint32_t io_number, const RtBusPdoCell* cell, const Alias* aliases,
                      uint32_t capacity, uint32_t mask, bool use_mask, Alias* prepared) const {
        const int ret = validate_alias(io_number, cell, aliases, capacity);
        if (ret != 0) {
            return ret;
        }

        Alias alias = {};
        alias.mask = 0u;
        alias.bit_offset = cell->bit_offset;
        alias.bit_size = cell->bit_size;
        alias.cell_index = static_cast<uint16_t>(cell - cells.data());
        alias.source_kind = IO_SOURCE_KIND_PDO;
        if (use_mask) {
            if (mask == 0u || (mask & (mask - 1u)) != 0u) {
                return ZMC_ERR_IO_MASK_INVALID;
            }

            uint8_t selected_bit = 0u;
            uint32_t shifted_mask = mask;
            while ((shifted_mask & 1u) == 0u) {
                shifted_mask >>= 1u;
                ++selected_bit;
            }
            if (selected_bit >= cell->bit_size) {
                return ZMC_ERR_IO_MASK_INVALID;
            }

            const uint32_t selected_bit_offset =
                static_cast<uint32_t>(cell->bit_offset) + selected_bit;
            if (selected_bit_offset > 0xffffu ||
                !bits_in_range(capacity, static_cast<uint16_t>(selected_bit_offset), 1u)) {
                return ZMC_ERR_IO_PROCESS_IMAGE_OVER_RANGE;
            }
            alias.mask = mask;
            alias.bit_offset = static_cast<uint16_t>(selected_bit_offset);
            alias.bit_size = 1u;
        }
        *prepared = alias;
        return 0;
    }

    int register_alias(uint32_t io_number, const RtBusPdoCell* cell, Alias* aliases,
                       uint32_t capacity, uint32_t mask, bool use_mask) {
        Alias prepared = {};
        const int ret =
            prepare_alias(io_number, cell, aliases, capacity, mask, use_mask, &prepared);
        if (ret != 0) {
            return ret;
        }
        aliases[io_number] = prepared;
        return 0;
    }

    int register_virtual_alias(uint32_t io_number, Alias* aliases, uint8_t bit_size) {
        // Virtual aliases use the same per-kind number table as PDO aliases.
        if (io_number >= kIoAliasCapacity) {
            return ZMC_ERR_IO_ALIAS_CAPACITY;
        }
        if (aliases[io_number].bit_size != 0u) {
            return ZMC_ERR_IO_ALIAS_DUPLICATE;
        }
        Alias alias = {};
        alias.bit_size = bit_size;
        alias.source_kind = IO_SOURCE_KIND_VIRTUAL;
        aliases[io_number] = alias;
        return 0;
    }

    void append_alias_metadata(const Alias* aliases, IoAliasKind kind,
                               std::vector<IoMetadataEntry>& out) const {
        for (std::size_t i = 0; i < kIoAliasCapacity; ++i) {
            const Alias& alias = aliases[i];
            if (alias.bit_size == 0u) {
                continue;
            }

            IoMetadataEntry entry = {};
            entry.kind = kind;
            entry.source_kind = alias.source_kind;
            entry.io_number = static_cast<uint16_t>(i);
            entry.mask = alias.mask;
            entry.bit_size = alias.bit_size;
            entry.is_input = kind == IO_ALIAS_KIND_DI || kind == IO_ALIAS_KIND_AI;
            if (alias.source_kind == IO_SOURCE_KIND_PDO) {
                const RtBusPdoCell& cell = cells[alias.cell_index];
                entry.slave_idx = cell.slave_idx;
                entry.index = cell.index;
                entry.subindex = cell.sub_index;
                entry.bit_offset = cell.bit_offset;
                entry.bit_size = cell.bit_size;
                entry.is_input = cell.is_input != 0u;
            }
            out.push_back(entry);
        }
    }

    bool overlaps_aliases(const Alias* aliases, const Alias& candidate) const {
        const uint32_t candidate_start = candidate.bit_offset;
        const uint32_t candidate_end = candidate_start + candidate.bit_size;
        for (std::size_t i = 0; i < kIoAliasCapacity; ++i) {
            const Alias& alias = aliases[i];
            if (alias.bit_size == 0u || alias.source_kind != IO_SOURCE_KIND_PDO) {
                continue;
            }
            const uint32_t alias_start = alias.bit_offset;
            const uint32_t alias_end = alias_start + alias.bit_size;
            if (candidate_start < alias_end && alias_start < candidate_end) {
                return true;
            }
        }
        return false;
    }

    int add_byte_range(std::vector<ByteRange>* ranges, std::size_t capacity, uint16_t byte_offset,
                       uint16_t byte_len) {
        // Keep ranges ordered and merge both overlapping and adjacent spans.
        if (byte_len == 0u) {
            return 0;
        }

        uint32_t merged_start = byte_offset;
        uint32_t merged_end = static_cast<uint32_t>(byte_offset) + byte_len;
        std::size_t first = 0;
        while (first < ranges->size() &&
               static_cast<uint32_t>((*ranges)[first].byte_offset) + (*ranges)[first].byte_len <
                   merged_start) {
            ++first;
        }

        std::size_t after = first;
        while (after < ranges->size() && (*ranges)[after].byte_offset <= merged_end) {
            if ((*ranges)[after].byte_offset < merged_start) {
                merged_start = (*ranges)[after].byte_offset;
            }
            const uint32_t range_end =
                static_cast<uint32_t>((*ranges)[after].byte_offset) + (*ranges)[after].byte_len;
            if (range_end > merged_end) {
                merged_end = range_end;
            }
            ++after;
        }

        if (first == after) {
            if (ranges->size() >= capacity || ranges->size() >= ranges->capacity()) {
                return ZMC_ERR_IO_ALIAS_CAPACITY;
            }
            const ByteRange empty_range = {};
            ranges->insert(ranges->begin() + first, empty_range);
        } else if (after > first + 1u) {
            ranges->erase(ranges->begin() + first + 1u, ranges->begin() + after);
        }

        ByteRange& range = (*ranges)[first];
        range.byte_offset = static_cast<uint16_t>(merged_start);
        range.byte_len = static_cast<uint16_t>(merged_end - merged_start);
        range.mask = 0u;
        return 0;
    }

    int add_mask_bits(std::vector<ByteRange>* ranges, std::size_t capacity, uint32_t bit_start,
                      uint32_t bit_end) {
        // Each partial-byte segment is compiled into one ordered masked-byte operation.
        const uint16_t byte_offset = static_cast<uint16_t>(bit_start / 8u);
        uint8_t mask = 0u;
        for (uint32_t bit = bit_start; bit < bit_end; ++bit) {
            mask = static_cast<uint8_t>(mask | static_cast<uint8_t>(1u << (bit % 8u)));
        }

        std::size_t index = 0u;
        while (index < ranges->size() && (*ranges)[index].byte_offset < byte_offset) {
            ++index;
        }
        if (index < ranges->size() && (*ranges)[index].byte_offset == byte_offset) {
            (*ranges)[index].mask = static_cast<uint8_t>((*ranges)[index].mask | mask);
            return 0;
        }
        if (ranges->size() >= capacity || ranges->size() >= ranges->capacity()) {
            return ZMC_ERR_IO_ALIAS_CAPACITY;
        }

        ByteRange range = {};
        range.byte_offset = byte_offset;
        range.byte_len = 1u;
        range.mask = mask;
        ranges->insert(ranges->begin() + index, range);
        return 0;
    }

    int append_input_copy_range(uint16_t source_byte_offset, uint16_t bank_byte_offset,
                                uint16_t byte_len) {
        // Keep one memcpy per logical-and-physical contiguous, same-phase PDO DI run.
        if (byte_len == 0u) {
            return 0;
        }
        if (input_copy_ranges.size() >= kInputAliasCapacity ||
            input_copy_ranges.size() >= input_copy_ranges.capacity()) {
            return ZMC_ERR_IO_ALIAS_CAPACITY;
        }

        InputCopyRange range = {};
        range.source_byte_offset = source_byte_offset;
        range.bank_byte_offset = bank_byte_offset;
        range.byte_len = byte_len;
        input_copy_ranges.push_back(range);
        return 0;
    }

    int add_input_mask_range(uint16_t source_byte_offset, uint16_t bank_byte_offset,
                             uint8_t mask) {
        // Same source/destination byte pairs can share one masked copy operation.
        for (std::size_t i = 0; i < input_mask_ranges.size(); ++i) {
            InputMaskRange& range = input_mask_ranges[i];
            if (range.source_byte_offset == source_byte_offset &&
                range.bank_byte_offset == bank_byte_offset) {
                range.mask = static_cast<uint8_t>(range.mask | mask);
                range.preserve_mask = static_cast<uint8_t>(~range.mask);
                return 0;
            }
        }
        if (input_mask_ranges.size() >= kInputMaskRangeCapacity ||
            input_mask_ranges.size() >= input_mask_ranges.capacity()) {
            return ZMC_ERR_IO_ALIAS_CAPACITY;
        }

        InputMaskRange range = {};
        range.source_byte_offset = source_byte_offset;
        range.bank_byte_offset = bank_byte_offset;
        range.mask = mask;
        range.preserve_mask = static_cast<uint8_t>(~mask);
        input_mask_ranges.push_back(range);
        return 0;
    }

    int add_input_shift_operation(std::vector<InputShiftOperation>* operations,
                                  uint16_t source_byte_offset, uint16_t bank_byte_offset,
                                  uint8_t shift, uint8_t write_mask) {
        // Operations with the same mapping can update their disjoint Bank bits together.
        for (std::size_t i = 0; i < operations->size(); ++i) {
            InputShiftOperation& operation = (*operations)[i];
            if (operation.source_byte_offset == source_byte_offset &&
                operation.bank_byte_offset == bank_byte_offset && operation.shift == shift) {
                operation.write_mask = static_cast<uint8_t>(operation.write_mask | write_mask);
                operation.preserve_mask = static_cast<uint8_t>(~operation.write_mask);
                return 0;
            }
        }
        if (operations->size() >= kInputAliasCapacity ||
            operations->size() >= operations->capacity()) {
            return ZMC_ERR_IO_ALIAS_CAPACITY;
        }

        InputShiftOperation operation = {};
        operation.source_byte_offset = source_byte_offset;
        operation.bank_byte_offset = bank_byte_offset;
        operation.shift = shift;
        operation.write_mask = write_mask;
        operation.preserve_mask = static_cast<uint8_t>(~write_mask);
        operations->push_back(operation);
        return 0;
    }

    int append_input_shift_operations(uint16_t source_bit_offset, uint16_t bank_bit_offset,
                                      uint16_t bit_len) {
        // Compile each destination byte to one left or right net-shift operation.
        uint32_t source_bit = source_bit_offset;
        uint32_t bank_bit = bank_bit_offset;
        uint32_t remaining = bit_len;
        while (remaining > 0u) {
            const uint32_t bank_shift = bank_bit % 8u;
            uint32_t chunk_len = 8u - bank_shift;
            if (chunk_len > remaining) {
                chunk_len = remaining;
            }

            const uint32_t source_shift = source_bit % 8u;
            const uint16_t source_byte = static_cast<uint16_t>(source_bit / 8u);
            const uint16_t bank_byte = static_cast<uint16_t>(bank_bit / 8u);
            const uint8_t write_mask =
                static_cast<uint8_t>(static_cast<uint8_t>((1u << chunk_len) - 1u) << bank_shift);

            int ret = 0;
            if (bank_shift > source_shift) {
                ret = add_input_shift_operation(
                    &input_shift_left_byte_operations, source_byte, bank_byte,
                    static_cast<uint8_t>(bank_shift - source_shift), write_mask);
            } else {
                std::vector<InputShiftOperation>* operations =
                    source_shift + chunk_len > 8u ? &input_shift_right_word_operations
                                                  : &input_shift_right_byte_operations;
                ret = add_input_shift_operation(operations, source_byte, bank_byte,
                                                static_cast<uint8_t>(source_shift - bank_shift),
                                                write_mask);
            }
            if (ret != 0) {
                return ret;
            }

            source_bit += chunk_len;
            bank_bit += chunk_len;
            remaining -= chunk_len;
        }
        return 0;
    }

    int append_input_span_plan(uint16_t source_bit_start, uint16_t bank_bit_start,
                               uint16_t bit_len) {
        // Equal bit phases use mask boundaries and one memcpy body.
        if ((source_bit_start % 8u) != (bank_bit_start % 8u)) {
            return append_input_shift_operations(source_bit_start, bank_bit_start, bit_len);
        }

        uint32_t cursor = 0u;
        const uint32_t bank_start = bank_bit_start;
        const uint32_t source_start = source_bit_start;
        uint32_t leading_len = (8u - (bank_start % 8u)) % 8u;
        if (leading_len > bit_len) {
            leading_len = bit_len;
        }
        if (leading_len > 0u) {
            uint8_t mask = 0u;
            for (uint32_t bit = bank_start; bit < bank_start + leading_len; ++bit) {
                mask = static_cast<uint8_t>(mask | static_cast<uint8_t>(1u << (bit % 8u)));
            }
            const int ret = add_input_mask_range(static_cast<uint16_t>(source_start / 8u),
                                                 static_cast<uint16_t>(bank_start / 8u), mask);
            if (ret != 0) {
                return ret;
            }
            cursor = leading_len;
        }

        const uint32_t full_byte_len = (bit_len - cursor) / 8u;
        if (full_byte_len > 0u) {
            const int ret =
                append_input_copy_range(static_cast<uint16_t>((source_start + cursor) / 8u),
                                        static_cast<uint16_t>((bank_start + cursor) / 8u),
                                        static_cast<uint16_t>(full_byte_len));
            if (ret != 0) {
                return ret;
            }
            cursor += full_byte_len * 8u;
        }

        if (cursor < bit_len) {
            uint8_t mask = 0u;
            for (uint32_t bit = bank_start + cursor; bit < bank_start + bit_len; ++bit) {
                mask = static_cast<uint8_t>(mask | static_cast<uint8_t>(1u << (bit % 8u)));
            }
            return add_input_mask_range(static_cast<uint16_t>((source_start + cursor) / 8u),
                                        static_cast<uint16_t>((bank_start + cursor) / 8u), mask);
        }
        return 0;
    }

    int rebuild_input_plan() {
        // Merge only adjacent one-bit IO numbers whose effective PDO bits are also adjacent.
        input_copy_ranges.clear();
        input_mask_ranges.clear();
        input_shift_left_byte_operations.clear();
        input_shift_right_byte_operations.clear();
        input_shift_right_word_operations.clear();

        bool span_active = false;
        uint16_t source_start = 0u;
        uint32_t source_end = 0u;
        uint16_t bank_start = 0u;
        std::size_t bank_end = 0u;
        for (std::size_t i = 0; i < kIoAliasCapacity; ++i) {
            const Alias& alias = di_aliases[i];
            const bool is_pdo = alias.bit_size == 1u && alias.source_kind == IO_SOURCE_KIND_PDO;
            if (!is_pdo) {
                if (span_active) {
                    const int ret = append_input_span_plan(
                        source_start, bank_start, static_cast<uint16_t>(bank_end - bank_start));
                    if (ret != 0) {
                        return ret;
                    }
                    span_active = false;
                }
                continue;
            }

            const uint32_t alias_start = alias.bit_offset;
            if (span_active && alias_start == source_end && i == bank_end) {
                source_end = alias_start + 1u;
                bank_end = i + 1u;
                continue;
            }
            if (span_active) {
                const int ret = append_input_span_plan(
                    source_start, bank_start, static_cast<uint16_t>(bank_end - bank_start));
                if (ret != 0) {
                    return ret;
                }
            }
            source_start = alias.bit_offset;
            source_end = alias_start + 1u;
            bank_start = static_cast<uint16_t>(i);
            bank_end = i + 1u;
            span_active = true;
        }
        if (span_active) {
            return append_input_span_plan(source_start, bank_start,
                                          static_cast<uint16_t>(bank_end - bank_start));
        }
        return 0;
    }

    template <typename SourceType, typename ShiftPolicy>
    void refresh_input_shift_operations(const std::vector<InputShiftOperation>& operations) {
        // Source width and shift direction are specialized out of the cycle loop.
        for (auto i = 0; i < operations.size(); ++i) {
            const InputShiftOperation& operation = operations[i];
            SourceType source = 0;
            std::memcpy(&source, input + operation.source_byte_offset, sizeof(source));
            const uint8_t shifted =
                static_cast<uint8_t>(ShiftPolicy::apply(source, operation.shift));
            const uint16_t bank_byte = operation.bank_byte_offset;
            value_bank.di_values[bank_byte] =
                static_cast<uint8_t>((value_bank.di_values[bank_byte] & operation.preserve_mask) |
                                     (shifted & operation.write_mask));
        }
    }

    void refresh_input_bank() {
        // this is in hot path! run as quick as possible.
        // The cycle path executes only precompiled copies and fixed net-shift operations.
        if (!input) {
            return;
        }
        for (auto i = 0; i < input_copy_ranges.size(); ++i) {
            const InputCopyRange& range = input_copy_ranges[i];
            std::memcpy(value_bank.di_values + range.bank_byte_offset,
                        input + range.source_byte_offset, range.byte_len);
        }
        for (auto i = 0; i < input_mask_ranges.size(); ++i) {
            const InputMaskRange& range = input_mask_ranges[i];
            const uint16_t bank_byte = range.bank_byte_offset;
            value_bank.di_values[bank_byte] =
                static_cast<uint8_t>((value_bank.di_values[bank_byte] & range.preserve_mask) |
                                     (input[range.source_byte_offset] & range.mask));
        }
        refresh_input_shift_operations<uint8_t, InputShiftLeft>(input_shift_left_byte_operations);
        refresh_input_shift_operations<uint8_t, InputShiftRight>(
            input_shift_right_byte_operations);
        refresh_input_shift_operations<uint16_t, InputShiftRight>(
            input_shift_right_word_operations);
    }

    int commit_input_alias(uint32_t io_number, const Alias& prepared) {
        // Rebuild transactionally so a failed plan never leaves an uncacheable alias behind.
        di_aliases[io_number] = prepared;
        const int ret = rebuild_input_plan();
        if (ret != 0) {
            std::memset(&di_aliases[io_number], 0, sizeof(di_aliases[io_number]));
            (void)rebuild_input_plan();
            return ret;
        }
        write_bank_bit(value_bank.di_values, io_number, false);
        refresh_input_bank();
        return 0;
    }

    int append_alias_plan(const Alias& alias) {
        // Split one PDO field into partial-byte masks and an aligned copy body.
        if (alias.source_kind != IO_SOURCE_KIND_PDO) {
            return 0;
        }

        const uint32_t bit_start = alias.bit_offset;
        const uint32_t bit_end = bit_start + alias.bit_size;
        uint32_t cursor = bit_start;
        uint32_t aligned_start = ((bit_start + 7u) / 8u) * 8u;
        if (aligned_start > bit_end) {
            aligned_start = bit_end;
        }
        if (cursor < aligned_start) {
            const int ret =
                add_mask_bits(&mask_ranges, kOutputMaskRangeCapacity, cursor, aligned_start);
            if (ret != 0) {
                return ret;
            }
            cursor = aligned_start;
        }

        const uint32_t aligned_end = (bit_end / 8u) * 8u;
        if (cursor < aligned_end) {
            const int ret = add_byte_range(&copy_ranges, kOutputAliasCapacity,
                                           static_cast<uint16_t>(cursor / 8u),
                                           static_cast<uint16_t>((aligned_end - cursor) / 8u));
            if (ret != 0) {
                return ret;
            }
            cursor = aligned_end;
        }
        if (cursor < bit_end) {
            return add_mask_bits(&mask_ranges, kOutputMaskRangeCapacity, cursor, bit_end);
        }
        return 0;
    }

    int rebuild_output_plan() {
        // Recompile from registered aliases so failed registration can roll back cleanly.
        copy_ranges.clear();
        mask_ranges.clear();
        if (output_capacity > 0u && !output_shadow) {
            return ZMC_ERR_HOST_MALLOC_FAILED;
        }

        for (std::size_t i = 0; i < kIoAliasCapacity; ++i) {
            if (do_aliases[i].bit_size != 0u) {
                const int ret = append_alias_plan(do_aliases[i]);
                if (ret != 0) {
                    return ret;
                }
            }
        }
        for (std::size_t i = 0; i < kIoAliasCapacity; ++i) {
            if (ao_aliases[i].bit_size != 0u) {
                const int ret = append_alias_plan(ao_aliases[i]);
                if (ret != 0) {
                    return ret;
                }
            }
        }
        return 0;
    }

    int commit_output_alias(uint32_t io_number, Alias* aliases, const Alias& prepared) {
        // Registration immediately takes zero ownership of the alias bit range.
        aliases[io_number] = prepared;
        const int zero_ret = write_bits(output_shadow.get(), output_capacity, prepared.bit_offset,
                                        prepared.bit_size, 0u);
        if (zero_ret != 0) {
            aliases[io_number] = Alias();
            return zero_ret;
        }
        const int ret = rebuild_output_plan();
        if (ret != 0) {
            aliases[io_number] = Alias();
            (void)rebuild_output_plan();
            return ret;
        }
        return 0;
    }

    void clear_do_bank_bit(uint32_t io_number) {
        // Registering an alias takes zero ownership of its cached logical DO bit.
        const uint32_t byte_offset = io_number / 8u;
        const uint8_t mask = static_cast<uint8_t>(1u << (io_number % 8u));
        value_bank.do_values[byte_offset] =
            static_cast<uint8_t>(value_bank.do_values[byte_offset] & static_cast<uint8_t>(~mask));
    }

    void flush_output_shadow() {
        // The real-time path executes only precompiled copy and byte-mask operations.
        if (!output || !output_shadow) {
            return;
        }
        for (std::size_t i = 0; i < copy_ranges.size(); ++i) {
            const ByteRange& range = copy_ranges[i];
            std::memcpy(output + range.byte_offset, output_shadow.get() + range.byte_offset,
                        range.byte_len);
        }
        for (std::size_t i = 0; i < mask_ranges.size(); ++i) {
            const ByteRange& range = mask_ranges[i];
            const uint16_t byte = range.byte_offset;
            output[byte] =
                static_cast<uint8_t>((output[byte] & static_cast<uint8_t>(~range.mask)) |
                                     (output_shadow[byte] & range.mask));
        }
    }
};

IoManager::IoManager() : impl_(new Impl()) {}

IoManager::~IoManager() { delete impl_; }

int IoManager::apply_pio_layout(const ZmcPioOffsetLayout* layout) {
    if (!layout || layout->descriptor_pod.pdo_cell_count > MC_ENI_POD_MAX_PDO_CELLS) {
        return ZMC_ERR_IO_INVALID_ARG;
    }

    const EniDescriptorPod& pod = layout->descriptor_pod;
    std::vector<RtBusPdoCell> cells;
    std::vector<Impl::InputCopyRange> input_copy_ranges;
    std::vector<Impl::InputMaskRange> input_mask_ranges;
    std::vector<Impl::InputShiftOperation> input_shift_left_byte_operations;
    std::vector<Impl::InputShiftOperation> input_shift_right_byte_operations;
    std::vector<Impl::InputShiftOperation> input_shift_right_word_operations;
    std::vector<Impl::ByteRange> copy_ranges;
    std::vector<Impl::ByteRange> mask_ranges;
    try {
        cells.assign(pod.pdo_cells, pod.pdo_cells + pod.pdo_cell_count);
        input_copy_ranges.reserve(kInputAliasCapacity);
        input_mask_ranges.reserve(kInputMaskRangeCapacity);
        input_shift_left_byte_operations.reserve(kInputAliasCapacity);
        input_shift_right_byte_operations.reserve(kInputAliasCapacity);
        input_shift_right_word_operations.reserve(kInputAliasCapacity);
        copy_ranges.reserve(kOutputAliasCapacity);
        mask_ranges.reserve(kOutputMaskRangeCapacity);
    } catch (const std::bad_alloc&) {
        return ZMC_ERR_HOST_MALLOC_FAILED;
    }

    std::unique_ptr<uint8_t[]> output_shadow;
    if (pod.rx_cap > 0u) {
        output_shadow.reset(new (std::nothrow) uint8_t[pod.rx_cap]);
        if (!output_shadow) {
            return ZMC_ERR_HOST_MALLOC_FAILED;
        }
        std::memset(output_shadow.get(), 0, pod.rx_cap);
    }

    reset();
    impl_->input_capacity = pod.tx_cap;
    impl_->output_capacity = pod.rx_cap;
    impl_->output_shadow = std::move(output_shadow);
    impl_->cells.swap(cells);
    impl_->input_copy_ranges.swap(input_copy_ranges);
    impl_->input_mask_ranges.swap(input_mask_ranges);
    impl_->input_shift_left_byte_operations.swap(input_shift_left_byte_operations);
    impl_->input_shift_right_byte_operations.swap(input_shift_right_byte_operations);
    impl_->input_shift_right_word_operations.swap(input_shift_right_word_operations);
    impl_->copy_ranges.swap(copy_ranges);
    impl_->mask_ranges.swap(mask_ranges);
    return 0;
}

void IoManager::bind_process_image(const void* input, void* output) {
    impl_->input = static_cast<const uint8_t*>(input);
    impl_->output = static_cast<uint8_t*>(output);
    impl_->refresh_input_bank();
}

void IoManager::restore_retained_outputs() { impl_->flush_output_shadow(); }

int IoManager::read_input(uint16_t byte_offset, uint8_t byte_len, uint32_t* value) const {
    return read_process_image(impl_->input, impl_->input_capacity, byte_offset, byte_len, value);
}

int IoManager::read_output(uint16_t byte_offset, uint8_t byte_len, uint32_t* value) const {
    return read_process_image(impl_->output, impl_->output_capacity, byte_offset, byte_len, value);
}

int IoManager::write_output(uint16_t byte_offset, uint8_t byte_len, uint32_t value) {
    return write_process_image(impl_->output, impl_->output_capacity, byte_offset, byte_len,
                               value);
}

const RtBusPdoCell* IoManager::find_input(const IoObjectKey& key) const {
    return impl_->find_cell(key, 1u);
}

const RtBusPdoCell* IoManager::find_output(const IoObjectKey& key) const {
    return impl_->find_cell(key, 0u);
}

int IoManager::read_input(const IoObjectKey& key, uint32_t* value) const {
    const RtBusPdoCell* cell = find_input(key);
    if (!cell) {
        return ZMC_ERR_IO_OBJECT_NOT_FOUND;
    }
    return read_bits(impl_->input, impl_->input_capacity, cell->bit_offset, cell->bit_size, value);
}

int IoManager::read_output(const IoObjectKey& key, uint32_t* value) const {
    const RtBusPdoCell* cell = find_output(key);
    if (!cell) {
        return ZMC_ERR_IO_OBJECT_NOT_FOUND;
    }
    return read_bits(impl_->output, impl_->output_capacity, cell->bit_offset, cell->bit_size,
                     value);
}

int IoManager::write_output(const IoObjectKey& key, uint32_t value) {
    const RtBusPdoCell* cell = find_output(key);
    if (!cell) {
        return ZMC_ERR_IO_OBJECT_NOT_FOUND;
    }
    return write_bits(impl_->output, impl_->output_capacity, cell->bit_offset, cell->bit_size,
                      value);
}

int IoManager::register_di_alias(uint32_t io_number, const IoObjectKey& key) {
    const RtBusPdoCell* cell = find_input(key);
    if (!cell) {
        return ZMC_ERR_IO_OBJECT_NOT_FOUND;
    }
    if (cell->bit_size != 1u) {
        return ZMC_ERR_IO_DATA_SIZE_UNSUPPORTED;
    }
    Impl::Alias prepared = {};
    const int ret = impl_->prepare_alias(io_number, cell, impl_->di_aliases, impl_->input_capacity,
                                         0u, false, &prepared);
    if (ret != 0) {
        return ret;
    }
    return impl_->commit_input_alias(io_number, prepared);
}

int IoManager::register_di_alias(uint32_t io_number, const IoObjectKey& key, uint32_t mask) {
    const RtBusPdoCell* cell = find_input(key);
    if (!cell) {
        return ZMC_ERR_IO_OBJECT_NOT_FOUND;
    }
    Impl::Alias prepared = {};
    const int ret = impl_->prepare_alias(io_number, cell, impl_->di_aliases, impl_->input_capacity,
                                         mask, true, &prepared);
    if (ret != 0) {
        return ret;
    }
    return impl_->commit_input_alias(io_number, prepared);
}

int IoManager::register_do_alias(uint32_t io_number, const IoObjectKey& key) {
    const RtBusPdoCell* cell = find_output(key);
    if (!cell) {
        return ZMC_ERR_IO_OBJECT_NOT_FOUND;
    }
    if (cell->bit_size != 1u) {
        return ZMC_ERR_IO_DATA_SIZE_UNSUPPORTED;
    }
    Impl::Alias prepared = {};
    const int ret = impl_->prepare_alias(io_number, cell, impl_->do_aliases,
                                         impl_->output_capacity, 0u, false, &prepared);
    if (ret != 0) {
        return ret;
    }
    if (impl_->overlaps_aliases(impl_->do_aliases, prepared) ||
        impl_->overlaps_aliases(impl_->ao_aliases, prepared)) {
        return ZMC_ERR_IO_ALIAS_DUPLICATE;
    }
    const int commit_ret = impl_->commit_output_alias(io_number, impl_->do_aliases, prepared);
    if (commit_ret == 0) {
        impl_->clear_do_bank_bit(io_number);
    }
    return commit_ret;
}

int IoManager::register_do_alias(uint32_t io_number, const IoObjectKey& key, uint32_t mask) {
    const RtBusPdoCell* cell = find_output(key);
    if (!cell) {
        return ZMC_ERR_IO_OBJECT_NOT_FOUND;
    }
    Impl::Alias prepared = {};
    const int ret = impl_->prepare_alias(io_number, cell, impl_->do_aliases,
                                         impl_->output_capacity, mask, true, &prepared);
    if (ret != 0) {
        return ret;
    }
    if (impl_->overlaps_aliases(impl_->do_aliases, prepared) ||
        impl_->overlaps_aliases(impl_->ao_aliases, prepared)) {
        return ZMC_ERR_IO_ALIAS_DUPLICATE;
    }
    const int commit_ret = impl_->commit_output_alias(io_number, impl_->do_aliases, prepared);
    if (commit_ret == 0) {
        impl_->clear_do_bank_bit(io_number);
    }
    return commit_ret;
}

int IoManager::register_ai_alias(uint32_t io_number, const IoObjectKey& key) {
    const RtBusPdoCell* cell = find_input(key);
    if (!cell) {
        return ZMC_ERR_IO_OBJECT_NOT_FOUND;
    }
    return impl_->register_alias(io_number, cell, impl_->ai_aliases, impl_->input_capacity, 0u,
                                 false);
}

int IoManager::register_ao_alias(uint32_t io_number, const IoObjectKey& key) {
    const RtBusPdoCell* cell = find_output(key);
    if (!cell) {
        return ZMC_ERR_IO_OBJECT_NOT_FOUND;
    }
    Impl::Alias prepared = {};
    const int ret = impl_->prepare_alias(io_number, cell, impl_->ao_aliases,
                                         impl_->output_capacity, 0u, false, &prepared);
    if (ret != 0) {
        return ret;
    }
    if (impl_->overlaps_aliases(impl_->do_aliases, prepared)) {
        return ZMC_ERR_IO_ALIAS_DUPLICATE;
    }
    return impl_->commit_output_alias(io_number, impl_->ao_aliases, prepared);
}

int IoManager::register_virtual_di(uint32_t io_number) {
    const int ret = impl_->register_virtual_alias(io_number, impl_->di_aliases, 1u);
    if (ret == 0) {
        write_bank_bit(impl_->value_bank.di_values, io_number, false);
    }
    return ret;
}

int IoManager::register_virtual_do(uint32_t io_number) {
    const int ret = impl_->register_virtual_alias(io_number, impl_->do_aliases, 1u);
    if (ret == 0) {
        impl_->clear_do_bank_bit(io_number);
    }
    return ret;
}

int IoManager::register_virtual_ai(uint32_t io_number) {
    const int ret = impl_->register_virtual_alias(io_number, impl_->ai_aliases, 32u);
    if (ret == 0) {
        impl_->value_bank.ai_values[io_number] = 0u;
    }
    return ret;
}

int IoManager::register_virtual_ao(uint32_t io_number) {
    const int ret = impl_->register_virtual_alias(io_number, impl_->ao_aliases, 32u);
    if (ret == 0) {
        impl_->value_bank.ao_values[io_number] = 0u;
    }
    return ret;
}

int IoManager::get_alias_metadata(std::vector<IoMetadataEntry>& out) const {
    out.clear();
    impl_->append_alias_metadata(impl_->di_aliases, IO_ALIAS_KIND_DI, out);
    impl_->append_alias_metadata(impl_->do_aliases, IO_ALIAS_KIND_DO, out);
    impl_->append_alias_metadata(impl_->ai_aliases, IO_ALIAS_KIND_AI, out);
    impl_->append_alias_metadata(impl_->ao_aliases, IO_ALIAS_KIND_AO, out);
    return 0;
}

void IoManager::clear_aliases() {
    std::memset(impl_->di_aliases, 0, sizeof(impl_->di_aliases));
    std::memset(impl_->do_aliases, 0, sizeof(impl_->do_aliases));
    std::memset(impl_->ai_aliases, 0, sizeof(impl_->ai_aliases));
    std::memset(impl_->ao_aliases, 0, sizeof(impl_->ao_aliases));
    std::memset(&impl_->value_bank, 0, sizeof(impl_->value_bank));
    impl_->input_copy_ranges.clear();
    impl_->input_mask_ranges.clear();
    impl_->input_shift_left_byte_operations.clear();
    impl_->input_shift_right_byte_operations.clear();
    impl_->input_shift_right_word_operations.clear();
    impl_->copy_ranges.clear();
    impl_->mask_ranges.clear();
    if (impl_->output_capacity > 0u && impl_->output_shadow) {
        std::memset(impl_->output_shadow.get(), 0, impl_->output_capacity);
    }
}

int IoManager::read_di(uint32_t io_number, bool* value) const {
    if (!value) {
        return ZMC_ERR_IO_INVALID_ARG;
    }
    const Impl::Alias* alias = impl_->alias_at(impl_->di_aliases, io_number);
    if (!alias) {
        return ZMC_ERR_IO_OBJECT_NOT_FOUND;
    }
    uint32_t raw_value = 0u;
    if (alias->source_kind == IO_SOURCE_KIND_VIRTUAL) {
        raw_value = read_bank_bit(impl_->value_bank.di_values, io_number) ? 1u : 0u;
    } else {
        const int ret = read_bits(impl_->input, impl_->input_capacity, alias->bit_offset,
                                  alias->bit_size, &raw_value);
        if (ret != 0) {
            return ret;
        }
    }
    *value = raw_value != 0u;
    return 0;
}

int IoManager::read_di_multi(uint32_t start_io_number, uint32_t end_io_number,
                             uint32_t* value) const {
    if (!value || !is_valid_multi_io_range(start_io_number, end_io_number) ||
        end_io_number >= kIoAliasCapacity) {
        return ZMC_ERR_IO_INVALID_ARG;
    }

    const uint32_t bit_count = end_io_number - start_io_number + 1u;
    *value = read_digital_bank_bits(impl_->value_bank.di_values, start_io_number, bit_count);
    return 0;
}

int IoManager::read_do(uint32_t io_number, bool* value) const {
    if (!value) {
        return ZMC_ERR_IO_INVALID_ARG;
    }
    const Impl::Alias* alias = impl_->alias_at(impl_->do_aliases, io_number);
    if (!alias) {
        return ZMC_ERR_IO_OBJECT_NOT_FOUND;
    }
    *value = read_bank_bit(impl_->value_bank.do_values, io_number);
    return 0;
}

int IoManager::read_do_multi(uint32_t start_io_number, uint32_t end_io_number,
                             uint32_t* value) const {
    if (!value || !is_valid_multi_io_range(start_io_number, end_io_number) ||
        end_io_number >= kIoAliasCapacity) {
        return ZMC_ERR_IO_INVALID_ARG;
    }

    const uint32_t bit_count = end_io_number - start_io_number + 1u;
    *value = read_digital_bank_bits(impl_->value_bank.do_values, start_io_number, bit_count);
    return 0;
}

int IoManager::write_do(uint32_t io_number, bool value) {
    const Impl::Alias* alias = impl_->alias_at(impl_->do_aliases, io_number);
    if (!alias) {
        return ZMC_ERR_IO_OBJECT_NOT_FOUND;
    }
    write_bank_bit(impl_->value_bank.do_values, io_number, value);
    if (alias->source_kind == IO_SOURCE_KIND_PDO) {
        write_bank_bit(impl_->output_shadow.get(), alias->bit_offset, value);
    }
    return 0;
}

int IoManager::write_do_multi(uint32_t start_io_number, uint32_t end_io_number, uint32_t value) {
    if (!is_valid_multi_io_range(start_io_number, end_io_number) ||
        end_io_number >= kIoAliasCapacity) {
        return ZMC_ERR_IO_INVALID_ARG;
    }

    const uint32_t bit_count = end_io_number - start_io_number + 1u;
    write_digital_bank_bits(impl_->value_bank.do_values, start_io_number, bit_count, value);
    for (uint32_t io_number = start_io_number; io_number <= end_io_number; ++io_number) {
        const Impl::Alias& alias = impl_->do_aliases[io_number];
        if (alias.bit_size == 1u && alias.source_kind == IO_SOURCE_KIND_PDO) {
            write_bank_bit(impl_->output_shadow.get(), alias.bit_offset,
                           read_bank_bit(impl_->value_bank.do_values, io_number));
        }
    }
    return 0;
}

int IoManager::read_ai(uint32_t io_number, uint32_t* value) const {
    if (!value) {
        return ZMC_ERR_IO_INVALID_ARG;
    }
    const Impl::Alias* alias = impl_->alias_at(impl_->ai_aliases, io_number);
    if (!alias) {
        return ZMC_ERR_IO_OBJECT_NOT_FOUND;
    }
    if (alias->source_kind == IO_SOURCE_KIND_VIRTUAL) {
        *value = impl_->value_bank.ai_values[io_number];
        return 0;
    }
    return read_bits(impl_->input, impl_->input_capacity, alias->bit_offset, alias->bit_size,
                     value);
}

int IoManager::read_ao(uint32_t io_number, uint32_t* value) const {
    if (!value) {
        return ZMC_ERR_IO_INVALID_ARG;
    }
    const Impl::Alias* alias = impl_->alias_at(impl_->ao_aliases, io_number);
    if (!alias) {
        return ZMC_ERR_IO_OBJECT_NOT_FOUND;
    }
    if (alias->source_kind == IO_SOURCE_KIND_VIRTUAL) {
        *value = impl_->value_bank.ao_values[io_number];
        return 0;
    }
    return read_bits(impl_->output_shadow.get(), impl_->output_capacity, alias->bit_offset,
                     alias->bit_size, value);
}

int IoManager::write_ao(uint32_t io_number, uint32_t value) {
    const Impl::Alias* alias = impl_->alias_at(impl_->ao_aliases, io_number);
    if (!alias) {
        return ZMC_ERR_IO_OBJECT_NOT_FOUND;
    }
    if (alias->source_kind == IO_SOURCE_KIND_VIRTUAL) {
        impl_->value_bank.ao_values[io_number] = value;
        return 0;
    }
    return write_bits(impl_->output_shadow.get(), impl_->output_capacity, alias->bit_offset,
                      alias->bit_size, value);
}

int IoManager::write_virtual_di(uint32_t io_number, bool value) {
    const Impl::Alias* alias = impl_->alias_at(impl_->di_aliases, io_number);
    if (!alias || alias->source_kind != IO_SOURCE_KIND_VIRTUAL) {
        return ZMC_ERR_IO_OBJECT_NOT_FOUND;
    }
    write_bank_bit(impl_->value_bank.di_values, io_number, value);
    return 0;
}

int IoManager::write_virtual_ai(uint32_t io_number, uint32_t value) {
    const Impl::Alias* alias = impl_->alias_at(impl_->ai_aliases, io_number);
    if (!alias || alias->source_kind != IO_SOURCE_KIND_VIRTUAL) {
        return ZMC_ERR_IO_OBJECT_NOT_FOUND;
    }
    impl_->value_bank.ai_values[io_number] = value;
    return 0;
}

void IoManager::reset() {
    clear_aliases();
    std::vector<RtBusPdoCell>().swap(impl_->cells);
    std::vector<Impl::InputCopyRange>().swap(impl_->input_copy_ranges);
    std::vector<Impl::InputMaskRange>().swap(impl_->input_mask_ranges);
    std::vector<Impl::InputShiftOperation>().swap(impl_->input_shift_left_byte_operations);
    std::vector<Impl::InputShiftOperation>().swap(impl_->input_shift_right_byte_operations);
    std::vector<Impl::InputShiftOperation>().swap(impl_->input_shift_right_word_operations);
    std::vector<Impl::ByteRange>().swap(impl_->copy_ranges);
    std::vector<Impl::ByteRange>().swap(impl_->mask_ranges);
    impl_->input_capacity = 0;
    impl_->output_capacity = 0;
    impl_->output_shadow.reset();
    impl_->input = nullptr;
    impl_->output = nullptr;
}

}  // namespace zmc
