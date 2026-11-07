#if 0  // Disabled legacy service bypass. Model signature: GPT-6.
/*
 * RPC server demo: motion loop + rpclib server (moveabs / mmoveabs).
 */
#include <iostream>

#ifdef ZMC_ENABLE_RPC

#include <cstring>
#include <string>
#include <thread>
#include <vector>

#include "AxisFbGraph.hpp"
#include "pou/axPouTmplRunner.hpp"
#include "rpc/MotionRpcServer.h"
#include "common/zmc/zmc_predef.h"


namespace {

bool parseRpcEndpoint(int argc, char** argv, std::string& host, int& port) {
    host = "0.0.0.0";
    port = ZMC_MOTAPP_HTTP_DEFAULT_PORT;
    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            host = argv[i + 1];
            ++i;
        } else if (std::strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = std::atoi(argv[i + 1]);
            ++i;
        }
    }
    return port > 0 && port <= 65535;
}

// Strip RPC listen flags before make_pou_config (its -h means --help).
int filterPouArgv(int argc, char** argv, std::vector<std::string>& storage,
                  std::vector<char*>& out) {
    storage.clear();
    out.clear();
    storage.push_back("zmc_motapp_rpc");
    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            ++i;
            continue;
        }
        if (std::strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            ++i;
            continue;
        }
        storage.push_back(argv[i]);
    }
    out.reserve(storage.size());
    for (std::size_t i = 0; i < storage.size(); ++i) {
        out.push_back(const_cast<char*>(storage[i].c_str()));
    }
    return static_cast<int>(out.size());
}

}  // namespace

int runRpcServer_demo(int argc, char** argv) {
    using namespace zmc;

    std::string host;
    int port = 0;
    if (!parseRpcEndpoint(argc, argv, host, port)) {
        std::cerr << "Invalid RPC port" << std::endl;
        return -1;
    }

    std::vector<std::string> pouArgStorage;
    std::vector<char*> pouArgv;
    const int pouArgc = filterPouArgv(argc, argv, pouArgStorage, pouArgv);

    auto* pRunner = make_pou_config<AxisFbGraphSimu>(pouArgc, pouArgv.data(), "");
    if (!pRunner) {
        std::cerr << "Failed to initialize AxisFbGraphSimu" << std::endl;
        return -1;
    }

    auto& axfbGrp = *pRunner;
    std::thread rpcThread([&]() { runRpcServer(host, port); });

    std::cout << "RPC server listening on " << host << ":" << port << std::endl;
    std::cout << "Starting motion control loop..." << std::endl;
    axfbGrp.run();

    stopRpcServer();
    if (rpcThread.joinable()) {
        rpcThread.join();
    }
    delete pRunner;
    return 0;
}

#else

int runRpcServer_demo(int /*argc*/, char** /*argv*/) {
    std::cerr << "RPC support not built (enable with -DENABLE_RPC=ON)" << std::endl;
    return -1;
}

#endif
#else
int runRpcServer_demo(int /*argc*/, char** /*argv*/) { return -1; }

#endif
