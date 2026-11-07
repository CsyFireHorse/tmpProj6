#ifndef _ZMC_HTTP_HELPER_HPP_
#define _ZMC_HTTP_HELPER_HPP_
#include "HttpFileService.hpp"
#include <string>
#include <vector>
namespace zmc {
// Parse script and port arguments for existing callers.
bool parse_script_path_from_argv(int argc, char** argv, std::string& scriptPath);
bool parse_script_path_from_args(const std::vector<std::string>& args, size_t beginIndex,
                                 std::string& scriptPath);
bool read_text_file_all(const std::string& path, std::string& out);
// Parse a TCP port string; accept only plain integers in [1, 65535].
bool parse_port_value(const std::string& value, int* port);
// HTTP motion API routes. The motion loop consumes commands and produces status;
// HTTP workers call the service, which owns command synchronization.
// CmdQueueType serializes producer pushes before the SPSC queue.
class MotHttpQcmd : public MotHttpFile {
protected:
    int bindQcmd(srv::MotSrv& service);
    // Register asynchronous alias-based IO command routes.
    void bindIoRoutes();
};
}  // namespace zmc
#endif
