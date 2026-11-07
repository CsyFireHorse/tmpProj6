#ifndef ZMC_MOT_RPC_BASE_HPP_
#define ZMC_MOT_RPC_BASE_HPP_

#include <memory>
#include <string>
#include "MotionApp.hpp"

namespace rpc {
class server;
}

namespace zmc {
namespace srv {
class MotSrv;
}

// Own the RPC server and borrow the service for the transport lifetime.
class MotRpcBase : public MotTransport {
public:
    // Start RPC request workers before optional business. GPT-6, 2026-10-29.
    int start() override;
    // Wait for core exit and join protocol workers.
    int wait() override;
    // Stop and join protocol workers.
    void stop() override;
protected:
    // Save listener settings; binding creates the native server after core initialization.
    MotRpcBase(const std::string& host, int port);

    // Release the server after its workers have stopped.
    ~MotRpcBase() override;

    // Register session open and close methods.
    void bindBase();

    std::string host_;
    int port_;
    std::unique_ptr<rpc::server> server_;
    srv::MotSrv* service_;

};

}  // namespace zmc

#endif
