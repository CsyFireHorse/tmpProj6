#ifndef ZML_CLIENT_TEST_HPP_
#define ZML_CLIENT_TEST_HPP_

#include <gtest/gtest.h>
#include <chrono>
#include <cmath>
#include <string>
#include <thread>
#include "../../../test/SrvTestRuntime.hpp"
#include "zml_motion_api_ex.h"

// GPT-6 Codex, 2026-10-29. Shared runtime and observations do not open client sessions.
class ZmlClientSupport {
   protected:
    // Public wire values stay together without including internal enum headers.
    enum {
        kAxisStateMask = 0x0f,
        kStandstill = 1,
        kAxisTarget = 1,
        kDone = 4,
        kMoveAbsolute = 0x43
    };
    void startRuntime(const char* ini, const char* script = nullptr);
    void prepareAxis(int axis);
    void selectConfig(const char* ini, const char* eni);
    bool position(int axis, double expected);
    bool axisState(int axis, unsigned expected);
    bool parameter(ZML_HANDLE handle, int axis, ZML_RESULT (*getter)(ZML_HANDLE, int32_t, double*),
                   double expected);
    std::string fixture(const char* name) const;
    // Poll only public observations; retain the final diagnostic on failure.
    template <class Predicate>
    bool wait(Predicate predicate) {
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
        do {
            if (predicate()) return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } while (std::chrono::steady_clock::now() < deadline);
        return false;
    }

    SrvTestRuntime runtime_;
    ZML_HANDLE handle_ = nullptr;
    ZML_HANDLE second_ = nullptr;
};

// GPT-6 Codex, 2026-10-22. Both transports execute this public C API contract.
class ZmlClientTest : public testing::Test, protected ZmlClientSupport {
   protected:
    void SetUp() override;
    void TearDown() override;
    void start(const char* ini, const char* script = nullptr);
    bool open(ZML_HANDLE& handle);
    void writeFile(const char* name, const std::string& bytes);
    std::string readFile(const char* name);
};

#endif
