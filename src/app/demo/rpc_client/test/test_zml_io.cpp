#include "ZmlClientTest.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include <memory>

// IO data differs by scenario, never by transport.
class ZmlIoTest : public ZmlClientTest {
   protected:
    static std::unique_ptr<SrvTestRuntime>& ioRuntime() {
        static std::unique_ptr<SrvTestRuntime> runtime;
        return runtime;
    }

    static void SetUpTestSuite() {
        auto& runtime = ioRuntime();
        runtime.reset(new SrvTestRuntime());
        ASSERT_EQ(0, runtime->start({std::string("--") + ZML_TEST_MODE,
                                     "--port=" + std::to_string(ZML_TEST_PORT), "--ini",
                                     std::string(ZML_TEST_FIXTURE_DIR) + "/zml_io.ini"}));
        zml_sleep_ms(100);
        ASSERT_EQ(0, zml_sleep_ms(100));
    }

    static void TearDownTestSuite() {
        auto& runtime = ioRuntime();
        if (runtime) {
            EXPECT_EQ(0, runtime->finish());
            runtime.reset();
        }
    }

    void SetUp() override {
        ASSERT_TRUE(open(handle_));
        ZML_FlowCtrlConfig flow = {};
        flow.period = 10;
        flow.verbose_flag = 1;
        ASSERT_EQ(0, zml_set_flow_config(handle_, &flow));
    }

    void TearDown() override {
        if (second_) EXPECT_EQ(0, zml_close(&second_));
        if (handle_) EXPECT_EQ(0, zml_close(&handle_));
    }
};

// GPT-6 Codex, 2026-10-22. Inputs use configured defaults; outputs prove executed writes.
TEST_F(ZmlIoTest, SyncIoReadsAndAsyncWritesRoundTrip) {
    bool digital = true;
    uint32_t analog = 0xffffffffu;
    ASSERT_EQ(0, zml_read_di(handle_, 10, &digital));
    EXPECT_FALSE(digital);
    ASSERT_EQ(0, zml_read_ai(handle_, 12, &analog));
    EXPECT_EQ(0u, analog);
    for (bool value : {true, false}) {
        ASSERT_EQ(0, zml_write_do(handle_, 11, value));
        ASSERT_TRUE(
            wait([&]() { return zml_read_do(handle_, 11, &digital) == 0 && digital == value; }));
    }
    for (uint32_t value : {0x1234u, 0x80000000u, 0xffffffffu}) {
        ASSERT_EQ(0, zml_write_ao(handle_, 13, value));
        ASSERT_TRUE(
            wait([&]() { return zml_read_ao(handle_, 13, &analog) == 0 && analog == value; }));
    }
}

TEST_F(ZmlIoTest, ReadErrorPreservesOutput) {
    uint32_t value = 0x12345678;
    EXPECT_EQ(ZMC_ERR_IO_OBJECT_NOT_FOUND, zml_read_ai(handle_, 500, &value));
    EXPECT_EQ(0x12345678u, value);
}

TEST_F(ZmlIoTest, StoppedCycleReadTimesOutAndRecovers) {
    ASSERT_NO_FATAL_FAILURE(selectConfig("zml_io.ini", "pse_servo1.xml"));
    ASSERT_EQ(0, zml_ecat_stop(handle_));
    uint32_t value = 0x12345678;
    // GPT-6.1 Sol, 2026-11-04. A stopped consumer is detected without a stop hook.
    ASSERT_TRUE(wait([&]() { return zml_ping(handle_, 100) == ZMC_ERR_MOTCMD_FLOW_PAUSED; }));
    EXPECT_EQ(ZMC_ERR_MOTCMD_FLOW_PAUSED, zml_read_ai(handle_, 12, &value));
    EXPECT_EQ(0x12345678u, value);
    ASSERT_EQ(0, zml_ecat_start(handle_));
    ASSERT_EQ(0, zml_write_ao(handle_, 13, 0x80000000u));
    ASSERT_TRUE(
        wait([&]() { return zml_read_ao(handle_, 13, &value) == 0 && value == 0x80000000u; }));
}
