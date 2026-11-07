#ifndef ZMC_MOT_RPC_FILE_HPP_
#define ZMC_MOT_RPC_FILE_HPP_

#include "MotRpcBase.hpp"

namespace zmc {

// Register file methods on the shared RPC server.
class MotRpcFile : public MotRpcBase {
protected:
    // Forward the server address and port to the base.
    MotRpcFile(const std::string& host, int port);

    // Register the existing file upload and download methods.
    void bindFiles();
};

}  // namespace zmc

#endif
