#include "AxisFbGraph.hpp"
#include "qcmd/MotionCmdInterface.hpp"
#include "pou/axPouTmplRunner.hpp"

#include <thread>
#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <atomic>
#include <cctype>

using namespace zmc;

namespace {

struct ScriptModeOptions {
    std::string script_path;
    int exit_delay_ms = 1000;
};

std::string trim_copy(const std::string& s) {
    size_t begin = 0;
    while (begin < s.size() && std::isspace(static_cast<unsigned char>(s[begin]))) {
        ++begin;
    }
    size_t end = s.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }
    return s.substr(begin, end - begin);
}

bool parse_nonneg_int(const std::string& s, int& out) {
    if (s.empty()) {
        return false;
    }
    try {
        size_t pos = 0;
        int value = std::stoi(s, &pos);
        if (pos != s.size() || value < 0) {
            return false;
        }
        out = value;
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_script_mode_options(int argc, char** argv, ScriptModeOptions& opt) {
    for (int i = 0; i < argc; ++i) {
        const std::string arg = argv[i] ? argv[i] : "";
        if (arg.rfind("--script=", 0) == 0) {
            opt.script_path = arg.substr(std::string("--script=").size());
            continue;
        }
        if (arg == "--script" && i + 1 < argc) {
            opt.script_path = argv[++i] ? argv[i] : "";
            continue;
        }
        if (arg.rfind("--script_exit_delay_ms=", 0) == 0) {
            int parsed = 0;
            if (parse_nonneg_int(arg.substr(std::string("--script_exit_delay_ms=").size()), parsed)) {
                opt.exit_delay_ms = parsed;
            } else {
                printf("Warning: invalid --script_exit_delay_ms value '%s', using default %d\n",
                       arg.c_str(), opt.exit_delay_ms);
            }
            continue;
        }
        if (arg == "--script_exit_delay_ms" && i + 1 < argc) {
            int parsed = 0;
            const std::string val = argv[++i] ? argv[i] : "";
            if (parse_nonneg_int(val, parsed)) {
                opt.exit_delay_ms = parsed;
            } else {
                printf("Warning: invalid --script_exit_delay_ms value '%s', using default %d\n",
                       val.c_str(), opt.exit_delay_ms);
            }
            continue;
        }
    }
    return !opt.script_path.empty();
}

int execute_script_file(MotionCmdInterface& cmdIf, const std::string& script_path) {
    std::ifstream ifs(script_path);
    if (!ifs.is_open()) {
        std::cerr << "Error: failed to open script file: " << script_path << std::endl;
        return -1;
    }

    std::string raw_line;
    int line_no = 0;
    int cmd_count = 0;
    while (std::getline(ifs, raw_line)) {
        ++line_no;
        std::string line = trim_copy(raw_line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::cout << "[script] line " << line_no << ": " << line << std::endl;
        int ret = cmdIf.executeCommand(line.c_str());
        if (ret != 0) {
            std::cerr << "[script] failed at line " << line_no
                      << ", ret=" << ret
                      << ", cmd='" << line << "'" << std::endl;
            return ret;
        }
        cmdIf.autoFlush();
        ++cmd_count;
    }

    std::cout << "[script] done, executed " << cmd_count << " command(s)." << std::endl;
    return 0;
}

} // namespace

int runScript_demo(int argc, char** argv) {
    ScriptModeOptions opt;
    if (!parse_script_mode_options(argc, argv, opt)) {
        std::cerr << "Error: missing --script <path>" << std::endl;
        std::cerr << "Usage: script --script <file> [--script_exit_delay_ms <ms>]" << std::endl;
        return -1;
    }

    auto pRunner = make_pou_config<AxisFbGraphSimu>(argc, argv, "");
    if (!pRunner) {
        std::cerr << "Failed to initialize AxisFbGraphSimu" << std::endl;
        return -1;
    }

    auto& axfbGrp = *pRunner;
    // NOTE: cmdIf is used from scriptThread and the motion loop thread without locking.
    // Prefer HTTP/scriptQueue path (httpServer_demo + ServerWrap) for thread-safe command ingress.
    MotionCmdInterface cmdIf(axfbGrp.fbGrp.getCmdInterface());
    std::atomic<int> script_ret{0};

    std::thread scriptThread([&]() {
        int ret = execute_script_file(cmdIf, opt.script_path);
        script_ret.store(ret);
        if (opt.exit_delay_ms > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(opt.exit_delay_ms));
        }
        cmdIf.sendExitFlag();
    });

    std::cout << "Starting motion control loop for script mode..." << std::endl;
    axfbGrp.run();

    cmdIf.sendExitFlag();
    if (scriptThread.joinable()) {
        scriptThread.join();
    }

    return script_ret.load();
}
