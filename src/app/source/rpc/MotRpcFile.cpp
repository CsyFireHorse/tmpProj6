#include "rpc/MotRpcFile.hpp"
#include "srv/MotSrv.hpp"
#include "rpc/server.h"
#include <tuple>
#include <vector>

namespace zmc {
// File calls use the shared server and service. Model signature: GPT-6.
MotRpcFile::MotRpcFile(const std::string& host, int port) : MotRpcBase(host, port) {}

void MotRpcFile::bindFiles() {
    // The established wire session slot is retained, but file operations ignore it.
    // GPT-6 Codex, 2026-10-23. Downloads create or replace through the file service.
    server_->bind("file_download", [this](RpcSession, const std::string& name,
                                          const std::vector<char>& data) {
        return static_cast<int32_t>(service_->writeFile(name, data));
    });
    server_->bind("file_upload", [this](RpcSession, const std::string& name) {
        std::vector<char> data;
        const int ret = service_->readFile(name, data, 16U * 1024U * 1024U);
        return std::make_tuple(static_cast<int32_t>(ret), data);
    });
}
}  // namespace zmc
