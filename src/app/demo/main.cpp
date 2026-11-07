/*
 * @Author: ChenSy
 * @LastEditors: ChenSy
 * @Date: 2024-03-21 17:57:29
 * @LastEditTime: 2024-03-21 17:57:30
 * @Description: Modify here please
 */
#include <cstring>
#include <sstream>

#include "common/get_file_path.hpp"
#include "common/misc/CommandParser.h"

using namespace zmc;

int ecat_mgr_demo(const char* fname);
int ecat_mgr_demo2(const char* fname);
int ecat_mgr_demo_nullptr() { return ecat_mgr_demo(nullptr); }
int ecat_mgr_demo2_nullptr() { return ecat_mgr_demo2(nullptr); }

int ecatServo_demo(const char* eni);
int ecatServo_demo_nullptr() { return ecatServo_demo(nullptr); }
int ecatServoMgr_demo(const char* eni);
int ecatServoMgr_demo_void() { return ecatServoMgr_demo(nullptr); }

int pdoMarquee_demo(const char* fname);
int pdoEnable_demo(const char* fname);
int pdoPower_demo(const std::vector<std::string>& args);
int pdoPower_demo_fak(const std::vector<std::string>& args);
int pdoPower_demo_fak2(const std::vector<std::string>& args);
int pdoPower_demo_fak3(const std::vector<std::string>& args);
int pdoPowerIni_demo(const std::vector<std::string>& args);
int pdoIni_demo_fak(const std::vector<std::string>& args);
int pdoIni_demo_fak2(const std::vector<std::string>& args);
int pdoIni_demo_fak3(const std::vector<std::string>& args);
int pdoIni_demo_db_axis(const std::vector<std::string>& args);
int pdoIni_demo_sg_axis(const std::vector<std::string>& args);
int pdoIni_demo_trd_axis(const std::vector<std::string>& args);

int runCmdLoop_demo(int argc, char** argv);
int runHttpServer_demo(int argc, char** argv);
int runRpcServer_demo(int argc, char** argv);
int runHttpClient_demo(const std::vector<std::string>& args);
int runScript_demo(int argc, char** argv);
int pci_main(int argc, char** argv) { return 0; }
int pcie_axis_ini_demo(int argc, char** argv);
int pcie_axis_ini_demo_fak(const std::vector<std::string>& args);
int pcie_axis_ini_demo_fak2(const std::vector<std::string>& args);
int pcie_axis_ini_demo_fak3(const std::vector<std::string>& args);
int pcie_axis_ini_demo_fak3a(const std::vector<std::string>& args);

static std::string help_str = "";

int printHelp() {
    printf("%s\n", help_str.c_str());
    return 0;
}

static const char* EXE_NAME = "zmc_motapp_demo";

int printVersion() {
    printf(
        "%s: plcfb demo tool\n"
        "version 1.0.0\n"
        "create by csy on 2024/04/07,"
        "update on 2024/04/07\n",
        EXE_NAME);
    return 0;
}

// Helper function to convert argc/argv to string
static std::string argvToString(int argc, char** argv) {
    std::ostringstream oss;
    for (int i = 0; i < argc; ++i) {
        if (i > 0) oss << " ";
        oss << argv[i];
    }
    return oss.str();
}

static int _main(int argc, char** argv) {
    static CommandParser parser;

    parser.bind("--help", []() { parser.showHelp(); });
    parser.bind("-h", []() { parser.showHelp(); });
    parser.bind("--version", printVersion);
    parser.bind("-V", printVersion);
    parser.bind("ecat", [](const std::vector<std::string>& args) {
        if (args.empty()) {
            return ecat_mgr_demo_nullptr();
        } else {
            return ecat_mgr_demo(args[0].c_str());
        }
    });
    parser.bind("ecat2", [](const std::vector<std::string>& args) {
        if (args.empty()) {
            return ecat_mgr_demo2_nullptr();
        } else {
            return ecat_mgr_demo2(args[0].c_str());
        }
    });
    parser.bind("servo", [](const std::vector<std::string>& args) {
        if (args.empty()) {
            return ecatServo_demo_nullptr();
        } else {
            return ecatServo_demo(args[0].c_str());
        }
    });
    parser.bind("servoMgr", [](const std::vector<std::string>& args) {
        if (args.empty()) {
            return ecatServoMgr_demo_void();
        } else {
            return ecatServoMgr_demo(args[0].c_str());
        }
    });
    parser.bind("scan", ecatServo_demo);
    parser.bind("marquee", pdoMarquee_demo);
    parser.bind("enable", pdoEnable_demo);
    parser.bind("cmd", runCmdLoop_demo);
    parser.bind("http", runHttpServer_demo);
    parser.bind("rpc", runRpcServer_demo, "RPC server: rpc [-h host] [-p port] [--ini ...]");
    parser.bind("http_client", runHttpClient_demo,
                "HTTP client demo: http_client [host] [port] [path] [method] [body]");
    parser.bind("script", runScript_demo);
    parser.bind("power", pdoPower_demo);
    parser.bind("power_move", pdoPower_demo);
    parser.bind("power_fak", pdoPower_demo_fak);
    parser.bind("power_fak2", pdoPower_demo_fak2);
    parser.bind("power_fak3", pdoPower_demo_fak3);
    parser.bind("axis_ini", pdoPowerIni_demo);
    parser.bind("axis_fak", pdoIni_demo_fak);
    parser.bind("axis_fak2", pdoIni_demo_fak2);
    parser.bind("axis_fak3", pdoIni_demo_fak3);
    parser.bind("axis_sg", pdoIni_demo_sg_axis);
    parser.bind("axis_db", pdoIni_demo_db_axis);
    parser.bind("axis_trd", pdoIni_demo_trd_axis);
    parser.bind("pci", pci_main);
    parser.bind("pcie_ini", pcie_axis_ini_demo,
                "PCIe EcatPcieServoMotion: pcie_ini <ini_path> [--init-opmode=N] [--board=N] "
                "[--channel=N]");
    parser.bind("pcie_fak", pcie_axis_ini_demo_fak);
    parser.bind("pcie_fak2", pcie_axis_ini_demo_fak2);
    parser.bind("pcie_fak3", pcie_axis_ini_demo_fak3);
    parser.bind("pcie_fak3a", pcie_axis_ini_demo_fak3a);

    // parser.bind("pcie_ini", [](const std::vector<std::string>& args) {
    //     std::vector<char*> ptrs;
    //     for (const auto& s : args) ptrs.push_back(const_cast<char*>(s.c_str()));
    //     int n = static_cast<int>(ptrs.size());
    //     char** argv = ptrs.empty() ? nullptr : ptrs.data();
    //     int ret = pcie_axis_ini_demo(n, argv);
    //     return ret == 0 ? CommandParser::SUCCESS : CommandParser::EXECUTION_ERROR;
    // }, "PCIe EcatPcieServoMotion: pcie_ini <ini_path> [--init-opmode=N] [--board=N]
    // [--channel=N]");
    CommandParser::ErrorCode result = parser.parseAndExecute(argc - 1, argv + 1);

    if (result == CommandParser::COMMAND_NOT_FOUND || result == CommandParser::EMPTY_COMMAND) {
        parser.showHelp();
        return result;
    }
    return result;
}

int main(int argc, char** argv) {
    if (argc == 1) {
#if _DEBUG
        // char* argv2[] = {"zmc_motapp_demo", "cmd", "--base.axis_num", "2"};
        char* argv2[] = {"zmc_motapp_demo",
                         "axis_ini",
                         "--ini",
                         new_get_file_path_pchar("mhelper/data/zmc_motapp_demo_fak2.ini"),
                         "--init-opmode=8",
                         "--axis-num=4",
                         "--bind-begin=1"};
        int argc2 = LEN(argv2);
        return _main(argc2, argv2);
#endif
    }
    return _main(argc, argv);
}
