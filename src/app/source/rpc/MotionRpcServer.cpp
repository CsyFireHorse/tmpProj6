#if 0  // Disabled legacy service bypass. Model signature: GPT-6.
#include "rpc/MotionRpcServer.h"
#include "MotionRpcServerInternal.h"
#include "srv/SrvCommandSingleton.hpp"
namespace zmc {
// Model signature: GPT-6. External entries only delegate protocol/runtime coordination.
int runRpcServer(const std::string& host, int port) {
    return motion_rpc_detail::runServer(host, port);
}
void stopRpcServer() { srv::SrvCommandSingleton::instance().requestExit(); }
}  // namespace zmc

#endif
