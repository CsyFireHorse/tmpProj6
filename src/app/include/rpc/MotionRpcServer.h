#if 0  // Disabled legacy service bypass. Model signature: GPT-6.
#ifndef _ZMC_MOTION_RPC_SERVER_H_
#define _ZMC_MOTION_RPC_SERVER_H_
#include <string>
namespace zmc {
// Model signature: GPT-6. Runtime owners initialize srv before accepting RPC requests.
int runRpcServer(const std::string& host, int port);
// Request runtime exit; the owning RPC runner joins protocol workers.
void stopRpcServer();
}  // namespace zmc
#endif

#endif
