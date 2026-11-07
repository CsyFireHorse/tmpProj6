#ifndef _ZMC_HTTP_FILE_MANAGER_HPP_
#define _ZMC_HTTP_FILE_MANAGER_HPP_

#include <stddef.h>
#include <stdint.h>

#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "common/utils/fnv1a128.h"

namespace zmc {

// Owns the isolated server-side file namespace and platform file operations.
class HttpFileManager {
   public:
    enum Result {
        RESULT_OK = 0,
        RESULT_INVALID_NAME,
        RESULT_NOT_FOUND,
        RESULT_ALREADY_EXISTS,
        RESULT_NOT_REGULAR,
        RESULT_IO_ERROR,
    };

    // Public metadata returned by file listing.
    struct Entry {
        std::string name;
        uint64_t size;
        uint64_t time;  // Last-modified Unix timestamp in seconds.
        std::string hash;  // Lowercase FNV-1a 128-bit hash of the file contents.
    };

    // Request-local temporary writer; unpublished files are discarded on
    // destruction.
    class Writer {
       public:
        Writer();
        ~Writer();

        // Writes one upload chunk to the temporary file.
        Result write(const char *data, size_t length);

        // Closes the temporary file before it is published.
        Result close();

        // Closes and removes an unpublished temporary file.
        void discard();

        // Returns the number of bytes written successfully.
        uint64_t size() const;

        // Returns the validated remote target name.
        const std::string &name() const;

        // Returns the hash of the chunks successfully written to the temporary file.
        std::string hash() const;

       private:
        friend class HttpFileManager;
        Writer(const Writer &) = delete;
        Writer &operator=(const Writer &) = delete;

        int fd_;
        uint64_t size_;
        std::string name_;
        std::string temp_path_;
        U128 hash_;
    };

    explicit HttpFileManager(const std::string &root);

    // Creates or validates the root and removes stale internal upload files.
    Result initialize();

    // Checks the portable single-segment remote name grammar.
    bool isValidName(const std::string &name) const;

    // Lists valid regular files in ascending name order.
    Result list(std::vector<Entry> &entries) const;

    // Hashes existing response bytes without copying them.
    static std::string hashBytes(const void *data, size_t length);

    // Opens a temporary writer after verifying that the target does not exist.
    Result beginCreate(const std::string &name, Writer &writer) const;

    // Opens a temporary writer for a later replacement operation.
    Result beginReplace(const std::string &name, Writer &writer) const;

    // Atomically publishes a complete temporary file without overwriting.
    Result commitCreate(Writer &writer) const;

    // Atomically replaces an existing regular target with a complete temporary
    // file.
    Result commitReplace(Writer &writer) const;

    // Opens a validated regular file for streaming download.
    Result openRead(const std::string &name, std::shared_ptr<std::ifstream> &stream,
                    uint64_t &size) const;

    // Removes a validated regular file.
    Result remove(const std::string &name) const;

   private:
    std::string filePath(const std::string &name) const;
    std::string tempPath(const std::string &name) const;
    Result beginWrite(const std::string &name, Writer &writer) const;
    Result hashFile(const std::string &name, std::string &hash) const;
    Result cleanupStaleTempFiles() const;

    std::string root_;
};

}  // namespace zmc

#endif
