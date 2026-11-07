#include "CmdExec.h"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "zml_motion_api_ex.h"
#include "rpc/MotionRpcTypes.h"

/* AI modification signature: GPT-5 Codex */

namespace zmc {

namespace {

std::string trim(const std::string& s) {
    const std::size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return std::string();
    }
    const std::size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string& s) {
    std::vector<std::string> parts;
    std::string token;
    for (std::size_t i = 0; i <= s.size(); ++i) {
        if (i == s.size() || s[i] == ' ' || s[i] == '\t') {
            if (!token.empty()) {
                parts.push_back(token);
                token.clear();
            }
        } else {
            token.push_back(s[i]);
        }
    }
    return parts;
}

std::string lowerCopy(const std::string& s) {
    std::string out = s;
    for (std::size_t i = 0; i < out.size(); ++i) {
        out[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(out[i])));
    }
    return out;
}

void printAxisState(const RpcAxisState& st) {
    std::cout << static_cast<int>(st.axis_id) << ' ' << static_cast<int>(st.axis_state) << ' '
              << st.error_id << ' ' << st.act_pos << ' ' << st.set_pos << ' ' << st.tick
              << std::endl;
}

void printAxisState2(const RpcAxisState2& st) {
    std::cout << static_cast<int>(st.axis_id) << ' ' << static_cast<int>(st.axis_state) << ' '
              << st.error_id << ' ' << st.act_pos << ' ' << st.set_pos << ' ' << st.set_vel << ' '
              << st.set_acc << ' ' << st.set_position << ' ' << st.act_position << ' ' << st.tick
              << std::endl;
}

int fillMoveArgs(const std::vector<std::string>& args, int* axes, double* distances) {
    int len = 0;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (len >= ZMC_RPC_MAX_AXES) {
            std::cerr << "Too many position values (max " << ZMC_RPC_MAX_AXES << ")" << std::endl;
            return -2;
        }
        axes[len] = static_cast<int>(len);
        distances[len++] = std::atof(args[i].c_str());
    }
    if (len <= 0) {
        std::cerr << "At least one position value required" << std::endl;
        return -2;
    }
    return 0;
}

int fillSinglePos(const std::vector<std::string>& args, double& out) {
    if (args.size() != 1) {
        std::cerr << "Exactly one position value required" << std::endl;
        return -2;
    }
    out = std::atof(args[0].c_str());
    return 0;
}

int fillAxisIndexList(const std::vector<std::string>& args, int* axes, int& len) {
    len = 0;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (len >= ZMC_RPC_MAX_AXES) {
            std::cerr << "Too many axis values (max " << ZMC_RPC_MAX_AXES << ")" << std::endl;
            return -2;
        }
        axes[len++] = std::atoi(args[i].c_str());
    }
    if (len <= 0) {
        std::cerr << "At least one axis index required" << std::endl;
        return -2;
    }
    return 0;
}

}  // namespace

int execCommand(ZML_HANDLE client, const std::string& cmd, const std::vector<std::string>& args) {
    if (cmd == "help") {
        printHelp();
        return 0;
    }
    if (cmd == "state") {
        std::cout << zml_get_connection_state(client) << std::endl;
        return 0;
    }
    if (cmd == "quit" || cmd == "exit") {
        return 0;
    }

    if (cmd == "movesgabs") {
        if (args.size() != 2) {
            std::cerr << "Usage: movesgabs <axis> <pos>" << std::endl;
            return -2;
        }
        const int32_t ret =
            zml_ax_move_abs(client, std::atoi(args[0].c_str()), std::atof(args[1].c_str()));
        // std::cout << ret << std::endl;
        return static_cast<int>(ret);
    }

    if (cmd == "moveabs") {
        int axes[ZMC_RPC_MAX_AXES] = {};
        double distances[ZMC_RPC_MAX_AXES] = {};
        const int fillRet = fillMoveArgs(args, axes, distances);
        if (fillRet != 0) {
            return fillRet;
        }
        const int32_t ret = zml_moveabs(client, static_cast<int>(args.size()), axes, distances);
        // std::cout << ret << std::endl;
        return static_cast<int>(ret);
    }

    if (cmd == "poweron") {
        if (args.size() != 1) {
            std::cerr << "Usage: poweron <axis>" << std::endl;
            return -2;
        }
        const int32_t ret = zml_ax_power_on(client, std::atoi(args[0].c_str()));
        // std::cout << ret << std::endl;
        return static_cast<int>(ret);
    }

    if (cmd == "poweroff") {
        if (args.size() != 1) {
            std::cerr << "Usage: poweroff <axis>" << std::endl;
            return -2;
        }
        const int32_t ret = zml_ax_power_off(client, std::atoi(args[0].c_str()));
        // std::cout << ret << std::endl;
        return static_cast<int>(ret);
    }

    if (cmd == "getaxisstate") {
        if (args.size() != 1) {
            std::cerr << "Usage: getaxisstate <axis>" << std::endl;
            return -2;
        }
        const int32_t axis = static_cast<int32_t>(std::atoi(args[0].c_str()));
        RpcAxisState st{};
        const int32_t ret = zml_ax_get_state(client, axis, &st);
        // std::cout << ret << std::endl;
        if (ret == 0) {
            printAxisState(st);
        }
        return static_cast<int>(ret);
    }

    if (cmd == "getaxisstate2") {
        if (args.size() != 1) {
            std::cerr << "Usage: getaxisstate2 <axis>" << std::endl;
            return -2;
        }
        const int32_t axis = static_cast<int32_t>(std::atoi(args[0].c_str()));
        RpcAxisState2 st{};
        const int32_t ret = zml_ax_get_state_ex(client, axis, &st);
        // std::cout << ret << std::endl;
        if (ret == 0) {
            printAxisState2(st);
        }
        return static_cast<int>(ret);
    }

    if (cmd == "setspeed") {
        if (args.size() != 2) return -2;
        const int32_t ret =
            zml_ax_set_velocity(client, std::atoi(args[0].c_str()), std::atof(args[1].c_str()));
        // std::cout << ret << std::endl;
        return static_cast<int>(ret);
    }

    if (cmd == "movesg" || cmd == "movesgrel") {
        if (args.size() != 2) return -2;
        const int32_t ret =
            zml_ax_move_rel(client, std::atoi(args[0].c_str()), std::atof(args[1].c_str()));
        // std::cout << ret << std::endl;
        return static_cast<int>(ret);
    }

    if (cmd == "moveadd") {
        if (args.size() != 2) return -2;
        const int32_t ret =
            zml_ax_moveadd(client, std::atoi(args[0].c_str()), std::atof(args[1].c_str()));
        // std::cout << ret << std::endl;
        return static_cast<int>(ret);
    }

    if (cmd == "move" || cmd == "moverel") {
        int axes[ZMC_RPC_MAX_AXES] = {};
        double distances[ZMC_RPC_MAX_AXES] = {};
        const int fillRet = fillMoveArgs(args, axes, distances);
        if (fillRet != 0) return fillRet;
        const int32_t ret = zml_moverel(client, static_cast<int>(args.size()), axes, distances);
        // std::cout << ret << std::endl;
        return static_cast<int>(ret);
    }

    if (cmd == "waitaxisidle") {
        int32_t axis = 0;
        if (!args.empty()) axis = static_cast<int32_t>(std::atoi(args[0].c_str()));
        const uint32_t timeout =
            args.size() > 1 ? static_cast<uint32_t>(std::strtoul(args[1].c_str(), nullptr, 10))
                            : 30000U;
        const int32_t ret = zml_ax_wait_idle(client, axis, timeout);
        // std::cout << ret << std::endl;
        return static_cast<int>(ret);
    }

    if (cmd == "groupsetaxes") {
        int axes[ZMC_RPC_MAX_AXES] = {};
        int axisCount = 0;
        const int fillRet = fillAxisIndexList(args, axes, axisCount);
        if (fillRet != 0) return fillRet;
        const int32_t ret = zml_group_setaxes(client, axisCount, axes);
        // std::cout << ret << std::endl;
        return static_cast<int>(ret);
    }

    if (cmd == "groupenable") {
        int axes[ZMC_RPC_MAX_AXES] = {};
        int axisCount = 0;
        const int fillRet = fillAxisIndexList(args, axes, axisCount);
        if (fillRet != 0) return fillRet;
        const int32_t ret = zml_group_enable(client, axisCount, axes);
        // std::cout << ret << std::endl;
        return static_cast<int>(ret);
    }

    if (cmd == "groupdisable") {
        int axes[ZMC_RPC_MAX_AXES] = {};
        int axisCount = 0;
        const int fillRet = fillAxisIndexList(args, axes, axisCount);
        if (fillRet != 0) return fillRet;
        const int32_t ret = zml_group_disable(client, axisCount, axes);
        // std::cout << ret << std::endl;
        return static_cast<int>(ret);
    }

    if (cmd == "groupreset") {
        int axes[ZMC_RPC_MAX_AXES] = {};
        int axisCount = 0;
        const int fillRet = fillAxisIndexList(args, axes, axisCount);
        if (fillRet != 0) return fillRet;
        const int32_t ret = zml_group_reset(client, axisCount, axes);
        // std::cout << ret << std::endl;
        return static_cast<int>(ret);
    }

    if (cmd == "setflowconfig") {
        if (args.size() != 3) return -2;
        RpcFlowCtrlConfig cfg{};
        cfg.period = static_cast<uint32_t>(std::strtoul(args[0].c_str(), nullptr, 10));
        cfg.batch_size = static_cast<uint32_t>(std::strtoul(args[1].c_str(), nullptr, 10));
        cfg.verbose_flag = static_cast<uint8_t>(std::strtoul(args[2].c_str(), nullptr, 10));
        cfg.reserved[0] = 0;
        cfg.reserved[1] = 0;
        cfg.reserved[2] = 0;
        const int32_t ret = zml_set_flow_config(client, &cfg);
        // std::cout << ret << std::endl;
        return static_cast<int>(ret);
    }

    if (cmd == "getflowstatus") {
        if (!args.empty()) return -2;
        RpcFlowCtrlStatus st{};
        (void)zml_get_flow_status(client, &st);
        std::cout << st.period << ' ' << st.batch_size << ' '
                  << static_cast<unsigned int>(st.verbose_flag) << ' ' << st.latency << ' '
                  << st.axis_num << ' ' << st.cycle_time << std::endl;
        return 0;
    }

    std::cerr << "Unknown command: " << cmd << std::endl;
    return -1;
}

int execLine(ZML_HANDLE client, const std::string& line) {
    const std::string trimmed = trim(line);
    if (trimmed.empty() || trimmed[0] == '#') {
        return 0;
    }
    const std::vector<std::string> parts = split(trimmed);
    if (parts.empty()) {
        return 0;
    }
    const std::string cmd = lowerCopy(parts[0]);
    std::vector<std::string> args;
    if (parts.size() > 1) {
        args.assign(parts.begin() + 1, parts.end());
    }
    return execCommand(client, cmd, args);
}

void printHelp() {
    std::cout << "Commands:\n"
              << "  movesg <pos0>                (single-axis relative)\n"
              << "  movesgabs <pos0>             (single-axis absolute)\n"
              << "  moveadd <pos0>               (single-axis additive)\n"
              << "  move | moverel <pos0> [pos1 ...]       (multi-axis relative)\n"
              << "  moveabs <pos0> [pos1 ...]    (multi-axis absolute)\n"
              << "  poweron\n"
              << "  poweroff\n"
              << "  getaxisstate <axis>\n"
              << "  getaxisstate2 <axis>\n"
              << "  setspeed <speed>\n"
              << "  waitaxisidle [axis] [timeout_ms]\n"
              << "  groupsetaxes <axis0> [axis1 ...]\n"
              << "  groupenable <axis0> [axis1 ...]\n"
              << "  groupdisable <axis0> [axis1 ...]\n"
              << "  groupreset <axis0> [axis1 ...]\n"
              << "  setflowconfig <period> <batchSize> <verboseFlag>\n"
              << "  getflowstatus\n"
              << "  state\n"
              << "  help\n"
              << "  quit | exit\n";
}

int runInteractive(ZML_HANDLE client) {
    std::string line;
    while (true) {
        std::cout << "cmd> " << std::flush;
        if (!std::getline(std::cin, line)) {
            break;
        }
        const std::string trimmed = trim(line);
        if (trimmed == "quit" || trimmed == "exit") {
            break;
        }
        const int ret = execLine(client, line);
        if (ret != 0 && ret != -1 && ret != -2) {
            return ret;
        }
    }
    return 0;
}

int runScript(ZML_HANDLE client, const std::string& path) {
    std::ifstream in(path.c_str());
    if (!in) {
        std::cerr << "Cannot open script: " << path << std::endl;
        return -1;
    }
    std::string line;
    while (std::getline(in, line)) {
        const int ret = execLine(client, line);
        if (ret != 0) {
            return ret;
        }
    }
    return 0;
}

}  // namespace zmc
