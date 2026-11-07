#include "MemoryVfs.hpp"

#include <chrono>

// Implements the platform-independent in-memory backend. AI modification signature: GPT-6 Codex.

namespace zmc {
namespace {

int64_t nowSeconds() {
    return std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string parentPath(const std::string& path) {
    const size_t slash = path.find_last_of('/');
    return slash == 0 ? "/" : path.substr(0, slash);
}

std::string baseName(const std::string& path) {
    return path.substr(path.find_last_of('/') + 1);
}

}  // namespace

struct MemoryVfs::Node {
    explicit Node(bool isDirectory)
        : directory(isDirectory), modifiedTime(nowSeconds()) {}

    bool directory;
    int64_t modifiedTime;
    std::string bytes;
    std::map<std::string, std::unique_ptr<Node> > children;
};

VfsResult MemoryVfs::findNode(Node* root, const std::string& path, Node*& node) {
    node = root;
    if (path == "/") return VfsResult::Ok;
    size_t start = 1;
    while (start < path.size()) {
        const size_t end = path.find('/', start);
        const std::string name = path.substr(start, end == std::string::npos ? end : end - start);
        if (!node->directory) return VfsResult::NotDirectory;
        std::map<std::string, std::unique_ptr<Node> >::iterator found = node->children.find(name);
        if (found == node->children.end()) return VfsResult::NotFound;
        node = found->second.get();
        if (end == std::string::npos) break;
        start = end + 1;
    }
    return VfsResult::Ok;
}

VfsResult MemoryVfs::findNode(const Node* root, const std::string& path, const Node*& node) {
    Node* mutableNode = NULL;
    VfsResult result = findNode(const_cast<Node*>(root), path, mutableNode);
    node = mutableNode;
    return result;
}

VfsResult MemoryVfs::makeMetadata(const Node* node, VfsMetadata& metadata) {
    metadata.type = node->directory ? VfsFileType::Directory : VfsFileType::File;
    metadata.size = node->directory ? 0 : static_cast<uint64_t>(node->bytes.size());
    metadata.modifiedTime = node->modifiedTime;
    return VfsResult::Ok;
}

MemoryVfs::MemoryVfs() : root_(new Node(true)) {}
MemoryVfs::~MemoryVfs() {}

VfsResult MemoryVfs::readFile(const std::string& path, std::string& bytes) const {
    std::string normalized;
    VfsResult result = normalizeVfsPath(path, normalized);
    if (result != VfsResult::Ok) return result;
    std::lock_guard<std::mutex> lock(mutex_);
    const Node* node = NULL;
    result = findNode(root_.get(), normalized, node);
    if (result != VfsResult::Ok) return result;
    if (node->directory) return VfsResult::NotFile;
    bytes = node->bytes;
    return VfsResult::Ok;
}

VfsResult MemoryVfs::writeFile(const std::string& path, const void* bytes, size_t size,
                               VfsWriteMode mode) {
    std::string normalized;
    VfsResult result = normalizeVfsPath(path, normalized);
    if (result != VfsResult::Ok) return result;
    if (normalized == "/") return VfsResult::NotFile;
    if (size && !bytes) return VfsResult::IoError;
    if (mode != VfsWriteMode::CreateOnly && mode != VfsWriteMode::ReplaceOnly &&
        mode != VfsWriteMode::CreateOrReplace) return VfsResult::IoError;
    std::string replacement;
    if (size) replacement.assign(static_cast<const char*>(bytes), size);
    std::lock_guard<std::mutex> lock(mutex_);
    Node* parent = NULL;
    result = findNode(root_.get(), parentPath(normalized), parent);
    if (result != VfsResult::Ok) return result;
    if (!parent->directory) return VfsResult::NotDirectory;
    const std::string name = baseName(normalized);
    std::map<std::string, std::unique_ptr<Node> >::iterator found = parent->children.find(name);
    if (found != parent->children.end()) {
        if (found->second->directory) return VfsResult::NotFile;
        if (mode == VfsWriteMode::CreateOnly) return VfsResult::AlreadyExists;
        found->second->bytes.swap(replacement);
        found->second->modifiedTime = nowSeconds();
    } else {
        if (mode == VfsWriteMode::ReplaceOnly) return VfsResult::NotFound;
        std::unique_ptr<Node> file(new Node(false));
        file->bytes.swap(replacement);
        parent->children.insert(std::make_pair(name, std::move(file)));
    }
    parent->modifiedTime = nowSeconds();
    return VfsResult::Ok;
}

VfsResult MemoryVfs::removeFile(const std::string& path) {
    std::string normalized;
    VfsResult result = normalizeVfsPath(path, normalized);
    if (result != VfsResult::Ok) return result;
    if (normalized == "/") return VfsResult::NotFile;
    std::lock_guard<std::mutex> lock(mutex_);
    Node* parent = NULL;
    result = findNode(root_.get(), parentPath(normalized), parent);
    if (result != VfsResult::Ok) return result;
    if (!parent->directory) return VfsResult::NotDirectory;
    std::map<std::string, std::unique_ptr<Node> >::iterator found =
        parent->children.find(baseName(normalized));
    if (found == parent->children.end()) return VfsResult::NotFound;
    if (found->second->directory) return VfsResult::NotFile;
    parent->children.erase(found);
    parent->modifiedTime = nowSeconds();
    return VfsResult::Ok;
}

VfsResult MemoryVfs::createDirectory(const std::string& path) {
    std::string normalized;
    VfsResult result = normalizeVfsPath(path, normalized);
    if (result != VfsResult::Ok) return result;
    if (normalized == "/") return VfsResult::AlreadyExists;
    std::lock_guard<std::mutex> lock(mutex_);
    Node* parent = NULL;
    result = findNode(root_.get(), parentPath(normalized), parent);
    if (result != VfsResult::Ok) return result;
    if (!parent->directory) return VfsResult::NotDirectory;
    const std::string name = baseName(normalized);
    std::map<std::string, std::unique_ptr<Node> >::iterator found = parent->children.find(name);
    if (found != parent->children.end())
        return found->second->directory ? VfsResult::AlreadyExists : VfsResult::NotDirectory;
    parent->children.insert(std::make_pair(name, std::unique_ptr<Node>(new Node(true))));
    parent->modifiedTime = nowSeconds();
    return VfsResult::Ok;
}

VfsResult MemoryVfs::listDirectory(const std::string& path,
                                   std::vector<VfsEntry>& entries) const {
    std::string normalized;
    VfsResult result = normalizeVfsPath(path, normalized);
    if (result != VfsResult::Ok) return result;
    std::lock_guard<std::mutex> lock(mutex_);
    const Node* node = NULL;
    result = findNode(root_.get(), normalized, node);
    if (result != VfsResult::Ok) return result;
    if (!node->directory) return VfsResult::NotDirectory;
    std::vector<VfsEntry> listed;
    for (std::map<std::string, std::unique_ptr<Node> >::const_iterator it =
             node->children.begin(); it != node->children.end(); ++it) {
        VfsEntry entry;
        entry.name = it->first;
        makeMetadata(it->second.get(), entry.metadata);
        listed.push_back(entry);
    }
    entries.swap(listed);
    return VfsResult::Ok;
}

VfsResult MemoryVfs::removeDirectory(const std::string& path) {
    std::string normalized;
    VfsResult result = normalizeVfsPath(path, normalized);
    if (result != VfsResult::Ok) return result;
    if (normalized == "/") return VfsResult::InvalidPath;
    std::lock_guard<std::mutex> lock(mutex_);
    Node* parent = NULL;
    result = findNode(root_.get(), parentPath(normalized), parent);
    if (result != VfsResult::Ok) return result;
    if (!parent->directory) return VfsResult::NotDirectory;
    std::map<std::string, std::unique_ptr<Node> >::iterator found =
        parent->children.find(baseName(normalized));
    if (found == parent->children.end()) return VfsResult::NotFound;
    if (!found->second->directory) return VfsResult::NotDirectory;
    if (!found->second->children.empty()) return VfsResult::DirectoryNotEmpty;
    parent->children.erase(found);
    parent->modifiedTime = nowSeconds();
    return VfsResult::Ok;
}

VfsResult MemoryVfs::stat(const std::string& path, VfsMetadata& metadata) const {
    std::string normalized;
    VfsResult result = normalizeVfsPath(path, normalized);
    if (result != VfsResult::Ok) return result;
    std::lock_guard<std::mutex> lock(mutex_);
    const Node* node = NULL;
    result = findNode(root_.get(), normalized, node);
    return result == VfsResult::Ok ? makeMetadata(node, metadata) : result;
}

}  // namespace zmc
