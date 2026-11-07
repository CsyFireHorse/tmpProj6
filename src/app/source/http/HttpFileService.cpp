#include "HttpFileService.hpp"
#include "srv/MotSrv.hpp"
#include "HttpJsonParseDetail.hpp"
#include "fbt_errorcode_def.h"
namespace zmc {
namespace {
using namespace http_json_parse_detail;
using json = nlohmann::json;
const uint64_t kMaxUploadBytes = 16U * 1024U * 1024U;

// Model signature: GPT-6. Preserve the original REST error envelopes.
void fileError(httplib::Response& res, int code) {
    const char* message = code == ZMC_ERR_RPC_FILE_INVALID_NAME     ? "invalid remote file name"
                          : code == ZMC_ERR_RPC_FILE_NOT_FOUND      ? "file not found"
                          : code == ZMC_ERR_RPC_FILE_ALREADY_EXISTS ? "file already exists"
                          : code == ZMC_ERR_RPC_FILE_TOO_LARGE      ? "file exceeds 16 MiB limit"
                                                                    : "file operation failed";
    replyJsonError(res, code, message);
}

// Decode exactly one file part; legacy session fields are ignored, never authenticated.
bool receiveFile(const httplib::Request& req, const httplib::ContentReader& reader, bool replace,
                 std::string& name, std::vector<char>& bytes, httplib::Response& res) {
    bool file_part = false, invalid = false, too_large = false;
    size_t parts = 0;
    bool read_ok = false;
    if (req.is_multipart_form_data()) {
        read_ok = reader(
            [&](const httplib::MultipartFormData& part) {
                file_part = false;
                if (part.name == "session") return true;
                ++parts;
                if (parts != 1 || part.name != "file") {
                    invalid = true;
                    return true;
                }
                if (!replace) name = part.filename;
                file_part = true;
                return true;
            },
            [&](const char* data, size_t length) {
                if (!file_part || too_large) return true;
                if (length > kMaxUploadBytes - bytes.size()) {
                    too_large = true;
                    return true;
                }
                bytes.insert(bytes.end(), data, data + length);
                return true;
            });
    } else {
        // Consume the old invalid-request path without retaining a session/body copy.
        uint64_t count = 0;
        read_ok = reader([&](const char*, size_t length) {
            if (length > kMaxUploadBytes - count) {
                too_large = true;
                return false;
            }
            count += length;
            return true;
        });
        invalid = true;
    }
    if (too_large) {
        res.set_header("Connection", "close");
        fileError(res, ZMC_ERR_RPC_FILE_TOO_LARGE);
        return false;
    }
    if (!read_ok || invalid || parts != 1) {
        if (!read_ok) res.set_header("Connection", "close");
        replyJsonError(res, ZMC_ERR_RPC_FILE_INVALID_REQUEST,
                       req.is_multipart_form_data()
                           ? "request must contain exactly one multipart file part"
                           : "multipart/form-data required");
        return false;
    }
    return true;
}
}  // namespace

int MotHttpFile::bindFiles() {
    httplib::Server& server = *pSvr;
    srv::MotSrv& service = *service_;
    server.Get("/zmot/api/files", [&service](const httplib::Request&, httplib::Response& res) {
        std::vector<srv::SrvFileEntry> values;
        const int ret = service.listFiles(values);
        if (ret != 0) {
            fileError(res, ret);
            return;
        }
        json files = json::array();
        for (const auto& value : values)
            files.push_back({{"name", value.name},
                             {"size", value.size},
                             {"time", value.time},
                             {"hash", value.hash}});
        replyJsonOk(res, {{"files", files}});
    });
    server.Post("/zmot/api/files", [&service](const httplib::Request& req, httplib::Response& res,
                                              const httplib::ContentReader& reader) {
        std::string name;
        std::string hash;
        std::vector<char> bytes;
        if (!receiveFile(req, reader, false, name, bytes, res)) return;
        const int ret = service.createFile(name, bytes, &hash);
        if (ret != 0) {
            fileError(res, ret);
            return;
        }
        replyJsonOk(res, {{"name", name}, {"size", bytes.size()}, {"hash", hash}});
    });
    const std::string pattern = "/zmot/api/files/(.*)";
    server.Get(pattern, [&service](const httplib::Request& req, httplib::Response& res) {
        const std::string name = req.matches[1].str();
        std::string hash;
        std::vector<char> bytes;
        const int ret = service.readFile(name, bytes, 0, &hash);
        if (ret != 0) {
            fileError(res, ret);
            return;
        }
        res.set_header("Content-Disposition", "attachment; filename=\"" + name + "\"");
        res.set_header("X-Content-Hash", hash);
        res.set_content(bytes.empty() ? "" : bytes.data(), bytes.size(),
                        "application/octet-stream");
    });
    server.Put(pattern, [&service](const httplib::Request& req, httplib::Response& res,
                                   const httplib::ContentReader& reader) {
        std::string name = req.matches[1].str();
        std::string hash;
        std::vector<char> bytes;
        if (!receiveFile(req, reader, true, name, bytes, res)) return;
        const int ret = service.replaceFile(name, bytes, &hash);
        if (ret != 0) {
            fileError(res, ret);
            return;
        }
        replyJsonOk(res, {{"name", name}, {"size", bytes.size()}, {"hash", hash}});
    });
    server.Delete(pattern, [&service](const httplib::Request& req, httplib::Response& res) {
        const std::string name = req.matches[1].str();
        const int ret = service.removeFile(name);
        if (ret != 0) {
            fileError(res, ret);
            return;
        }
        replyJsonOk(res, {{"name", name}});
    });
    return 0;
}
#if 0  // Disabled injected-service file binding bypass. GPT-6.
int http_server_bind_files(httplib::Server& server, srv::SrvFileService& service) {
    HttpFileService adapter(service);
    return adapter.bind(server);
}
#endif
}  // namespace zmc
