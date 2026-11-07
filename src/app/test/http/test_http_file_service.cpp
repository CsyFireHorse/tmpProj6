// Date: 2026-11-05; Drafted by: GPT-6 (Codex).
#include <gtest/gtest.h>

#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <dirent.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "HttpTestRuntime.hpp"
#include "common/thirdlib/nlohmann/json.hpp"
#include "common/thirdlib/yhirose/httplib.h"

namespace zmc {
namespace {

using json = nlohmann::json;

const char kTestFileRoot[] = "server_files";

#ifdef _WIN32

bool get_current_directory(std::string& path) {
    char buffer[MAX_PATH];
    const DWORD length = GetCurrentDirectoryA(MAX_PATH, buffer);
    if (length == 0 || length >= MAX_PATH) {
        return false;
    }
    path.assign(buffer, length);
    return true;
}

bool make_test_directory(std::string& path) {
    char temp_path[MAX_PATH];
    char temp_file[MAX_PATH];
    const DWORD length = GetTempPathA(MAX_PATH, temp_path);
    if (length == 0 || length >= MAX_PATH || !GetTempFileNameA(temp_path, "zhf", 0, temp_file)) {
        return false;
    }
    if (!DeleteFileA(temp_file) || !CreateDirectoryA(temp_file, NULL)) {
        return false;
    }
    path = temp_file;
    return true;
}

bool change_directory(const std::string& path) { return SetCurrentDirectoryA(path.c_str()) != 0; }

void remove_tree(const std::string& path) {
    WIN32_FIND_DATAA data;
    HANDLE find = FindFirstFileA((path + "\\*").c_str(), &data);
    if (find != INVALID_HANDLE_VALUE) {
        do {
            const std::string name = data.cFileName;
            if (name == "." || name == "..") {
                continue;
            }
            const std::string child = path + "\\" + name;
            if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 &&
                (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == 0) {
                remove_tree(child);
            } else {
                (void)DeleteFileA(child.c_str());
            }
        } while (FindNextFileA(find, &data));
        (void)FindClose(find);
    }
    (void)RemoveDirectoryA(path.c_str());
}

#else

bool get_current_directory(std::string& path) {
    char buffer[PATH_MAX];
    if (!getcwd(buffer, sizeof(buffer))) {
        return false;
    }
    path = buffer;
    return true;
}

bool make_test_directory(std::string& path) {
    char pattern[] = "/tmp/zmc_http_files_XXXXXX";
    char* created = mkdtemp(pattern);
    if (!created) {
        return false;
    }
    path = created;
    return true;
}

bool change_directory(const std::string& path) { return chdir(path.c_str()) == 0; }

void remove_tree(const std::string& path) {
    DIR* directory = opendir(path.c_str());
    if (directory) {
        for (struct dirent* item = readdir(directory); item != NULL; item = readdir(directory)) {
            const std::string name = item->d_name;
            if (name == "." || name == "..") {
                continue;
            }
            const std::string child = path + "/" + name;
            struct stat state;
            if (lstat(child.c_str(), &state) == 0 && S_ISDIR(state.st_mode)) {
                remove_tree(child);
            } else {
                (void)unlink(child.c_str());
            }
        }
        (void)closedir(directory);
    }
    (void)rmdir(path.c_str());
}

#endif

httplib::MultipartFormDataItems make_file_items(const std::string& field,
                                                const std::string& file_name,
                                                const std::string& content) {
    httplib::MultipartFormDataItems items;
    items.push_back({field, content, file_name, "application/octet-stream"});
    return items;
}

class HttpFileServiceTest : public ::testing::Test {
   protected:
    HttpFileServiceTest() : port_(28970) {}

    void SetUp() override {
        ASSERT_TRUE(get_current_directory(previous_directory_));
        ASSERT_TRUE(make_test_directory(test_directory_));
        ASSERT_TRUE(change_directory(test_directory_));
        // Start the complete core; file operations need no bus configuration.
        ASSERT_EQ(0, runtime_.start({"--http", "--port=" + std::to_string(port_)}));
        ASSERT_TRUE(runtime_.openSessionAndWaitHttp(port_, session_, 100));
    }

    void TearDown() override {
        if (!session_.empty()) {
            auto client = make_client();
            const auto closed = client.Post("/zmot/api/close", "{}", "application/json");
            EXPECT_TRUE(closed);
            if (closed) EXPECT_EQ(0, json::parse(closed->body).at("code").get<int>());
        }
        EXPECT_EQ(0, runtime_.finish());
        if (!previous_directory_.empty()) {
            (void)change_directory(previous_directory_);
        }
        if (!test_directory_.empty()) {
            remove_tree(test_directory_);
        }
    }

    httplib::Client make_client() const {
        httplib::Client client("127.0.0.1", port_);
        client.set_connection_timeout(5, 0);
        client.set_read_timeout(30, 0);
        client.set_write_timeout(30, 0);
        client.set_default_headers({{"X-Zmot-Session", session_}});
        return client;
    }

    std::string previous_directory_;
    std::string test_directory_;
    int port_;
    HttpTestRuntime runtime_;
    std::string session_;
};

// Files are independent of credential lifetime and placement. Model signature: GPT-6.
TEST_F(HttpFileServiceTest, FilesRemainAvailableWithoutSessions) {
    httplib::Client client = make_client();
    auto result = client.Post("/zmot/api/close", "{}", "application/json");
    ASSERT_TRUE(result);
    ASSERT_EQ(0, json::parse(result->body).at("code").get<int>());
    session_.clear();
    client.set_default_headers({});

    const std::string binary("a\0b", 3U);
    result = client.Post("/zmot/api/files", make_file_items("file", "alpha.bin", binary));
    ASSERT_TRUE(result);
    ASSERT_EQ(200, result->status);
    const auto uploaded = json::parse(result->body);
    ASSERT_EQ(0, uploaded.at("code").get<int>());
    result = client.Get("/zmot/api/files");
    ASSERT_TRUE(result);
    const auto files = json::parse(result->body).at("files");
    ASSERT_EQ(1U, files.size());
    EXPECT_EQ("alpha.bin", files[0].at("name"));
    EXPECT_EQ(binary.size(), files[0].at("size").get<size_t>());
    result = client.Get("/zmot/api/files/alpha.bin");
    ASSERT_TRUE(result);
    EXPECT_EQ(200, result->status);
    EXPECT_EQ(binary, result->body);
    EXPECT_EQ(uploaded.at("hash").get<std::string>(),
              result->get_header_value("X-Content-Hash"));

    // GPT-6 Codex: replace through HTTP before reading the updated file.
    result = client.Put("/zmot/api/files/alpha.bin",
                        make_file_items("file", "alpha.bin", "updated"));
    ASSERT_TRUE(result);
    ASSERT_EQ(0, json::parse(result->body).at("code").get<int>());
    result = client.Get("/zmot/api/files/alpha.bin");
    ASSERT_TRUE(result);
    EXPECT_EQ("updated", result->body);
    result = client.Delete("/zmot/api/files/alpha.bin");
    ASSERT_TRUE(result);
    ASSERT_EQ(0, json::parse(result->body).at("code").get<int>());
    result = client.Get("/zmot/api/files");
    ASSERT_TRUE(result);
    EXPECT_TRUE(json::parse(result->body).at("files").empty());
}

}  // namespace
}  // namespace zmc
