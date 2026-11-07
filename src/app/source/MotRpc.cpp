#include "rpc/MotRpc.hpp"

#ifndef ZMC_ENABLE_RPC
#include "common/zmc/fbt_errorcode_def.h"

// GPT-6 (Codex), 2026-10-29. The disabled transport owns no RPC resources.
namespace zmc {
MotRpc::MotRpc(const std::string&, int) {}
int MotRpc::bind(srv::MotSrv&) { return ZMC_ERR_HOST_NOT_IMPLEMENTED; }
int MotRpc::start() { return ZMC_ERR_HOST_NOT_IMPLEMENTED; }
int MotRpc::wait() { return ZMC_ERR_HOST_NOT_IMPLEMENTED; }
void MotRpc::stop() {}
}  // namespace zmc
#endif
