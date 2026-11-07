#ifndef ZMC_ECAT_MOTION_CLI_HPP_
#define ZMC_ECAT_MOTION_CLI_HPP_
#include <string>
#include <vector>
namespace zmc {
struct PdoPowerIniArgs;
// Parse existing CLI values without creating a motion runtime. Model signature: GPT-6.
int parse_ecat_motion_args(const std::vector<std::string>& args, PdoPowerIniArgs* parsed,
                           bool* eni_option_explicit = nullptr);
// Resolve the command-line INI compatibility path before arguments enter the service layer.
int prepare_ecat_motion_cli_args(const std::vector<std::string>& args, PdoPowerIniArgs* parsed,
                                std::vector<std::string>* prepared_args);
}  // namespace zmc
#endif
