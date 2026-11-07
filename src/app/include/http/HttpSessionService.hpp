#ifndef _ZMC_HTTP_SESSION_SERVICE_HPP_
#define _ZMC_HTTP_SESSION_SERVICE_HPP_
#include <stdint.h>
#include <string>
#include <thread>
#include "MotionApp.hpp"
#include "common/zmc/zmc_predef.h"
namespace httplib {
class Server;
struct Request;
struct Response;
}
namespace zmc {
namespace srv { class MotSrv; }
// Shared HTTP transport and session adapter. Model signature: GPT-6.
class MotHttpBase : public MotTransport {
public:
    // Start the listener and wait for its existing readiness state. GPT-6, 2026-10-29.
    int start() override;
    // Join the listener before reading its result.
    int wait() override;
    // Stop accepting requests and join response workers on the application thread.
    void stop() override;
    // Configure startup logging before binding and starting the listener.
    void set_log_level(int level) override { log_level_ = level; }
    // Decode and validate HTTP session credentials.
    bool validate(const httplib::Request& req, httplib::Response& res,
                  uint64_t* session = nullptr) const;
    // The transport owns the server; the service is borrowed for the listen lifetime.
    httplib::Server* pSvr;
    srv::MotSrv* service_;
    std::string host = "0.0.0.0";
    int port = ZMC_MOTAPP_HTTP_DEFAULT_PORT;
protected:
    MotHttpBase();
    ~MotHttpBase() override;
    int log_level_ = 0;
    int run();
    void bindSessions();
private:
    // Create a session after opening or joining the shared device.
    void open(const httplib::Request& req, httplib::Response& res);
    // Revoke one session without changing device resources.
    void close(const httplib::Request& req, httplib::Response& res);
    std::thread thread_;
    int result_ = 0;  // Written by the listener; read only after join.
};
}  // namespace zmc
#endif
