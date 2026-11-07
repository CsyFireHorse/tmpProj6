#include <gtest/gtest.h>

#include "AxisCiaConfig.hpp"
#include "AxPou.hpp"
#include "bridge/AxisFbBridge.h"
#include "common/get_file_path.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "common/zmc/zmc_predef.h"
#include "pou/AxisIniLoader.h"

namespace {

const char kIni1Axis[] = R"(
[base]
axis_num = 1
[axis_0]
is_axis_virtual = true
)";

const char kIni2Axis[] = R"(
[base]
axis_num = 2
[axis_0]
is_axis_virtual = true
[axis_1]
is_axis_virtual = true
)";

const char kIniOverrideAxis0[] = R"(
[base]
axis_num = 1
[axis_0]
is_axis_virtual = true
id = 7
)";

const char kIniOverrideAxis2[] = R"(
[base]
axis_num = 2
[axis_0]
id = 11
[axis_1]
is_axis_virtual = true
id = 22
)";

const char kIniInvalid[] = R"(
axis_num = 1
)";

const char kIniBaseWithStrings[] = R"(
[base]
axis_num = 1
eni_path = file_eni.xml
net_name = file_net
device_name = file_device
verbose_flag = 9
)";

const char kIniBaseAxisOnlyPatch[] = R"(
[base]
axis_num = 2
)";

}  // namespace

TEST(AxisFbBridgeReconfigure, DefaultConstructZeroAxes) {
    zmc::AxisFbBridge bridge;
    EXPECT_EQ(0, bridge.axis_count());
}

TEST(AxisFbBridgeReconfigure, LoadEniOwnsPdoLayout) {
    zmc::AxisFbBridge bridge;
    const std::string eni_path = zmc::_get_file_path("mhelper/data/pse_servo1.xml");

    ASSERT_EQ(0, bridge.load_eni(eni_path.c_str()));
    EXPECT_TRUE(bridge.pdo_layout_ready());
    EXPECT_EQ(1, bridge.servo_count());
}

TEST(AxisFbBridgeReconfigure, LoadEniRejectsMissingPath) {
    zmc::AxisFbBridge bridge;

    EXPECT_EQ(ZMC_ERR_BRIDGE_INVALID_ARG, bridge.load_eni(nullptr));
    EXPECT_FALSE(bridge.pdo_layout_ready());
}

TEST(AxisFbBridgeReconfigure, LoadConfigIniContent) {
    zmc::AxisFbBridge bridge;
    ASSERT_EQ(0, bridge.configure(nullptr, kIni1Axis, 1u));
    EXPECT_EQ(1, bridge.axis_count());
    EXPECT_EQ(0, bridge.auto_bind(0, 1));
}

TEST(AxisFbBridgeReconfigure, AxisCountResize) {
    zmc::AxisFbBridge bridge;
    ASSERT_EQ(0, bridge.configure(nullptr, kIni1Axis, 1u));
    EXPECT_EQ(1, bridge.axis_count());
    ASSERT_EQ(0, bridge.configure(nullptr, kIni2Axis, 2u));
    EXPECT_EQ(2, bridge.axis_count());
}

TEST(AxisFbBridgeReconfigure, ReconfigureSgAxisArrConsistent) {
    zmc::AxisFbBridge bridge;
    ASSERT_EQ(0, bridge.configure(nullptr, kIni1Axis, 1u));
    EXPECT_EQ(1, zmc::AxPouTaskBinderAuto::getAxisCount());
    ASSERT_EQ(0, bridge.configure(nullptr, kIni2Axis, 2u));
    EXPECT_EQ(2, zmc::AxPouTaskBinderAuto::getAxisCount());
    EXPECT_NE(nullptr, zmc::AxPouTaskBinderAuto::getAxisByIndex(1));
    ASSERT_EQ(0, bridge.configure(nullptr, kIni1Axis, 1u));
    EXPECT_EQ(1, zmc::AxPouTaskBinderAuto::getAxisCount());
    EXPECT_EQ(nullptr, zmc::AxPouTaskBinderAuto::getAxisByIndex(1));
}

TEST(AxisFbBridgeReconfigure, ReconfigureCommandInterfaceStable) {
    zmc::AxisFbBridge bridge;
    ASSERT_EQ(0, bridge.configure(nullptr, kIni1Axis, 1u));
    zmc::MotionCmdQueueBase* cmd_if = &bridge.command_interface();
    ASSERT_EQ(0, bridge.configure(nullptr, kIni2Axis, 2u));
    EXPECT_EQ(cmd_if, &bridge.command_interface());
}

TEST(AxisFbBridgeReconfigure, ConfigureNullIniPath) {
    zmc::AxisFbBridge bridge;
    EXPECT_EQ(0, bridge.configure(nullptr, kIni1Axis, 1u));
    EXPECT_EQ(1, bridge.axis_count());
}

TEST(AxisFbBridgeReconfigure, IniContentOverridesFileConfig) {
    zmc::AxPou pou("", kIniOverrideAxis0, 1u);

    ASSERT_EQ(0, pou.configure("", kIniOverrideAxis2, 1u));

    EXPECT_EQ(2, pou.axis_num());
    EXPECT_EQ(11, pou.get_config_ref(0).id);
    EXPECT_EQ(22, pou.get_config_ref(1).id);
}

TEST(AxisFbBridgeReconfigure, AxisCountUsesConfiguredAndRequestedMax) {
    zmc::AxPou pou;

    ASSERT_EQ(0, pou.configure("", kIni1Axis, 3u));

    EXPECT_EQ(3, pou.axis_num());
}

TEST(AxisFbBridgeReconfigure, AxisCountOverCapReturnsHostOverRange) {
    zmc::AxPou pou;

    EXPECT_EQ(ZMC_ERR_HOST_OVER_RANGE,
              pou.configure("", kIni1Axis, MC_MOTION_MAX_AXES + 1u));
}

TEST(AxisFbBridgeReconfigure, LogicalAxisCapacityIncludesSixtyFourAxes) {
    zmc::AxPouTaskBinderAuto pou;

    ASSERT_EQ(0, pou.configure("", "", MC_MOTION_MAX_AXES));
    EXPECT_EQ(64u, pou.axis_num());
    EXPECT_EQ(64, zmc::AxPouTaskBinderAuto::getAxisCount());
    EXPECT_EQ(32, MAX_NUM_SERVOS);
    EXPECT_EQ(64, MAX_AXIS_AUX);
}

TEST(AxisFbBridgeReconfigure, InvalidIniReturnsIniParse) {
    zmc::AxPou pou;

    EXPECT_EQ(ZMC_ERR_INI_MISSING_SECTION, pou.configure("", kIniInvalid, 1u));
}

TEST(AxisFbBridgeReconfigure, BaseConfigMergeUsesMask) {
    AxisCommonConfig baseCfg;
    AxisCommonConfig patchCfg;
    axisComcfg_default_init(&baseCfg);
    axisComcfg_default_init(&patchCfg);

    ASSERT_EQ(0, load_axis_ini_base_from_content(kIniBaseWithStrings, &baseCfg));
    ASSERT_EQ(0, load_axis_ini_base_from_content(kIniBaseAxisOnlyPatch, &patchCfg));
    axisComcfg_merge_by_mask(&baseCfg, &patchCfg);

    EXPECT_EQ(2u, baseCfg.axis_num);
    EXPECT_STREQ("file_eni.xml", baseCfg.eni_path);
    EXPECT_STREQ("file_net", baseCfg.net_name);
    EXPECT_STREQ("file_device", baseCfg.device_name);
    EXPECT_EQ(9u, baseCfg.verbose_flag);
}
