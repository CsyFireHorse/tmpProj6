#include <gtest/gtest.h>

#include <cstring>

#include "bridge/AxisFbBridge.h"
#include "common/zmc/fbt_errorcode_def.h"
#include "io/IoManager.hpp"

namespace {

ZmcPioOffsetLayout make_io_layout() {
    ZmcPioOffsetLayout layout;
    std::memset(&layout, 0, sizeof(layout));
    layout.slot_count = 1;
    layout.descriptor_pod.tx_cap = 8;
    layout.descriptor_pod.rx_cap = 16;
    layout.descriptor_pod.slave_count = 3;
    layout.descriptor_pod.pdo_cell_count = 6;

    RtBusPdoCell& input_u8 = layout.descriptor_pod.pdo_cells[0];
    input_u8.index = 0x6000;
    input_u8.bit_offset = 8; /* byte 1 */
    input_u8.sub_index = 1;
    input_u8.bit_size = 8;
    input_u8.is_input = 1;
    input_u8.slave_idx = 2;

    RtBusPdoCell& input_u16 = layout.descriptor_pod.pdo_cells[1];
    input_u16.index = 0x6010;
    input_u16.bit_offset = 16; /* byte 2 */
    input_u16.sub_index = 2;
    input_u16.bit_size = 16;
    input_u16.is_input = 1;
    input_u16.slave_idx = 2;

    RtBusPdoCell& output_bit = layout.descriptor_pod.pdo_cells[2];
    output_bit.index = 0x7000;
    output_bit.bit_offset = 32; /* byte 4, bit 0 */
    output_bit.sub_index = 1;
    output_bit.bit_size = 1;
    output_bit.is_input = 0;
    output_bit.slave_idx = 2;

    RtBusPdoCell& output_u32 = layout.descriptor_pod.pdo_cells[3];
    output_u32.index = 0x7010;
    output_u32.bit_offset = 64; /* byte 8 */
    output_u32.sub_index = 2;
    output_u32.bit_size = 32;
    output_u32.is_input = 0;
    output_u32.slave_idx = 2;

    RtBusPdoCell& digital_output_u32 = layout.descriptor_pod.pdo_cells[4];
    digital_output_u32.index = 0x7020;
    digital_output_u32.bit_offset = 96; /* byte 12 */
    digital_output_u32.sub_index = 3;
    digital_output_u32.bit_size = 32;
    digital_output_u32.is_input = 0;
    digital_output_u32.slave_idx = 2;

    RtBusPdoCell& overlapping_output_u16 = layout.descriptor_pod.pdo_cells[5];
    overlapping_output_u16.index = 0x7030;
    overlapping_output_u16.bit_offset = 112; /* byte 14 */
    overlapping_output_u16.sub_index = 4;
    overlapping_output_u16.bit_size = 16;
    overlapping_output_u16.is_input = 0;
    overlapping_output_u16.slave_idx = 2;
    return layout;
}

const uint32_t kMultiIoCount = 32u;
const uint32_t kMultiIoStart = 40u;

ZmcPioOffsetLayout make_multi_io_layout() {
    ZmcPioOffsetLayout layout;
    std::memset(&layout, 0, sizeof(layout));
    layout.slot_count = 1;
    layout.descriptor_pod.tx_cap = kMultiIoCount;
    layout.descriptor_pod.rx_cap = kMultiIoCount;
    layout.descriptor_pod.slave_count = 1;
    layout.descriptor_pod.pdo_cell_count = kMultiIoCount * 2u;

    for (uint16_t i = 0; i < kMultiIoCount; ++i) {
        RtBusPdoCell& input = layout.descriptor_pod.pdo_cells[i];
        input.index = static_cast<uint16_t>(0x6100u + i);
        input.bit_offset = static_cast<uint16_t>(i * 8u);
        input.sub_index = 1;
        input.bit_size = 8;
        input.is_input = 1;
        input.slave_idx = 0;

        RtBusPdoCell& output = layout.descriptor_pod.pdo_cells[kMultiIoCount + i];
        output.index = static_cast<uint16_t>(0x7100u + i);
        output.bit_offset = static_cast<uint16_t>(i * 8u);
        output.sub_index = 1;
        output.bit_size = 1;
        output.is_input = 0;
        output.slave_idx = 0;
    }
    return layout;
}

zmc::IoObjectKey multi_input_key(uint32_t index) {
    return zmc::IoObjectKey{0, static_cast<uint16_t>(0x6100u + index), 1};
}

zmc::IoObjectKey multi_output_key(uint32_t index) {
    return zmc::IoObjectKey{0, static_cast<uint16_t>(0x7100u + index), 1};
}

ZmcPioOffsetLayout make_di_bank_layout() {
    ZmcPioOffsetLayout layout;
    std::memset(&layout, 0, sizeof(layout));
    layout.slot_count = 1;
    layout.descriptor_pod.tx_cap = 8u;
    layout.descriptor_pod.slave_count = 1u;
    layout.descriptor_pod.pdo_cell_count = 40u;

    for (uint16_t i = 0; i < 16u; ++i) {
        RtBusPdoCell& input = layout.descriptor_pod.pdo_cells[i];
        input.index = static_cast<uint16_t>(0x6200u + i);
        input.bit_offset = static_cast<uint16_t>(3u + i);
        input.sub_index = 1u;
        input.bit_size = 1u;
        input.is_input = 1u;
        input.slave_idx = 0u;
    }
    for (uint16_t i = 0; i < 20u; ++i) {
        RtBusPdoCell& input = layout.descriptor_pod.pdo_cells[16u + i];
        input.index = static_cast<uint16_t>(0x6210u + i);
        input.bit_offset = static_cast<uint16_t>(32u + i);
        input.sub_index = 1u;
        input.bit_size = 1u;
        input.is_input = 1u;
        input.slave_idx = 0u;
    }

    RtBusPdoCell& full_object = layout.descriptor_pod.pdo_cells[36u];
    full_object.index = 0x6224u;
    full_object.bit_offset = 52u;
    full_object.sub_index = 1u;
    full_object.bit_size = 5u;
    full_object.is_input = 1u;
    full_object.slave_idx = 0u;

    const uint16_t split_offsets[] = {57u, 59u, 63u};
    for (uint16_t i = 0; i < 3u; ++i) {
        RtBusPdoCell& input = layout.descriptor_pod.pdo_cells[37u + i];
        input.index = static_cast<uint16_t>(0x6225u + i);
        input.bit_offset = split_offsets[i];
        input.sub_index = 1u;
        input.bit_size = 1u;
        input.is_input = 1u;
        input.slave_idx = 0u;
    }
    return layout;
}

zmc::IoObjectKey di_bank_input_key(uint32_t index) {
    return zmc::IoObjectKey{0u, static_cast<uint16_t>(0x6200u + index), 1u};
}

void set_image_bit(uint8_t* image, uint16_t bit_offset, bool value) {
    const uint16_t byte_offset = static_cast<uint16_t>(bit_offset / 8u);
    const uint8_t mask = static_cast<uint8_t>(1u << (bit_offset % 8u));
    if (value) {
        image[byte_offset] = static_cast<uint8_t>(image[byte_offset] | mask);
    } else {
        image[byte_offset] =
            static_cast<uint8_t>(image[byte_offset] & static_cast<uint8_t>(~mask));
    }
}

}  // namespace

TEST(IoManagerOffsetAccess, ReadsAndWritesProcessImageOffsets) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_io_layout();
    uint8_t input[8] = {0};
    uint8_t output[16] = {0};
    input[2] = 0x34;
    input[3] = 0x12;

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    manager.bind_process_image(input, output);

    uint32_t value = 0;
    EXPECT_EQ(0, manager.read_input(2u, 2u, &value));
    EXPECT_EQ(0x1234u, value);
    EXPECT_EQ(0, manager.write_output(8u, 4u, 0x78563412u));
    EXPECT_EQ(0, manager.read_output(8u, 4u, &value));
    EXPECT_EQ(0x78563412u, value);
}

TEST(IoManagerDirectAccess, ReadsAndWritesConcretePdoObjects) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_io_layout();
    uint8_t input[8] = {0};
    uint8_t output[16] = {0};
    input[1] = 0x5A;
    input[2] = 0x34;
    input[3] = 0x12;

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    manager.bind_process_image(input, output);

    uint32_t value = 0;
    EXPECT_EQ(0, manager.read_input(zmc::IoObjectKey{2, 0x6000, 1}, &value));
    EXPECT_EQ(0x5Au, value);
    EXPECT_EQ(0, manager.read_input(zmc::IoObjectKey{2, 0x6010, 2}, &value));
    EXPECT_EQ(0x1234u, value);

    EXPECT_EQ(0, manager.write_output(zmc::IoObjectKey{2, 0x7010, 2}, 0x78563412u));
    EXPECT_EQ(0x12u, output[8]);
    EXPECT_EQ(0x34u, output[9]);
    EXPECT_EQ(0x56u, output[10]);
    EXPECT_EQ(0x78u, output[11]);
    EXPECT_EQ(0, manager.read_output(zmc::IoObjectKey{2, 0x7010, 2}, &value));
    EXPECT_EQ(0x78563412u, value);
}

TEST(IoManagerAliasAccess, SupportsFourIndependentAliasGroupsWithSameNumber) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_io_layout();
    uint8_t input[8] = {0};
    uint8_t output[16] = {0};
    input[1] = 2;
    input[2] = 0x34;
    input[3] = 0x12;

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    manager.bind_process_image(input, output);
    ASSERT_EQ(0, manager.register_di_alias(32, zmc::IoObjectKey{2, 0x6000, 1}, 0x02u));
    ASSERT_EQ(0, manager.register_do_alias(32, zmc::IoObjectKey{2, 0x7000, 1}));
    ASSERT_EQ(0, manager.register_ai_alias(32, zmc::IoObjectKey{2, 0x6010, 2}));
    ASSERT_EQ(0, manager.register_ao_alias(32, zmc::IoObjectKey{2, 0x7010, 2}));

    bool digital_value = false;
    EXPECT_EQ(0, manager.read_di(32, &digital_value));
    EXPECT_TRUE(digital_value);
    EXPECT_EQ(0, manager.write_do(32, true));
    EXPECT_EQ(0u, output[4]);
    manager.restore_retained_outputs();
    EXPECT_EQ(1u, output[4]);
    EXPECT_EQ(0, manager.write_do(32, false));
    EXPECT_EQ(1u, output[4]);
    manager.restore_retained_outputs();
    EXPECT_EQ(0u, output[4]);

    uint32_t analog_value = 0;
    EXPECT_EQ(0, manager.read_ai(32, &analog_value));
    EXPECT_EQ(0x1234u, analog_value);
    EXPECT_EQ(0, manager.write_ao(32, 0x78563412u));
    manager.restore_retained_outputs();
    EXPECT_EQ(0x12u, output[8]);
    EXPECT_EQ(0x34u, output[9]);
    EXPECT_EQ(0x56u, output[10]);
    EXPECT_EQ(0x78u, output[11]);
    EXPECT_EQ(0, manager.read_ao(32, &analog_value));
    EXPECT_EQ(0x78563412u, analog_value);

    EXPECT_EQ(ZMC_ERR_IO_DATA_SIZE_UNSUPPORTED,
              manager.register_do_alias(33, zmc::IoObjectKey{2, 0x7020, 3}));

    EXPECT_EQ(ZMC_ERR_IO_ALIAS_DUPLICATE,
              manager.register_di_alias(32, zmc::IoObjectKey{2, 0x6010, 2}, 0x01u));
    EXPECT_EQ(ZMC_ERR_IO_DATA_SIZE_UNSUPPORTED,
              manager.register_do_alias(32, zmc::IoObjectKey{2, 0x7010, 2}));
    EXPECT_EQ(ZMC_ERR_IO_ALIAS_DUPLICATE,
              manager.register_ai_alias(32, zmc::IoObjectKey{2, 0x6000, 1}));
    EXPECT_EQ(ZMC_ERR_IO_ALIAS_DUPLICATE,
              manager.register_ao_alias(32, zmc::IoObjectKey{2, 0x7000, 1}));

    const uint32_t alias_capacity = MC_IO_MAX_COUNT;
    EXPECT_EQ(ZMC_ERR_IO_ALIAS_CAPACITY,
              manager.register_di_alias(alias_capacity, zmc::IoObjectKey{2, 0x6000, 1},
                                        0x01u));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, manager.read_di(alias_capacity, &digital_value));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, manager.read_ai(31, &analog_value));
}

TEST(IoManagerAliasMetadata, ReturnsResolvedCellsInStableGroupAndNumberOrder) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_io_layout();
    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    ASSERT_EQ(0, manager.register_di_alias(7, zmc::IoObjectKey{2, 0x6010, 2}, 0x01u));
    ASSERT_EQ(0, manager.register_di_alias(2, zmc::IoObjectKey{2, 0x6000, 1}, 0x02u));
    ASSERT_EQ(0, manager.register_do_alias(32, zmc::IoObjectKey{2, 0x7000, 1}));
    ASSERT_EQ(0, manager.register_ai_alias(32, zmc::IoObjectKey{2, 0x6010, 2}));
    ASSERT_EQ(0, manager.register_ao_alias(32, zmc::IoObjectKey{2, 0x7010, 2}));

    std::vector<zmc::IoMetadataEntry> metadata;
    ASSERT_EQ(0, manager.get_alias_metadata(metadata));
    ASSERT_EQ(5u, metadata.size());

    EXPECT_EQ(zmc::IO_ALIAS_KIND_DI, metadata[0].kind);
    EXPECT_EQ(zmc::IO_SOURCE_KIND_PDO, metadata[0].source_kind);
    EXPECT_EQ(2u, metadata[0].io_number);
    EXPECT_EQ(0x02u, metadata[0].mask);
    EXPECT_EQ(2u, metadata[0].slave_idx);
    EXPECT_EQ(0x6000u, metadata[0].index);
    EXPECT_EQ(1u, metadata[0].subindex);
    EXPECT_EQ(8u, metadata[0].bit_offset);
    EXPECT_EQ(8u, metadata[0].bit_size);
    EXPECT_TRUE(metadata[0].is_input);

    EXPECT_EQ(zmc::IO_ALIAS_KIND_DI, metadata[1].kind);
    EXPECT_EQ(7u, metadata[1].io_number);
    EXPECT_EQ(0x01u, metadata[1].mask);
    EXPECT_EQ(0x6010u, metadata[1].index);
    EXPECT_EQ(16u, metadata[1].bit_offset);
    EXPECT_EQ(16u, metadata[1].bit_size);
    EXPECT_TRUE(metadata[1].is_input);

    EXPECT_EQ(zmc::IO_ALIAS_KIND_DO, metadata[2].kind);
    EXPECT_EQ(32u, metadata[2].io_number);
    EXPECT_EQ(0u, metadata[2].mask);
    EXPECT_EQ(0x7000u, metadata[2].index);
    EXPECT_FALSE(metadata[2].is_input);

    EXPECT_EQ(zmc::IO_ALIAS_KIND_AI, metadata[3].kind);
    EXPECT_EQ(32u, metadata[3].io_number);
    EXPECT_EQ(0u, metadata[3].mask);
    EXPECT_EQ(0x6010u, metadata[3].index);
    EXPECT_TRUE(metadata[3].is_input);

    EXPECT_EQ(zmc::IO_ALIAS_KIND_AO, metadata[4].kind);
    EXPECT_EQ(zmc::IO_SOURCE_KIND_PDO, metadata[4].source_kind);
    EXPECT_EQ(32u, metadata[4].io_number);
    EXPECT_EQ(0u, metadata[4].mask);
    EXPECT_EQ(0x7010u, metadata[4].index);
    EXPECT_EQ(2u, metadata[4].subindex);
    EXPECT_EQ(64u, metadata[4].bit_offset);
    EXPECT_EQ(32u, metadata[4].bit_size);
    EXPECT_FALSE(metadata[4].is_input);

    manager.clear_aliases();
    ASSERT_EQ(0, manager.get_alias_metadata(metadata));
    EXPECT_TRUE(metadata.empty());

    ASSERT_EQ(0, manager.register_di_alias(2, zmc::IoObjectKey{2, 0x6000, 1}, 0x01u));
    manager.reset();
    ASSERT_EQ(0, manager.get_alias_metadata(metadata));
    EXPECT_TRUE(metadata.empty());
}

TEST(IoManagerAliasRegistration, RejectsUnmaskedAndAcceptsMaskedMultiBitDigitalOutputs) {
    ZmcPioOffsetLayout layout = make_io_layout();
    uint8_t input[8] = {0};
    uint8_t output[16] = {0};

    zmc::IoManager manager;
    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    manager.bind_process_image(input, output);
    EXPECT_EQ(ZMC_ERR_IO_DATA_SIZE_UNSUPPORTED,
              manager.register_do_alias(1, zmc::IoObjectKey{2, 0x7020, 3}));
    ASSERT_EQ(0, manager.register_do_alias(2, zmc::IoObjectKey{2, 0x7020, 3}, 0x01u));
    ASSERT_EQ(0, manager.write_do(2, true));
    manager.restore_retained_outputs();
    EXPECT_EQ(1u, output[12] & 0x01u);

    zmc::IoManager same_object;
    ASSERT_EQ(0, same_object.apply_pio_layout(&layout));
    ASSERT_EQ(0, same_object.register_do_alias(1, zmc::IoObjectKey{2, 0x7000, 1}));
    EXPECT_EQ(ZMC_ERR_IO_ALIAS_DUPLICATE,
              same_object.register_do_alias(2, zmc::IoObjectKey{2, 0x7000, 1}));
}

TEST(IoManagerAliasMultiAccess, PacksAndWritesThirtyTwoAliasesFromNonzeroStart) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_multi_io_layout();
    uint8_t input[kMultiIoCount] = {0};
    uint8_t output[kMultiIoCount] = {0};
    const uint32_t input_pattern = 0xA5C39E71u;
    const uint32_t output_pattern = 0x5A3C618Eu;
    for (uint32_t bit = 0; bit < kMultiIoCount; ++bit) {
        input[bit] = ((input_pattern >> bit) & 1u) != 0u ? 2u : 0u;
    }

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    manager.bind_process_image(input, output);
    for (uint32_t bit = 0; bit < kMultiIoCount; ++bit) {
        ASSERT_EQ(0,
                  manager.register_di_alias(kMultiIoStart + bit, multi_input_key(bit), 0x02u));
        ASSERT_EQ(0, manager.register_do_alias(kMultiIoStart + bit, multi_output_key(bit)));
    }

    uint32_t value = 0;
    EXPECT_EQ(0, manager.read_di_multi(kMultiIoStart, kMultiIoStart + 31u, &value));
    EXPECT_EQ(input_pattern, value);
    EXPECT_EQ(0, manager.write_do_multi(kMultiIoStart, kMultiIoStart + 31u, output_pattern));
    value = 0;
    EXPECT_EQ(0, manager.read_do_multi(kMultiIoStart, kMultiIoStart + 31u, &value));
    EXPECT_EQ(output_pattern, value);
    for (uint32_t bit = 0; bit < kMultiIoCount; ++bit) {
        EXPECT_EQ(0u, output[bit]);
    }
    manager.restore_retained_outputs();
    for (uint32_t bit = 0; bit < kMultiIoCount; ++bit) {
        EXPECT_EQ((output_pattern >> bit) & 1u, output[bit]);
    }

    uint8_t rebound_output[kMultiIoCount];
    std::memset(rebound_output, 0xA5, sizeof(rebound_output));
    manager.bind_process_image(input, rebound_output);
    manager.restore_retained_outputs();
    value = 0;
    EXPECT_EQ(0, manager.read_do_multi(kMultiIoStart, kMultiIoStart + 31u, &value));
    EXPECT_EQ(output_pattern, value);
    for (uint32_t bit = 0; bit < kMultiIoCount; ++bit) {
        EXPECT_EQ(static_cast<uint8_t>(0xA4u | ((output_pattern >> bit) & 1u)),
                  rebound_output[bit]);
    }
}

TEST(IoManagerAliasMultiAccess, SkipsMissingAliasesAndIgnoresUnusedHighBits) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_multi_io_layout();
    uint8_t input[kMultiIoCount] = {0};
    uint8_t output[kMultiIoCount];
    input[2] = 1u;
    input[7] = 2u;
    std::memset(output, 0xA5, sizeof(output));

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    manager.bind_process_image(input, output);
    ASSERT_EQ(0,
              manager.register_di_alias(kMultiIoStart + 2u, multi_input_key(2u), 0x01u));
    ASSERT_EQ(0,
              manager.register_di_alias(kMultiIoStart + 7u, multi_input_key(7u), 0x02u));
    ASSERT_EQ(0, manager.register_do_alias(kMultiIoStart + 2u, multi_output_key(2u)));
    ASSERT_EQ(0, manager.register_do_alias(kMultiIoStart + 7u, multi_output_key(7u)));

    uint32_t value = 0;
    EXPECT_EQ(0, manager.read_di_multi(kMultiIoStart, kMultiIoStart + 7u, &value));
    EXPECT_EQ((1u << 2) | (1u << 7), value);
    EXPECT_EQ(0, manager.write_do_multi(kMultiIoStart, kMultiIoStart + 7u, 0xFFFFFF80u));
    manager.restore_retained_outputs();
    EXPECT_EQ(0xA4u, output[2]);
    EXPECT_EQ(0xA5u, output[7]);
    EXPECT_EQ(0xA5u, output[3]);
    value = 0;
    EXPECT_EQ(0, manager.read_do_multi(kMultiIoStart, kMultiIoStart + 7u, &value));
    EXPECT_EQ(1u << 7, value);

    uint8_t rebound_output[kMultiIoCount];
    std::memset(rebound_output, 0x5A, sizeof(rebound_output));
    manager.bind_process_image(input, rebound_output);
    manager.restore_retained_outputs();
    EXPECT_EQ(0x5Au, rebound_output[2]);
    EXPECT_EQ(0x5Bu, rebound_output[7]);
    EXPECT_EQ(0x5Au, rebound_output[3]);
    manager.bind_process_image(input, output);

    value = 0;
    EXPECT_EQ(0, manager.read_di_multi(kMultiIoStart + 7u, kMultiIoStart + 7u, &value));
    EXPECT_EQ(1u, value);
    EXPECT_EQ(0, manager.write_do_multi(kMultiIoStart + 7u, kMultiIoStart + 7u, 0xFFFFFFFEu));
    manager.restore_retained_outputs();
    EXPECT_EQ(0xA4u, output[7]);
    bool output_value = true;
    EXPECT_EQ(0, manager.read_do(kMultiIoStart + 7u, &output_value));
    EXPECT_FALSE(output_value);
}

TEST(IoManagerAliasMultiErrors, ReadsEmptyBankAndReportsInvalidRanges) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_multi_io_layout();
    uint8_t input[kMultiIoCount] = {0};
    uint8_t output[kMultiIoCount];
    uint8_t expected_output[kMultiIoCount];
    std::memset(output, 0x5A, sizeof(output));
    std::memcpy(expected_output, output, sizeof(output));
    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    manager.bind_process_image(input, output);

    uint32_t value = 0x12345678u;
    EXPECT_EQ(0, manager.read_di_multi(kMultiIoStart, kMultiIoStart + 31u, &value));
    EXPECT_EQ(0u, value);
    value = 0x12345678u;
    EXPECT_EQ(0, manager.read_do_multi(kMultiIoStart, kMultiIoStart + 31u, &value));
    EXPECT_EQ(0u, value);
    EXPECT_EQ(0, manager.write_do_multi(kMultiIoStart, kMultiIoStart + 31u, 0xFFFFFFFFu));
    EXPECT_EQ(0, manager.read_do_multi(kMultiIoStart, kMultiIoStart + 31u, &value));
    EXPECT_EQ(0xFFFFFFFFu, value);
    EXPECT_EQ(0, std::memcmp(expected_output, output, sizeof(output)));

    EXPECT_EQ(ZMC_ERR_IO_INVALID_ARG, manager.read_di_multi(5u, 4u, &value));
    EXPECT_EQ(ZMC_ERR_IO_INVALID_ARG, manager.read_di_multi(5u, 37u, &value));
    EXPECT_EQ(ZMC_ERR_IO_INVALID_ARG, manager.read_di_multi(5u, 5u, nullptr));
    EXPECT_EQ(ZMC_ERR_IO_INVALID_ARG,
              manager.read_di_multi(MC_IO_MAX_COUNT - 1u, MC_IO_MAX_COUNT, &value));
    EXPECT_EQ(ZMC_ERR_IO_INVALID_ARG, manager.read_do_multi(5u, 4u, &value));
    EXPECT_EQ(ZMC_ERR_IO_INVALID_ARG, manager.read_do_multi(5u, 37u, &value));
    EXPECT_EQ(ZMC_ERR_IO_INVALID_ARG, manager.read_do_multi(5u, 5u, nullptr));
    EXPECT_EQ(ZMC_ERR_IO_INVALID_ARG, manager.write_do_multi(5u, 4u, 0u));
    EXPECT_EQ(ZMC_ERR_IO_INVALID_ARG, manager.write_do_multi(5u, 37u, 0u));

    input[0] = 1u;
    ASSERT_EQ(0, manager.register_di_alias(kMultiIoStart, multi_input_key(0u), 0x01u));
    ASSERT_EQ(0, manager.register_do_alias(kMultiIoStart, multi_output_key(0u)));
    uint8_t rebound_output[kMultiIoCount];
    std::memset(rebound_output, 0x3C, sizeof(rebound_output));
    manager.bind_process_image(input, rebound_output);
    EXPECT_EQ(0x3Cu, rebound_output[0]);
    manager.bind_process_image(nullptr, nullptr);
    EXPECT_EQ(0, manager.read_di_multi(kMultiIoStart, kMultiIoStart, &value));
    EXPECT_EQ(1u, value);
    EXPECT_EQ(0, manager.read_do_multi(kMultiIoStart, kMultiIoStart, &value));
    EXPECT_EQ(0u, value);
    EXPECT_EQ(0, manager.write_do_multi(kMultiIoStart, kMultiIoStart, 1u));
    EXPECT_EQ(0, manager.read_do_multi(kMultiIoStart, kMultiIoStart, &value));
    EXPECT_EQ(1u, value);
    manager.reset();
    EXPECT_EQ(0, manager.read_di_multi(kMultiIoStart, kMultiIoStart, &value));
    EXPECT_EQ(0u, value);
}

TEST(IoManagerDiBank, CachesSamePhaseCopyAndMaskedBoundaries) {
    // IO 11..26 and PDO bits 3..18 share a phase: leading mask, memcpy, trailing mask.
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_di_bank_layout();
    uint8_t input[8] = {0u};
    uint8_t output[1] = {0u};
    const uint32_t input_pattern = 0xA55Au;
    for (uint16_t bit = 0u; bit < 16u; ++bit) {
        set_image_bit(input, static_cast<uint16_t>(3u + bit),
                      ((input_pattern >> bit) & 1u) != 0u);
    }

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    ASSERT_EQ(0, manager.register_virtual_di(10u));
    ASSERT_EQ(0, manager.register_virtual_di(27u));
    ASSERT_EQ(0, manager.write_virtual_di(10u, true));
    ASSERT_EQ(0, manager.write_virtual_di(27u, true));
    for (uint32_t bit = 0u; bit < 16u; ++bit) {
        ASSERT_EQ(0, manager.register_di_alias(11u + bit, di_bank_input_key(bit)));
    }
    manager.bind_process_image(input, output);

    for (uint16_t bit = 0u; bit < 16u; ++bit) {
        set_image_bit(input, static_cast<uint16_t>(3u + bit),
                      ((input_pattern >> bit) & 1u) == 0u);
    }
    uint32_t multi_value = 0u;
    ASSERT_EQ(0, manager.read_di_multi(10u, 27u, &multi_value));
    EXPECT_EQ(1u | (input_pattern << 1u) | (1u << 17u), multi_value);

    bool value = false;
    ASSERT_EQ(0, manager.read_di(11u, &value));
    EXPECT_TRUE(value);

    // Rebinding replaces only the PDO bits and preserves neighboring virtual values.
    manager.bind_process_image(input, output);
    ASSERT_EQ(0, manager.read_di_multi(10u, 27u, &multi_value));
    EXPECT_EQ(1u | (((~input_pattern) & 0xffffu) << 1u) | (1u << 17u), multi_value);
}

TEST(IoManagerDiBank, CachesShiftedRunsAndRejectsMultiBitObjects) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_di_bank_layout();
    uint8_t input[8] = {0u};
    uint8_t output[1] = {0u};
    const uint32_t shifted_pattern = 0xDB35Au;
    for (uint16_t bit = 0u; bit < 20u; ++bit) {
        set_image_bit(input, static_cast<uint16_t>(32u + bit),
                      ((shifted_pattern >> bit) & 1u) != 0u);
    }
    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    for (uint32_t bit = 0u; bit < 20u; ++bit) {
        ASSERT_EQ(0, manager.register_di_alias(34u + bit, di_bank_input_key(16u + bit)));
    }
    EXPECT_EQ(ZMC_ERR_IO_DATA_SIZE_UNSUPPORTED,
              manager.register_di_alias(70u, di_bank_input_key(36u)));
    manager.bind_process_image(input, output);

    std::memset(input, 0, sizeof(input));
    uint32_t multi_value = 0u;
    ASSERT_EQ(0, manager.read_di_multi(34u, 53u, &multi_value));
    EXPECT_EQ(shifted_pattern, multi_value);
    bool value = false;
    ASSERT_EQ(0, manager.read_di(34u, &value));
    EXPECT_FALSE(value);

    manager.bind_process_image(input, output);
    ASSERT_EQ(0, manager.read_di_multi(34u, 53u, &multi_value));
    EXPECT_EQ(0u, multi_value);
}

TEST(IoManagerDiBank, CompilesContinuousOneHotAliasesIntoThePackedBank) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_io_layout();
    uint8_t input[8] = {0u, 0xA5u, 0u, 0u, 0u, 0u, 0u, 0u};
    uint8_t output[16] = {0u};
    const zmc::IoObjectKey input_key{2u, 0x6000u, 1u};

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    for (uint32_t bit = 0u; bit < 8u; ++bit) {
        ASSERT_EQ(0, manager.register_di_alias(16u + bit, input_key, 1u << bit));
    }
    manager.bind_process_image(input, output);
    input[1] = 0u;

    uint32_t multi_value = 0u;
    ASSERT_EQ(0, manager.read_di_multi(16u, 23u, &multi_value));
    EXPECT_EQ(0xA5u, multi_value);
    bool direct_value = true;
    ASSERT_EQ(0, manager.read_di(16u, &direct_value));
    EXPECT_FALSE(direct_value);
}

TEST(IoManagerDiBank, SplitsPhysicalAndLogicalGapsAndPreservesErrorLifecycle) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_di_bank_layout();
    uint8_t input[8] = {0u};
    uint8_t output[1] = {0u};
    set_image_bit(input, 57u, true);
    set_image_bit(input, 63u, true);

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    manager.bind_process_image(input, output);
    ASSERT_EQ(0, manager.register_di_alias(80u, di_bank_input_key(37u)));
    ASSERT_EQ(0, manager.register_di_alias(81u, di_bank_input_key(38u)));
    ASSERT_EQ(0, manager.register_virtual_di(82u));
    ASSERT_EQ(0, manager.write_virtual_di(82u, true));
    ASSERT_EQ(0, manager.register_di_alias(84u, di_bank_input_key(39u)));

    uint32_t multi_value = 0u;
    ASSERT_EQ(0, manager.read_di_multi(80u, 84u, &multi_value));
    EXPECT_EQ((1u << 0u) | (1u << 2u) | (1u << 4u), multi_value);

    std::memset(input, 0, sizeof(input));
    bool direct_value = true;
    ASSERT_EQ(0, manager.read_di(80u, &direct_value));
    EXPECT_FALSE(direct_value);
    ASSERT_EQ(0, manager.read_di_multi(80u, 84u, &multi_value));
    EXPECT_EQ((1u << 0u) | (1u << 2u) | (1u << 4u), multi_value);

    manager.bind_process_image(nullptr, output);
    ASSERT_EQ(0, manager.read_di_multi(80u, 84u, &multi_value));
    EXPECT_EQ((1u << 0u) | (1u << 2u) | (1u << 4u), multi_value);
    ASSERT_EQ(0, manager.read_di_multi(82u, 82u, &multi_value));
    EXPECT_EQ(1u, multi_value);

    manager.clear_aliases();
    ASSERT_EQ(0, manager.read_di_multi(80u, 84u, &multi_value));
    EXPECT_EQ(0u, multi_value);
    ASSERT_EQ(0, manager.register_di_alias(80u, di_bank_input_key(37u)));
    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, manager.read_di(80u, &direct_value));
    ASSERT_EQ(0, manager.read_di_multi(80u, 84u, &multi_value));
    EXPECT_EQ(0u, multi_value);
}

TEST(IoManagerDigitalBank, PacksVirtualDiAndDoAcrossByteBoundaries) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_io_layout();
    ASSERT_EQ(0, manager.apply_pio_layout(&layout));

    const uint32_t io_numbers[] = {7u, 8u, 31u, 32u};
    for (std::size_t i = 0; i < 4u; ++i) {
        ASSERT_EQ(0, manager.register_virtual_di(io_numbers[i]));
        ASSERT_EQ(0, manager.register_virtual_do(io_numbers[i]));
    }
    ASSERT_EQ(0, manager.write_virtual_di(7u, true));
    ASSERT_EQ(0, manager.write_virtual_di(31u, true));
    ASSERT_EQ(0, manager.write_virtual_di(32u, true));
    ASSERT_EQ(0, manager.write_do(7u, true));
    ASSERT_EQ(0, manager.write_do(31u, true));
    ASSERT_EQ(0, manager.write_do(32u, true));

    uint32_t value = 0u;
    ASSERT_EQ(0, manager.read_di_multi(7u, 32u, &value));
    EXPECT_EQ((1u << 0u) | (1u << 24u) | (1u << 25u), value);
    ASSERT_EQ(0, manager.register_virtual_di(MC_IO_MAX_COUNT - 1u));
    ASSERT_EQ(0, manager.write_virtual_di(MC_IO_MAX_COUNT - 1u, true));
    ASSERT_EQ(0,
              manager.read_di_multi(MC_IO_MAX_COUNT - 1u, MC_IO_MAX_COUNT - 1u, &value));
    EXPECT_EQ(1u, value);
    ASSERT_EQ(0, manager.read_do_multi(7u, 32u, &value));
    EXPECT_EQ((1u << 0u) | (1u << 24u) | (1u << 25u), value);
}

TEST(IoManagerShadowDo, WritesWithoutBindingAndOverridesDirectWritesOnFlush) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_io_layout();
    uint8_t input[8] = {0};
    uint8_t rebound_output[16] = {0};
    const zmc::IoObjectKey output_key{2, 0x7000, 1};

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    ASSERT_EQ(0, manager.register_do_alias(7, output_key));
    ASSERT_EQ(0, manager.write_do(7, true));

    manager.bind_process_image(input, rebound_output);
    manager.restore_retained_outputs();
    EXPECT_EQ(1u, rebound_output[4]);

    ASSERT_EQ(0, manager.write_output(output_key, 0u));
    EXPECT_EQ(0u, rebound_output[4]);
    bool value = false;
    ASSERT_EQ(0, manager.read_do(7, &value));
    EXPECT_TRUE(value);
    manager.restore_retained_outputs();
    EXPECT_EQ(1u, rebound_output[4]);
}

TEST(IoManagerRetainedDo, ClearsValuesWithAliasesResetAndLayout) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_io_layout();
    uint8_t input[8] = {0};
    uint8_t output[16] = {0};
    uint8_t rebound_output[16];
    const zmc::IoObjectKey output_key{2, 0x7000, 1};

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    manager.bind_process_image(input, output);
    ASSERT_EQ(0, manager.register_do_alias(7, output_key));
    ASSERT_EQ(0, manager.write_do(7, true));

    manager.clear_aliases();
    std::memset(rebound_output, 0x5A, sizeof(rebound_output));
    manager.bind_process_image(input, rebound_output);
    manager.restore_retained_outputs();
    EXPECT_EQ(0x5Au, rebound_output[4]);

    ASSERT_EQ(0, manager.register_do_alias(7, output_key));
    ASSERT_EQ(0, manager.write_do(7, false));
    manager.reset();
    std::memset(rebound_output, 0x5A, sizeof(rebound_output));
    manager.bind_process_image(input, rebound_output);
    manager.restore_retained_outputs();
    EXPECT_EQ(0x5Au, rebound_output[4]);

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    manager.bind_process_image(input, output);
    ASSERT_EQ(0, manager.register_do_alias(7, output_key));
    ASSERT_EQ(0, manager.write_do(7, true));
    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    std::memset(rebound_output, 0x5A, sizeof(rebound_output));
    manager.bind_process_image(input, rebound_output);
    EXPECT_EQ(0x5Au, rebound_output[4]);
}

TEST(IoManagerShadowAo, WritesWithoutBindingAndOverridesDirectWritesOnFlush) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_io_layout();
    uint8_t input[8] = {0};
    uint8_t rebound_output[16] = {0};
    const zmc::IoObjectKey output_key{2, 0x7010, 2};
    const uint32_t retained_value = 0x78563412u;

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    ASSERT_EQ(0, manager.register_ao_alias(7, output_key));
    ASSERT_EQ(0, manager.write_ao(7, retained_value));

    manager.bind_process_image(input, rebound_output);
    manager.restore_retained_outputs();
    uint32_t value = 0;
    ASSERT_EQ(0, manager.read_ao(7, &value));
    EXPECT_EQ(retained_value, value);

    ASSERT_EQ(0, manager.write_output(output_key, 0u));
    ASSERT_EQ(0, manager.read_ao(7, &value));
    EXPECT_EQ(retained_value, value);
    manager.restore_retained_outputs();
    ASSERT_EQ(0, manager.read_output(output_key, &value));
    EXPECT_EQ(retained_value, value);
}

TEST(IoManagerRetainedAo, ClearsValuesWithAliasesResetAndLayout) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_io_layout();
    uint8_t input[8] = {0};
    uint8_t output[16] = {0};
    uint8_t rebound_output[16];
    const zmc::IoObjectKey output_key{2, 0x7010, 2};

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    manager.bind_process_image(input, output);
    ASSERT_EQ(0, manager.register_ao_alias(7, output_key));
    ASSERT_EQ(0, manager.write_ao(7, 0x78563412u));

    manager.clear_aliases();
    std::memset(rebound_output, 0x5A, sizeof(rebound_output));
    manager.bind_process_image(input, rebound_output);
    manager.restore_retained_outputs();
    EXPECT_EQ(0x5Au, rebound_output[8]);

    ASSERT_EQ(0, manager.register_ao_alias(7, output_key));
    ASSERT_EQ(0, manager.write_ao(7, 0x12345678u));
    manager.reset();
    std::memset(rebound_output, 0x5A, sizeof(rebound_output));
    manager.bind_process_image(input, rebound_output);
    manager.restore_retained_outputs();
    EXPECT_EQ(0x5Au, rebound_output[8]);

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    manager.bind_process_image(input, output);
    ASSERT_EQ(0, manager.register_ao_alias(7, output_key));
    ASSERT_EQ(0, manager.write_ao(7, 0x78563412u));
    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    std::memset(rebound_output, 0x5A, sizeof(rebound_output));
    manager.bind_process_image(input, rebound_output);
    EXPECT_EQ(0x5Au, rebound_output[8]);
}

TEST(IoManagerErrors, ReportsIdentityBindingSizeAndRangeFailures) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_io_layout();
    const zmc::IoObjectKey input_key{2, 0x6000, 1};
    uint32_t value = 0;

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    EXPECT_EQ(ZMC_ERR_IO_PROCESS_IMAGE_UNBOUND, manager.read_input(input_key, &value));
    EXPECT_EQ(ZMC_ERR_IO_PROCESS_IMAGE_UNBOUND, manager.read_input(0u, 1u, &value));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND,
              manager.read_input(zmc::IoObjectKey{1, 0x6000, 1}, &value));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND,
              manager.read_input(zmc::IoObjectKey{2, 0x6000, 2}, &value));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND,
              manager.read_output(zmc::IoObjectKey{2, 0x6000, 1}, &value));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND,
              manager.register_di_alias(7, zmc::IoObjectKey{2, 0x7777, 0}));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND,
              manager.register_di_alias(7, zmc::IoObjectKey{2, 0x7000, 1}));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND,
              manager.register_do_alias(7, zmc::IoObjectKey{2, 0x6000, 1}));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND,
              manager.register_ai_alias(7, zmc::IoObjectKey{2, 0x7000, 1}));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND,
              manager.register_ao_alias(7, zmc::IoObjectKey{2, 0x6000, 1}));

    layout.descriptor_pod.pdo_cells[0].bit_size = 40;
    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    uint8_t input[8] = {0};
    uint8_t output[16] = {0};
    manager.bind_process_image(input, output);
    EXPECT_EQ(ZMC_ERR_IO_DATA_SIZE_UNSUPPORTED, manager.read_input(input_key, &value));
    EXPECT_EQ(ZMC_ERR_IO_DATA_SIZE_UNSUPPORTED, manager.read_input(0u, 0u, &value));
    EXPECT_EQ(ZMC_ERR_IO_INVALID_ARG, manager.read_input(0u, 1u, nullptr));
    EXPECT_EQ(ZMC_ERR_IO_DATA_SIZE_UNSUPPORTED, manager.register_di_alias(0, input_key));

    layout = make_io_layout();
    layout.descriptor_pod.tx_cap = 1;
    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    manager.bind_process_image(input, output);
    EXPECT_EQ(ZMC_ERR_IO_PROCESS_IMAGE_OVER_RANGE, manager.read_input(input_key, &value));
    EXPECT_EQ(ZMC_ERR_IO_PROCESS_IMAGE_OVER_RANGE, manager.read_input(1u, 1u, &value));
    EXPECT_EQ(ZMC_ERR_IO_PROCESS_IMAGE_OVER_RANGE,
              manager.register_di_alias(0, input_key, 0x01u));
}

TEST(IoManagerAliasErrors, ReportsInvalidTargetsAndKeepsOutputsInShadow) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_io_layout();
    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    ASSERT_EQ(0, manager.register_di_alias(1, zmc::IoObjectKey{2, 0x6000, 1}, 0x01u));
    ASSERT_EQ(0, manager.register_do_alias(1, zmc::IoObjectKey{2, 0x7000, 1}));
    ASSERT_EQ(0, manager.register_ai_alias(1, zmc::IoObjectKey{2, 0x6010, 2}));
    ASSERT_EQ(0, manager.register_ao_alias(1, zmc::IoObjectKey{2, 0x7010, 2}));

    bool digital_value = false;
    bool output_value = false;
    uint32_t analog_value = 0;
    EXPECT_EQ(ZMC_ERR_IO_INVALID_ARG, manager.read_di(1, nullptr));
    EXPECT_EQ(ZMC_ERR_IO_INVALID_ARG, manager.read_do(1, nullptr));
    EXPECT_EQ(ZMC_ERR_IO_INVALID_ARG, manager.read_ao(1, nullptr));
    EXPECT_EQ(ZMC_ERR_IO_PROCESS_IMAGE_UNBOUND, manager.read_di(1, &digital_value));
    EXPECT_EQ(0, manager.read_do(1, &output_value));
    EXPECT_FALSE(output_value);
    EXPECT_EQ(0, manager.write_do(1, true));
    EXPECT_EQ(ZMC_ERR_IO_PROCESS_IMAGE_UNBOUND, manager.read_ai(1, &analog_value));
    EXPECT_EQ(0, manager.read_ao(1, &analog_value));
    EXPECT_EQ(0u, analog_value);
    EXPECT_EQ(0, manager.write_ao(1, 0x12345678u));

    manager.reset();
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, manager.read_di(1, &digital_value));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, manager.read_do(1, &output_value));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, manager.read_ai(1, &analog_value));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, manager.read_ao(1, &analog_value));
}

TEST(AxisFbBridgeIoManager, AppliesLayoutBindsCycleBuffersAndResetsManager) {
    zmc::AxisFbBridge bridge;
    ZmcPioOffsetLayout layout = make_io_layout();
    ASSERT_EQ(0, bridge.apply_pio_layout(&layout));

    uint8_t input[8] = {0};
    uint8_t output[16] = {0};
    input[1] = 0x66;
    zmc::proc_img_i_addr parsed_input;
    zmc::proc_img_o_addr parsed_output;
    std::memset(&parsed_input, 0, sizeof(parsed_input));
    std::memset(&parsed_output, 0, sizeof(parsed_output));
    parsed_input.pBuf = input;
    parsed_output.pBuf = output;

    ASSERT_EQ(0, bridge.run_cycle_parsed(&parsed_input, &parsed_output));
    ASSERT_NE(nullptr, bridge.native_io());

    uint32_t value = 0;
    EXPECT_EQ(0, bridge.native_io()->read_input(zmc::IoObjectKey{2, 0x6000, 1}, &value));
    EXPECT_EQ(0x66u, value);
    EXPECT_EQ(0, bridge.native_io()->write_output(zmc::IoObjectKey{2, 0x7000, 1}, 1u));
    EXPECT_EQ(1u, output[4]);

    ASSERT_EQ(0, bridge.reset());
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND,
              bridge.native_io()->read_input(zmc::IoObjectKey{2, 0x6000, 1}, &value));
}

/* Non-byte-aligned 1-bit DI/DO: shared byte, write one bit without corrupting neighbors. */
TEST(IoManagerBitAccess, ReadsAndWritesUnalignedOneBitFields) {
    ZmcPioOffsetLayout layout;
    std::memset(&layout, 0, sizeof(layout));
    layout.slot_count = 1;
    layout.descriptor_pod.tx_cap = 2;
    layout.descriptor_pod.rx_cap = 2;
    layout.descriptor_pod.slave_count = 1;
    layout.descriptor_pod.pdo_cell_count = 4;

    RtBusPdoCell& di0 = layout.descriptor_pod.pdo_cells[0];
    di0.index = 0x6001;
    di0.bit_offset = 1; /* byte 0, bit 1 */
    di0.sub_index = 1;
    di0.bit_size = 1;
    di0.is_input = 1;
    di0.slave_idx = 0;

    RtBusPdoCell& di1 = layout.descriptor_pod.pdo_cells[1];
    di1.index = 0x6002;
    di1.bit_offset = 2; /* byte 0, bit 2 */
    di1.sub_index = 1;
    di1.bit_size = 1;
    di1.is_input = 1;
    di1.slave_idx = 0;

    RtBusPdoCell& do0 = layout.descriptor_pod.pdo_cells[2];
    do0.index = 0x7001;
    do0.bit_offset = 3; /* byte 0, bit 3 */
    do0.sub_index = 1;
    do0.bit_size = 1;
    do0.is_input = 0;
    do0.slave_idx = 0;

    RtBusPdoCell& do1 = layout.descriptor_pod.pdo_cells[3];
    do1.index = 0x7002;
    do1.bit_offset = 4; /* byte 0, bit 4 */
    do1.sub_index = 1;
    do1.bit_size = 1;
    do1.is_input = 0;
    do1.slave_idx = 0;

    zmc::IoManager manager;
    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    ASSERT_EQ(0, manager.register_di_alias(0, zmc::IoObjectKey{0, 0x6001, 1}));
    ASSERT_EQ(0, manager.register_di_alias(1, zmc::IoObjectKey{0, 0x6002, 1}));
    ASSERT_EQ(0, manager.register_do_alias(0, zmc::IoObjectKey{0, 0x7001, 1}));
    ASSERT_EQ(0, manager.register_do_alias(1, zmc::IoObjectKey{0, 0x7002, 1}));

    uint8_t input[2] = {0};
    uint8_t output[2] = {0};
    input[0] = static_cast<uint8_t>((1u << 1) | (1u << 2)); /* both DI bits set */
    output[0] = 0xffu;                                      /* all output bits preset */
    manager.bind_process_image(input, output);

    bool di_value = false;
    EXPECT_EQ(0, manager.read_di(0, &di_value));
    EXPECT_TRUE(di_value);
    EXPECT_EQ(0, manager.read_di(1, &di_value));
    EXPECT_TRUE(di_value);

    EXPECT_EQ(0, manager.write_do(0, false));
    EXPECT_EQ(0, manager.write_do(1, true));
    manager.restore_retained_outputs();
    /* Bit 3 cleared, bit 4 set; other bits in the shared byte stay 1. */
    EXPECT_EQ(0xF7u, output[0]);
    EXPECT_EQ(0u, (output[0] >> 3) & 1u);
    EXPECT_EQ(1u, (output[0] >> 4) & 1u);
    EXPECT_EQ(1u, (output[0] >> 0) & 1u);
    EXPECT_EQ(1u, (output[0] >> 1) & 1u);
    EXPECT_EQ(1u, (output[0] >> 2) & 1u);
    EXPECT_EQ(1u, (output[0] >> 5) & 1u);

    bool do_value = false;
    EXPECT_EQ(0, manager.read_do(0, &do_value));
    EXPECT_FALSE(do_value);
    EXPECT_EQ(0, manager.read_do(1, &do_value));
    EXPECT_TRUE(do_value);

    uint32_t key_value = 0;
    EXPECT_EQ(0, manager.read_input(zmc::IoObjectKey{0, 0x6001, 1}, &key_value));
    EXPECT_EQ(1u, key_value);
    EXPECT_EQ(0, manager.write_output(zmc::IoObjectKey{0, 0x7001, 1}, 1u));
    EXPECT_EQ(1u, (output[0] >> 3) & 1u);
    manager.restore_retained_outputs();
    EXPECT_EQ(0u, (output[0] >> 3) & 1u);
}

TEST(IoManagerBitMaskAccess, SupportsMultiBitDigitalOutputMasks) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_io_layout();
    uint8_t input[8] = {0};
    uint8_t output[16] = {0};
    const zmc::IoObjectKey input_key{2, 0x6000, 1};
    const zmc::IoObjectKey output_key{2, 0x7020, 3};
    input[1] = 0x88u;

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    manager.bind_process_image(input, output);
    ASSERT_EQ(0, manager.register_di_alias(10u, input_key, 0x08u));
    ASSERT_EQ(0, manager.register_di_alias(11u, input_key, 0x04u));
    ASSERT_EQ(0, manager.register_do_alias(20u, output_key, 0x01u));
    ASSERT_EQ(0, manager.register_do_alias(21u, output_key, 0x80u));

    bool value = false;
    ASSERT_EQ(0, manager.read_di(10u, &value));
    EXPECT_TRUE(value);
    ASSERT_EQ(0, manager.read_di(11u, &value));
    EXPECT_FALSE(value);

    ASSERT_EQ(0, manager.write_do(20u, true));
    ASSERT_EQ(0, manager.write_do(21u, true));
    manager.restore_retained_outputs();
    EXPECT_EQ(0x81u, output[12]);

    std::vector<zmc::IoMetadataEntry> metadata;
    ASSERT_EQ(0, manager.get_alias_metadata(metadata));
    ASSERT_EQ(4u, metadata.size());
    EXPECT_EQ(0x08u, metadata[0].mask);
    EXPECT_EQ(8u, metadata[0].bit_offset);
    EXPECT_EQ(8u, metadata[0].bit_size);
    EXPECT_EQ(0x04u, metadata[1].mask);
    EXPECT_EQ(0x01u, metadata[2].mask);
    EXPECT_EQ(96u, metadata[2].bit_offset);
    EXPECT_EQ(32u, metadata[2].bit_size);
    EXPECT_EQ(0x80u, metadata[3].mask);
    EXPECT_EQ(96u, metadata[3].bit_offset);
    EXPECT_EQ(32u, metadata[3].bit_size);
}

TEST(IoManagerBitMaskRegistration, ValidatesMasksAndUsesEffectiveOverlapRanges) {
    ZmcPioOffsetLayout layout = make_io_layout();
    const zmc::IoObjectKey input_key{2, 0x6000, 1};
    const zmc::IoObjectKey output_u32_key{2, 0x7020, 3};
    const zmc::IoObjectKey overlapping_ao_key{2, 0x7030, 4};

    zmc::IoManager manager;
    uint8_t input[8] = {0};
    uint8_t output[16] = {0};
    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    manager.bind_process_image(input, output);

    EXPECT_EQ(ZMC_ERR_IO_MASK_INVALID, manager.register_di_alias(1u, input_key, 0u));
    EXPECT_EQ(ZMC_ERR_IO_MASK_INVALID, manager.register_di_alias(1u, input_key, 0x03u));
    EXPECT_EQ(ZMC_ERR_IO_MASK_INVALID, manager.register_di_alias(1u, input_key, 0x100u));

    ASSERT_EQ(0, manager.register_do_alias(2u, output_u32_key, 0x80000000u));
    ASSERT_EQ(0, manager.register_do_alias(3u, output_u32_key, 0x00000001u));

    zmc::IoManager ao_first;
    ASSERT_EQ(0, ao_first.apply_pio_layout(&layout));
    ASSERT_EQ(0, ao_first.register_ao_alias(5u, overlapping_ao_key));
    EXPECT_EQ(ZMC_ERR_IO_ALIAS_DUPLICATE,
              ao_first.register_do_alias(2u, output_u32_key, 0x00010000u));
}

TEST(IoManagerBitMaskRetainedDo, WritesMultiBitDigitalOutputMask) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_io_layout();
    uint8_t input[8] = {0};
    uint8_t output[16] = {0};
    const zmc::IoObjectKey output_key{2, 0x7020, 3};

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    manager.bind_process_image(input, output);
    ASSERT_EQ(0, manager.register_do_alias(7u, output_key, 0x04u));
    ASSERT_EQ(0, manager.write_do(7u, true));
    manager.restore_retained_outputs();
    EXPECT_EQ(0x04u, output[12]);
}

TEST(IoManagerShadowPlan, CopiesActualWidthsAndAppliesZeroInitializedMasks) {
    // Mix aligned widths, masked bits, and a non-byte-aligned analog field in one plan.
    ZmcPioOffsetLayout layout;
    std::memset(&layout, 0, sizeof(layout));
    layout.descriptor_pod.rx_cap = 14u;
    layout.descriptor_pod.slave_count = 1u;
    layout.descriptor_pod.pdo_cell_count = 6u;

    RtBusPdoCell& ao8 = layout.descriptor_pod.pdo_cells[0];
    ao8.index = 0x7100u;
    ao8.bit_offset = 0u;
    ao8.sub_index = 1u;
    ao8.bit_size = 8u;
    ao8.slave_idx = 0u;

    RtBusPdoCell& ao16 = layout.descriptor_pod.pdo_cells[1];
    ao16.index = 0x7101u;
    ao16.bit_offset = 8u;
    ao16.sub_index = 1u;
    ao16.bit_size = 16u;
    ao16.slave_idx = 0u;

    RtBusPdoCell& ao24 = layout.descriptor_pod.pdo_cells[2];
    ao24.index = 0x7102u;
    ao24.bit_offset = 24u;
    ao24.sub_index = 1u;
    ao24.bit_size = 24u;
    ao24.slave_idx = 0u;

    RtBusPdoCell& do8 = layout.descriptor_pod.pdo_cells[3];
    do8.index = 0x7200u;
    do8.bit_offset = 48u;
    do8.sub_index = 1u;
    do8.bit_size = 8u;
    do8.slave_idx = 0u;

    RtBusPdoCell& ao12 = layout.descriptor_pod.pdo_cells[4];
    ao12.index = 0x7103u;
    ao12.bit_offset = 60u;
    ao12.sub_index = 1u;
    ao12.bit_size = 12u;
    ao12.slave_idx = 0u;

    RtBusPdoCell& ao32 = layout.descriptor_pod.pdo_cells[5];
    ao32.index = 0x7104u;
    ao32.bit_offset = 72u;
    ao32.sub_index = 1u;
    ao32.bit_size = 32u;
    ao32.slave_idx = 0u;

    zmc::IoManager manager;
    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    ASSERT_EQ(0, manager.register_ao_alias(1u, zmc::IoObjectKey{0u, 0x7100u, 1u}));
    ASSERT_EQ(0, manager.register_ao_alias(2u, zmc::IoObjectKey{0u, 0x7101u, 1u}));
    ASSERT_EQ(0, manager.register_ao_alias(3u, zmc::IoObjectKey{0u, 0x7102u, 1u}));
    ASSERT_EQ(0, manager.register_do_alias(4u, zmc::IoObjectKey{0u, 0x7200u, 1u}, 0x01u));
    ASSERT_EQ(0, manager.register_do_alias(5u, zmc::IoObjectKey{0u, 0x7200u, 1u}, 0x80u));
    ASSERT_EQ(0, manager.register_ao_alias(6u, zmc::IoObjectKey{0u, 0x7103u, 1u}));
    ASSERT_EQ(0, manager.register_ao_alias(7u, zmc::IoObjectKey{0u, 0x7104u, 1u}));

    ASSERT_EQ(0, manager.write_ao(1u, 0x11u));
    ASSERT_EQ(0, manager.write_ao(2u, 0x2233u));
    ASSERT_EQ(0, manager.write_ao(3u, 0x445566u));
    ASSERT_EQ(0, manager.write_ao(6u, 0xABCu));
    ASSERT_EQ(0, manager.write_ao(7u, 0xDEADBEEFu));

    uint8_t output[14];
    std::memset(output, 0xA5, sizeof(output));
    manager.bind_process_image(nullptr, output);
    manager.restore_retained_outputs();

    EXPECT_EQ(0x11u, output[0]);
    EXPECT_EQ(0x33u, output[1]);
    EXPECT_EQ(0x22u, output[2]);
    EXPECT_EQ(0x66u, output[3]);
    EXPECT_EQ(0x55u, output[4]);
    EXPECT_EQ(0x44u, output[5]);
    EXPECT_EQ(0x24u, output[6]);
    EXPECT_EQ(0xC5u, output[7]);
    EXPECT_EQ(0xABu, output[8]);
    EXPECT_EQ(0xEFu, output[9]);
    EXPECT_EQ(0xBEu, output[10]);
    EXPECT_EQ(0xADu, output[11]);
    EXPECT_EQ(0xDEu, output[12]);
    EXPECT_EQ(0xA5u, output[13]);
}

TEST(IoManagerShadowPlan, FlushesMaskedBitAtUint16OffsetBoundary) {
    ZmcPioOffsetLayout layout;
    std::memset(&layout, 0, sizeof(layout));
    layout.descriptor_pod.rx_cap = 8192u;
    layout.descriptor_pod.slave_count = 1u;
    layout.descriptor_pod.pdo_cell_count = 1u;

    RtBusPdoCell& output_bit = layout.descriptor_pod.pdo_cells[0];
    output_bit.index = 0x7300u;
    output_bit.bit_offset = 0xffffu;
    output_bit.sub_index = 1u;
    output_bit.bit_size = 1u;
    output_bit.slave_idx = 0u;

    zmc::IoManager manager;
    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    ASSERT_EQ(0, manager.register_do_alias(1u, zmc::IoObjectKey{0u, 0x7300u, 1u}));
    ASSERT_EQ(0, manager.write_do(1u, true));

    std::vector<uint8_t> output(layout.descriptor_pod.rx_cap, 0x25u);
    manager.bind_process_image(nullptr, output.data());
    manager.restore_retained_outputs();

    EXPECT_EQ(0x25u, output[output.size() - 2u]);
    EXPECT_EQ(0xA5u, output.back());
}

TEST(IoManagerVirtualBank, SharesNumberedAccessWithoutUsingPdoShadow) {
    // Exercise mixed-source dispatch while rebinding and flushing the PDO image.
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_io_layout();
    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    ASSERT_EQ(0, manager.register_virtual_di(10u));
    ASSERT_EQ(0, manager.register_virtual_do(19u));
    ASSERT_EQ(0, manager.register_virtual_ai(12u));
    ASSERT_EQ(0, manager.register_virtual_ao(13u));
    ASSERT_EQ(0, manager.register_do_alias(20u, zmc::IoObjectKey{2u, 0x7000u, 1u}));
    EXPECT_EQ(ZMC_ERR_IO_ALIAS_DUPLICATE, manager.register_virtual_do(20u));

    bool digital_value = true;
    uint32_t analog_value = 1u;
    ASSERT_EQ(0, manager.read_di(10u, &digital_value));
    EXPECT_FALSE(digital_value);
    ASSERT_EQ(0, manager.read_do(19u, &digital_value));
    EXPECT_FALSE(digital_value);
    ASSERT_EQ(0, manager.read_ai(12u, &analog_value));
    EXPECT_EQ(0u, analog_value);
    ASSERT_EQ(0, manager.read_ao(13u, &analog_value));
    EXPECT_EQ(0u, analog_value);

    ASSERT_EQ(0, manager.write_virtual_di(10u, true));
    ASSERT_EQ(0, manager.write_do(19u, true));
    ASSERT_EQ(0, manager.write_virtual_ai(12u, 0x12345678u));
    ASSERT_EQ(0, manager.write_ao(13u, 0x87654321u));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, manager.write_virtual_di(19u, true));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, manager.write_virtual_ai(13u, 1u));

    uint8_t output[16];
    std::memset(output, 0xA5, sizeof(output));
    manager.bind_process_image(nullptr, output);
    manager.restore_retained_outputs();
    EXPECT_EQ(0xA4u, output[4]);
    ASSERT_EQ(0, manager.read_di(10u, &digital_value));
    EXPECT_TRUE(digital_value);
    ASSERT_EQ(0, manager.read_do(19u, &digital_value));
    EXPECT_TRUE(digital_value);
    ASSERT_EQ(0, manager.read_ai(12u, &analog_value));
    EXPECT_EQ(0x12345678u, analog_value);
    ASSERT_EQ(0, manager.read_ao(13u, &analog_value));
    EXPECT_EQ(0x87654321u, analog_value);

    ASSERT_EQ(0, manager.write_do_multi(19u, 20u, 3u));
    ASSERT_EQ(0, manager.read_do_multi(19u, 20u, &analog_value));
    EXPECT_EQ(3u, analog_value);
    manager.restore_retained_outputs();
    EXPECT_EQ(0xA5u, output[4]);

    uint8_t rebound_output[16];
    std::memset(rebound_output, 0x5Au, sizeof(rebound_output));
    manager.bind_process_image(nullptr, rebound_output);
    manager.restore_retained_outputs();
    EXPECT_EQ(0x5Bu, rebound_output[4]);
    ASSERT_EQ(0, manager.read_do(19u, &digital_value));
    EXPECT_TRUE(digital_value);
    ASSERT_EQ(0, manager.read_ai(12u, &analog_value));
    EXPECT_EQ(0x12345678u, analog_value);

    std::vector<zmc::IoMetadataEntry> metadata;
    ASSERT_EQ(0, manager.get_alias_metadata(metadata));
    ASSERT_EQ(5u, metadata.size());
    EXPECT_EQ(zmc::IO_SOURCE_KIND_VIRTUAL, metadata[0].source_kind);
    EXPECT_EQ(1u, metadata[0].bit_size);
    EXPECT_EQ(zmc::IO_SOURCE_KIND_VIRTUAL, metadata[1].source_kind);
    EXPECT_EQ(zmc::IO_SOURCE_KIND_PDO, metadata[2].source_kind);
    EXPECT_EQ(zmc::IO_SOURCE_KIND_VIRTUAL, metadata[3].source_kind);
    EXPECT_EQ(32u, metadata[3].bit_size);
    EXPECT_EQ(zmc::IO_SOURCE_KIND_VIRTUAL, metadata[4].source_kind);

    manager.clear_aliases();
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, manager.read_di(10u, &digital_value));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, manager.read_do(19u, &digital_value));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, manager.read_ai(12u, &analog_value));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, manager.read_ao(13u, &analog_value));
    ASSERT_EQ(0, manager.register_virtual_do(19u));
    ASSERT_EQ(0, manager.read_do(19u, &digital_value));
    EXPECT_FALSE(digital_value);
}
