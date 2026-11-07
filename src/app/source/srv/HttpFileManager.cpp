#include "HttpFileManager.hpp"

#include <stdio.h>

#include <algorithm>
#include <cerrno>
#include <climits>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <sys/stat.h>
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

const char kTempPrefix[] = ".http-upload-";
const char kTempSuffix[] = ".tmp";
const size_t kMaxFileNameBytes = 128U;

enum FileKind {
    FILE_KIND_MISSING = 0,
    FILE_KIND_REGULAR,
    FILE_KIND_OTHER,
    FILE_KIND_ERROR,
};

bool is_internal_temp_name(const std::string &name) {
    const size_t prefix_len = sizeof(kTempPrefix) - 1U;
    const size_t suffix_len = sizeof(kTempSuffix) - 1U;
    return name.size() > prefix_len + suffix_len &&
           name.compare(0, prefix_len, kTempPrefix) == 0 &&
           name.compare(name.size() - suffix_len, suffix_len, kTempSuffix) == 0;
}

#ifdef _WIN32

uint64_t file_time_to_unix_seconds(const FILETIME &file_time) {
    // FILETIME uses 100-nanosecond intervals since 1601-01-01 UTC.
    ULARGE_INTEGER value;
    value.LowPart = file_time.dwLowDateTime;
    value.HighPart = file_time.dwHighDateTime;
    const uint64_t windows_epoch_ticks = 11644473600ULL * 10000000ULL;
    return value.QuadPart >= windows_epoch_ticks
               ? (value.QuadPart - windows_epoch_ticks) / 10000000ULL
               : 0;
}

FileKind query_file_kind(const std::string &path, uint64_t *size,
                         uint64_t *modified_time = NULL) {
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &data)) {
        const DWORD error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
            return FILE_KIND_MISSING;
        }
        return FILE_KIND_ERROR;
    }
    if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ||
        (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
        return FILE_KIND_OTHER;
    }
    if (size) {
        *size = (static_cast<uint64_t>(data.nFileSizeHigh) << 32U) |
                static_cast<uint64_t>(data.nFileSizeLow);
    }
    if (modified_time) {
        *modified_time = file_time_to_unix_seconds(data.ftLastWriteTime);
    }
    return FILE_KIND_REGULAR;
}

int prepare_root(const std::string &root) {
    const DWORD attributes = GetFileAttributesA(root.c_str());
    if (attributes != INVALID_FILE_ATTRIBUTES) {
        return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0 &&
                       (attributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0
                   ? 0
                   : -1;
    }
    const DWORD query_error = GetLastError();
    if (query_error != ERROR_FILE_NOT_FOUND && query_error != ERROR_PATH_NOT_FOUND) {
        return -1;
    }
    if (!CreateDirectoryA(root.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        return -1;
    }
    const DWORD created_attributes = GetFileAttributesA(root.c_str());
    return created_attributes != INVALID_FILE_ATTRIBUTES &&
                   (created_attributes & FILE_ATTRIBUTE_DIRECTORY) != 0 &&
                   (created_attributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0
               ? 0
               : -1;
}

int open_temp_file(const std::string &path) {
    int fd = -1;
    const errno_t error = _sopen_s(&fd, path.c_str(), _O_BINARY | _O_CREAT | _O_EXCL | _O_WRONLY,
                                   _SH_DENYRW, _S_IREAD | _S_IWRITE);
    return error == 0 ? fd : -1;
}

bool write_file_chunk(int fd, const char *data, size_t length) {
    size_t written = 0;
    while (written < length) {
        const size_t remaining = length - written;
        const unsigned int chunk =
            static_cast<unsigned int>((std::min)(remaining, static_cast<size_t>(INT_MAX)));
        const int ret = _write(fd, data + written, chunk);
        if (ret <= 0) {
            return false;
        }
        written += static_cast<size_t>(ret);
    }
    return true;
}

bool close_file(int fd) { return _close(fd) == 0; }

bool remove_file(const std::string &path) { return DeleteFileA(path.c_str()) != 0; }

int publish_created_file(const std::string &temp_path, const std::string &target_path) {
    if (MoveFileExA(temp_path.c_str(), target_path.c_str(), MOVEFILE_WRITE_THROUGH)) {
        return 0;
    }
    const DWORD error = GetLastError();
    return error == ERROR_FILE_EXISTS || error == ERROR_ALREADY_EXISTS ? 1 : -1;
}

bool publish_replaced_file(const std::string &temp_path, const std::string &target_path) {
    return MoveFileExA(temp_path.c_str(), target_path.c_str(),
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
}

#else

FileKind query_file_kind(const std::string &path, uint64_t *size,
                         uint64_t *modified_time = NULL) {
    struct stat state;
    if (lstat(path.c_str(), &state) != 0) {
        return errno == ENOENT || errno == ENOTDIR ? FILE_KIND_MISSING : FILE_KIND_ERROR;
    }
    if (!S_ISREG(state.st_mode)) {
        return FILE_KIND_OTHER;
    }
    if (size) {
        *size = static_cast<uint64_t>(state.st_size);
    }
    if (modified_time) {
        *modified_time = state.st_mtime > 0 ? static_cast<uint64_t>(state.st_mtime) : 0;
    }
    return FILE_KIND_REGULAR;
}

int prepare_root(const std::string &root) {
    struct stat state;
    if (lstat(root.c_str(), &state) == 0) {
        return S_ISDIR(state.st_mode) ? 0 : -1;
    }
    if (errno != ENOENT || mkdir(root.c_str(), 0700) != 0) {
        return -1;
    }
    return lstat(root.c_str(), &state) == 0 && S_ISDIR(state.st_mode) ? 0 : -1;
}

int open_temp_file(const std::string &path) {
    return open(path.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0600);
}

bool write_file_chunk(int fd, const char *data, size_t length) {
    size_t written = 0;
    while (written < length) {
        const ssize_t ret = write(fd, data + written, length - written);
        if (ret <= 0) {
            return false;
        }
        written += static_cast<size_t>(ret);
    }
    return true;
}

bool close_file(int fd) { return close(fd) == 0; }

bool remove_file(const std::string &path) { return unlink(path.c_str()) == 0; }

int publish_created_file(const std::string &temp_path, const std::string &target_path) {
    if (link(temp_path.c_str(), target_path.c_str()) != 0) {
        return errno == EEXIST ? 1 : -1;
    }
    // The target is complete once link succeeds; stale hidden files are cleaned
    // at startup.
    (void)unlink(temp_path.c_str());
    return 0;
}

bool publish_replaced_file(const std::string &temp_path, const std::string &target_path) {
    return rename(temp_path.c_str(), target_path.c_str()) == 0;
}

#endif

HttpFileManager::Result result_from_kind(FileKind kind) {
    if (kind == FILE_KIND_MISSING) {
        return HttpFileManager::RESULT_NOT_FOUND;
    }
    if (kind == FILE_KIND_OTHER) {
        return HttpFileManager::RESULT_NOT_REGULAR;
    }
    if (kind == FILE_KIND_ERROR) {
        return HttpFileManager::RESULT_IO_ERROR;
    }
    return HttpFileManager::RESULT_OK;
}

bool entry_less(const HttpFileManager::Entry &left, const HttpFileManager::Entry &right) {
    return left.name < right.name;
}

}  // namespace

HttpFileManager::Writer::Writer() : fd_(-1), size_(0) { fnv1a_128_init(&hash_); }

HttpFileManager::Writer::~Writer() { discard(); }

HttpFileManager::Result HttpFileManager::Writer::write(const char *data, size_t length) {
    if (fd_ < 0 || !write_file_chunk(fd_, data, length)) {
        return RESULT_IO_ERROR;
    }
    fnv1a_128_update(&hash_, data, length);
    size_ += static_cast<uint64_t>(length);
    return RESULT_OK;
}

HttpFileManager::Result HttpFileManager::Writer::close() {
    if (fd_ < 0) {
        return RESULT_IO_ERROR;
    }
    const bool closed = close_file(fd_);
    fd_ = -1;
    return closed ? RESULT_OK : RESULT_IO_ERROR;
}

void HttpFileManager::Writer::discard() {
    if (fd_ >= 0) {
        (void)close_file(fd_);
        fd_ = -1;
    }
    if (!temp_path_.empty()) {
        (void)remove_file(temp_path_);
        temp_path_.clear();
    }
}

uint64_t HttpFileManager::Writer::size() const { return size_; }

const std::string &HttpFileManager::Writer::name() const { return name_; }

std::string HttpFileManager::Writer::hash() const {
    char digest[33];
    fnv1a_128_digest(&hash_, digest);
    return std::string(digest, 32U);
}

HttpFileManager::HttpFileManager(const std::string &root) : root_(root) {}

HttpFileManager::Result HttpFileManager::initialize() {
    if (prepare_root(root_) != 0) {
        return RESULT_IO_ERROR;
    }
    return cleanupStaleTempFiles();
}

bool HttpFileManager::isValidName(const std::string &name) const {
    if (name.empty() || name.size() > kMaxFileNameBytes) {
        return false;
    }
    const unsigned char first = static_cast<unsigned char>(name[0]);
    if (!((first >= 'A' && first <= 'Z') || (first >= 'a' && first <= 'z') ||
          (first >= '0' && first <= '9'))) {
        return false;
    }
    for (size_t i = 1; i < name.size(); ++i) {
        const unsigned char ch = static_cast<unsigned char>(name[i]);
        if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
              ch == '.' || ch == '_' || ch == '-')) {
            return false;
        }
    }
    return true;
}

std::string HttpFileManager::filePath(const std::string &name) const {
#ifdef _WIN32
    return root_ + "\\" + name;
#else
    return root_ + "/" + name;
#endif
}

std::string HttpFileManager::tempPath(const std::string &name) const {
    return filePath(std::string(kTempPrefix) + name + kTempSuffix);
}

HttpFileManager::Result HttpFileManager::beginWrite(const std::string &name,
                                                    Writer &writer) const {
    if (!isValidName(name)) {
        return RESULT_INVALID_NAME;
    }
    writer.discard();
    writer.name_ = name;
    writer.size_ = 0;
    fnv1a_128_init(&writer.hash_);
    writer.temp_path_ = tempPath(name);
    writer.fd_ = open_temp_file(writer.temp_path_);
    if (writer.fd_ < 0) {
        writer.temp_path_.clear();
        return RESULT_IO_ERROR;
    }
    return RESULT_OK;
}

HttpFileManager::Result HttpFileManager::beginCreate(const std::string &name,
                                                     Writer &writer) const {
    if (!isValidName(name)) {
        return RESULT_INVALID_NAME;
    }
    const FileKind kind = query_file_kind(filePath(name), NULL);
    if (kind == FILE_KIND_REGULAR) {
        return RESULT_ALREADY_EXISTS;
    }
    const Result kind_result = result_from_kind(kind);
    if (kind_result != RESULT_NOT_FOUND) {
        return kind_result;
    }
    return beginWrite(name, writer);
}

HttpFileManager::Result HttpFileManager::beginReplace(const std::string &name,
                                                      Writer &writer) const {
    return beginWrite(name, writer);
}

HttpFileManager::Result HttpFileManager::commitCreate(Writer &writer) const {
    if (writer.fd_ >= 0 || writer.temp_path_.empty()) {
        return RESULT_IO_ERROR;
    }
    const int publish_result = publish_created_file(writer.temp_path_, filePath(writer.name_));
    if (publish_result == 1) {
        return RESULT_ALREADY_EXISTS;
    }
    if (publish_result != 0) {
        return RESULT_IO_ERROR;
    }
    writer.temp_path_.clear();
    return RESULT_OK;
}

HttpFileManager::Result HttpFileManager::commitReplace(Writer &writer) const {
    if (writer.fd_ >= 0 || writer.temp_path_.empty()) {
        return RESULT_IO_ERROR;
    }
    const Result target_result = result_from_kind(query_file_kind(filePath(writer.name_), NULL));
    if (target_result != RESULT_OK) {
        return target_result;
    }
    if (!publish_replaced_file(writer.temp_path_, filePath(writer.name_))) {
        return RESULT_IO_ERROR;
    }
    writer.temp_path_.clear();
    return RESULT_OK;
}

HttpFileManager::Result HttpFileManager::openRead(const std::string &name,
                                                  std::shared_ptr<std::ifstream> &stream,
                                                  uint64_t &size) const {
    stream.reset();
    size = 0;
    if (!isValidName(name)) {
        return RESULT_INVALID_NAME;
    }
    const Result target_result = result_from_kind(query_file_kind(filePath(name), &size));
    if (target_result != RESULT_OK) {
        return target_result;
    }
    stream.reset(new std::ifstream(filePath(name).c_str(), std::ios::in | std::ios::binary));
    if (!stream->is_open()) {
        stream.reset();
        return RESULT_IO_ERROR;
    }
    return RESULT_OK;
}

HttpFileManager::Result HttpFileManager::hashFile(const std::string &name,
                                                  std::string &hash) const {
    hash.clear();
    std::shared_ptr<std::ifstream> input;
    uint64_t size = 0;
    const Result open_result = openRead(name, input, size);
    if (open_result != RESULT_OK) {
        return open_result;
    }

    U128 state;
    fnv1a_128_init(&state);
    char buffer[64U * 1024U];
    uint64_t bytes_read = 0;
    while (true) {
        input->read(buffer, static_cast<std::streamsize>(sizeof(buffer)));
        const std::streamsize count = input->gcount();
        if (count > 0) {
            fnv1a_128_update(&state, buffer, static_cast<size_t>(count));
            bytes_read += static_cast<uint64_t>(count);
        }
        if (input->bad()) {
            return RESULT_IO_ERROR;
        }
        if (input->eof()) {
            break;
        }
        if (input->fail() || count == 0) {
            return RESULT_IO_ERROR;
        }
    }
    if (bytes_read != size) {
        return RESULT_IO_ERROR;
    }

    char digest[33];
    fnv1a_128_digest(&state, digest);
    hash.assign(digest, 32U);
    return RESULT_OK;
}

std::string HttpFileManager::hashBytes(const void *data, size_t length) {
    U128 state;
    fnv1a_128_init(&state);
    fnv1a_128_update(&state, data, length);
    char digest[33];
    fnv1a_128_digest(&state, digest);
    return std::string(digest, 32U);
}

HttpFileManager::Result HttpFileManager::remove(const std::string &name) const {
    if (!isValidName(name)) {
        return RESULT_INVALID_NAME;
    }
    const std::string path = filePath(name);
    const Result target_result = result_from_kind(query_file_kind(path, NULL));
    if (target_result != RESULT_OK) {
        return target_result;
    }
    return remove_file(path) ? RESULT_OK : RESULT_IO_ERROR;
}

HttpFileManager::Result HttpFileManager::list(std::vector<Entry> &entries) const {
    entries.clear();
#ifdef _WIN32
    WIN32_FIND_DATAA data;
    HANDLE find = FindFirstFileA((root_ + "\\*").c_str(), &data);
    if (find == INVALID_HANDLE_VALUE) {
        return GetLastError() == ERROR_FILE_NOT_FOUND ? RESULT_OK : RESULT_IO_ERROR;
    }
    bool ok = true;
    do {
        const std::string name = data.cFileName;
        if (!isValidName(name) || is_internal_temp_name(name) ||
            (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ||
            (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
            continue;
        }
        const uint64_t size = (static_cast<uint64_t>(data.nFileSizeHigh) << 32U) |
                              static_cast<uint64_t>(data.nFileSizeLow);
        entries.push_back(
            Entry{name, size, file_time_to_unix_seconds(data.ftLastWriteTime), std::string()});
    } while (FindNextFileA(find, &data));
    if (GetLastError() != ERROR_NO_MORE_FILES || !FindClose(find)) {
        ok = false;
    }
#else
    DIR *directory = opendir(root_.c_str());
    if (!directory) {
        return RESULT_IO_ERROR;
    }
    bool ok = true;
    errno = 0;
    for (struct dirent *item = readdir(directory); item != NULL; item = readdir(directory)) {
        const std::string name = item->d_name;
        if (!isValidName(name) || is_internal_temp_name(name)) {
            errno = 0;
            continue;
        }
        uint64_t size = 0;
        uint64_t modified_time = 0;
        if (query_file_kind(filePath(name), &size, &modified_time) == FILE_KIND_REGULAR) {
            entries.push_back(Entry{name, size, modified_time, std::string()});
        }
        errno = 0;
    }
    if (errno != 0 || closedir(directory) != 0) {
        ok = false;
    }
#endif
    if (!ok) {
        entries.clear();
        return RESULT_IO_ERROR;
    }
    std::sort(entries.begin(), entries.end(), entry_less);
    for (Entry &entry : entries) {
        const Result hash_result = hashFile(entry.name, entry.hash);
        if (hash_result != RESULT_OK) {
            entries.clear();
            return hash_result;
        }
    }
    return RESULT_OK;
}

HttpFileManager::Result HttpFileManager::cleanupStaleTempFiles() const {
#ifdef _WIN32
    WIN32_FIND_DATAA data;
    HANDLE find = FindFirstFileA((root_ + "\\*").c_str(), &data);
    if (find == INVALID_HANDLE_VALUE) {
        return GetLastError() == ERROR_FILE_NOT_FOUND ? RESULT_OK : RESULT_IO_ERROR;
    }
    bool ok = true;
    do {
        const std::string name = data.cFileName;
        if (is_internal_temp_name(name) &&
            (data.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ==
                0 &&
            !remove_file(filePath(name))) {
            ok = false;
        }
    } while (FindNextFileA(find, &data));
    if (GetLastError() != ERROR_NO_MORE_FILES || !FindClose(find)) {
        ok = false;
    }
#else
    DIR *directory = opendir(root_.c_str());
    if (!directory) {
        return RESULT_IO_ERROR;
    }
    bool ok = true;
    errno = 0;
    for (struct dirent *item = readdir(directory); item != NULL; item = readdir(directory)) {
        const std::string name = item->d_name;
        if (is_internal_temp_name(name) &&
            query_file_kind(filePath(name), NULL) == FILE_KIND_REGULAR &&
            !remove_file(filePath(name))) {
            ok = false;
        }
        errno = 0;
    }
    if (errno != 0 || closedir(directory) != 0) {
        ok = false;
    }
#endif
    return ok ? RESULT_OK : RESULT_IO_ERROR;
}

}  // namespace zmc
