// Date: 2026-11-05; Drafted by: GPT-6 (Codex).
#include <gtest/gtest.h>
#include <cmath>
#include <fstream>
#include <iterator>
#include "HttpTestRuntime.hpp"
#include "common/get_file_path.hpp"

namespace {
using json = nlohmann::json;

// GPT-6 Codex, 2026-10-22. Every business action and observation uses real HTTP.
class SrvServiceTest : public testing::Test {
   protected:
    void SetUp() override {
        ASSERT_EQ(0,
                  runtime_.start({"--http", "--port=28971", "--ini",
                                  zmc::_get_file_path("mhelper/data/zmc_motapp_demo_fak2.ini")}));
        ASSERT_TRUE(runtime_.openSessionAndWaitHttp(28971, session_));
        ASSERT_TRUE(runtime_.waitEcatHttp(28971, session_));
        ASSERT_EQ(0, post("/flow_config", {{"period", 10}, {"verboseFlag", 1}}));
    }
    void TearDown() override {
        if (!session_.empty()) {
            EXPECT_EQ(0, post("/close", json::object()));
        }
        EXPECT_EQ(0, runtime_.finish());
    }
    httplib::Client client() {
        httplib::Client result("127.0.0.1", 28971);
        result.set_default_headers({{"X-Zmot-Session", session_}});
        return result;
    }
    json get(const std::string& path) {
        auto http = client();
        const auto result = http.Get("/zmot/api" + path);
        EXPECT_TRUE(result) << path;
        if (result) {
            EXPECT_FALSE(result->body.empty()) << path << " status=" << result->status;
        }
        return result ? json::parse(result->body) : json();
    }
    int post(const char* path, const json& body) {
        auto http = client();
        const auto result =
            http.Post(std::string("/zmot/api") + path, body.dump(), "application/json");
        EXPECT_TRUE(result) << path;
        if (result) {
            EXPECT_FALSE(result->body.empty()) << path << " status=" << result->status;
        }
        return result ? json::parse(result->body).at("code").get<int>() : -1;
    }
    template <class Predicate>
    bool wait(Predicate predicate) {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        do {
            if (predicate()) return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } while (std::chrono::steady_clock::now() < deadline);
        return false;
    }
    bool position(int axis, double target_position) {
        return wait([this, axis, target_position]() {
            const auto state = get("/axis_state?axis=" + std::to_string(axis)).at("axis");
            return !state.is_null() && state.at("error_id") == 0 &&
                   std::fabs(state.at("act_pos").get<double>() - target_position) < 0.01;
        });
    }
    void prepareAxis(int axis) {
        ASSERT_EQ(0, post("/poweron", {{"axis", axis}}));
        ASSERT_TRUE(wait([&]() {
            const auto state = get("/axis_state?axis=" + std::to_string(axis)).at("axis");
            return !state.is_null() && (state.at("axis_state").get<unsigned>() & 15) == 1;
        }));
        ASSERT_EQ(0, post("/setposition", {{"axis", axis}, {"pos", 0}}));
        ASSERT_TRUE(position(axis, 0));
        ASSERT_EQ(0, post("/speed", {{"axis", axis}, {"value", 100}}));
        ASSERT_EQ(0, post("/accel", {{"axis", axis}, {"value", 1000}}));
        ASSERT_EQ(0, post("/decel", {{"axis", axis}, {"value", 1000}}));
    }
    std::string bytes(const std::string& path) {
        std::ifstream input(path.c_str(), std::ios::binary);
        EXPECT_TRUE(input.is_open());
        return std::string(std::istreambuf_iterator<char>(input),
                           std::istreambuf_iterator<char>());
    }
    void selectFile(const std::string& path, const char* name, const char* route,
                    std::string& hash) {
        auto http = client();
        httplib::MultipartFormDataItems items = {
            {"file", bytes(path), name, "application/octet-stream"}};
        const auto result = http.Post("/zmot/api/files", items);
        ASSERT_TRUE(result);
        ASSERT_FALSE(result->body.empty()) << path << " status=" << result->status;
        const auto body = json::parse(result->body);
        ASSERT_EQ(0, body.at("code").get<int>());
        hash = body.at("hash").get<std::string>();
        ASSERT_EQ(0, post(route, {{"remote_file", std::string("server_files/") + name}}));
    }
    HttpTestRuntime runtime_;
    std::string session_;
};

TEST_F(SrvServiceTest, CacheResetReheatsExecutedParameters) {
    // Units posts carry the explicit num/den ratio: 5/2 = 2.5. GLM-5.3 (opencode).
    ASSERT_EQ(0, post("/units", {{"axis", 0}, {"num", 5}, {"den", 2}}));
    ASSERT_TRUE(wait([&]() { return get("/units?axis=0").at("value") == 2.5; }));
    ASSERT_EQ(0, post("/cache_reset", json::object()));
    ASSERT_TRUE(wait([&]() {
        const auto result = get("/units?axis=0");
        return result.at("code") == 0 && result.at("value") == 2.5;
    }));
}

TEST_F(SrvServiceTest, StatusStaysOnCurrentConfigAfterNextSelection) {
    const auto before = get("/session_status");
    const std::string current_ini = zmc::_get_file_path("mhelper/data/zmc_motapp_demo_fak2.ini");
    std::string eni_hash, ini_hash;
    ASSERT_NO_FATAL_FAILURE(selectFile(zmc::_get_file_path("mhelper/data/pse_servo1.xml"),
                                       "rpc_eni.xml", "/eni_select", eni_hash));
    ASSERT_NO_FATAL_FAILURE(selectFile(current_ini, "rpc_syscfg.ini", "/ini_select", ini_hash));
    auto files = get("/selected_config_files").at("files");
    ASSERT_EQ(2u, files.size());
    EXPECT_EQ("server_files/rpc_eni.xml", files[0].at("name"));
    EXPECT_EQ(eni_hash, files[0].at("hash"));
    EXPECT_EQ("server_files/rpc_syscfg.ini", files[1].at("name"));
    EXPECT_EQ(ini_hash, files[1].at("hash"));
    const auto after = get("/session_status");
    EXPECT_EQ(before.at("eni_name"), after.at("eni_name"));
    EXPECT_EQ(before.at("eni_hash"), after.at("eni_hash"));
}

TEST_F(SrvServiceTest, BasicExtendedAndTerminalJsonSchemas) {
    ASSERT_NO_FATAL_FAILURE(prepareAxis(0));
    ASSERT_NO_FATAL_FAILURE(prepareAxis(1));
    ASSERT_EQ(0, post("/movesgabs", {{"axis", 1}, {"pos", 15}}));
    // An omitted axis still selects zero after an explicit axis-one request.
    ASSERT_EQ(0, post("/movesgabs", {{"pos", 1}}));
    ASSERT_TRUE(position(1, 15));
    ASSERT_TRUE(position(0, 1));
    ASSERT_EQ(0, post("/flow_config", {{"period", 1}, {"verboseFlag", 0}}));
    ASSERT_TRUE(wait([&]() { return get("/axis_state2?axis=0").at("axis").is_null(); }));
    ASSERT_TRUE(wait([&]() { return !get("/axis_state?axis=0").at("axis").is_null(); }));
    const auto basic = get("/axis_state?axis=0").at("axis");
    EXPECT_NEAR(1, basic.at("act_pos").get<double>(), 0.01);
    EXPECT_FALSE(basic.contains("act_position"));
    ASSERT_EQ(0, post("/flow_config", {{"verboseFlag", 1}}));
    ASSERT_TRUE(wait([&]() { return !get("/axis_state2?axis=0").at("axis").is_null(); }));
    const auto extended = get("/axis_state2?axis=0").at("axis");
    EXPECT_NEAR(1, extended.at("set_pos").get<double>(), 0.01);
    EXPECT_EQ(0, extended.at("error_id"));
    EXPECT_GT(extended.at("tick").get<unsigned>(), 0u);
    EXPECT_TRUE(extended.at("set_position").is_number_integer());
    EXPECT_TRUE(extended.at("act_position").is_number_integer());
    // Typed reads publish integer JSON values. GLM-5.3 (opencode), 2026-10-23.
    const auto state = get("/read_axis_state?axis=0");
    ASSERT_EQ(0, state.at("code").get<int>());
    EXPECT_TRUE(state.at("value").is_number_integer());
    EXPECT_EQ(basic.at("axis_state"), state.at("value"));
    json terminal;
    ASSERT_TRUE(wait([&]() {
        const auto results = get("/cmdresult");
        for (const auto& item : results.at("cmd_results")) {
            if (item.at("command_type") == 0x43 && item.at("target_index") == 0 &&
                item.at("status") == 4) {
                terminal = item;
                return true;
            }
        }
        return false;
    }));
    EXPECT_EQ(10u, terminal.size());
    EXPECT_TRUE(terminal.contains("data_ext"));
    EXPECT_EQ(1, terminal.at("target_kind"));
    EXPECT_EQ(0, terminal.at("error_code"));
    EXPECT_NE(0u, terminal.at("id").get<unsigned>());
}

}  // namespace
