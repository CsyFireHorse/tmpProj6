#ifndef ZMC_MEMORY_VFS_HPP
#define ZMC_MEMORY_VFS_HPP

// In-memory virtual file backend. AI modification signature: GPT-6 Codex.

#include "IVfs.hpp"

#include <map>
#include <memory>
#include <mutex>

namespace zmc {

// Stores a private, thread-safe directory tree in memory.
class MemoryVfs : public IVfs {
public:
    MemoryVfs();
    ~MemoryVfs();

    VfsResult readFile(const std::string& path, std::string& bytes) const;
    VfsResult writeFile(const std::string& path, const void* bytes, size_t size,
                        VfsWriteMode mode);
    VfsResult removeFile(const std::string& path);
    VfsResult createDirectory(const std::string& path);
    VfsResult listDirectory(const std::string& path, std::vector<VfsEntry>& entries) const;
    VfsResult removeDirectory(const std::string& path);
    VfsResult stat(const std::string& path, VfsMetadata& metadata) const;

private:
    struct Node;
    static VfsResult findNode(Node* root, const std::string& path, Node*& node);
    static VfsResult findNode(const Node* root, const std::string& path, const Node*& node);
    static VfsResult makeMetadata(const Node* node, VfsMetadata& metadata);
    std::unique_ptr<Node> root_;
    mutable std::mutex mutex_;
};

}  // namespace zmc

#endif  // ZMC_MEMORY_VFS_HPP
