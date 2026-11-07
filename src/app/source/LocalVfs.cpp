#include "LocalVfs.hpp"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

// Implements root-scoped host file operations. AI modification signature: GPT-6 Codex.

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace zmc {
namespace {

#ifdef _WIN32
std::string parentPath(const std::string& path) {
    const size_t slash = path.find_last_of('/');
    return slash == 0 ? "/" : path.substr(0, slash);
}

std::string baseName(const std::string& path) {
    return path.substr(path.find_last_of('/') + 1);
}
#endif

#ifndef _WIN32

VfsResult errorResult(int error) {
    if (error == ENOENT) return VfsResult::NotFound;
    if (error == ENOTDIR) return VfsResult::NotDirectory;
    if (error == EEXIST) return VfsResult::AlreadyExists;
    if (error == ENOTEMPTY) return VfsResult::DirectoryNotEmpty;
    if (error == ELOOP) return VfsResult::InvalidPath;
    return VfsResult::IoError;
}

VfsResult openDirectory(int parent, const std::string& name, int& fd) {
    fd = openat(parent, name.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (fd >= 0) return VfsResult::Ok;
    if (errno == ELOOP) return VfsResult::InvalidPath;
    if (errno == ENOTDIR) {
        struct stat state;
        if (fstatat(parent, name.c_str(), &state, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(state.st_mode))
            return VfsResult::InvalidPath;
    }
    return errorResult(errno);
}

VfsResult openParent(int root, const std::string& path, int& parent, std::string& name) {
    parent = dup(root);
    if (parent < 0) return VfsResult::IoError;
    size_t start = 1;
    while (start < path.size()) {
        const size_t end = path.find('/', start);
        if (end == std::string::npos) {
            name = path.substr(start);
            return VfsResult::Ok;
        }
        const std::string part = path.substr(start, end - start);
        int next = -1;
        VfsResult result = openDirectory(parent, part, next);
        close(parent);
        if (result != VfsResult::Ok) return result;
        parent = next;
        start = end + 1;
    }
    name.clear();
    return VfsResult::Ok;
}

VfsResult inspectAt(int parent, const std::string& name, struct stat& state) {
    if (fstatat(parent, name.c_str(), &state, AT_SYMLINK_NOFOLLOW) != 0) {
        if (errno == ELOOP) return VfsResult::InvalidPath;
        return errorResult(errno);
    }
    if (S_ISLNK(state.st_mode)) return VfsResult::InvalidPath;
    return VfsResult::Ok;
}

VfsResult metadataFromStat(const struct stat& state, VfsMetadata& metadata) {
    if (S_ISDIR(state.st_mode)) metadata.type = VfsFileType::Directory;
    else if (S_ISREG(state.st_mode) && state.st_nlink == 1) metadata.type = VfsFileType::File;
    else return VfsResult::InvalidPath;
    metadata.size = metadata.type == VfsFileType::File ? static_cast<uint64_t>(state.st_size) : 0;
    metadata.modifiedTime = static_cast<int64_t>(state.st_mtime);
    return VfsResult::Ok;
}

VfsResult fillMetadataAt(int parent, const std::string& name, VfsMetadata& metadata) {
    struct stat state;
    VfsResult result = inspectAt(parent, name, state);
    return result == VfsResult::Ok ? metadataFromStat(state, metadata) : result;
}

VfsResult rootMetadata(int root, VfsMetadata& metadata) {
    struct stat state;
    if (fstat(root, &state) != 0) return VfsResult::IoError;
    return metadataFromStat(state, metadata);
}

bool writeAll(int fd, const char* bytes, size_t size) {
    size_t offset = 0;
    while (offset < size) {
        const ssize_t written = write(fd, bytes + offset, size - offset);
        if (written < 0 && errno == EINTR) continue;
        if (written <= 0) return false;
        offset += static_cast<size_t>(written);
    }
    return true;
}

#else

std::wstring wide(const std::string& value) {
    if (value.empty()) return std::wstring();
    const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                                          static_cast<int>(value.size()), NULL, 0);
    if (count <= 0) return std::wstring();
    std::wstring result(static_cast<size_t>(count), L'\0');
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                             static_cast<int>(value.size()), &result[0], count)) return std::wstring();
    return result;
}

VfsResult windowsError(DWORD error) {
    if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) return VfsResult::NotFound;
    if (error == ERROR_ALREADY_EXISTS || error == ERROR_FILE_EXISTS) return VfsResult::AlreadyExists;
    if (error == ERROR_DIRECTORY) return VfsResult::NotDirectory;
    if (error == ERROR_DIR_NOT_EMPTY) return VfsResult::DirectoryNotEmpty;
    if (error == ERROR_CANT_ACCESS_FILE) return VfsResult::InvalidPath;
    return VfsResult::IoError;
}

std::wstring physicalPath(const std::wstring& root, const std::string& path) {
    std::wstring result = root;
    std::wstring suffix = wide(path == "/" ? std::string() : path.substr(1));
    std::replace(suffix.begin(), suffix.end(), L'/', L'\\');
    if (!suffix.empty() && !result.empty() && result[result.size() - 1] != L'\\') result += L'\\';
    result += suffix;
    return result;
}

VfsResult windowsMetadata(const std::wstring& path, VfsMetadata& metadata) {
    HANDLE handle = CreateFileW(path.c_str(), FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
    if (handle == INVALID_HANDLE_VALUE) return windowsError(GetLastError());
    BY_HANDLE_FILE_INFORMATION data;
    if (!GetFileInformationByHandle(handle, &data)) {
        DWORD error = GetLastError(); CloseHandle(handle); return windowsError(error);
    }
    if (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
        CloseHandle(handle); return VfsResult::InvalidPath;
    }
    metadata.type = data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? VfsFileType::Directory
                                                                     : VfsFileType::File;
    if (metadata.type == VfsFileType::File && data.nNumberOfLinks != 1) {
        CloseHandle(handle); return VfsResult::InvalidPath;
    }
    metadata.size = metadata.type == VfsFileType::Directory ? 0 :
                    (static_cast<uint64_t>(data.nFileSizeHigh) << 32) | data.nFileSizeLow;
    ULARGE_INTEGER ticks;
    ticks.LowPart = data.ftLastWriteTime.dwLowDateTime;
    ticks.HighPart = data.ftLastWriteTime.dwHighDateTime;
    metadata.modifiedTime = ticks.QuadPart >= 116444736000000000ULL
        ? static_cast<int64_t>((ticks.QuadPart - 116444736000000000ULL) / 10000000ULL) : 0;
    CloseHandle(handle);
    return VfsResult::Ok;
}

bool setCaseSensitive(HANDLE directory) {
    typedef BOOL (WINAPI *SetFileInformationFn)(HANDLE, FILE_INFO_BY_HANDLE_CLASS, LPVOID, DWORD);
    HMODULE kernel = GetModuleHandleW(L"kernel32.dll");
    SetFileInformationFn setInfo = reinterpret_cast<SetFileInformationFn>(
        GetProcAddress(kernel, "SetFileInformationByHandle"));
    if (!setInfo) return false;
    ULONG flags = 1;
    return setInfo(directory, static_cast<FILE_INFO_BY_HANDLE_CLASS>(23), &flags, sizeof(flags)) != 0;
}

bool ensureCaseSensitive(const std::wstring& directory) {
    HANDLE handle = CreateFileW(directory.c_str(), FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
    if (handle == INVALID_HANDLE_VALUE) return false;
    BY_HANDLE_FILE_INFORMATION info;
    const bool directoryOk = GetFileInformationByHandle(handle, &info) &&
        (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        !(info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT);
    const bool result = directoryOk && setCaseSensitive(handle);
    CloseHandle(handle);
    return result;
}

VfsResult verifyWindowsPath(const std::wstring& root, const std::string& path,
                            bool allowMissingFinal) {
    std::wstring current = root;
    size_t start = 1;
    while (start < path.size()) {
        const size_t end = path.find('/', start);
        const bool final = end == std::string::npos;
        const std::wstring part = wide(path.substr(start, final ? end : end - start));
        if (current.empty() || part.empty()) return VfsResult::InvalidPath;
        if (current[current.size() - 1] != L'\\') current += L'\\';
        current += part;
        VfsMetadata metadata;
        VfsResult result = windowsMetadata(current, metadata);
        if (result == VfsResult::NotFound && final && allowMissingFinal) return VfsResult::Ok;
        if (result != VfsResult::Ok) return result;
        if (!final && metadata.type != VfsFileType::Directory) return VfsResult::NotDirectory;
        if (!final && !ensureCaseSensitive(current)) return VfsResult::IoError;
        if (final) return VfsResult::Ok;
        start = end + 1;
    }
    return VfsResult::Ok;
}

#endif

}  // namespace

LocalVfs::LocalVfs(const std::string& rootPath)
    : rootPath_(rootPath)
#ifdef _WIN32
      , rootHandle_(INVALID_HANDLE_VALUE)
#else
      , rootFd_(-1)
#endif
{}

LocalVfs::~LocalVfs() {
#ifdef _WIN32
    if (rootHandle_ != INVALID_HANDLE_VALUE) CloseHandle(rootHandle_);
#else
    if (rootFd_ >= 0) close(rootFd_);
#endif
}

VfsResult LocalVfs::initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
#ifdef _WIN32
    std::wstring root = wide(rootPath_);
    if (root.empty()) return VfsResult::InvalidPath;
    HANDLE handle = CreateFileW(root.c_str(), FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES |
        FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
    if (handle == INVALID_HANDLE_VALUE) return windowsError(GetLastError());
    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(handle, &info) ||
        !(info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
        (info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
        CloseHandle(handle);
        return VfsResult::InvalidPath;
    }
    if (!setCaseSensitive(handle)) {
        CloseHandle(handle);
        return VfsResult::IoError;
    }
    rootHandle_ = handle;
    return VfsResult::Ok;
#else
    if (rootFd_ >= 0) return VfsResult::Ok;
    rootFd_ = open(rootPath_.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (rootFd_ >= 0) return VfsResult::Ok;
    if (errno == ELOOP) return VfsResult::InvalidPath;
    if (errno == ENOTDIR) {
        struct stat state;
        if (lstat(rootPath_.c_str(), &state) == 0 && S_ISLNK(state.st_mode))
            return VfsResult::InvalidPath;
    }
    return errorResult(errno);
#endif
}

VfsResult LocalVfs::readFile(const std::string& path, std::string& bytes) const {
    std::string normalized;
    VfsResult result = normalizeVfsPath(path, normalized);
    if (result != VfsResult::Ok) return result;
    if (normalized == "/") return VfsResult::NotFile;
    std::lock_guard<std::mutex> lock(mutex_);
#ifdef _WIN32
    if (rootHandle_ == INVALID_HANDLE_VALUE) return VfsResult::IoError;
    result = verifyWindowsPath(wide(rootPath_), normalized, false);
    if (result != VfsResult::Ok) return result;
    const std::wstring file = physicalPath(wide(rootPath_), normalized);
    VfsMetadata metadata;
    result = windowsMetadata(file, metadata);
    if (result != VfsResult::Ok) return result;
    if (metadata.type != VfsFileType::File) return VfsResult::NotFile;
    HANDLE handle = CreateFileW(file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                FILE_FLAG_OPEN_REPARSE_POINT, NULL);
    if (handle == INVALID_HANDLE_VALUE) return windowsError(GetLastError());
    if (metadata.size > static_cast<uint64_t>((std::numeric_limits<size_t>::max)())) {
        CloseHandle(handle); return VfsResult::IoError;
    }
    std::string content(static_cast<size_t>(metadata.size), '\0');
    size_t offset = 0;
    while (offset < content.size()) {
        DWORD count = 0;
        const DWORD request = static_cast<DWORD>((std::min)(content.size() - offset,
                                                   static_cast<size_t>(0x7fffffff)));
        if (!ReadFile(handle, &content[offset], request, &count, NULL) || count == 0) {
            CloseHandle(handle); return VfsResult::IoError;
        }
        offset += count;
    }
    CloseHandle(handle);
    bytes.swap(content);
    return VfsResult::Ok;
#else
    if (rootFd_ < 0) return VfsResult::IoError;
    int parent = -1;
    std::string name;
    result = openParent(rootFd_, normalized, parent, name);
    if (result != VfsResult::Ok) return result;
    int fd = openat(parent, name.c_str(), O_RDONLY | O_NONBLOCK | O_NOFOLLOW | O_CLOEXEC);
    const int savedError = errno;
    struct stat state;
    if (fd < 0) result = savedError == ELOOP ? VfsResult::InvalidPath : errorResult(savedError);
    else if (fstat(fd, &state) != 0) result = VfsResult::IoError;
    else if (!S_ISREG(state.st_mode)) result = VfsResult::NotFile;
    else if (state.st_nlink != 1) result = VfsResult::InvalidPath;
    else {
        std::string content;
        char buffer[8192];
        ssize_t count;
        while ((count = read(fd, buffer, sizeof(buffer))) > 0) content.append(buffer, count);
        result = count == 0 ? VfsResult::Ok : VfsResult::IoError;
        if (result == VfsResult::Ok) bytes.swap(content);
    }
    if (fd >= 0) close(fd);
    close(parent);
    return result;
#endif
}

VfsResult LocalVfs::writeFile(const std::string& path, const void* bytes, size_t size,
                              VfsWriteMode mode) {
    std::string normalized;
    VfsResult result = normalizeVfsPath(path, normalized);
    if (result != VfsResult::Ok) return result;
    if (normalized == "/") return VfsResult::NotFile;
    if (size && !bytes) return VfsResult::IoError;
    if (mode != VfsWriteMode::CreateOnly && mode != VfsWriteMode::ReplaceOnly &&
        mode != VfsWriteMode::CreateOrReplace) return VfsResult::IoError;
    std::lock_guard<std::mutex> lock(mutex_);
#ifdef _WIN32
    if (rootHandle_ == INVALID_HANDLE_VALUE) return VfsResult::IoError;
    result = verifyWindowsPath(wide(rootPath_), normalized, true);
    if (result != VfsResult::Ok) return result;
    const std::wstring file = physicalPath(wide(rootPath_), normalized);
    VfsMetadata old;
    VfsResult exists = windowsMetadata(file, old);
    if (exists == VfsResult::Ok && old.type != VfsFileType::File) return VfsResult::NotFile;
    if (exists != VfsResult::Ok && exists != VfsResult::NotFound) return exists;
    if (mode == VfsWriteMode::CreateOnly && exists == VfsResult::Ok) return VfsResult::AlreadyExists;
    if (mode == VfsWriteMode::ReplaceOnly && exists == VfsResult::NotFound) return VfsResult::NotFound;
    const std::wstring temp = file + L".ivfs-tmp";
    HANDLE handle = CreateFileW(temp.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW,
                                FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_OPEN_REPARSE_POINT, NULL);
    if (handle == INVALID_HANDLE_VALUE) return windowsError(GetLastError());
    size_t offset = 0;
    bool ok = true;
    while (offset < size) {
        DWORD written = 0;
        DWORD count = static_cast<DWORD>((std::min)(size - offset,
                                                static_cast<size_t>(0x7fffffff)));
        if (!WriteFile(handle, static_cast<const char*>(bytes) + offset, count, &written, NULL) ||
            written == 0) { ok = false; break; }
        offset += written;
    }
    if (!CloseHandle(handle)) ok = false;
    if (!ok) { DeleteFileW(temp.c_str()); return VfsResult::IoError; }
    DWORD flags = MOVEFILE_WRITE_THROUGH;
    if (mode != VfsWriteMode::CreateOnly) flags |= MOVEFILE_REPLACE_EXISTING;
    if (!MoveFileExW(temp.c_str(), file.c_str(), flags)) {
        DWORD error = GetLastError(); DeleteFileW(temp.c_str()); return windowsError(error);
    }
    return VfsResult::Ok;
#else
    if (rootFd_ < 0) return VfsResult::IoError;
    int parent = -1;
    std::string name;
    result = openParent(rootFd_, normalized, parent, name);
    if (result != VfsResult::Ok) return result;
    struct stat old;
    VfsResult exists = inspectAt(parent, name, old);
    if (exists == VfsResult::Ok && !S_ISREG(old.st_mode)) result = VfsResult::NotFile;
    else if (exists == VfsResult::Ok && old.st_nlink != 1) result = VfsResult::InvalidPath;
    else if (exists != VfsResult::Ok && exists != VfsResult::NotFound) result = exists;
    else if (mode == VfsWriteMode::CreateOnly && exists == VfsResult::Ok)
        result = VfsResult::AlreadyExists;
    else if (mode == VfsWriteMode::ReplaceOnly && exists == VfsResult::NotFound)
        result = VfsResult::NotFound;
    else result = VfsResult::Ok;
    if (result != VfsResult::Ok) { close(parent); return result; }

    char temp[48];
    int fd = -1;
    for (unsigned int attempt = 0; attempt < 100; ++attempt) {
        std::snprintf(temp, sizeof(temp), ".ivfs-tmp-%ld-%u", static_cast<long>(getpid()), attempt);
        fd = openat(parent, temp, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
        if (fd >= 0 || errno != EEXIST) break;
    }
    if (fd < 0) { result = VfsResult::IoError; close(parent); return result; }
    const bool written = writeAll(fd, static_cast<const char*>(bytes), size);
    const bool closed = close(fd) == 0;
    bool tempExists = true;
    if (!written || !closed) result = VfsResult::IoError;
    else if (mode == VfsWriteMode::CreateOnly) {
        if (linkat(parent, temp, parent, name.c_str(), 0) != 0) result = errorResult(errno);
        else result = VfsResult::Ok;
    } else if (renameat(parent, temp, parent, name.c_str()) != 0) result = errorResult(errno);
    else tempExists = false;
    if (tempExists && unlinkat(parent, temp, 0) != 0 && result == VfsResult::Ok)
        result = VfsResult::IoError;
    close(parent);
    return result;
#endif
}

VfsResult LocalVfs::removeFile(const std::string& path) {
    std::string normalized;
    VfsResult result = normalizeVfsPath(path, normalized);
    if (result != VfsResult::Ok) return result;
    if (normalized == "/") return VfsResult::NotFile;
    std::lock_guard<std::mutex> lock(mutex_);
#ifdef _WIN32
    if (rootHandle_ == INVALID_HANDLE_VALUE) return VfsResult::IoError;
    result = verifyWindowsPath(wide(rootPath_), normalized, false);
    if (result != VfsResult::Ok) return result;
    const std::wstring file = physicalPath(wide(rootPath_), normalized);
    VfsMetadata metadata;
    result = windowsMetadata(file, metadata);
    if (result != VfsResult::Ok) return result;
    if (metadata.type != VfsFileType::File) return VfsResult::NotFile;
    return DeleteFileW(file.c_str()) ? VfsResult::Ok : windowsError(GetLastError());
#else
    if (rootFd_ < 0) return VfsResult::IoError;
    int parent = -1; std::string name;
    result = openParent(rootFd_, normalized, parent, name);
    if (result != VfsResult::Ok) return result;
    struct stat state;
    result = inspectAt(parent, name, state);
    if (result == VfsResult::Ok && !S_ISREG(state.st_mode)) result = VfsResult::NotFile;
    else if (result == VfsResult::Ok && state.st_nlink != 1) result = VfsResult::InvalidPath;
    else if (result == VfsResult::Ok && unlinkat(parent, name.c_str(), 0) != 0) result = errorResult(errno);
    close(parent); return result;
#endif
}

VfsResult LocalVfs::createDirectory(const std::string& path) {
    std::string normalized;
    VfsResult result = normalizeVfsPath(path, normalized);
    if (result != VfsResult::Ok) return result;
    if (normalized == "/") return VfsResult::AlreadyExists;
    std::lock_guard<std::mutex> lock(mutex_);
#ifdef _WIN32
    if (rootHandle_ == INVALID_HANDLE_VALUE) return VfsResult::IoError;
    result = verifyWindowsPath(wide(rootPath_), normalized, true);
    if (result != VfsResult::Ok) return result;
    const std::wstring dir = physicalPath(wide(rootPath_), normalized);
    if (CreateDirectoryW(dir.c_str(), NULL)) return VfsResult::Ok;
    DWORD error = GetLastError();
    if (error == ERROR_ALREADY_EXISTS) {
        VfsMetadata metadata;
        result = windowsMetadata(dir, metadata);
        return result == VfsResult::Ok && metadata.type == VfsFileType::File
                   ? VfsResult::NotDirectory : VfsResult::AlreadyExists;
    }
    return windowsError(error);
#else
    if (rootFd_ < 0) return VfsResult::IoError;
    int parent = -1; std::string name;
    result = openParent(rootFd_, normalized, parent, name);
    if (result != VfsResult::Ok) return result;
    struct stat old;
    VfsResult exists = inspectAt(parent, name, old);
    if (exists == VfsResult::Ok) result = S_ISDIR(old.st_mode) ? VfsResult::AlreadyExists
                                                               : VfsResult::NotDirectory;
    else if (exists != VfsResult::NotFound) result = exists;
    else if (mkdirat(parent, name.c_str(), 0700) != 0) result = errorResult(errno);
    else result = VfsResult::Ok;
    close(parent); return result;
#endif
}

VfsResult LocalVfs::listDirectory(const std::string& path,
                                  std::vector<VfsEntry>& entries) const {
    std::string normalized;
    VfsResult result = normalizeVfsPath(path, normalized);
    if (result != VfsResult::Ok) return result;
    std::lock_guard<std::mutex> lock(mutex_);
#ifdef _WIN32
    if (rootHandle_ == INVALID_HANDLE_VALUE) return VfsResult::IoError;
    result = verifyWindowsPath(wide(rootPath_), normalized, false);
    if (result != VfsResult::Ok) return result;
    const std::wstring pattern = physicalPath(wide(rootPath_), normalized) + L"\\*";
    WIN32_FIND_DATAW data;
    HANDLE find = FindFirstFileW(pattern.c_str(), &data);
    if (find == INVALID_HANDLE_VALUE) return windowsError(GetLastError());
    std::vector<VfsEntry> listed;
    do {
        if (wcscmp(data.cFileName, L".") == 0 || wcscmp(data.cFileName, L"..") == 0) continue;
        if (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
            FindClose(find); return VfsResult::InvalidPath;
        }
        int count = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, data.cFileName, -1,
                                        NULL, 0, NULL, NULL);
        if (count <= 1) { FindClose(find); return VfsResult::InvalidPath; }
        std::string name(static_cast<size_t>(count), '\0');
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, data.cFileName, -1,
                            &name[0], count, NULL, NULL);
        name.resize(static_cast<size_t>(count - 1));
        VfsEntry entry; entry.name = name;
        result = windowsMetadata(physicalPath(wide(rootPath_), normalized + "/" + name),
                                 entry.metadata);
        if (result != VfsResult::Ok) { FindClose(find); return result; }
        listed.push_back(entry);
    } while (FindNextFileW(find, &data));
    FindClose(find);
    std::sort(listed.begin(), listed.end(), [](const VfsEntry& a, const VfsEntry& b) {
        return a.name < b.name;
    });
    entries.swap(listed); return VfsResult::Ok;
#else
    if (rootFd_ < 0) return VfsResult::IoError;
    int dirfd = rootFd_;
    bool closeDirfd = false;
    if (normalized != "/") {
        int parent = -1; std::string name;
        result = openParent(rootFd_, normalized, parent, name);
        if (result != VfsResult::Ok) return result;
        dirfd = openat(parent, name.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
        const int saved = errno; close(parent);
        if (dirfd < 0) return saved == ELOOP ? VfsResult::InvalidPath : errorResult(saved);
        closeDirfd = true;
    }
    const int scanfd = openat(dirfd, ".", O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (scanfd < 0) {
        if (closeDirfd) close(dirfd);
        return VfsResult::IoError;
    }
    DIR* dir = fdopendir(scanfd);
    if (!dir) {
        close(scanfd);
        if (closeDirfd) close(dirfd);
        return VfsResult::IoError;
    }
    std::vector<VfsEntry> listed;
    struct dirent* item;
    bool readOk = true;
    while (true) {
        errno = 0;
        item = readdir(dir);
        if (!item) { readOk = errno == 0; break; }
        if (!strcmp(item->d_name, ".") || !strcmp(item->d_name, "..")) continue;
        VfsEntry entry; entry.name = item->d_name;
        std::string normalizedEntry;
        result = normalizeVfsPath("/" + entry.name, normalizedEntry);
        if (result != VfsResult::Ok || normalizedEntry != "/" + entry.name) {
            closedir(dir);
            if (closeDirfd) close(dirfd);
            return VfsResult::InvalidPath;
        }
        result = fillMetadataAt(dirfd, entry.name, entry.metadata);
        if (result != VfsResult::Ok) {
            closedir(dir);
            if (closeDirfd) close(dirfd);
            return result;
        }
        listed.push_back(entry);
    }
    closedir(dir);
    if (closeDirfd) close(dirfd);
    if (!readOk) return VfsResult::IoError;
    std::sort(listed.begin(), listed.end(), [](const VfsEntry& a, const VfsEntry& b) {
        return a.name < b.name;
    });
    entries.swap(listed); return VfsResult::Ok;
#endif
}

VfsResult LocalVfs::removeDirectory(const std::string& path) {
    std::string normalized;
    VfsResult result = normalizeVfsPath(path, normalized);
    if (result != VfsResult::Ok) return result;
    if (normalized == "/") return VfsResult::InvalidPath;
    std::lock_guard<std::mutex> lock(mutex_);
#ifdef _WIN32
    if (rootHandle_ == INVALID_HANDLE_VALUE) return VfsResult::IoError;
    result = verifyWindowsPath(wide(rootPath_), normalized, false);
    if (result != VfsResult::Ok) return result;
    const std::wstring dir = physicalPath(wide(rootPath_), normalized);
    VfsMetadata metadata; result = windowsMetadata(dir, metadata);
    if (result != VfsResult::Ok) return result;
    if (metadata.type != VfsFileType::Directory) return VfsResult::NotDirectory;
    return RemoveDirectoryW(dir.c_str()) ? VfsResult::Ok : windowsError(GetLastError());
#else
    if (rootFd_ < 0) return VfsResult::IoError;
    int parent = -1; std::string name;
    result = openParent(rootFd_, normalized, parent, name);
    if (result != VfsResult::Ok) return result;
    struct stat state;
    result = inspectAt(parent, name, state);
    if (result == VfsResult::Ok && !S_ISDIR(state.st_mode)) result = VfsResult::NotDirectory;
    else if (result == VfsResult::Ok && unlinkat(parent, name.c_str(), AT_REMOVEDIR) != 0)
        result = errorResult(errno);
    close(parent); return result;
#endif
}

VfsResult LocalVfs::stat(const std::string& path, VfsMetadata& metadata) const {
    std::string normalized;
    VfsResult result = normalizeVfsPath(path, normalized);
    if (result != VfsResult::Ok) return result;
    std::lock_guard<std::mutex> lock(mutex_);
#ifdef _WIN32
    if (rootHandle_ == INVALID_HANDLE_VALUE) return VfsResult::IoError;
    result = verifyWindowsPath(wide(rootPath_), normalized, false);
    if (result != VfsResult::Ok) return result;
    if (normalized == "/") return windowsMetadata(wide(rootPath_), metadata);
    return windowsMetadata(physicalPath(wide(rootPath_), normalized), metadata);
#else
    if (rootFd_ < 0) return VfsResult::IoError;
    if (normalized == "/") return rootMetadata(rootFd_, metadata);
    int parent = -1; std::string name;
    result = openParent(rootFd_, normalized, parent, name);
    if (result != VfsResult::Ok) return result;
    result = fillMetadataAt(parent, name, metadata);
    close(parent); return result;
#endif
}

}  // namespace zmc
