#include <string>
#include <vector>

#include "MotionApp.hpp"
#include "common/get_file_path.hpp"
#include "mhelper/zspdlog_init.h"

using namespace zmc;

int pdoPowerIni_demo(const std::vector<std::string>& args) {
    zmc_logger_init();
    // Service startup owns optional file preparation and runtime cleanup.
    return runMotionApp(args);
}

int _pdoIni_demo_with_ini(const std::vector<std::string>& args, const std::string iniPath) {
    std::vector<std::string> new_args;
    new_args.push_back("--ini");
    new_args.push_back(iniPath);
    new_args.insert(new_args.end(), args.begin(), args.end());
    return pdoPowerIni_demo(new_args);
}

int pdoIni_demo_fak(const std::vector<std::string>& args) {
    auto eni_file = _get_file_path("mhelper/data/zmc_motapp_demo_fak.ini");
    return _pdoIni_demo_with_ini(args, eni_file);
}

int pdoIni_demo_fak2(const std::vector<std::string>& args) {
    auto eni_file = _get_file_path("mhelper/data/zmc_motapp_demo_fak2.ini");
    return _pdoIni_demo_with_ini(args, eni_file);
}

int pdoIni_demo_sg_axis(const std::vector<std::string>& args) {
    auto eni_file = _get_file_path("mhelper/data/zmc_motapp_demo_sg.ini");
    return _pdoIni_demo_with_ini(args, eni_file);
}

int pdoIni_demo_db_axis(const std::vector<std::string>& args) {
    auto eni_file = _get_file_path("mhelper/data/zmc_motapp_demo_db.ini");
    return _pdoIni_demo_with_ini(args, eni_file);
}

int pdoIni_demo_fak3(const std::vector<std::string>& args) {
    auto eni_file = _get_file_path("mhelper/data/zmc_motapp_demo_fak3.ini");
    return _pdoIni_demo_with_ini(args, eni_file);
}

int pdoIni_demo_trd_axis(const std::vector<std::string>& args) {
    auto eni_file = _get_file_path("mhelper/data/zmc_motapp_demo_trd.ini");
    return _pdoIni_demo_with_ini(args, eni_file);
}
