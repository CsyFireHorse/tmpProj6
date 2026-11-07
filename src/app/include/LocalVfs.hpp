#ifndef ZMC_LOCAL_VFS_HPP
#define ZMC_LOCAL_VFS_HPP

// Host filesystem backend for the application layer. AI modification signature: GPT-6 Codex.

#include "IVfs.hpp"

#include <mutex>
#include <string>

namespace zmc {

// Maps virtual paths into an existing, isolated physical root directory.
class LocalVfs : public IVfs {
public:
    explicit LocalVfs(const std::string& rootPath);
    ~LocalVfs();

    VfsResult initialize();
    VfsResult readFile(const std::string& path, std::string& bytes) const;
    VfsResult writeFile(const std::string& path, const void* bytes, size_t size,
                        VfsWriteMode mode);
    VfsResult removeFile(const std::string& path);
    VfsResult createDirectory(const std::string& path);
    VfsResult listDirectory(const std::string& path, std::vector<VfsEntry>& entries) const;
    VfsResult removeDirectory(const std::string& path);
    VfsResult stat(const std::string& path, VfsMetadata& metadata) const;

private:
    std::string rootPath_;
    mutable std::mutex mutex_;
#ifdef _WIN32
    void* rootHandle_;
#else
    int rootFd_;
#endif
};

}  // namespace zmc

#endif  // ZMC_LOCAL_VFS_HPP
