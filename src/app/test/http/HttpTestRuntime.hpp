#ifndef ZMC_HTTP_TEST_RUNTIME_HPP_
#define ZMC_HTTP_TEST_RUNTIME_HPP_

#include <chrono>
#include <string>
#include <thread>

#include "../SrvTestRuntime.hpp"
#include "common/thirdlib/nlohmann/json.hpp"
#include "common/thirdlib/yhirose/httplib.h"

// Open a session through HTTP and wait for the listener.
class HttpTestRuntime : public SrvTestRuntime {
   public:
    // GPT-6 (Codex), 2026-10-29. Protocol readiness precedes optional bus startup.
    bool waitEcatHttp(int port, const std::string& session, int attempts = 500) {
        httplib::Client client("127.0.0.1", port);
        client.set_default_headers({{"X-Zmot-Session", session}});
        for (int i = 0; i < attempts; ++i) {
            const auto result = client.Get("/zmot/api/ecat_bus_is_running");
            if (result && nlohmann::json::parse(result->body).value("value", false)) return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return false;
    }

    bool openSessionAndWaitHttp(int port, std::string& session, int attempts = 500) {
        for (int i = 0; i < attempts; ++i) {
            httplib::Client client("127.0.0.1", port);
            client.set_connection_timeout(0, 100000);
            const auto result = client.Post("/zmot/api/open", "{}", "application/json");
            if (result && result->status == 200) {
                session = nlohmann::json::parse(result->body).at("session").get<std::string>();
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return false;
    }
};

#endif
