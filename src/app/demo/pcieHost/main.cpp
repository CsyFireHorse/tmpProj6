/*
 * Host-side PCIe demo: mc_pdo_thread + AxisFbBridge, usage aligned with ecatAxisIni_demo.
 *
 * Usage: zmc_pcie_host_demo <ini_path> [EcatMotion options] [--board=ID0] [--channel=N]
 * Presets: zmc_pcie_host_demo fak|axis_fak|fak2|fak3|sg|db [options]
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "EcatMotion.h"
#include "HttpScriptHelper.hpp"
#include "common/get_file_path.hpp"
#include "mc_core/mc_motion.h"
#include "mc_pdo_thread.h"
#include "mc_zecm.h"
#include "mhelper/zspdlog_init.h"
#include "pou/AxisIniLoader.h"

using namespace zmc;

namespace {

class PcieHostIniCtx : public EcatMotion {
   public:
    const PdoPowerIniArgs& parsed() const { return ini_args(); }
};

int load_startup_script_if_needed(const std::vector<std::string>& args, EcatMotion& ctx,
                                std::string* script_out) {
    std::string scriptPath;
    if (!parse_script_path_from_args(args, 1, scriptPath)) {
        return 0;
    }

    std::string scriptContent;
    if (!read_text_file_all(scriptPath, scriptContent)) {
        std::cerr << "Error: failed to open script file: " << scriptPath << std::endl;
        return -1;
    }
    if (scriptContent.empty()) {
        std::cerr << "Error: script file is empty: " << scriptPath << std::endl;
        return -1;
    }

    std::cout << "[startup-script] startup script: " << scriptPath << std::endl;
    ctx.set_startup_script(scriptContent);
    if (script_out) {
        *script_out = scriptContent;
    }
    return 0;
}

std::vector<std::string> strip_pcie_board_channel(const std::vector<std::string>& in,
                                                  std::string* board, uint32_t* channel) {
    std::vector<std::string> out;
    *board = "ID0";
    *channel = 0;
    for (size_t i = 0; i < in.size(); ++i) {
        const std::string& s = in[i];
        if (s.find("--board=") == 0) {
            *board = s.substr(8);
        } else if (s.find("--channel=") == 0) {
            *channel = static_cast<uint32_t>(std::atoi(s.substr(10).c_str()));
        } else {
            out.push_back(s);
        }
    }
    return out;
}

void wait_for_exit(PdoPowerRunMode run_mode, bool script_exit) {
    if (run_mode == MODE_CMD && script_exit) {
        return;
    }
    std::printf("Press e key to exit\n");
    while (std::getchar() != 'e') {
    }
}

void pcie_host_shutdown(void) {
    MC_motionStopSdo();
    MC_EcatHostClose();
    MC_motionShutdown();
}

}  // namespace

int pcieHostPowerIni_demo(const std::vector<std::string>& args) {
    zmc_logger_init();
    PcieHostIniCtx ctx;
    int ret;

    std::string board;
    uint32_t channel = 0;
    const std::vector<std::string> motion_args = strip_pcie_board_channel(args, &board, &channel);

    ret = ctx.parse_args(motion_args);
    if (ret != 0) {
        return ret < 0 ? -1 : 0;
    }
    std::string startup_script;
    ret = load_startup_script_if_needed(motion_args, ctx, &startup_script);
    if (ret != 0) return ret;

    AxisCommonConfig cfg;
    ret = load_axis_ini_base(ctx.parsed().iniName.c_str(), &cfg);
    if (ret != 0) {
        std::printf("load %s failed %d\n", ctx.parsed().iniName.c_str(), ret);
        return ret;
    }
    const char* eni = cfg.eni_path ? cfg.eni_path : "";
    if (!eni[0]) {
        std::printf("eni_path not set in INI\n");
        return -1;
    }

    const char* ini = ctx.parsed().iniName.c_str();
    ret = MC_motionInit(ini, ctx.parsed().axis_num);
    if (ret != 0) {
        std::fprintf(stderr, "MC_motionInit failed: %d\n", ret);
        return ret;
    }

    ret = MC_EcatHostOpen(board.c_str(), channel, eni);
    if (ret != 0) {
        std::fprintf(stderr, "MC_EcatHostOpen failed: %d\n", ret);
        MC_motionShutdown();
        return ret;
    }

    ret = MC_motionLoadEni(eni);
    if (ret != 0) {
        std::fprintf(stderr, "MC_motionLoadEni failed: %d\n", ret);
        pcie_host_shutdown();
        return ret;
    }

    const PdoPowerIniArgs& parsed = ctx.parsed();
    MC_motionPrintAddress();

    ret = MC_motionStartSdo();
    if (ret != 0) {
        std::fprintf(stderr, "MC_motionStartSdo failed: %d\n", ret);
        pcie_host_shutdown();
        return ret;
    }

    ret = MC_motionAddOpmodeWatch(parsed.init_opmode);
    if (ret != 0) {
        std::fprintf(stderr, "MC_motionAddOpmodeWatch failed: %d\n", ret);
        pcie_host_shutdown();
        return ret;
    }

    std::thread(mc_pdo_thread).detach();

    ret = MC_motionDispatchRunMode(static_cast<int>(ctx.run_mode()), ctx.port(), &startup_script,
                                   ctx.script_exit());
    if (ret != 0) {
        pcie_host_shutdown();
        return ret;
    }

    wait_for_exit(ctx.run_mode(), ctx.script_exit());
    pcie_host_shutdown();
    return 0;
}

int _pcieHostIni_demo_with_ini(const std::vector<std::string>& args, const std::string& iniPath) {
    std::vector<std::string> new_args = args;
    new_args.insert(new_args.begin(), iniPath);
    return pcieHostPowerIni_demo(new_args);
}

int pcieHostIni_demo_fak(const std::vector<std::string>& args) {
    auto eni_file = _get_file_path("mhelper/data/zmc_motapp_demo_fak.ini");
    return _pcieHostIni_demo_with_ini(args, eni_file);
}

int pcieHostIni_demo_fak2(const std::vector<std::string>& args) {
    auto eni_file = _get_file_path("mhelper/data/zmc_motapp_demo_fak2.ini");
    return _pcieHostIni_demo_with_ini(args, eni_file);
}

int pcieHostIni_demo_sg_axis(const std::vector<std::string>& args) {
    auto eni_file = _get_file_path("mhelper/data/zmc_motapp_demo_sg.ini");
    return _pcieHostIni_demo_with_ini(args, eni_file);
}

int pcieHostIni_demo_db_axis(const std::vector<std::string>& args) {
    auto eni_file = _get_file_path("mhelper/data/zmc_motapp_demo_db.ini");
    return _pcieHostIni_demo_with_ini(args, eni_file);
}

int pcieHostIni_demo_fak3(const std::vector<std::string>& args) {
    auto eni_file = _get_file_path("mhelper/data/zmc_motapp_demo_fak3.ini");
    return _pcieHostIni_demo_with_ini(args, eni_file);
}

namespace {

int dispatch_preset(const std::vector<std::string>& args,
                    int (*fn)(const std::vector<std::string>&)) {
    std::vector<std::string> rest(args.begin() + 1, args.end());
    return fn(rest);
}

}  // namespace

int main(int argc, char** argv) {
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    if (args.empty()) {
        return pcieHostIni_demo_fak(args);
    }

    const std::string& cmd = args[0];
    if (cmd == "fak" || cmd == "axis_fak") {
        return dispatch_preset(args, pcieHostIni_demo_fak);
    }
    if (cmd == "fak2" || cmd == "axis_fak2") {
        return dispatch_preset(args, pcieHostIni_demo_fak2);
    }
    if (cmd == "fak3" || cmd == "axis_fak3") {
        return dispatch_preset(args, pcieHostIni_demo_fak3);
    }
    if (cmd == "sg" || cmd == "axis_sg") {
        return dispatch_preset(args, pcieHostIni_demo_sg_axis);
    }
    if (cmd == "db" || cmd == "axis_db") {
        return dispatch_preset(args, pcieHostIni_demo_db_axis);
    }

    return pcieHostPowerIni_demo(args);
}
