#if 0  // Disabled legacy service bypass. Model signature: GPT-6.
#include "MotionRunApi.h"
#include "EcatHttpServerWrap.hpp"

namespace zmc {
namespace {
// Model signature: GPT-6. Coordinate HTTP resources without native command ownership.
template <class Wrap>
int runServer(int port) {
    Wrap server;
    server.port = port;
    const int ret = server.bind();
    if (ret != 0 || server.probePort() != 0) {
        server.service().requestExit();
        return -1;
    }
    const int run_ret = server.run();
    server.service().requestExit();
    return run_ret != 0 ? -1 : 0;
}
}  // namespace
int runHttpServer(int port) { return runServer<ServerWrap>(port); }
int runEcatHttpServer(int port) { return runServer<EcatHttpServerWrap>(port); }
}  // namespace zmc

#endif
