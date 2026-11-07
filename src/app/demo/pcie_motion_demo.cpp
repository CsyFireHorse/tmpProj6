/*
 * Demo: EcatPcieServoMotion with open -> load_eni_json -> SDO watch -> config_servo_op_mode -> start -> cycle_step loop -> stop.
 * Usage: axis_ini <ini_path> [--init-opmode=N] [--board=N] [--channel=N]
 */
#include "EcatPcieServo.h"
#include "AxisIniLoader.h"
#include "common/ecat/ecat_pdo_key.h"
#include "common/get_file_path.hpp"
#include "FbWrapCmd.h"
#include "common/zmc/fbt_errorcode_def.h"
#include "common/zmc/zmc_predef.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace zmc;

static void sleep_ms(unsigned int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

// argv: [ini_path, optional --init-opmode=N, --board=N, --channel=N, --http|--cmd, --port=N]
int pcie_axis_ini_demo(int argc, char** argv) {
    if (argc < 1) {
        printf("usage: axis_ini <ini_path> [--init-opmode=N] [--board=N] [--channel=N] [--http|--cmd] [--port=N]\n");
        return -1;
    }
    const char* ini_path = argv[0];
    int init_opmode = 8;  // OP_MODE_CSP
    uint32_t board_index = 0;
    uint32_t channel = 0;
    bool use_http = true;

    int port = ZMC_MOTAPP_HTTP_DEFAULT_PORT;
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--init-opmode=", 14) == 0) {
            init_opmode = atoi(argv[i] + 14);
        } else if (strncmp(argv[i], "--board=", 8) == 0) {
            board_index = static_cast<uint32_t>(atoi(argv[i] + 8));
        } else if (strncmp(argv[i], "--channel=", 10) == 0) {
            channel = static_cast<uint32_t>(atoi(argv[i] + 10));
        } else if (strcmp(argv[i], "--cmd") == 0) {
            use_http = false;
        } else if (strcmp(argv[i], "--http") == 0) {
            use_http = true;
        } else if (strncmp(argv[i], "--port=", 7) == 0) {
            port = atoi(argv[i] + 7);
        }
    }

    if (use_http) {
        // Reject the disabled attachment before creating any hardware/runtime resources. GPT-6.
        std::fprintf(stderr, "PCIe HTTP attachment is disabled; use the singleton service entry.\n");
        return ZMC_ERR_HOST_NOT_IMPLEMENTED;
    }

    AxisCommonConfig cfg;
    int errCode = load_axis_ini_base(ini_path, &cfg);
    if (errCode) {
        printf("load_axis_ini_base %s failed %d\n", ini_path, errCode);
        return errCode;
    }
    const char* eni = cfg.eni_path ? cfg.eni_path : "";
    if (!eni[0]) {
        printf("eni_path not set in INI\n");
        return -1;
    }

    EcatPcieServoMotion motion(ini_path, "");
    int ret = motion.open(board_index, channel);
    if (ret) {
        printf("EcatPcieServo open failed %d\n", ret);
        return ret;
    }
    ret = motion.load_eni_json(eni);
    if (ret) {
        printf("load_eni_json failed %d\n", ret);
        return ret;
    }
    ret = motion.start();
    if (ret) {
        printf("start failed %d\n", ret);
        return ret;
    }
    if (!motion.request_state(8, 30)) {  // EcatStateO = 8
        printf("request_state Op failed\n");
        return -1;
    }
    ret = motion.config_servo_op_mode(static_cast<uint32_t>(init_opmode));
    if (ret) {
        printf("config_servo_op_mode failed %d\n", ret);
        return ret;
    }
    motion.add_opmode_watch();

    ret = motion.start_pi_sdo();
    if (ret) {
        printf("start_pi_sdo failed %d\n", ret);
        return ret;
    }
#if 0  // Disabled external queue attachment; retained legacy code. GPT-6.
    // Bind the HTTP core before starting its protocol thread.
    // Start HTTP server or command loop on AxisFbGraph in a separate thread.
    std::thread ui_thread([&motion, use_http, port]() {
        AxisFbGraph& fb = motion.fb_graph();
        if (use_http) {
            // Default port follows existing HTTP demo convention.
            zmc::runHttpServer(port);
        } else {
            runCmdLoop(fb.fbGrp);
        }
    });
    if (!use_http) ui_thread.detach();

#endif
    std::thread ui_thread([&motion]() { runCmdLoop(motion.fb_graph().fbGrp); });
    ui_thread.detach();

    printf("EcatPcieServoMotion running. Press Ctrl+C or close to exit.\n");
    uint64_t cycles = 0;
    // pi started and running. pi will run 1 extra cycle then exit if stop_pi
    while (motion.is_pi_start()) {
        ret = motion.cycle_step();
        // if (ret) {
        //     printf("cycle_step failed %d\n", ret);
        //     break;
        // }
    }

#if 0  // Disabled external lifecycle cleanup. GPT-6.
    if (use_http) {
        // Release the singleton only after HTTP workers stop using the runtime queues.
        srv::SrvCommandSingleton::instance().requestExit();
        ui_thread.join();
    }
#endif
    motion.stop();
    return 0;
}

// Run pcie_axis_ini_demo with given ini path and optional args (e.g. --http, --init-opmode=8).
int _pcie_axis_ini_demo_with_ini(const std::vector<std::string>& args, const std::string& iniPath) {
    std::vector<std::string> new_args;
    new_args.push_back(iniPath);
    for (const auto& s : args) new_args.push_back(s);
    std::vector<char*> ptrs;
    for (auto& s : new_args) ptrs.push_back(const_cast<char*>(s.c_str()));
    int n = static_cast<int>(ptrs.size());
    return pcie_axis_ini_demo(n, ptrs.empty() ? nullptr : ptrs.data());
}

int pcie_axis_ini_demo_fak(const std::vector<std::string>& args) {
    std::string eni_file = _get_file_path("mhelper/data/zmc_motapp_demo_fak.ini");
    return _pcie_axis_ini_demo_with_ini(args, eni_file);
}

int pcie_axis_ini_demo_fak2(const std::vector<std::string>& args) {
    std::string eni_file = _get_file_path("mhelper/data/zmc_motapp_demo_fak2.ini");
    return _pcie_axis_ini_demo_with_ini(args, eni_file);
}

int pcie_axis_ini_demo_fak3(const std::vector<std::string>& args) {
    std::string eni_file = _get_file_path("mhelper/data/zmc_motapp_demo_fak3.ini");
    return _pcie_axis_ini_demo_with_ini(args, eni_file);
}
int pcie_axis_ini_demo_fak3a(const std::vector<std::string>& args) {
    std::string eni_file = _get_file_path("mhelper/data/zmc_motapp_demo_fak3a.ini");
    return _pcie_axis_ini_demo_with_ini(args, eni_file);
}
