#ifndef ZMC_MOT_RPC_QCMD_HPP_
#define ZMC_MOT_RPC_QCMD_HPP_

#include "MotRpcFile.hpp"

namespace zmc {

// Register motion, IO and command methods on the shared RPC server.
class MotRpcQcmd : public MotRpcFile {
protected:
    // Forward the server address and port to the base.
    MotRpcQcmd(const std::string& host, int port);

    // Register the existing command methods.
    void bindQcmd();
};

}  // namespace zmc

#endif
