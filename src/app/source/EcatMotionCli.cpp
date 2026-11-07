#include "EcatMotionCli.hpp"
#include "EcatMotion.h"
#include "http/HttpScriptHelper.hpp"
#include "common/zmc/zmc_predef.h"
#include <cstdio>
#include "common/ecat/ecat_print.h"
#include "pou/AxisIniLoader.h"
// Model signature: GPT-6. The established CLI grammar is shared without native resource creation.
namespace zmc {
static void print_usage() {
    printf("Usage: <exe> [options]\n");
    printf("Options:\n");
    printf("  --ini <path>         Axis INI; reads eni_path if --eni is absent\n");
    printf("  --eni <path>         ENI path; overrides the INI compatibility fallback\n");
    printf("  --netname <name>     EtherCAT net name (optional, default %s)\n",
           ZMC_DEFAULT_NET_NAME);
    printf("  --http               Run in HTTP server mode (default)\n");
    printf("  --rpc                Run in RPC server mode\n");
    printf("  --cmd                Run in command line mode\n");
    printf("  --axis-num=N         Set logical axis count (1..%u)\n",
           static_cast<unsigned int>(MC_MOTION_MAX_AXES));
    printf("  --init-opmode=N      Configure CiA402 operation mode for all servos\n");
    printf("  --port=N             Set HTTP server port (default %s)\n",
           ZMC_MOTAPP_HTTP_DEFAULT_PORT_STR);
    printf("  --script <file>      Queue startup script for supported run modes\n");
    printf("  --script_exit        Exit CMD mode after startup script finishes\n");
    printf("  -h, --help           Show this help message and exit\n");
}

int parse_ecat_motion_args(const std::vector<std::string>& args, PdoPowerIniArgs* parsed,
                           bool* eni_option_explicit) {
    if (!parsed) {
        return -1;
    }
    if (eni_option_explicit) *eni_option_explicit = false;

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--cmd") {
            parsed->runMode = MODE_CMD;
        } else if (args[i] == "--http") {
            parsed->runMode = MODE_HTTP;
        } else if (args[i] == "--rpc") {
            parsed->runMode = MODE_RPC;
        } else if (args[i] == "--script_exit") {
            parsed->scriptExit = true;
        } else if (args[i].find("--ini=") == 0) {
            parsed->iniName = args[i].substr(6);
            if (parsed->iniName.empty()) {
                printf("Error: --ini requires a non-empty path\n");
                return -1;
            }
        } else if (args[i] == "--ini") {
            if (i + 1 >= args.size() || args[i + 1].empty()) {
                printf("Error: --ini requires a path argument\n");
                return -1;
            }
            parsed->iniName = args[++i];
        } else if (args[i].find("--eni=") == 0) {
            if (eni_option_explicit) *eni_option_explicit = true;
            parsed->eniName = args[i].substr(6);
            if (parsed->eniName.empty()) {
                printf("Error: --eni requires a non-empty path\n");
                return -1;
            }
        } else if (args[i] == "--eni") {
            if (eni_option_explicit) *eni_option_explicit = true;
            if (i + 1 >= args.size() || args[i + 1].empty()) {
                printf("Error: --eni requires a path argument\n");
                return -1;
            }
            parsed->eniName = args[++i];
        } else if (args[i].find("--netname=") == 0) {
            parsed->netName = args[i].substr(10);
            if (parsed->netName.empty()) {
                printf("Error: --netname requires a non-empty name\n");
                return -1;
            }
        } else if (args[i] == "--netname") {
            if (i + 1 >= args.size() || args[i + 1].empty()) {
                printf("Error: --netname requires a name argument\n");
                return -1;
            }
            parsed->netName = args[++i];
        } else if (args[i].find("--port=") == 0) {
            const std::string value_str = args[i].substr(7);
            if (!parse_port_value(value_str, &parsed->port)) {
                printf("Error: invalid --port value: %s\n", value_str.c_str());
                return -1;
            }
        } else if (args[i].find("--axis-num=") == 0) {
            std::string value_str = args[i].substr(11);
            parsed->axis_num = static_cast<unsigned int>(std::stoi(value_str));
            if (parsed->axis_num > MC_MOTION_MAX_AXES) {
                _ERR_("axis_num > %u, set to %u", static_cast<unsigned int>(MC_MOTION_MAX_AXES),
                      static_cast<unsigned int>(MC_MOTION_MAX_AXES));
                parsed->axis_num = MC_MOTION_MAX_AXES;
            }
        } else if (args[i].find("--init-opmode=") == 0) {
            std::string value_str = args[i].substr(14);
            parsed->init_opmode = std::stoi(value_str);
            _INF_("Init op mode set to: %d", parsed->init_opmode);
        } else if (args[i] == "-h" || args[i] == "--help") {
            print_usage();
            return 1;
        } else if (args[i] == "--script" || args[i].find("--script=") == 0) {
            // Handled later in prepare_config via parse_script_path_from_args.
            if (args[i] == "--script") {
                if (i + 1 >= args.size()) {
                    printf("Error: --script requires a file argument\n");
                    return -1;
                }
                ++i;
            }
        } else if (!args[i].empty() && args[i][0] != '-') {
            // Positional INI path (CLI compatibility).
            if (!parsed->iniName.empty()) {
                printf("Error: only one INI path is allowed\n");
                return -1;
            }
            parsed->iniName = args[i];
        } else {
            printf("Error: unknown argument: %s\n", args[i].c_str());
            print_usage();
            return -1;
        }
    }
    return 0;
}

int prepare_ecat_motion_cli_args(const std::vector<std::string>& args, PdoPowerIniArgs* parsed,
                                std::vector<std::string>* prepared_args) {
    if (!parsed || !prepared_args) {
        return -1;
    }
    *parsed = PdoPowerIniArgs();
    bool eni_option_explicit = false;
    const int ret = parse_ecat_motion_args(args, parsed, &eni_option_explicit);
    if (ret != 0) {
        return ret;
    }
    *prepared_args = args;
    if (eni_option_explicit || parsed->iniName.empty()) {
        return 0;
    }

    AxisCommonConfig config;
    axisComcfg_default_init(&config);
    const int ini_ret = load_axis_ini_base(parsed->iniName.c_str(), &config);
    if (ini_ret != 0) {
        return ini_ret;
    }
    if (!config.eni_path || config.eni_path[0] == '\0' ||
        !(config.config_mask & AX_COM_MASK_ENI_PATH)) {
        return 0;
    }

    // Preserve the one-level command-line fallback by making ENI explicit before service startup.
    parsed->eniName = config.eni_path;
    prepared_args->push_back("--eni");
    prepared_args->push_back(parsed->eniName);
    return 0;
}

}  // namespace zmc
