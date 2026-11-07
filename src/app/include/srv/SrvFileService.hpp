#ifndef ZMC_SRV_FILE_SERVICE_HPP_
#define ZMC_SRV_FILE_SERVICE_HPP_

#include "SrvBase.hpp"
// Model signature: GPT-6.
namespace zmc {
namespace srv {
class SrvFileService : public SrvBase {
   public:
    // List valid files in the existing order.
    int listFiles(std::vector<SrvFileEntry>& out);
    // Create without overwriting.
    int createFile(const std::string& name, const std::vector<char>& data,
                   std::string* hash = nullptr);
    // Replace an existing file.
    int replaceFile(const std::string& name, const std::vector<char>& data,
                    std::string* hash = nullptr);
    // GPT-6 Codex, 2026-10-23. Create or replace a regular file.
    int writeFile(const std::string& name, const std::vector<char>& data,
                  std::string* hash = nullptr);
    // Read bytes with the caller transfer limit; zero means unlimited.
    int readFile(const std::string& name, std::vector<char>& data, uint64_t max_bytes,
                 std::string* hash = nullptr);
    // Remove a validated regular file.
    int removeFile(const std::string& name);

   protected:
    // Initialize shared file storage before accepting protocol requests.
    // GPT-6 (Codex), 2026-10-29.
    int initializeFiles();

   private:
    friend class SrvQcmd;
    // Construct the file layer.
    SrvFileService();
    // Release the file layer.
    ~SrvFileService();
};
}  // namespace srv
}  // namespace zmc
#endif
