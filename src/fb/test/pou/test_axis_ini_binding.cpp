#include <gtest/gtest.h>

#include <string>

#include "bridge/AxisFbBridge.h"
#include "common/get_file_path.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "pou/AxisIniLoader.hpp"

namespace {

const char kBindingIni[] =
    "[base]\n"
    "axis_num = 2\n"
    "[axis_binding]\n"
    "axis_1.index = 1\n"
    "axis_0.name = X\n"
    "axis_1.name = Y\n"
    "axis_0.index = 2\n"
    "[axis_0]\n"
    "cycle_time = 1000\n"
    "[axis_1]\n"
    "cycle_time = 1000\n";

}  // namespace

TEST(AxisIniBinding, LoadsNamedAddressEntries) {
    AxisIniLoader loader(kBindingIni, true);
    AxisBindingConfig binding = AxisBindingConfig();
    ASSERT_EQ(0, loader.load_binding(&binding));
    ASSERT_EQ(1, binding.section_present);
    ASSERT_EQ(2, binding.binding_count);
    EXPECT_STREQ("X", binding.entries[0].name);
    EXPECT_EQ(0, binding.entries[0].axis_index);
    EXPECT_EQ(2u, binding.entries[0].address);
    EXPECT_STREQ("Y", binding.entries[1].name);
    EXPECT_EQ(1, binding.entries[1].axis_index);
    EXPECT_EQ(1u, binding.entries[1].address);
}

TEST(AxisIniBinding, WritesIndexValueDirectlyToAxisAddress) {
    zmc::AxisFbBridge bridge;

    ASSERT_EQ(0, bridge.configure(nullptr, kBindingIni, 2u));
    EXPECT_EQ(2, bridge.get_address(0));
    EXPECT_EQ(1, bridge.get_address(1));
}

TEST(AxisIniBinding, MissingSectionUsesDefaultAutoBinding) {
    // A missing section uses one-based sequential addresses for all axes.
    // AI modification signature: GPT-5 Codex
    const char* ini = "[base]\naxis_num = 2\n";
    zmc::AxisFbBridge bridge;

    ASSERT_EQ(0, bridge.configure(nullptr, ini, 2u));
    EXPECT_EQ(1, bridge.get_address(0));
    EXPECT_EQ(2, bridge.get_address(1));

    ASSERT_EQ(0, bridge.configure(nullptr, "[base]\naxis_num = 1\n", 1u));
    EXPECT_EQ(1, bridge.get_address(0));
    EXPECT_EQ(-1, bridge.get_address(1));
}

TEST(AxisIniBinding, LoadsBindingFromFile) {
    const std::string ini_path =
        zmc::_get_file_path("mhelper/data/zmc_motapp_demo_fak2.ini");
    AxisIniLoader loader(ini_path.c_str(), false);
    AxisBindingConfig binding = AxisBindingConfig();

    ASSERT_EQ(0, loader.load_binding(&binding));
    ASSERT_EQ(2, binding.binding_count);
    EXPECT_EQ(0, binding.entries[0].axis_index);
    EXPECT_EQ(2u, binding.entries[0].address);
    EXPECT_EQ(1, binding.entries[1].axis_index);
    EXPECT_EQ(1u, binding.entries[1].address);
}

TEST(AxisIniBinding, MissingSectionLeavesBindingEmpty) {
    AxisIniLoader loader("[base]\naxis_num = 1\n", true);
    AxisBindingConfig binding = AxisBindingConfig();

    ASSERT_EQ(0, loader.load_binding(&binding));
    EXPECT_EQ(0, binding.section_present);
    EXPECT_EQ(0, binding.binding_count);
}

TEST(AxisIniBinding, RejectsIncompleteRecord) {
    AxisIniLoader loader("[axis_binding]\naxis_0.name = X\n", true);
    AxisBindingConfig binding = AxisBindingConfig();

    EXPECT_EQ(ZMC_ERR_INI_MISSING_KEY, loader.load_binding(&binding));
}

TEST(AxisIniBinding, LoadsAddressOnlyRecord) {
    AxisIniLoader loader("[axis_binding]\naxis_0.index = 1\n", true);
    AxisBindingConfig binding = AxisBindingConfig();

    ASSERT_EQ(0, loader.load_binding(&binding));
    ASSERT_EQ(1, binding.binding_count);
    EXPECT_EQ(0, binding.entries[0].axis_index);
    EXPECT_EQ(1u, binding.entries[0].address);
}

TEST(AxisIniBinding, RejectsDuplicateName) {
    const char ini[] =
        "[axis_binding]\n"
        "axis_0.name = X\n"
        "axis_0.index = 1\n"
        "axis_1.name = X\n"
        "axis_1.index = 2\n";
    AxisIniLoader loader(ini, true);
    AxisBindingConfig binding = AxisBindingConfig();

    EXPECT_EQ(ZMC_ERR_INI_DUPLICATE, loader.load_binding(&binding));
}

TEST(AxisIniBinding, RejectsDuplicateOrZeroAddress) {
    const char duplicate_ini[] =
        "[axis_binding]\n"
        "axis_0.name = X\n"
        "axis_0.index = 1\n"
        "axis_1.name = Y\n"
        "axis_1.index = 1\n";
    const char zero_ini[] =
        "[axis_binding]\n"
        "axis_0.name = X\n"
        "axis_0.index = 0\n";
    AxisBindingConfig binding = AxisBindingConfig();

    AxisIniLoader duplicate_loader(duplicate_ini, true);
    EXPECT_EQ(ZMC_ERR_INI_DUPLICATE, duplicate_loader.load_binding(&binding));

    AxisIniLoader zero_loader(zero_ini, true);
    EXPECT_EQ(ZMC_ERR_INI_VALUE_RANGE, zero_loader.load_binding(&binding));
}

TEST(AxisIniBinding, RejectsEmptyName) {
    AxisIniLoader loader("[axis_binding]\naxis_0.name =\naxis_0.index = 1\n", true);
    AxisBindingConfig binding = AxisBindingConfig();

    EXPECT_EQ(ZMC_ERR_INI_VALUE_FORMAT, loader.load_binding(&binding));
}

TEST(AxisIniBinding, RejectsAxisIndexOutsideCapacity) {
    AxisIniLoader loader("[axis_binding]\naxis_64.name = X\naxis_64.index = 1\n", true);
    AxisBindingConfig binding = AxisBindingConfig();

    EXPECT_EQ(ZMC_ERR_INI_VALUE_RANGE, loader.load_binding(&binding));
}

TEST(AxisIniBinding, AcceptsHighestLogicalAxisIndex) {
    AxisIniLoader loader("[axis_binding]\naxis_63.name = X\naxis_63.index = 32\n", true);
    AxisBindingConfig binding = AxisBindingConfig();

    ASSERT_EQ(0, loader.load_binding(&binding));
    ASSERT_EQ(1, binding.binding_count);
    EXPECT_EQ(63, binding.entries[0].axis_index);
    EXPECT_EQ(32u, binding.entries[0].address);
}
