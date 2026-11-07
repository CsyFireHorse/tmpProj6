// Date: 2026-11-05; Drafted by: GPT-6 (Codex).
#include <gtest/gtest.h>
#include <chrono>
#include <string>
#include <thread>
#include "HttpTestRuntime.hpp"
#include "common/thirdlib/nlohmann/json.hpp"
#include "common/get_file_path.hpp"
namespace {
using json = nlohmann::json;


// GPT-6 Codex, 2026-10-22. Fixed protocol values avoid implementation enum dependencies.
const unsigned CMD_TARGET_DI = 0x10;
namespace wire {
enum { FB_STAT_DONE = 4, FBM_IO_READ_DO = 0x72, FBM_IO_WRITE_DO = 0x74,
       FBM_IO_WRITE_AO = 0x77, FBM_IO_READ_AO = 0x78 };
}
// Model signature: GPT-6. Observe real IO completion through the production core and fake PDO.
class HttpIoCommandsTest : public testing::Test {
   protected:
    void SetUp() override {
        ASSERT_EQ(0, runtime_.start({"--http", "--port=" + std::to_string(port_), "--ini",
                        zmc::_get_file_path("mhelper/data/zmc_motapp_http_io_metadata_test.ini")}));
        ASSERT_TRUE(runtime_.openSessionAndWaitHttp(port_, session_));
    }
    void TearDown() override {
        if (!session_.empty()) {
            auto client = makeClient();
            const auto closed = client.Post("/zmot/api/close", "{}", "application/json");
            EXPECT_TRUE(closed);
            if (closed) EXPECT_EQ(0, json::parse(closed->body).at("code").get<int>());
        }
        EXPECT_EQ(0, runtime_.finish());
    }
    httplib::Client makeClient() const {
        httplib::Client client("127.0.0.1", port_);
        client.set_default_headers({{"X-Zmot-Session", session_}});
        return client;
    }
    json waitResult(httplib::Client& client, uint16_t type, uint32_t io) {
        for (int i = 0; i < 300; ++i) {
            const auto response = client.Get("/zmot/api/io_results");
            if (response) {
                const json body = json::parse(response->body);
                for (const auto& item : body.at("io_results"))
                    if (item.at("command_type") == type && ((item.at("target_kind").get<unsigned>() - CMD_TARGET_DI) & 3) * 256 +
                            item.at("target_index").get<unsigned>() == io)
                        return item;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return json();
    }
    int port_ = 28968;
    HttpTestRuntime runtime_;
    std::string session_;
};

TEST_F(HttpIoCommandsTest, SessionTransportsAndIndependentClose) {
    httplib::Client client = makeClient();
    auto result = client.Post("/zmot/api/open_device", "{}", "application/json");
    ASSERT_TRUE(result);
    ASSERT_EQ(200, result->status);
    const std::string second = json::parse(result->body).at("session").get<std::string>();
    EXPECT_NE(session_, second);

    client.set_default_headers({});
    result = client.Get("/zmot/api/io_results?session=" + second);
    ASSERT_TRUE(result);
    EXPECT_EQ(0, json::parse(result->body).at("code").get<int>());
    result = client.Post("/zmot/api/write_do",
                         "{\"session\":\"" + second + "\",\"io_number\":11,\"value\":true}",
                         "application/json");
    ASSERT_TRUE(result);
    EXPECT_EQ(0, json::parse(result->body).at("code").get<int>());
    client.set_default_headers({{"X-Zmot-Session", second}});
    result = client.Post("/zmot/api/cache_reset", "{}", "application/json");
    ASSERT_TRUE(result);
    EXPECT_EQ(0, json::parse(result->body).at("code").get<int>());
    client.set_default_headers({});
    result = client.Get("/zmot/api/selected_config_files");
    ASSERT_TRUE(result);
    EXPECT_EQ(401, result->status);
    httplib::MultipartFormDataItems fields = {{"session", second, "", "text/plain"}};
    result = client.Post("/zmot/api/cache_reset", fields);
    ASSERT_TRUE(result);
    EXPECT_EQ(200, result->status);
    EXPECT_EQ(0, json::parse(result->body).at("code").get<int>());

    result = client.Post("/zmot/api/close_device", "{\"session\":\"" + second + "\"}",
                         "application/json");
    ASSERT_TRUE(result);
    EXPECT_EQ(0, json::parse(result->body).at("code").get<int>());
    client.set_default_headers({{"X-Zmot-Session", second}});
    result = client.Get("/zmot/api/io_results");
    ASSERT_TRUE(result);
    EXPECT_EQ(401, result->status);
    client.set_default_headers({{"X-Zmot-Session", session_}});
    result = client.Get("/zmot/api/io_results");
    ASSERT_TRUE(result);
    EXPECT_EQ(0, json::parse(result->body).at("code").get<int>());
}

TEST_F(HttpIoCommandsTest, CompletesTypedIoThroughTheRealBackend) {
    const struct {
        const char* path;
        const char* body;
        uint16_t type;
        uint32_t io;
    } cases[] = {
        {"write_do", "{\"io_number\":11,\"value\":true}", wire::FBM_IO_WRITE_DO, 11},
        {"read_do", "{\"io_number\":11}", wire::FBM_IO_READ_DO, 11},
        {"write_ao", "{\"io_number\":13,\"value\":1234}", wire::FBM_IO_WRITE_AO, 13},
        {"read_ao", "{\"io_number\":13}", wire::FBM_IO_READ_AO, 13},
    };
    auto client = makeClient();
    for (const auto& item : cases) {
        const auto response =
            client.Post(std::string("/zmot/api/") + item.path, item.body, "application/json");
        ASSERT_TRUE(response);
        ASSERT_EQ(0, json::parse(response->body).at("code").get<int>());
        const json result = waitResult(client, item.type, item.io);
        ASSERT_TRUE(result.is_object()) << item.path;
        EXPECT_EQ(wire::FB_STAT_DONE, result.at("status").get<int>());
        EXPECT_EQ(0, result.at("error_code").get<int>()) << item.path;
        if (item.type == wire::FBM_IO_READ_DO) EXPECT_EQ(1, result.at("data").get<int>());
        if (item.type == wire::FBM_IO_READ_AO) EXPECT_EQ(1234, result.at("data").get<int>());
    }

    // Retained HTTP IO results survive dashboard clearing.
    const json first = waitResult(client, wire::FBM_IO_READ_AO, 13);
    ASSERT_TRUE(first.is_object());
    EXPECT_EQ(10u, first.size());
    EXPECT_TRUE(first.contains("data_ext"));
    EXPECT_NE(0u, first.at("id").get<uint32_t>());
    EXPECT_EQ(1u, first.at("io_count").get<unsigned>());
    EXPECT_EQ(first, waitResult(client, wire::FBM_IO_READ_AO, 13));
    const auto cleared = client.Post("/zmot/api/dashboard_clear", "{}", "application/json");
    ASSERT_TRUE(cleared);
    EXPECT_EQ(0, json::parse(cleared->body).at("code").get<int>());
    EXPECT_EQ(first, waitResult(client, wire::FBM_IO_READ_AO, 13));
}
}  // namespace
