#ifndef ZMC_IVFS_HPP
#define ZMC_IVFS_HPP

// Platform-independent virtual file interface. AI modification signature: GPT-6 Codex.

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

namespace zmc {

// Result values are shared by every backend.
enum class VfsResult {
    Ok,
    NotFound,
    AlreadyExists,
    InvalidPath,
    NotDirectory,
    NotFile,
    DirectoryNotEmpty,
    IoError
};

// CreateOnly fails for existing files; ReplaceOnly fails for missing files.
// CreateOrReplace creates a missing file or atomically replaces an existing file.
enum class VfsWriteMode { CreateOnly, ReplaceOnly, CreateOrReplace };
enum class VfsFileType { File, Directory };

struct VfsMetadata {
    VfsFileType type;
    uint64_t size;          // File bytes; directories always report zero.
    int64_t modifiedTime;   // Unix time in seconds.
};

struct VfsEntry {
    std::string name;
    VfsMetadata metadata;
};

// Normalizes UTF-8 paths using '/' and case-sensitive names. Relative paths are
// rooted at '/', repeated separators and '.' are removed, and '..' is rejected.
// Host absolute paths, backslashes, invalid UTF-8, and nonportable names fail.
VfsResult normalizeVfsPath(const std::string& path, std::string& normalized);

// Defines the platform-independent file and directory operations.
class IVfs {
public:
    virtual ~IVfs() {}

    // Reads the entire binary file and leaves bytes unchanged on failure.
    virtual VfsResult readFile(const std::string& path, std::string& bytes) const = 0;
    // Writes the entire binary file. A nonzero size requires non-null bytes.
    virtual VfsResult writeFile(const std::string& path, const void* bytes, size_t size,
                                VfsWriteMode mode) = 0;
    virtual VfsResult removeFile(const std::string& path) = 0;
    // Creates one directory; its parent must already exist.
    virtual VfsResult createDirectory(const std::string& path) = 0;
    // Lists immediate children in ascending name order.
    virtual VfsResult listDirectory(const std::string& path,
                                    std::vector<VfsEntry>& entries) const = 0;
    virtual VfsResult removeDirectory(const std::string& path) = 0;
    virtual VfsResult stat(const std::string& path, VfsMetadata& metadata) const = 0;
};

}  // namespace zmc

#endif  // ZMC_IVFS_HPP
