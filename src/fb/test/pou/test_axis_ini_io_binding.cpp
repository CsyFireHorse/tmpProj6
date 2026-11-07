#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

#include "bridge/AxisFbBridge.h"
#include "common/zmc/fbt_errorcode_def.h"
#include "motbuf/MotionCmdQueueBase.hpp"
#include "pou/AxPou.hpp"
#include "pou/AxPouImpl.hpp"
#include "pou/AxisIniLoader.hpp"
#include "qcmd/MotionCmdPacket.hpp"

namespace {

const char kIoBindingIni[] =
    "[base]\n"
    "axis_num = 0\n"
    "[io_binding]\n"
    "di_32.source = pdo\n"
    "do_32.source = pdo\n"
    "ai_32.source = pdo\n"
    "ao_32.source = pdo\n"
    "do_32.subindex = 1\n"
    "di_32.index = 0x6000\n"
    "ao_32.slave_idx = 2\n"
    "ai_32.subindex = 0x01\n"
    "do_32.slave_idx = 2\n"
    "di_32.subindex = 0x01\n"
    "di_32.mask = 0x02\n"
    "ao_32.index = 0x7010\n"
    "ai_32.index = 0x6000\n"
    "do_32.index = 0x7000\n"
    "di_32.slave_idx = 2\n"
    "ao_32.subindex = 2\n"
    "ai_32.slave_idx = 2\n";

const char kNoIoBindingIni[] =
    "[base]\n"
    "axis_num = 0\n";

const char kIoBindingInitialValuesIni[] =
    "[base]\n"
    "axis_num = 0\n"
    "[io_binding]\n"
    "do_32.source = pdo\n"
    "do_32.slave_idx = 2\n"
    "do_32.index = 0x7000\n"
    "do_32.subindex = 1\n"
    "do_32.initial_value = 1\n"
    "ao_32.source = pdo\n"
    "ao_32.slave_idx = 2\n"
    "ao_32.index = 0x7010\n"
    "ao_32.subindex = 2\n"
    "ao_32.initial_value = 0x1234\n"
    "do_33.source = virtual\n"
    "do_33.initial_value = 1\n"
    "ao_33.source = virtual\n"
    "ao_33.initial_value = 0xFFFFFFFF\n"
    "do_34.source = virtual\n"
    "ao_34.source = virtual\n";

ZmcPioOffsetLayout make_io_binding_layout() {
    ZmcPioOffsetLayout layout;
    std::memset(&layout, 0, sizeof(layout));
    layout.slot_count = 1;
    layout.descriptor_pod.tx_cap = 4;
    layout.descriptor_pod.rx_cap = 4;
    layout.descriptor_pod.slave_count = 3;
    layout.descriptor_pod.pdo_cell_count = 3;

    RtBusPdoCell& input = layout.descriptor_pod.pdo_cells[0];
    input.slave_idx = 2;
    input.index = 0x6000;
    input.sub_index = 1;
    input.bit_offset = 8; /* byte 1 */
    input.bit_size = 8;
    input.is_input = 1;

    RtBusPdoCell& digital_output = layout.descriptor_pod.pdo_cells[1];
    digital_output.slave_idx = 2;
    digital_output.index = 0x7000;
    digital_output.sub_index = 1;
    digital_output.bit_offset = 16; /* byte 2 */
    digital_output.bit_size = 1;
    digital_output.is_input = 0;

    RtBusPdoCell& analog_output = layout.descriptor_pod.pdo_cells[2];
    analog_output.slave_idx = 2;
    analog_output.index = 0x7010;
    analog_output.sub_index = 2;
    analog_output.bit_offset = 24; /* byte 3 */
    analog_output.bit_size = 8;
    analog_output.is_input = 0;
    return layout;
}

void bind_process_image(zmc::AxisFbBridge* bridge, uint8_t* input, uint8_t* output) {
    zmc::proc_img_i_addr parsed_input;
    zmc::proc_img_o_addr parsed_output;
    std::memset(&parsed_input, 0, sizeof(parsed_input));
    std::memset(&parsed_output, 0, sizeof(parsed_output));
    parsed_input.pBuf = input;
    parsed_output.pBuf = output;
    ASSERT_EQ(0, bridge->run_cycle_parsed(&parsed_input, &parsed_output));
}

class TempIniFile {
   public:
    TempIniFile(const char* path, const char* content) : path_(path) {
        std::remove(path_.c_str());
        std::ofstream stream(path_.c_str(), std::ios::out | std::ios::trunc);
        stream << content;
        valid_ = stream.good();
    }

    ~TempIniFile() { std::remove(path_.c_str()); }

    bool valid() const { return valid_; }
    const std::string& path() const { return path_; }

   private:
    std::string path_;
    bool valid_ = false;
};

}  // namespace

TEST(AxisIniIoBinding, LoadsFourAliasGroupsInNumberOrder) {
    const char ini[] =
        "[io_binding]\n"
        "di_2.source = pdo\n"
        "di_7.source = pdo\n"
        "do_32.source = pdo\n"
        "ai_32.source = pdo\n"
        "ao_32.source = pdo\n"
        "di_7.subindex = 2\n"
        "do_32.index = 0x7000\n"
        "di_2.index = 0x6000\n"
        "di_7.index = 0x6010\n"
        "ao_32.subindex = 1\n"
        "ai_32.index = 0x6000\n"
        "do_32.subindex = 1\n"
        "di_2.slave_idx = 2\n"
        "di_7.slave_idx = 3\n"
        "ao_32.index = 0x7000\n"
        "ai_32.subindex = 1\n"
        "do_32.slave_idx = 2\n"
        "di_2.subindex = 1\n"
        "ao_32.slave_idx = 2\n"
        "ai_32.slave_idx = 2\n";
    AxisIniLoader loader(ini, true);
    IoBindingConfig binding = IoBindingConfig();

    ASSERT_EQ(0, loader.load_io_binding(&binding));
    ASSERT_EQ(1, binding.section_present);
    ASSERT_EQ(2, binding.di_count);
    ASSERT_EQ(1, binding.do_count);
    ASSERT_EQ(1, binding.ai_count);
    ASSERT_EQ(1, binding.ao_count);
    EXPECT_EQ(2u, binding.di_entries[0].io_number);
    EXPECT_EQ(zmc::IO_SOURCE_KIND_PDO, binding.di_entries[0].source_kind);
    EXPECT_EQ(2u, binding.di_entries[0].key.slave_idx);
    EXPECT_EQ(0x6000u, binding.di_entries[0].key.index);
    EXPECT_EQ(1u, binding.di_entries[0].key.subindex);
    EXPECT_EQ(0u, binding.di_entries[0].mask);
    EXPECT_EQ(7u, binding.di_entries[1].io_number);
    EXPECT_EQ(32u, binding.do_entries[0].io_number);
    EXPECT_EQ(0x7000u, binding.do_entries[0].key.index);
    EXPECT_EQ(0u, binding.do_entries[0].mask);
    EXPECT_EQ(32u, binding.ai_entries[0].io_number);
    EXPECT_EQ(0x6000u, binding.ai_entries[0].key.index);
    EXPECT_EQ(0u, binding.ai_entries[0].mask);
    EXPECT_EQ(32u, binding.ao_entries[0].io_number);
    EXPECT_EQ(zmc::IO_SOURCE_KIND_PDO, binding.ao_entries[0].source_kind);
    EXPECT_EQ(0x7000u, binding.ao_entries[0].key.index);
    EXPECT_EQ(0u, binding.ao_entries[0].mask);
}

TEST(AxisIniIoBinding, LoadsOutputInitialValues) {
    const char ini[] =
        "[io_binding]\n"
        "do_1.source = virtual\n"
        "do_1.initial_value = 1\n"
        "do_2.source = virtual\n"
        "ao_3.source = virtual\n"
        "ao_3.initial_value = 4294967295\n"
        "ao_4.source = virtual\n"
        "ao_4.initial_value = 0x12345678\n";
    AxisIniLoader loader(ini, true);
    IoBindingConfig binding = IoBindingConfig();

    ASSERT_EQ(0, loader.load_io_binding(&binding));
    ASSERT_EQ(2, binding.do_count);
    ASSERT_EQ(2, binding.ao_count);
    EXPECT_TRUE(binding.do_entries[0].has_initial_value);
    EXPECT_EQ(1u, binding.do_entries[0].initial_value);
    EXPECT_FALSE(binding.do_entries[1].has_initial_value);
    EXPECT_TRUE(binding.ao_entries[0].has_initial_value);
    EXPECT_EQ(0xFFFFFFFFu, binding.ao_entries[0].initial_value);
    EXPECT_TRUE(binding.ao_entries[1].has_initial_value);
    EXPECT_EQ(0x12345678u, binding.ao_entries[1].initial_value);
}

TEST(AxisIniIoBinding, RejectsInvalidOutputInitialValues) {
    const char* invalid_ini[] = {
        "[io_binding]\ndo_1.source = virtual\ndo_1.initial_value = 2\n",
        "[io_binding]\nao_1.source = virtual\nao_1.initial_value = invalid\n",
        "[io_binding]\nao_1.source = virtual\nao_1.initial_value = 4294967296\n",
        "[io_binding]\nao_1.source = virtual\nao_1.initial_value = 0x-1\n",
    };
    const int expected_errors[] = {
        ZMC_ERR_INI_VALUE_RANGE,
        ZMC_ERR_INI_VALUE_FORMAT,
        ZMC_ERR_INI_VALUE_RANGE,
        ZMC_ERR_INI_VALUE_RANGE,
    };

    for (std::size_t i = 0; i < sizeof(invalid_ini) / sizeof(invalid_ini[0]); ++i) {
        AxisIniLoader loader(invalid_ini[i], true);
        IoBindingConfig binding = IoBindingConfig();
        EXPECT_EQ(expected_errors[i], loader.load_io_binding(&binding)) << "case " << i;
    }
}

TEST(AxisIniIoBinding, LoadsDigitalOneHotMasksIncludingHighestBit) {
    const char ini[] =
        "[io_binding]\n"
        "di_1.source = pdo\n"
        "di_1.slave_idx = 2\n"
        "di_1.index = 0x6000\n"
        "di_1.subindex = 1\n"
        "di_1.mask = 0x80000000\n"
        "do_2.source = pdo\n"
        "do_2.slave_idx = 2\n"
        "do_2.index = 0x7000\n"
        "do_2.subindex = 1\n"
        "do_2.mask = 8\n"
        "ai_3.source = pdo\n"
        "ai_3.slave_idx = 2\n"
        "ai_3.index = 0x6010\n"
        "ai_3.subindex = 2\n";
    AxisIniLoader loader(ini, true);
    IoBindingConfig binding = IoBindingConfig();

    ASSERT_EQ(0, loader.load_io_binding(&binding));
    ASSERT_EQ(1, binding.di_count);
    ASSERT_EQ(1, binding.do_count);
    ASSERT_EQ(1, binding.ai_count);
    EXPECT_EQ(0x80000000u, binding.di_entries[0].mask);
    EXPECT_EQ(8u, binding.do_entries[0].mask);
    EXPECT_EQ(0u, binding.ai_entries[0].mask);
}

TEST(AxisIniIoBinding, LoadsAllSourceKindsCaseInsensitively) {
    const char ini[] =
        "[io_binding]\n"
        "di_1.source = NoP\n"
        "do_2.source = ViRtUaL\n"
        "ai_3.source = GPIO\n"
        "ao_4.source = modBUS\n"
        "do_5.source = PdO\n"
        "do_5.slave_idx = 2\n"
        "do_5.index = 0x7000\n"
        "do_5.subindex = 1\n";
    AxisIniLoader loader(ini, true);
    IoBindingConfig binding = IoBindingConfig();

    ASSERT_EQ(0, loader.load_io_binding(&binding));
    ASSERT_EQ(1, binding.di_count);
    ASSERT_EQ(2, binding.do_count);
    ASSERT_EQ(1, binding.ai_count);
    ASSERT_EQ(1, binding.ao_count);
    EXPECT_EQ(zmc::IO_SOURCE_KIND_NOP, binding.di_entries[0].source_kind);
    EXPECT_EQ(zmc::IO_SOURCE_KIND_VIRTUAL, binding.do_entries[0].source_kind);
    EXPECT_EQ(zmc::IO_SOURCE_KIND_PDO, binding.do_entries[1].source_kind);
    EXPECT_EQ(0x7000u, binding.do_entries[1].key.index);
    EXPECT_EQ(zmc::IO_SOURCE_KIND_GPIO, binding.ai_entries[0].source_kind);
    EXPECT_EQ(zmc::IO_SOURCE_KIND_MODBUS, binding.ao_entries[0].source_kind);
}

TEST(AxisIniIoBinding, RejectsPdoFieldsWithoutExplicitSource) {
    const char ini[] =
        "[io_binding]\n"
        "di_1.slave_idx = 2\n"
        "di_1.index = 0x6000\n"
        "di_1.subindex = 1\n";
    AxisIniLoader loader(ini, true);
    IoBindingConfig binding = IoBindingConfig();

    EXPECT_EQ(ZMC_ERR_INI_MISSING_KEY, loader.load_io_binding(&binding));
}

TEST(AxisIniIoBinding, RejectsInvalidAndMixedParameterlessSources) {
    const char* invalid_ini[] = {
        "[io_binding]\ndi_1.source = unknown\n",
        "[io_binding]\ndi_1.source = end\n",
        "[io_binding]\ndi_1.source = nop\ndi_1.slave_idx = 2\n",
        "[io_binding]\ndi_1.source = virtual\ndi_1.slave_idx = 2\n",
        "[io_binding]\ndo_1.source = virtual\ndo_1.mask = 1\n",
        "[io_binding]\ndi_1.virtual = 1\n",
    };
    const int expected_errors[] = {
        ZMC_ERR_INI_VALUE_FORMAT,
        ZMC_ERR_INI_VALUE_FORMAT,
        ZMC_ERR_INI_IO_BINDING_CONFIG,
        ZMC_ERR_INI_IO_BINDING_CONFIG,
        ZMC_ERR_INI_IO_BINDING_CONFIG,
        ZMC_ERR_INI_KEY,
    };

    for (std::size_t i = 0; i < sizeof(invalid_ini) / sizeof(invalid_ini[0]); ++i) {
        AxisIniLoader loader(invalid_ini[i], true);
        IoBindingConfig binding = IoBindingConfig();
        EXPECT_EQ(expected_errors[i], loader.load_io_binding(&binding)) << "case " << i;
    }
}

TEST(AxisIniIoBinding, RejectsInvalidDigitalMasksAndAnalogMaskFields) {
    const char* invalid_ini[] = {
        "[io_binding]\ndi_1.source=pdo\ndi_1.slave_idx=2\ndi_1.index=0x6000\n"
        "di_1.subindex=1\n"
        "di_1.mask=0\n",
        "[io_binding]\ndi_1.source=pdo\ndi_1.slave_idx=2\ndi_1.index=0x6000\n"
        "di_1.subindex=1\n"
        "di_1.mask=3\n",
        "[io_binding]\ndi_1.source=pdo\ndi_1.slave_idx=2\ndi_1.index=0x6000\n"
        "di_1.subindex=1\n"
        "di_1.mask=010\n",
        "[io_binding]\ndi_1.source=pdo\ndi_1.slave_idx=2\ndi_1.index=0x6000\n"
        "di_1.subindex=1\n"
        "di_1.mask=-1\n",
        "[io_binding]\ndi_1.source=pdo\ndi_1.slave_idx=2\ndi_1.index=0x6000\n"
        "di_1.subindex=1\n"
        "di_1.mask=0x100000000\n",
        "[io_binding]\ndi_1.source=pdo\ndi_1.slave_idx=2\ndi_1.index=0x6000\n"
        "di_1.subindex=1\n"
        "di_1.mask=x\n",
        "[io_binding]\nai_1.source=pdo\nai_1.slave_idx=2\nai_1.index=0x6000\n"
        "ai_1.subindex=1\n"
        "ai_1.mask=1\n",
        "[io_binding]\nao_1.source=pdo\nao_1.slave_idx=2\nao_1.index=0x7000\n"
        "ao_1.subindex=1\n"
        "ao_1.mask=1\n",
    };
    const int expected_errors[] = {
        ZMC_ERR_IO_MASK_INVALID, ZMC_ERR_IO_MASK_INVALID, ZMC_ERR_IO_MASK_INVALID,
        ZMC_ERR_INI_VALUE_RANGE, ZMC_ERR_INI_VALUE_RANGE, ZMC_ERR_INI_VALUE_FORMAT,
        ZMC_ERR_INI_KEY, ZMC_ERR_INI_KEY,
    };

    for (std::size_t i = 0; i < sizeof(invalid_ini) / sizeof(invalid_ini[0]); ++i) {
        AxisIniLoader loader(invalid_ini[i], true);
        IoBindingConfig binding = IoBindingConfig();
        EXPECT_EQ(expected_errors[i], loader.load_io_binding(&binding)) << "case " << i;
    }
}

TEST(AxisIniIoBinding, MissingOrEmptySectionLeavesBindingsEmpty) {
    AxisIniLoader loader(kNoIoBindingIni, true);
    IoBindingConfig binding = IoBindingConfig();

    ASSERT_EQ(0, loader.load_io_binding(&binding));
    EXPECT_EQ(0, binding.section_present);
    EXPECT_EQ(0, binding.di_count);
    EXPECT_EQ(0, binding.do_count);
    EXPECT_EQ(0, binding.ai_count);
    EXPECT_EQ(0, binding.ao_count);

    AxisIniLoader empty_loader("[io_binding]\n", true);
    ASSERT_EQ(0, empty_loader.load_io_binding(&binding));
    EXPECT_EQ(0, binding.section_present);
}

TEST(AxisIniIoBinding, RejectsInvalidSectionRecords) {
    const char* invalid_ini[] = {
        "[io_binding]\ndi_1.source = pdo\ndi_1.slave_idx = 2\ndi_1.index = 0x6000\n",
        "[io_binding]\ndi_1.source = pdo\ndi_1.slave_idx = 2\ndi_1.index = 0x6000\n"
        "di_1.bad = 1\n",
        "[io_binding]\ninput_1.slave_idx = 2\n",
        "[io_binding]\ndi_1.source = pdo\ndi_1.slave_idx = 65536\n"
        "di_1.index = 0x6000\n"
        "di_1.subindex = 1\n",
        "[io_binding]\ndi_1.source = pdo\ndi_1.slave_idx = 2\ndi_1.index = 0x10000\n"
        "di_1.subindex = 1\n",
        "[io_binding]\ndi_1.source = pdo\ndi_1.slave_idx = 2\ndi_1.index = 0x6000\n"
        "di_1.subindex = 256\n",
        "[io_binding]\ndi_1.source = pdo\ndi_1.slave_idx = -1\ndi_1.index = 0x6000\n"
        "di_1.subindex = 1\n",
        "[io_binding]\ndi_1.source = pdo\ndi_1.slave_idx = x\ndi_1.index = 0x6000\n"
        "di_1.subindex = 1\n",
        "[io_binding]\n"
        "di_1.source = pdo\ndi_1.slave_idx = 2\ndi_1.index = 0x6000\n"
        "di_1.subindex = 1\ndi_01.source = pdo\ndi_01.slave_idx = 2\n"
        "di_01.index = 0x6010\ndi_01.subindex = 2\n",
    };
    const int expected_errors[] = {
        ZMC_ERR_INI_MISSING_KEY,
        ZMC_ERR_INI_KEY,
        ZMC_ERR_INI_KEY,
        ZMC_ERR_INI_VALUE_RANGE,
        ZMC_ERR_INI_VALUE_RANGE,
        ZMC_ERR_INI_VALUE_RANGE,
        ZMC_ERR_INI_VALUE_RANGE,
        ZMC_ERR_INI_VALUE_FORMAT,
        ZMC_ERR_INI_DUPLICATE,
    };

    // Expected errors follow INI error-category mapping. AI modification signature: GPT-5 Codex
    for (std::size_t i = 0; i < sizeof(invalid_ini) / sizeof(invalid_ini[0]); ++i) {
        AxisIniLoader loader(invalid_ini[i], true);
        IoBindingConfig binding = IoBindingConfig();
        EXPECT_EQ(expected_errors[i], loader.load_io_binding(&binding)) << "case " << i;
    }
}

TEST(AxisIniIoBinding, ReturnsNamedErrorsForInvalidArgumentsAndAliasCapacity) {
    const char alias_overflow_ini[] =
        "[io_binding]\n"
        "di_1024.source = pdo\n"
        "di_1024.slave_idx = 2\n"
        "di_1024.index = 0x6000\n"
        "di_1024.subindex = 1\n";
    AxisIniLoader loader(alias_overflow_ini, true);
    IoBindingConfig binding = IoBindingConfig();

    EXPECT_EQ(ZMC_ERR_HOST_INVALID_ARG, loader.load_io_binding(nullptr));
    EXPECT_EQ(ZMC_ERR_IO_ALIAS_CAPACITY, loader.load_io_binding(&binding));
}

TEST(AxisIniIoBinding, ContentSectionReplacesFileSection) {
    const char file_ini[] =
        "[base]\naxis_num = 0\n"
        "[io_binding]\n"
        "di_1.source = pdo\n"
        "di_1.slave_idx = 2\n"
        "di_1.index = 0x6000\n"
        "di_1.subindex = 1\n";
    const char content_ini[] =
        "[base]\naxis_num = 0\n"
        "[io_binding]\n"
        "ao_9.source = pdo\n"
        "ao_9.slave_idx = 2\n"
        "ao_9.index = 0x7000\n"
        "ao_9.subindex = 1\n";
    TempIniFile file("axis_ini_io_binding_override_test.ini", file_ini);
    ASSERT_TRUE(file.valid());
    zmc::AxPouImpl pou;

    ASSERT_EQ(0, pou.configure(file.path(), kNoIoBindingIni, 0u));
    ASSERT_EQ(1, pou.io_binding_cfg_.di_count);
    EXPECT_EQ(1u, pou.io_binding_cfg_.di_entries[0].io_number);

    ASSERT_EQ(0, pou.configure(file.path(), content_ini, 0u));
    EXPECT_EQ(0, pou.io_binding_cfg_.di_count);
    ASSERT_EQ(1, pou.io_binding_cfg_.ao_count);
    EXPECT_EQ(9u, pou.io_binding_cfg_.ao_entries[0].io_number);
}

TEST(AxisIniIoBinding, AppliesAliasesWhenConfigurePrecedesLayout) {
    zmc::AxisFbBridge bridge;
    ZmcPioOffsetLayout layout = make_io_binding_layout();

    ASSERT_EQ(0, bridge.configure(nullptr, kIoBindingIni, 0u));
    ASSERT_EQ(0, bridge.apply_pio_layout(&layout));

    uint8_t input[4] = {0, 0x5A, 0, 0};
    uint8_t output[4] = {0};
    bind_process_image(&bridge, input, output);

    bool digital_value = false;
    EXPECT_EQ(0, bridge.native_io()->read_di(32u, &digital_value));
    EXPECT_TRUE(digital_value);
    uint32_t analog_value = 0;
    EXPECT_EQ(0, bridge.native_io()->read_ai(32u, &analog_value));
    EXPECT_EQ(0x5Au, analog_value);
    EXPECT_EQ(0, bridge.native_io()->write_do(32u, true));
    EXPECT_EQ(0, bridge.native_io()->write_ao(32u, 0xA5u));
    bind_process_image(&bridge, input, output);
    EXPECT_EQ(1u, output[2]);
    EXPECT_EQ(0xA5u, output[3]);
}

TEST(AxisIniIoBinding, AppliesAliasesWhenLayoutPrecedesConfigure) {
    zmc::AxisFbBridge bridge;
    ZmcPioOffsetLayout layout = make_io_binding_layout();

    ASSERT_EQ(0, bridge.apply_pio_layout(&layout));
    ASSERT_EQ(0, bridge.configure(nullptr, kIoBindingIni, 0u));

    uint8_t input[4] = {0, 0x66, 0, 0};
    uint8_t output[4] = {0};
    bind_process_image(&bridge, input, output);

    bool digital_value = false;
    EXPECT_EQ(0, bridge.native_io()->read_di(32u, &digital_value));
    EXPECT_TRUE(digital_value);
    uint32_t analog_value = 0;
    EXPECT_EQ(0, bridge.native_io()->read_ai(32u, &analog_value));
    EXPECT_EQ(0x66u, analog_value);
    EXPECT_EQ(0, bridge.native_io()->write_do(32u, false));
    EXPECT_EQ(0, bridge.native_io()->write_ao(32u, 0x99u));
    bind_process_image(&bridge, input, output);
    EXPECT_EQ(0u, output[2]);
    EXPECT_EQ(0x99u, output[3]);
}

TEST(AxisIniIoBinding, AppliesOutputInitialValuesForEitherLayoutOrder) {
    ZmcPioOffsetLayout layout = make_io_binding_layout();
    uint8_t input[4] = {0};
    uint8_t output[4] = {0};

    zmc::AxisFbBridge configure_first;
    ASSERT_EQ(0, configure_first.configure(nullptr, kIoBindingInitialValuesIni, 0u));
    ASSERT_EQ(0, configure_first.apply_pio_layout(&layout));

    bool digital_value = false;
    uint32_t analog_value = 0u;
    ASSERT_EQ(0, configure_first.native_io()->read_do(32u, &digital_value));
    EXPECT_TRUE(digital_value);
    ASSERT_EQ(0, configure_first.native_io()->read_ao(32u, &analog_value));
    EXPECT_EQ(0x34u, analog_value);
    ASSERT_EQ(0, configure_first.native_io()->read_do(33u, &digital_value));
    EXPECT_TRUE(digital_value);
    ASSERT_EQ(0, configure_first.native_io()->read_ao(33u, &analog_value));
    EXPECT_EQ(0xFFFFFFFFu, analog_value);
    ASSERT_EQ(0, configure_first.native_io()->read_do(34u, &digital_value));
    EXPECT_FALSE(digital_value);
    ASSERT_EQ(0, configure_first.native_io()->read_ao(34u, &analog_value));
    EXPECT_EQ(0u, analog_value);

    bind_process_image(&configure_first, input, output);
    EXPECT_EQ(1u, output[2] & 0x01u);
    EXPECT_EQ(0x34u, output[3]);

    zmc::AxisFbBridge layout_first;
    ASSERT_EQ(0, layout_first.apply_pio_layout(&layout));
    ASSERT_EQ(0, layout_first.configure(nullptr, kIoBindingInitialValuesIni, 0u));
    ASSERT_EQ(0, layout_first.native_io()->read_do(32u, &digital_value));
    EXPECT_TRUE(digital_value);
    ASSERT_EQ(0, layout_first.native_io()->read_ao(32u, &analog_value));
    EXPECT_EQ(0x34u, analog_value);
    ASSERT_EQ(0, layout_first.native_io()->read_do(33u, &digital_value));
    EXPECT_TRUE(digital_value);
    ASSERT_EQ(0, layout_first.native_io()->read_ao(33u, &analog_value));
    EXPECT_EQ(0xFFFFFFFFu, analog_value);
    ASSERT_EQ(0, layout_first.native_io()->read_do(34u, &digital_value));
    EXPECT_FALSE(digital_value);
    ASSERT_EQ(0, layout_first.native_io()->read_ao(34u, &analog_value));
    EXPECT_EQ(0u, analog_value);

    std::memset(output, 0, sizeof(output));
    bind_process_image(&layout_first, input, output);
    EXPECT_EQ(1u, output[2] & 0x01u);
    EXPECT_EQ(0x34u, output[3]);
}

TEST(AxisIniIoBinding, ClearAliasesPreservesLayoutAndProcessImage) {
    zmc::IoManager manager;
    ZmcPioOffsetLayout layout = make_io_binding_layout();
    uint8_t input[4] = {0, 0x77, 0, 0};
    uint8_t output[4] = {0};
    const zmc::IoObjectKey input_key{2, 0x6000, 1};

    ASSERT_EQ(0, manager.apply_pio_layout(&layout));
    manager.bind_process_image(input, output);
    ASSERT_EQ(0, manager.register_di_alias(5u, input_key, 0x01u));
    ASSERT_EQ(0, manager.register_ai_alias(5u, input_key));

    bool digital_value = false;
    uint32_t analog_value = 0;
    ASSERT_EQ(0, manager.read_di(5u, &digital_value));
    EXPECT_TRUE(digital_value);
    ASSERT_EQ(0, manager.read_ai(5u, &analog_value));
    EXPECT_EQ(0x77u, analog_value);

    manager.clear_aliases();
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, manager.read_di(5u, &digital_value));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, manager.read_ai(5u, &analog_value));
    ASSERT_EQ(0, manager.register_ai_alias(5u, input_key));
    EXPECT_EQ(0, manager.read_ai(5u, &analog_value));
    EXPECT_EQ(0x77u, analog_value);
}

TEST(AxisIniIoBinding, ReconfigureWithoutSectionClearsAliases) {
    zmc::AxisFbBridge bridge;
    ZmcPioOffsetLayout layout = make_io_binding_layout();
    ASSERT_EQ(0, bridge.apply_pio_layout(&layout));
    ASSERT_EQ(0, bridge.configure(nullptr, kIoBindingIni, 0u));

    bool digital_value = false;
    uint32_t analog_value = 0;
    ASSERT_EQ(ZMC_ERR_IO_PROCESS_IMAGE_UNBOUND,
              bridge.native_io()->read_di(32u, &digital_value));
    ASSERT_EQ(0, bridge.configure(nullptr, kNoIoBindingIni, 0u));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND,
              bridge.native_io()->read_di(32u, &digital_value));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND,
              bridge.native_io()->write_do(32u, true));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND,
              bridge.native_io()->read_ai(32u, &analog_value));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND,
              bridge.native_io()->write_ao(32u, analog_value));
}

TEST(AxisIniIoBinding, RegistrationFailureClearsPartialAliases) {
    const char invalid_object_ini[] =
        "[base]\naxis_num = 0\n"
        "[io_binding]\n"
        "di_3.source = pdo\n"
        "di_3.slave_idx = 2\n"
        "di_3.index = 0x6000\n"
        "di_3.subindex = 1\n"
        "di_3.mask = 0x01\n"
        "ao_4.source = pdo\n"
        "ao_4.slave_idx = 2\n"
        "ao_4.index = 0x7777\n"
        "ao_4.subindex = 1\n";
    zmc::AxisFbBridge bridge;
    ZmcPioOffsetLayout layout = make_io_binding_layout();
    ASSERT_EQ(0, bridge.apply_pio_layout(&layout));

    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND,
              bridge.configure(nullptr, invalid_object_ini, 0u));

    bool digital_value = false;
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND,
              bridge.native_io()->read_di(3u, &digital_value));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, bridge.native_io()->write_ao(4u, 0u));
}

TEST(AxisIniIoBinding, AppliesMaskedMultiBitPdoDigitalOutputBindings) {
    const char masked_ini[] =
        "[base]\naxis_num = 0\n"
        "[io_binding]\n"
        "di_10.source = pdo\n"
        "di_10.slave_idx = 2\n"
        "di_10.index = 0x6000\n"
        "di_10.subindex = 1\n"
        "di_10.mask = 0x02\n"
        "di_11.source = pdo\n"
        "di_11.slave_idx = 2\n"
        "di_11.index = 0x6000\n"
        "di_11.subindex = 1\n"
        "di_11.mask = 0x80\n"
        "do_20.source = pdo\n"
        "do_20.slave_idx = 2\n"
        "do_20.index = 0x7000\n"
        "do_20.subindex = 1\n"
        "do_20.mask = 0x01\n"
        "do_21.source = pdo\n"
        "do_21.slave_idx = 2\n"
        "do_21.index = 0x7000\n"
        "do_21.subindex = 1\n"
        "do_21.mask = 0x04\n";
    ZmcPioOffsetLayout layout = make_io_binding_layout();
    layout.descriptor_pod.pdo_cells[1].bit_size = 8u;

    zmc::AxisFbBridge configure_first;
    ASSERT_EQ(0, configure_first.configure(nullptr, masked_ini, 0u));
    EXPECT_EQ(0, configure_first.apply_pio_layout(&layout));

    zmc::AxisFbBridge layout_first;
    ASSERT_EQ(0, layout_first.apply_pio_layout(&layout));
    EXPECT_EQ(0, layout_first.configure(nullptr, masked_ini, 0u));
}

TEST(AxisIniIoBinding, ClearsAliasesWhenMaskExceedsPdoWidth) {
    const char invalid_mask_ini[] =
        "[base]\naxis_num = 0\n"
        "[io_binding]\n"
        "di_3.source = pdo\n"
        "di_3.slave_idx = 2\n"
        "di_3.index = 0x6000\n"
        "di_3.subindex = 1\n"
        "di_3.mask = 0x100\n";
    ZmcPioOffsetLayout layout = make_io_binding_layout();
    bool value = false;

    zmc::AxisFbBridge configure_first;
    ASSERT_EQ(0, configure_first.configure(nullptr, invalid_mask_ini, 0u));
    EXPECT_EQ(ZMC_ERR_IO_MASK_INVALID, configure_first.apply_pio_layout(&layout));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, configure_first.native_io()->read_di(3u, &value));

    zmc::AxisFbBridge layout_first;
    ASSERT_EQ(0, layout_first.apply_pio_layout(&layout));
    EXPECT_EQ(ZMC_ERR_IO_MASK_INVALID, layout_first.configure(nullptr, invalid_mask_ini, 0u));
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, layout_first.native_io()->read_di(3u, &value));
}

TEST(AxisIniIoBinding, DefersCompleteVirtualRegistryUntilPdoLayout) {
    const char virtual_ini[] =
        "[base]\naxis_num = 0\n"
        "[io_binding]\n"
        "di_10.source = virtual\n"
        "do_11.source = virtual\n"
        "ai_12.source = virtual\n"
        "ao_13.source = virtual\n";
    zmc::AxisFbBridge bridge;

    ASSERT_EQ(0, bridge.configure(nullptr, virtual_ini, 0u));
    EXPECT_FALSE(bridge.pdo_layout_ready());
    bool digital_value = false;
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, bridge.native_io()->read_di(10u, &digital_value));
    std::vector<zmc::IoMetadataEntry> metadata;
    ASSERT_EQ(0, bridge.get_io_metadata(metadata));
    EXPECT_TRUE(metadata.empty());

    // One layout transition materializes the complete configured registry.
    ZmcPioOffsetLayout layout = make_io_binding_layout();
    ASSERT_EQ(0, bridge.apply_pio_layout(&layout));
    ASSERT_EQ(0, bridge.native_io()->write_virtual_di(10u, true));
    ASSERT_EQ(0, bridge.native_io()->write_do(11u, true));
    ASSERT_EQ(0, bridge.native_io()->write_virtual_ai(12u, 0x12345678u));
    ASSERT_EQ(0, bridge.native_io()->write_ao(13u, 0x87654321u));

    uint32_t analog_value = 0u;
    ASSERT_EQ(0, bridge.native_io()->read_di(10u, &digital_value));
    EXPECT_TRUE(digital_value);
    ASSERT_EQ(0, bridge.native_io()->read_do(11u, &digital_value));
    EXPECT_TRUE(digital_value);
    ASSERT_EQ(0, bridge.native_io()->read_ai(12u, &analog_value));
    EXPECT_EQ(0x12345678u, analog_value);
    ASSERT_EQ(0, bridge.native_io()->read_ao(13u, &analog_value));
    EXPECT_EQ(0x87654321u, analog_value);

    ASSERT_EQ(0, bridge.get_io_metadata(metadata));
    ASSERT_EQ(4u, metadata.size());
    for (std::size_t i = 0; i < metadata.size(); ++i) {
        EXPECT_EQ(zmc::IO_SOURCE_KIND_VIRTUAL, metadata[i].source_kind);
    }
}

TEST(AxisIniIoBinding, AppliesNopSourcesWithoutRegisteringAliases) {
    const char nop_ini[] =
        "[base]\naxis_num = 0\n"
        "[io_binding]\n"
        "di_10.source = NOP\n"
        "do_11.source = nop\n"
        "ai_12.source = NoP\n"
        "ao_13.source = nOp\n";
    zmc::AxisFbBridge bridge;
    ZmcPioOffsetLayout layout = make_io_binding_layout();

    ASSERT_EQ(0, bridge.apply_pio_layout(&layout));
    ASSERT_EQ(0, bridge.configure(nullptr, nop_ini, 0u));

    std::vector<zmc::IoMetadataEntry> metadata;
    ASSERT_EQ(0, bridge.get_io_metadata(metadata));
    EXPECT_TRUE(metadata.empty());
}

TEST(AxisIniIoBinding, AppliesMixedPdoAndVirtualBindingsInBothOrders) {
    // Both configuration orders must converge on the same complete mixed-source registry.
    const char mixed_ini[] =
        "[base]\naxis_num = 0\n"
        "[io_binding]\n"
        "di_32.source = pdo\n"
        "di_32.slave_idx = 2\n"
        "di_32.index = 0x6000\n"
        "di_32.subindex = 1\n"
        "di_32.mask = 0x01\n"
        "do_33.source = virtual\n"
        "ai_34.source = virtual\n"
        "ao_32.source = pdo\n"
        "ao_32.slave_idx = 2\n"
        "ao_32.index = 0x7010\n"
        "ao_32.subindex = 2\n";
    ZmcPioOffsetLayout layout = make_io_binding_layout();

    zmc::AxisFbBridge configure_first;
    ASSERT_EQ(0, configure_first.configure(nullptr, mixed_ini, 0u));
    ASSERT_EQ(0, configure_first.apply_pio_layout(&layout));

    uint8_t first_input[4] = {0, 1u, 0, 0};
    uint8_t first_output[4] = {0};
    bind_process_image(&configure_first, first_input, first_output);
    bool digital_value = true;
    uint32_t analog_value = 1u;
    ASSERT_EQ(0, configure_first.native_io()->read_di(32u, &digital_value));
    EXPECT_TRUE(digital_value);
    ASSERT_EQ(0, configure_first.native_io()->read_do(33u, &digital_value));
    EXPECT_FALSE(digital_value);
    ASSERT_EQ(0, configure_first.native_io()->read_ai(34u, &analog_value));
    EXPECT_EQ(0u, analog_value);
    ASSERT_EQ(0, configure_first.native_io()->write_ao(32u, 0x5Au));
    bind_process_image(&configure_first, first_input, first_output);
    EXPECT_EQ(0x5Au, first_output[3]);

    zmc::AxisFbBridge layout_first;
    ASSERT_EQ(0, layout_first.apply_pio_layout(&layout));
    ASSERT_EQ(0, layout_first.configure(nullptr, mixed_ini, 0u));
    uint8_t second_input[4] = {0, 0u, 0, 0};
    uint8_t second_output[4] = {0};
    bind_process_image(&layout_first, second_input, second_output);
    ASSERT_EQ(0, layout_first.native_io()->read_di(32u, &digital_value));
    EXPECT_FALSE(digital_value);
    ASSERT_EQ(0, layout_first.native_io()->write_do(33u, true));
    ASSERT_EQ(0, layout_first.native_io()->write_virtual_ai(34u, 0x12345678u));
    ASSERT_EQ(0, layout_first.native_io()->read_do(33u, &digital_value));
    EXPECT_TRUE(digital_value);
    ASSERT_EQ(0, layout_first.native_io()->read_ai(34u, &analog_value));
    EXPECT_EQ(0x12345678u, analog_value);

    std::vector<zmc::IoMetadataEntry> metadata;
    ASSERT_EQ(0, layout_first.get_io_metadata(metadata));
    ASSERT_EQ(4u, metadata.size());
    EXPECT_EQ(zmc::IO_SOURCE_KIND_PDO, metadata[0].source_kind);
    EXPECT_EQ(zmc::IO_SOURCE_KIND_VIRTUAL, metadata[1].source_kind);
    EXPECT_EQ(zmc::IO_SOURCE_KIND_VIRTUAL, metadata[2].source_kind);
    EXPECT_EQ(zmc::IO_SOURCE_KIND_PDO, metadata[3].source_kind);
}

TEST(AxPouZeroAxis, RunHonorsCycleLimit) {
    zmc::AxPou pou(0);

    ASSERT_EQ(0u, pou.axis_num());
    pou.run(1);

    EXPECT_EQ(2, pou.getIndex());
}

TEST(AxPouZeroAxis, IgnoresSingleAxisCommandWithoutDefaultAxis) {
    zmc::AxisFbBridge bridge;
    zmc::MotionCmdPacket packet;
    static_cast<zmc::MotionPacket&>(packet).reset();
    ASSERT_EQ(static_cast<int>(ZMC_NO_ERROR), packet.powerOn());
    ASSERT_TRUE(bridge.command_interface().push_cmd_packet(packet));

    uint8_t input[1] = {0};
    uint8_t output[1] = {0};
    zmc::proc_img_i_addr parsed_input = {};
    zmc::proc_img_o_addr parsed_output = {};
    parsed_input.pBuf = input;
    parsed_output.pBuf = output;

    // The first cycle initializes the runner; the second dispatches the queued command.
    ASSERT_EQ(0, bridge.run_cycle_parsed(&parsed_input, &parsed_output));
    EXPECT_EQ(0, bridge.run_cycle_parsed(&parsed_input, &parsed_output));
    EXPECT_EQ(0, bridge.axis_count());
}
