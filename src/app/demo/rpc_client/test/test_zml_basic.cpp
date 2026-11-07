#include "ZmlClientTest.hpp"
#include "common/zmc/fbt_errorcode_def.h"

// GPT-6 Codex, 2026-10-29. Shared client contract tests.
class ZmlBasicTest : public testing::Test, protected ZmlClientSupport {
   protected:
    void SetUp() override { ASSERT_NO_FATAL_FAILURE(startRuntime("zml_motion.ini")); }
    void TearDown() override {
        if (second_) EXPECT_EQ(0, zml_close(&second_));
        if (handle_) EXPECT_EQ(0, zml_close(&handle_));
        EXPECT_EQ(0, runtime_.finish());
    }

    ZML_RESULT configureFlow() {
        ZML_FlowCtrlConfig flow = {};
        flow.period = 10;
        flow.verbose_flag = 1;
        return zml_set_flow_config(handle_, &flow);
    }

    const std::string endpoint_ = "127.0.0.1:" + std::to_string(ZML_TEST_PORT);
};

// The startup script must be selected before the runtime starts.
class ZmlBasicStartupTest : public ZmlBasicTest {
    void SetUp() override {
        ASSERT_NO_FATAL_FAILURE(startRuntime("zml_motion.ini", "zml_startup.zcode"));
    }
};

// Exercise the complete C client, transport and EcatMotion echo path. GPT-6 Codex.
TEST_F(ZmlBasicTest, MailboxPingEchoesUnsignedValues) {
    ASSERT_TRUE(wait([&]() { return zml_open(&handle_, endpoint_.c_str()) == 0; }));
    ASSERT_EQ(0, configureFlow());
    uint32_t pongValue = 0;
    for (uint32_t pingValue : {0u, 0x12345678u, 0xffffffffu}) {
        pongValue = ~pingValue;
        ASSERT_EQ(0, zml_mailbox_ping(handle_, pingValue, &pongValue));
        EXPECT_EQ(pingValue, pongValue);
    }
    EXPECT_EQ(ZMC_ERR_RPC_CLIENT_NOT_OPEN, zml_mailbox_ping(nullptr, 7, &pongValue));
    EXPECT_EQ(0xffffffffu, pongValue);
    EXPECT_EQ(ZMC_ERR_RPC_CLIENT_INVALID_ARG, zml_mailbox_ping(handle_, 7, nullptr));
}

TEST_F(ZmlBasicTest, TwoHandlesCloseIndependently) {
    ASSERT_TRUE(wait([&]() { return zml_open(&handle_, endpoint_.c_str()) == 0; }));
    ASSERT_EQ(0, configureFlow());
    ASSERT_TRUE(wait([&]() { return zml_open(&second_, endpoint_.c_str()) == 0; }));
    ASSERT_EQ(0, zml_ax_set_velocity(handle_, 0, 31.5));
    ASSERT_TRUE(parameter(second_, 0, zml_ax_get_speed, 31.5));
    ASSERT_EQ(0, zml_close(&handle_));
    EXPECT_EQ(nullptr, handle_);
    ASSERT_EQ(0, zml_ax_set_velocity(second_, 0, 42.5));
    ASSERT_TRUE(parameter(second_, 0, zml_ax_get_speed, 42.5));
}

TEST_F(ZmlBasicTest, LastClosePreservesRuntime) {
    ASSERT_TRUE(wait([&]() { return zml_open(&handle_, endpoint_.c_str()) == 0; }));
    ASSERT_EQ(0, configureFlow());
    ASSERT_NO_FATAL_FAILURE(prepareAxis(0));
    ASSERT_EQ(0, zml_ax_move_abs(handle_, 0, 5));
    ASSERT_TRUE(position(0, 5));
    ASSERT_EQ(0, zml_close(&handle_));
    ASSERT_TRUE(wait([&]() { return zml_open(&handle_, endpoint_.c_str()) == 0; }));
    bool running = false;
    ASSERT_EQ(0, zml_ecat_bus_is_running(handle_, &running));
    EXPECT_TRUE(running);
    ASSERT_TRUE(position(0, 5));
}

TEST_F(ZmlBasicTest, RestartLoadsConfiguredParameters) {
    ASSERT_TRUE(wait([&]() { return zml_open(&handle_, endpoint_.c_str()) == 0; }));
    ASSERT_EQ(0, configureFlow());
    ASSERT_NO_FATAL_FAILURE(selectConfig("zml_motion.ini", "pse_servo2.xml"));
    ASSERT_TRUE(parameter(handle_, 0, zml_ax_get_units, 10));
    ASSERT_EQ(0, zml_ax_set_units(handle_, 0, 5, 2));
    ASSERT_EQ(0, zml_ax_set_velocity(handle_, 0, 31.5));
    ASSERT_TRUE(parameter(handle_, 0, zml_ax_get_units, 2.5));
    ASSERT_TRUE(parameter(handle_, 0, zml_ax_get_speed, 31.5));
    ASSERT_EQ(0, zml_ecat_stop(handle_));
    ASSERT_EQ(0, zml_ecat_start(handle_));
    ASSERT_TRUE(parameter(handle_, 0, zml_ax_get_units, 10));
    ASSERT_TRUE(parameter(handle_, 0, zml_ax_get_speed, 5));
    ASSERT_TRUE(parameter(handle_, 0, zml_ax_get_accel, 20));
}

TEST_F(ZmlBasicTest, SelectedConfigAppliesOnNextStart) {
    ASSERT_TRUE(wait([&]() { return zml_open(&handle_, endpoint_.c_str()) == 0; }));
    ASSERT_EQ(0, configureFlow());
    ASSERT_TRUE(parameter(handle_, 0, zml_ax_get_units, 10));
    const std::string eni = fixture("pse_servo1.xml");
    const std::string ini = fixture("zml_next.ini");
    ASSERT_EQ(0, zml_download_eni(handle_, eni.c_str()));
    ASSERT_EQ(0, zml_download_syscfg(handle_, ini.c_str()));
    ASSERT_TRUE(parameter(handle_, 0, zml_ax_get_units, 10));
    ZML_FlowCtrlStatus flow = {};
    ASSERT_TRUE(
        wait([&]() { return zml_get_flow_status(handle_, &flow) == 0 && flow.axis_num == 2; }));
    ASSERT_EQ(0, zml_ecat_stop(handle_));
    ASSERT_EQ(0, zml_ecat_start(handle_));
    ASSERT_TRUE(parameter(handle_, 0, zml_ax_get_units, 25));
    ASSERT_TRUE(
        wait([&]() { return zml_get_flow_status(handle_, &flow) == 0 && flow.axis_num == 1; }));
}

TEST_F(ZmlBasicTest, StopStartDiscardsPendingScript) {
    ASSERT_TRUE(wait([&]() { return zml_open(&handle_, endpoint_.c_str()) == 0; }));
    ASSERT_EQ(0, configureFlow());
    ASSERT_NO_FATAL_FAILURE(selectConfig("zml_motion.ini", "pse_servo2.xml"));
    ASSERT_EQ(0, zml_execute_command(handle_, "wait_ticks 1000000\naxis 0\ndefpos 1234"));
    ASSERT_EQ(0, zml_ecat_stop(handle_));
    ASSERT_EQ(0, zml_ecat_start(handle_));
    ASSERT_NO_FATAL_FAILURE(prepareAxis(0));
    ASSERT_EQ(0, zml_ax_move_abs(handle_, 0, 1));
    ASSERT_EQ(0, zml_ax_wait_idle(handle_, 0, 10000));
    ASSERT_TRUE(position(0, 1));
}

TEST_F(ZmlBasicStartupTest, StartupScriptAndClientShareRuntime) {
    ASSERT_TRUE(wait([&]() { return zml_open(&handle_, endpoint_.c_str()) == 0; }));
    ASSERT_EQ(0, configureFlow());
    ASSERT_TRUE(position(0, 12));
    ASSERT_EQ(0, zml_ax_wait_ticks(handle_, 2, 0));
    ASSERT_EQ(0, zml_ax_move_rel(handle_, 0, 4));
    ASSERT_TRUE(position(0, 16));
}
