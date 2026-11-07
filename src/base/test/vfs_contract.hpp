#ifndef ZMC_VFS_CONTRACT_HPP
#define ZMC_VFS_CONTRACT_HPP

#include <gtest/gtest.h>

// Shared backend behavior contract. AI modification signature: GPT-6 Codex.

#include <atomic>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

#include "IVfs.hpp"

namespace zmc {
namespace test {

inline void runVfsContract(IVfs& vfs) {
    const char binary[] = {'A', '\0', static_cast<char>(0xff), 'Z'};
    std::string bytes;
    VfsMetadata metadata;
    std::vector<VfsEntry> entries;

    EXPECT_EQ(VfsResult::Ok, vfs.stat("/", metadata));
    EXPECT_EQ(VfsFileType::Directory, metadata.type);
    EXPECT_EQ(0U, metadata.size);
    EXPECT_EQ(VfsResult::Ok, vfs.createDirectory("a"));
    EXPECT_EQ(VfsResult::Ok, vfs.createDirectory("/a/b"));
    EXPECT_EQ(VfsResult::Ok, vfs.writeFile("a/b/empty", NULL, 0, VfsWriteMode::CreateOnly));
    EXPECT_EQ(VfsResult::Ok, vfs.readFile("/a//b/./empty", bytes));
    EXPECT_TRUE(bytes.empty());
    EXPECT_EQ(VfsResult::Ok, vfs.writeFile("a/b/data", binary, sizeof(binary),
                                           VfsWriteMode::CreateOnly));
    EXPECT_EQ(VfsResult::Ok, vfs.readFile("a/b/data", bytes));
    ASSERT_EQ(sizeof(binary), bytes.size());
    EXPECT_EQ(0, std::memcmp(binary, bytes.data(), sizeof(binary)));

    EXPECT_EQ(VfsResult::AlreadyExists,
              vfs.writeFile("a/b/data", "x", 1, VfsWriteMode::CreateOnly));
    EXPECT_EQ(VfsResult::Ok,
              vfs.writeFile("a/b/data", "new", 3, VfsWriteMode::ReplaceOnly));
    EXPECT_EQ(VfsResult::Ok, vfs.readFile("a/b/data", bytes));
    EXPECT_EQ("new", bytes);
    EXPECT_EQ(VfsResult::NotFound,
              vfs.writeFile("a/b/missing", "x", 1, VfsWriteMode::ReplaceOnly));
    EXPECT_EQ(VfsResult::NotFound,
              vfs.writeFile("missing/child", "x", 1, VfsWriteMode::CreateOrReplace));
    EXPECT_EQ(VfsResult::InvalidPath,
              vfs.writeFile("a/../escape", "x", 1, VfsWriteMode::CreateOrReplace));
    EXPECT_EQ(VfsResult::InvalidPath,
              vfs.writeFile("bad\\name", "x", 1, VfsWriteMode::CreateOrReplace));
    const std::string invalidUtf8("\xc0\xaf", 2);
    EXPECT_EQ(VfsResult::InvalidPath,
              vfs.writeFile(invalidUtf8, "x", 1, VfsWriteMode::CreateOrReplace));
    EXPECT_EQ(VfsResult::NotFound, vfs.readFile("a/b/Data", bytes));

    EXPECT_EQ(VfsResult::Ok, vfs.stat("a/b/data", metadata));
    EXPECT_EQ(VfsFileType::File, metadata.type);
    EXPECT_EQ(3U, metadata.size);
    const int64_t modified = metadata.modifiedTime;
    EXPECT_EQ(VfsResult::Ok, vfs.readFile("a/b/data", bytes));
    EXPECT_EQ(VfsResult::Ok, vfs.stat("a/b/data", metadata));
    EXPECT_EQ(modified, metadata.modifiedTime);
    EXPECT_EQ(VfsResult::Ok, vfs.listDirectory("a/b", entries));
    ASSERT_EQ(2U, entries.size());
    EXPECT_EQ("data", entries[0].name);
    EXPECT_EQ("empty", entries[1].name);
    EXPECT_EQ(VfsFileType::File, entries[0].metadata.type);
    EXPECT_EQ(3U, entries[0].metadata.size);
    EXPECT_EQ(VfsResult::NotDirectory, vfs.listDirectory("a/b/data", entries));
    EXPECT_EQ(VfsResult::NotFile, vfs.readFile("a/b", bytes));
    EXPECT_EQ(VfsResult::NotFound, vfs.stat("a/missing", metadata));
    EXPECT_EQ(VfsResult::NotDirectory, vfs.createDirectory("a/b/data/child"));
    EXPECT_EQ(VfsResult::NotDirectory, vfs.createDirectory("a/b/data"));
    EXPECT_EQ(VfsResult::AlreadyExists, vfs.createDirectory("a/b"));
    EXPECT_EQ(VfsResult::DirectoryNotEmpty, vfs.removeDirectory("a/b"));
    EXPECT_EQ(VfsResult::NotDirectory, vfs.removeDirectory("a/b/data"));
    EXPECT_EQ(VfsResult::Ok, vfs.removeFile("a/b/data"));
    EXPECT_EQ(VfsResult::Ok, vfs.removeFile("a/b/empty"));
    EXPECT_EQ(VfsResult::Ok, vfs.removeDirectory("a/b"));
    EXPECT_EQ(VfsResult::Ok, vfs.removeDirectory("a"));
    EXPECT_EQ(VfsResult::InvalidPath, vfs.removeDirectory("/"));
    EXPECT_EQ(VfsResult::AlreadyExists, vfs.createDirectory("/"));

    EXPECT_EQ(VfsResult::Ok, vfs.createDirectory("parallel"));
    EXPECT_EQ(VfsResult::Ok, vfs.writeFile("parallel/file", "first", 5,
                                           VfsWriteMode::CreateOnly));
    const std::string versions[] = {std::string(1024, 'A'), std::string(4096, 'B')};
    std::atomic<int> createWinners(0);
    std::atomic<int> createLosers(0);
    std::atomic<int> writeFailures(0);
    std::atomic<bool> start(false);
    std::vector<std::thread> workers;
    for (int i = 0; i < 4; ++i) {
        workers.push_back(std::thread([&, i]() {
            while (!start.load()) std::this_thread::yield();
            if (i == 0 || i == 1) {
                for (int n = 0; n < 100; ++n) {
                    const std::string& version = versions[(i + n) % 2];
                    if (vfs.writeFile("parallel/file", version.data(), version.size(),
                                      VfsWriteMode::CreateOrReplace) != VfsResult::Ok)
                        writeFailures.fetch_add(1);
                }
            } else {
                for (int n = 0; n < 100; ++n) {
                    std::string snapshot;
                    if (vfs.readFile("parallel/file", snapshot) == VfsResult::Ok &&
                        snapshot != "first" && snapshot != versions[0] && snapshot != versions[1])
                        createLosers.fetch_add(1);
                }
            }
        }));
    }
    start.store(true);
    for (size_t i = 0; i < workers.size(); ++i) workers[i].join();
    EXPECT_EQ(0, createLosers.load());
    EXPECT_EQ(0, writeFailures.load());

    workers.clear();
    start.store(false);
    for (int i = 0; i < 8; ++i) {
        workers.push_back(std::thread([&]() {
            while (!start.load()) std::this_thread::yield();
            VfsResult result = vfs.writeFile("parallel/once", "x", 1, VfsWriteMode::CreateOnly);
            if (result == VfsResult::Ok) createWinners.fetch_add(1);
            else if (result == VfsResult::AlreadyExists) createLosers.fetch_add(1);
        }));
    }
    start.store(true);
    for (size_t i = 0; i < workers.size(); ++i) workers[i].join();
    EXPECT_EQ(1, createWinners.load());
    EXPECT_EQ(7, createLosers.load());
}

}  // namespace test
}  // namespace zmc

#endif  // ZMC_VFS_CONTRACT_HPP
