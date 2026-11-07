#include "rpc/MotRpcBase.hpp"
#include "srv/MotSrv.hpp"
#include "rpc/server.h"
#include <chrono>
#include <thread>
#include <tuple>

namespace zmc {
// The runner owns protocol resources; srv owns command state. Model signature: GPT-6.
MotRpcBase::MotRpcBase(const std::string& host, int port)
    : host_(host), port_(port), service_(nullptr) {}

MotRpcBase::~MotRpcBase() = default;

void MotRpcBase::bindBase() {
    server_->bind("rpc_open", [this](uint32_t device) {
        RpcSession session = 0;
        const int ret = service_->open(std::to_string(device), session);
        return std::make_tuple(static_cast<int32_t>(ret), session);
    });
    server_->bind("rpc_close", [this](RpcSession session) {
        return static_cast<int32_t>(service_->close(session));
    });
}

// GPT-6 (Codex), 2026-10-29. Separate protocol readiness from lifetime waiting.
int MotRpcBase::start() {
    server_->async_run(2);
    return 0;
}

void MotRpcBase::stop() {
    // An assembled transport may be destroyed before it is bound.
    if (server_) server_->stop();
}

int MotRpcBase::wait() {
    // The core drives heartbeat; this runner only stops and joins protocol workers.
    while (!service_->exitRequested()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    server_->stop();
    return 0;
}
}  // namespace zmc
