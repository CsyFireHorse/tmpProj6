// RPC session lifecycle and wire validation tests.
// GPT-6 Codex, 2026-11-05. Client behavior lives in the shared public API suite.

#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <tuple>

#include "../SrvTestRuntime.hpp"
#include "common/get_file_path.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "rpc/MotionRpcWireTypes.h"
#include "rpc/client.h"

namespace {

const uint16_t kSessionTestPort = 28779;
const int32_t kInvalidSession = static_cast<int32_t>(ZMC_ERR_HOST_INVALID_HANDLE);

class RpcSessionTest : public ::testing::Test {
   protected:
    void SetUp() override {
        ASSERT_EQ(0, runtime_.start({"--rpc", "--port=" + std::to_string(kSessionTestPort),
                        "--ini", zmc::_get_file_path("mhelper/data/zmc_motapp_demo_fak2.ini")}));
        bool ready = false;
        for (int attempt = 0; attempt < 50 && !ready; ++attempt) {
            try {
                rpc::client probe("127.0.0.1", kSessionTestPort);
                probe.set_timeout(200);
                const auto opened =
                    probe.call("rpc_open", uint32_t(0)).as<std::tuple<int32_t, RpcSession>>();
                const RpcSession session = std::get<1>(opened);
                ready = std::get<0>(opened) == 0 &&
                        probe.call("rpc_close", session).as<int32_t>() == 0;
            } catch (const std::exception&) {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        }
        ASSERT_TRUE(ready);
    }

    void TearDown() override {
        EXPECT_EQ(0, runtime_.finish());
    }

    SrvTestRuntime runtime_;
};

TEST_F(RpcSessionTest, BusinessCallsRequireAnActiveSession) {
    rpc::client raw("127.0.0.1", kSessionTestPort);
    raw.set_timeout(1000);
    const auto opened = raw.call("rpc_open", uint32_t(0)).as<std::tuple<int32_t, RpcSession>>();
    ASSERT_EQ(0, std::get<0>(opened));
    const RpcSession session = std::get<1>(opened);
    ASSERT_NE(0U, session);

    const auto valid =
        raw.call("get_version_info", session).as<std::tuple<int32_t, RpcVersionInfoWire>>();
    EXPECT_EQ(0, std::get<0>(valid));
    EXPECT_EQ(0, raw.call("rpc_close", session).as<int32_t>());
    const auto closed =
        raw.call("get_version_info", session).as<std::tuple<int32_t, RpcVersionInfoWire>>();
    EXPECT_EQ(kInvalidSession, std::get<0>(closed));
}

}  // namespace
