#include "srv/SrvFileService.hpp"
#include "SrvState.hpp"
#include <limits>
// Model signature: GPT-6. Business state stays inside the service implementation.
namespace zmc {
namespace srv {
namespace {
// Preserve the existing HTTP/RPC business error mapping.
int fileResult(HttpFileManager::Result result) {
    switch (result) {
        case HttpFileManager::RESULT_OK:
            return ZMC_NO_ERROR;
        case HttpFileManager::RESULT_INVALID_NAME:
            return ZMC_ERR_RPC_FILE_INVALID_NAME;
        case HttpFileManager::RESULT_NOT_FOUND:
        case HttpFileManager::RESULT_NOT_REGULAR:
            return ZMC_ERR_RPC_FILE_NOT_FOUND;
        case HttpFileManager::RESULT_ALREADY_EXISTS:
            return ZMC_ERR_RPC_FILE_ALREADY_EXISTS;
        default:
            return ZMC_ERR_RPC_FILE_IO_FAILED;
    }
}
}  // namespace
SrvFileService::SrvFileService() {}
SrvFileService::~SrvFileService() {}

// GPT-6 (Codex), 2026-10-29. Keep file-specific initialization and error mapping here.
int SrvFileService::initializeFiles() { return fileResult(impl_->files.initialize()); }

int SrvFileService::listFiles(std::vector<SrvFileEntry>& out) {
    std::vector<HttpFileManager::Entry> entries;
    const int ret = fileResult(impl_->files.list(entries));
    out.clear();
    if (ret != 0) return ret;
    out.reserve(entries.size());
    for (const auto& item : entries)
        out.push_back(SrvFileEntry{item.name, item.size, item.time, item.hash});
    return ret;
}

// The request-local writer discards unpublished data; no session participates.
int SrvFileService::createFile(const std::string& name, const std::vector<char>& data,
                               std::string* hash) {
    if (hash) hash->clear();
    if (data.size() > 16U * 1024U * 1024U) return ZMC_ERR_RPC_FILE_TOO_LARGE;
    // Exclusive temporary creation and atomic publication keep the writer request-local.
    HttpFileManager::Writer writer;
    HttpFileManager::Result result = impl_->files.beginCreate(name, writer);
    if (result == HttpFileManager::RESULT_OK) result = writer.write(data.data(), data.size());
    if (result == HttpFileManager::RESULT_OK) result = writer.close();
    if (result == HttpFileManager::RESULT_OK) result = impl_->files.commitCreate(writer);
    if (result == HttpFileManager::RESULT_OK && hash) *hash = writer.hash();
    return fileResult(result);
}

// The request-local writer discards unpublished data; no session participates.
int SrvFileService::replaceFile(const std::string& name, const std::vector<char>& data,
                                std::string* hash) {
    if (hash) hash->clear();
    if (data.size() > 16U * 1024U * 1024U) return ZMC_ERR_RPC_FILE_TOO_LARGE;
    // Exclusive temporary creation and atomic publication keep the writer request-local.
    HttpFileManager::Writer writer;
    HttpFileManager::Result result = impl_->files.beginReplace(name, writer);
    if (result == HttpFileManager::RESULT_OK) result = writer.write(data.data(), data.size());
    if (result == HttpFileManager::RESULT_OK) result = writer.close();
    if (result == HttpFileManager::RESULT_OK) result = impl_->files.commitReplace(writer);
    if (result == HttpFileManager::RESULT_OK && hash) *hash = writer.hash();
    return fileResult(result);
}

// GPT-6 Codex, 2026-10-23. Preserve exclusive creation and existing-file replacement.
int SrvFileService::writeFile(const std::string& name, const std::vector<char>& data,
                             std::string* hash) {
    const int ret = createFile(name, data, hash);
    return ret == ZMC_ERR_RPC_FILE_ALREADY_EXISTS ? replaceFile(name, data, hash) : ret;
}

int SrvFileService::readFile(const std::string& name, std::vector<char>& data,
                             uint64_t max_bytes, std::string* hash) {
    if (hash) hash->clear();
    data.clear();
    uint64_t size = 0;
    std::shared_ptr<std::ifstream> input;
    const int ret = fileResult(impl_->files.openRead(name, input, size));
    if (ret != 0) return ret;
    // File operations do not acquire the command lock; the open stream retains its snapshot.
    if (max_bytes != 0 && size > max_bytes) return ZMC_ERR_RPC_FILE_TOO_LARGE;
    if (size > static_cast<uint64_t>((std::numeric_limits<size_t>::max)()))
        return ZMC_ERR_RPC_FILE_IO_FAILED;
    data.resize(static_cast<size_t>(size));
    if (size != 0) {
        input->read(data.data(), static_cast<std::streamsize>(size));
        if (!*input || static_cast<uint64_t>(input->gcount()) != size) {
            data.clear();
            return ZMC_ERR_RPC_FILE_IO_FAILED;
        }
    }
    if (hash) *hash = HttpFileManager::hashBytes(data.empty() ? NULL : data.data(), data.size());
    return 0;
}

int SrvFileService::removeFile(const std::string& name) {
    return fileResult(impl_->files.remove(name));
}

}  // namespace srv
}  // namespace zmc
