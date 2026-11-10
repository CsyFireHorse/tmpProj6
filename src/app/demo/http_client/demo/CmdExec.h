#ifndef _ZMC_HTTP_CMD_EXEC_H_
#define _ZMC_HTTP_CMD_EXEC_H_

#include <string>
#include <vector>

#include "zmc_http_client.h"

namespace zmc {

int execCommand(ZML_HANDLE client, const std::string& cmd, const std::vector<std::string>& args);
int execLine(ZML_HANDLE client, const std::string& line);
void printHelp();

int runInteractive(ZML_HANDLE client);
int runScript(ZML_HANDLE client, const std::string& path);

}  // namespace zmc

#endif  // _ZMC_HTTP_CMD_EXEC_H_
