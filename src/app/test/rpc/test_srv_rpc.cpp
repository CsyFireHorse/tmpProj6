// Date: 2026-11-05; Drafted by: GPT-6 (Codex).
#include <gtest/gtest.h>
#include <chrono>
#include <cmath>
#include <future>
#include <memory>
#include <thread>
#include <tuple>
#include <vector>
#include "../SrvTestRuntime.hpp"
#include "common/get_file_path.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "rpc/MotionRpcWireTypes.h"
#include "rpc/client.h"

namespace {
using namespace zmc;

// GPT-6 Codex, 2026-10-22. These cases observe MsgPack without a C client or core calls.
class SrvRpcTest : public testing::Test {
   protected:
    void SetUp() override {
        ASSERT_EQ(0, runtime_.start({"--rpc", "--port=28969", "--ini",
                                     _get_file_path("mhelper/data/zmc_motapp_demo_fak2.ini")}));
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        do {
            if (!client_) client_.reset(new rpc::client("127.0.0.1", 28969));
            const auto state = client_->get_connection_state();
            if (state == rpc::client::connection_state::connected) break;
            if (state == rpc::client::connection_state::disconnected ||
                state == rpc::client::connection_state::reset)
                client_.reset();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } while (std::chrono::steady_clock::now() < deadline);
        ASSERT_TRUE(client_);
        ASSERT_EQ(rpc::client::connection_state::connected, client_->get_connection_state());
        client_->set_timeout(5000);
        const auto opened =
            client_->call("rpc_open", uint32_t(0)).as<std::tuple<int32_t, RpcSession>>();
        ASSERT_EQ(0, std::get<0>(opened));
        session_ = std::get<1>(opened);
    }
    void TearDown() override {
        if (session_) {
            EXPECT_EQ(0, client_->call("rpc_close", session_).as<int32_t>());
        }
        client_.reset();
        EXPECT_EQ(0, runtime_.finish());
    }
    // Wait for the heartbeat probe to restore command publishing.
    int waitForPing() {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        int result;
        do {
            result = client_->call("ping", session_, uint32_t(3000)).as<int32_t>();
            if (result != ZMC_ERR_MOTCMD_FLOW_PAUSED) return result;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        } while (std::chrono::steady_clock::now() < deadline);
        return result;
    }
    SrvTestRuntime runtime_;
    std::unique_ptr<rpc::client> client_;
    RpcSession session_ = 0;
};

TEST_F(SrvRpcTest, AxisAndTerminalMsgPackSchemas) {
    RpcFlowCtrlConfigWire cfg = {};
    cfg.period = 10;
    cfg.verboseFlag = 1;
    ASSERT_EQ(0, client_->call("setflowconfig", session_, cfg).as<int32_t>());
    ASSERT_EQ(0, client_->call("poweron", session_, int32_t(1)).as<int32_t>());
    ASSERT_EQ(
        0, client_->call("waitticks", session_, uint32_t(50), int32_t(1), uint32_t(0), uint32_t(2))
               .as<int32_t>());
    ASSERT_EQ(0, client_->call("setposition", session_, int32_t(1), 0.0).as<int32_t>());
    ASSERT_EQ(0, client_->call("setspeed", session_, int32_t(1), 100.0).as<int32_t>());
    ASSERT_EQ(0, client_->call("setaccel", session_, int32_t(1), 1000.0).as<int32_t>());
    ASSERT_EQ(0, client_->call("setdecel", session_, int32_t(1), 1000.0).as<int32_t>());
    ASSERT_EQ(0, client_->call("movesgabs", session_, int32_t(1), 12.0).as<int32_t>());
    RpcAxisState2Wire extended = {};
    bool reached = false;
    for (int i = 0; i < 500 && !reached; ++i) {
        const auto response = client_->call("getaxisstate2", session_, int32_t(1))
                                  .as<std::tuple<int32_t, RpcAxisState2Wire>>();
        extended = std::get<1>(response);
        reached = std::get<0>(response) == 0 && std::fabs(extended.act_pos - 12) < 0.01 &&
                  (extended.axis_state & 15) == 1;
        if (!reached) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ASSERT_TRUE(reached);
    EXPECT_EQ(0, extended.error_id);
    EXPECT_GT(extended.tick, 0u);
    const auto basic = client_->call("getaxisstate", session_, int32_t(1))
                           .as<std::tuple<int32_t, RpcAxisStateWire>>();
    ASSERT_EQ(0, std::get<0>(basic));
    EXPECT_EQ(0, std::get<1>(basic).error_id);
    EXPECT_NEAR(12, std::get<1>(basic).act_pos, 0.01);
    bool completed = false;
    for (int i = 0; i < 100 && !completed; ++i) {
        const auto response = client_->call("getcmdresults", session_)
                                  .as<std::tuple<int32_t, std::vector<RpcCmdResultInfoWire>>>();
        ASSERT_EQ(0, std::get<0>(response));
        for (const auto& result : std::get<1>(response)) {
            if (result.command_type != 0x43 || result.target_index != 1 || result.status != 4)
                continue;
            EXPECT_EQ(1u, result.target_kind);
            EXPECT_EQ(0u, result.error_code);
            EXPECT_NE(0u, result.id);
            completed = true;
        }
        if (!completed) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    ASSERT_TRUE(completed);
}

TEST_F(SrvRpcTest, ConcurrentPingsCompleteThroughTheMotionCycle) {
    ASSERT_EQ(0, waitForPing());
    auto first = client_->async_call("ping", session_, uint32_t(3000));
    std::vector<decltype(first)> pending;
    pending.push_back(std::move(first));
    for (int i = 1; i < 8; ++i)
        pending.push_back(client_->async_call("ping", session_, uint32_t(3000)));
    for (auto& response : pending) {
        ASSERT_EQ(std::future_status::ready, response.wait_for(std::chrono::seconds(4)));
        EXPECT_EQ(0, response.get().as<int32_t>());
    }
}

TEST_F(SrvRpcTest, StoppedCyclePingTimesOutAndRecovers) {
    ASSERT_EQ(0, client_->call("eni_select", session_,
                             _get_file_path("mhelper/data/pse_servo2.xml")).as<int32_t>());
    ASSERT_EQ(0, client_->call("ini_select", session_,
                             _get_file_path("mhelper/data/zmc_motapp_demo_fak2.ini")).as<int32_t>());
    ASSERT_EQ(0, client_->call("ecat_stop", session_).as<int32_t>());
    const int stopped = client_->call("ping", session_, uint32_t(100)).as<int32_t>();
    EXPECT_TRUE(stopped == ZMC_ERR_HOST_RECV_TIMEOUT || stopped == ZMC_ERR_MOTCMD_FLOW_PAUSED);
    ASSERT_EQ(0, client_->call("ecat_start", session_).as<int32_t>());
    EXPECT_EQ(0, waitForPing());
}

}  // namespace
