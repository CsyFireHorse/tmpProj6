#ifndef ZMC_SRV_TEST_RUNTIME_HPP_
#define ZMC_SRV_TEST_RUNTIME_HPP_
#include <string>
#include <thread>
#include <vector>
#include "MotionApp.hpp"

// GPT-6, 2026-10-29. Integration tests own an application returned by the production factory.
class SrvTestRuntime {
   public:
    // Prepare production arguments and launch the service; readiness belongs to the client.
    int start(const std::vector<std::string>& args);
    // Request service exit, join its thread, then return the service result.
    int finish();
    ~SrvTestRuntime();

   private:
    std::unique_ptr<zmc::MotionApp> app_;
    std::thread thread_;
    int result_ = 0;  // Read only after join; no test synchronization or core-state injection.
};
#endif
