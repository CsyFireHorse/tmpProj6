/*
 * zmc_motapp_rpc_client - fixed-format RPC CLI for motion commands.
 *
 * Usage:
 *   zmc_motapp_rpc_client -h <host> -p <port> call <method> [args...]
 *   zmc_motapp_rpc_client -h <host> -p <port> repl
 *   zmc_motapp_rpc_client -h <host> -p <port> run <script.txt>
 *
 * Examples (omit -p to use default port):
 *   zmc_motapp_rpc_client -h 127.0.0.1 call moveabs 100.0 200.0 300.0
 *   zmc_motapp_rpc_client -h 127.0.0.1 call poweron
 *   zmc_motapp_rpc_client -h 127.0.0.1 call getaxisstate 0
 */
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "CmdExec.h"
#include "zml_motion_api_ex.h"
#include "common/zmc/zmc_predef.h"


namespace {

enum class RunMode { Call, Shell, Script, Unknown };

void printUsage(const char* exe) {
    std::cerr << "Usage:\n"
              << "  " << exe << " -h <host> -p <port> call <method> [args...]\n"
              << "  " << exe << " -h <host> -p <port> repl\n"
              << "  " << exe << " -h <host> -p <port> run <script.txt>\n"
              << "  default port: " ZMC_MOTAPP_HTTP_DEFAULT_PORT_STR "\n"
              << "  method: movesgabs | moveabs | poweron | poweroff\n"
              << "          getaxisstate <axis> | getaxisstate2 <axis>\n";
}

int connect(const std::string& host, int port, ZML_HANDLE* client) {
    const std::string device = host + ":" + std::to_string(port);
    if (zml_open(client, device.c_str()) != 0) {
        std::cerr << "zml_open failed" << std::endl;
        return -1;
    }
    if (zml_wait_until_connection_ready(*client, 200) != 0) {
        std::cerr << "connect " << host << ":" << port
                  << " failed, state=" << zml_get_connection_state(*client) << std::endl;
        zml_close(client);
        *client = nullptr;
        return -1;
    }
    // std::cout << host << ":" << port << " connected, state=" <<
    // zml_get_connection_state(*client)
    //           << std::endl;
    return 0;
}

std::string joinCallArgs(int argc, char** argv, int startIdx) {
    std::string line;
    for (int i = startIdx; i < argc; ++i) {
        if (i > startIdx) {
            line.push_back(' ');
        }
        line += argv[i];
    }
    return line;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string host = "127.0.0.1";
    int port = ZMC_MOTAPP_HTTP_DEFAULT_PORT;
    RunMode mode = RunMode::Unknown;
    int modeIdx = -1;
    int callMethodIdx = -1;
    int scriptPathIdx = -1;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            host = argv[++i];
        } else if (std::strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = std::atoi(argv[++i]);
        } else if (std::strcmp(argv[i], "call") == 0) {
            mode = RunMode::Call;
            modeIdx = i;
            callMethodIdx = i + 1;
        } else if (std::strcmp(argv[i], "repl") == 0) {
            mode = RunMode::Shell;
            modeIdx = i;
        } else if (std::strcmp(argv[i], "run") == 0) {
            mode = RunMode::Script;
            modeIdx = i;
            scriptPathIdx = i + 1;
        }
    }

    if (mode == RunMode::Unknown || modeIdx < 0) {
        printUsage(argv[0]);
        return 1;
    }

    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port: " << port << std::endl;
        return 1;
    }

    if (mode == RunMode::Call && (callMethodIdx < 0 || callMethodIdx >= argc)) {
        std::cerr << "call requires <method> [args...]" << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    if (mode == RunMode::Script && (scriptPathIdx < 0 || scriptPathIdx >= argc)) {
        std::cerr << "run requires <script.txt>" << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    ZML_HANDLE client = nullptr;
    const int connRet = connect(host, port, &client);
    if (connRet != 0) {
        zml_close(&client);
        return 1;
    }

    int exitCode = 0;
    if (mode == RunMode::Call) {
        const std::string line = joinCallArgs(argc, argv, callMethodIdx);
        exitCode = zmc::execLine(client, line);
    } else if (mode == RunMode::Shell) {
        exitCode = zmc::runInteractive(client);
    } else {
        exitCode = zmc::runScript(client, argv[scriptPathIdx]);
    }

    zml_close(&client);
    return exitCode;
}
