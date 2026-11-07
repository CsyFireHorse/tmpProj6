#ifndef ZMC_MOTION_APP_HPP_
#define ZMC_MOTION_APP_HPP_

#include <memory>
#include <string>
#include <vector>
#include "srv/MotSrv.hpp"

// Drafted by GPT-6 (Codex), 2026-10-29.
namespace zmc {

// Own protocol resources while borrowing the initialized service.
class MotTransport {
   public:
    // Join protocol workers before releasing their routes.
    virtual ~MotTransport() {}
    // Bind requests to the application's service before starting workers.
    virtual int bind(srv::MotSrv& service) = 0;
    // Return only after request processing is ready or startup fails.
    virtual int start() = 0;
    // Wait for termination and return the protocol result.
    virtual int wait() = 0;
    // Stop protocol request processing.
    virtual void stop() = 0;
    // Configure startup logging before binding or starting the transport.
    virtual void set_log_level(int) {}
};

// Own one service and its selected protocol. GPT-6 (Codex), 2026-10-29.
class MotionApp {
   public:
    // Construct without starting workers or reading optional configuration.
    MotionApp();
    // Release resources after run has returned.
    ~MotionApp();
    // Start srv, protocol and optional business, then wait and clean up.
    int run(const std::vector<std::string>& args);
    // Configure protocol startup logging before run.
    void set_log_level(int level);
    // Request exit without joining the caller's run thread.
    void requestExit();

   private:
    MotionApp(const MotionApp&) = delete;
    MotionApp& operator=(const MotionApp&) = delete;
    // Stop protocol workers before releasing core state.
    int shutdown();
    // Assemble the command-line-selected transport without opening its listener.
    int configure(const std::vector<std::string>& args);
    srv::MotSrv srv_;  // Construct first and destroy last.
    std::unique_ptr<MotTransport> transport_;
    int log_level_ = 0;
    friend std::unique_ptr<MotionApp> createMotionApp(const std::vector<std::string>&);
};
// Return an assembled application, or null for invalid or unsupported protocol arguments.
std::unique_ptr<MotionApp> createMotionApp(const std::vector<std::string>& args);
// Start srv, then the protocol, then optional business as one startup transaction.
int runMotionApp(const std::vector<std::string>& args);

}  // namespace zmc
#endif
