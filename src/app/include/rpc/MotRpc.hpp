#ifndef ZMC_MOT_RPC_ECAT_HPP_
#define ZMC_MOT_RPC_ECAT_HPP_

#ifdef ZMC_ENABLE_RPC
#include "MotRpcQcmd.hpp"
#else
#include "MotionApp.hpp"
#endif

namespace zmc {

#ifdef ZMC_ENABLE_RPC
// Register EtherCAT methods after the existing RPC method groups.
class MotRpcEcat : public MotRpcQcmd {
public:
    // Associate the service and register all existing RPC methods.
    int bind(srv::MotSrv& service) override;
private:
    // Construct the application-owned RPC transport. GPT-6, 2026-10-29.
    MotRpcEcat(const std::string& host, int port);

    // Release the transport after its request workers stop.
    ~MotRpcEcat() override;

    // Register the existing EtherCAT and identity methods.
    void bindEcat();

    friend class MotionApp;
};

// Select the complete RPC transport.
using MotRpc = MotRpcEcat;
#else
// Keep the factory interface without RPC resources. GPT-6 (Codex), 2026-10-29.
class MotRpc : public MotTransport {
public:
    // Report the existing unsupported-protocol error.
    int bind(srv::MotSrv& service) override;
    // A disabled transport cannot start or wait for requests.
    int start() override;
    int wait() override;
    // There are no protocol resources to stop.
    void stop() override;
private:
    // Accept the same factory arguments without creating a native server.
    MotRpc(const std::string& host, int port);
    friend class MotionApp;
};
#endif

}  // namespace zmc

#endif
