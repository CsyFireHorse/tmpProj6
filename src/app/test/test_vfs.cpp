#include <gtest/gtest.h>

#include "LocalVfs.hpp"
#include "vfs_contract.hpp"

#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <stdlib.h>
#include <unistd.h>
#endif

#include <fstream>

// Exercises the common contract and root isolation against the local backend. AI modification signature: GPT-6 Codex.

namespace {

std::string makeTempDirectory() {
#ifdef _WIN32
    char tempPath[MAX_PATH];
    DWORD length = GetTempPathA(MAX_PATH, tempPath);
    if (!length || length >= MAX_PATH) return std::string();
    for (unsigned int i = 0; i < 100; ++i) {
        char path[MAX_PATH];
        sprintf(path, "%szmc-vfs-%lu-%u", tempPath,
                static_cast<unsigned long>(GetCurrentProcessId()), i);
        if (CreateDirectoryA(path, NULL)) return path;
    }
    return std::string();
#else
    char path[] = "/tmp/zmc-vfs-XXXXXX";
    char* result = mkdtemp(path);
    return result ? result : "";
#endif
}

void removeTestDirectory(const std::string& root) {
    std::remove((root + "/parallel/file").c_str());
    std::remove((root + "/parallel/once").c_str());
    std::remove((root + "/parallel").c_str());
#ifdef _WIN32
    std::string native = root;
    for (size_t i = 0; i < native.size(); ++i) if (native[i] == '/') native[i] = '\\';
    RemoveDirectoryA(native.c_str());
#else
    rmdir(root.c_str());
#endif
}

TEST(LocalVfsTest, SharedContract) {
    const std::string root = makeTempDirectory();
    ASSERT_FALSE(root.empty());
    {
        zmc::LocalVfs vfs(root);
        ASSERT_EQ(zmc::VfsResult::Ok, vfs.initialize());
        zmc::test::runVfsContract(vfs);
        zmc::VfsMetadata metadata;
        zmc::LocalVfs uninitialized(root);
        EXPECT_EQ(zmc::VfsResult::IoError, uninitialized.stat("/", metadata));
        zmc::LocalVfs missing(root + "/missing");
        EXPECT_EQ(zmc::VfsResult::NotFound, missing.initialize());
    }
    removeTestDirectory(root);
}

#ifndef _WIN32
TEST(LocalVfsTest, RejectsSymlinksAndKeepsAccessInsideRoot) {
    const std::string root = makeTempDirectory();
    ASSERT_FALSE(root.empty());
    const std::string outside = makeTempDirectory();
    ASSERT_FALSE(outside.empty());
    const std::string sentinel = outside + "/sentinel";
    {
        std::ofstream file(sentinel.c_str(), std::ios::binary);
        file << "protected";
    }
    ASSERT_EQ(0, symlink(outside.c_str(), (root + "/parent-link").c_str()));
    ASSERT_EQ(0, symlink(sentinel.c_str(), (root + "/file-link").c_str()));
    ASSERT_EQ(0, link(sentinel.c_str(), (root + "/hard-link").c_str()));
    {
        zmc::LocalVfs vfs(root);
        ASSERT_EQ(zmc::VfsResult::Ok, vfs.initialize());
        std::string bytes;
        zmc::VfsMetadata metadata;
        EXPECT_EQ(zmc::VfsResult::InvalidPath, vfs.readFile("parent-link/sentinel", bytes));
        EXPECT_EQ(zmc::VfsResult::InvalidPath,
                  vfs.writeFile("parent-link/escaped", "bad", 3,
                                zmc::VfsWriteMode::CreateOrReplace));
        EXPECT_EQ(zmc::VfsResult::InvalidPath, vfs.readFile("file-link", bytes));
        EXPECT_EQ(zmc::VfsResult::InvalidPath, vfs.readFile("hard-link", bytes));
        EXPECT_EQ(zmc::VfsResult::InvalidPath,
                  vfs.writeFile("file-link", "escaped", 7, zmc::VfsWriteMode::CreateOrReplace));
        EXPECT_EQ(zmc::VfsResult::InvalidPath, vfs.stat("parent-link/sentinel", metadata));
    }
    std::ifstream file(sentinel.c_str(), std::ios::binary);
    std::string bytes;
    file >> bytes;
    EXPECT_EQ("protected", bytes);
    unlink((root + "/parent-link").c_str());
    unlink((root + "/file-link").c_str());
    unlink((root + "/hard-link").c_str());
    rmdir(root.c_str());
    std::remove(sentinel.c_str());
    rmdir(outside.c_str());
}

TEST(LocalVfsTest, RootsAreIndependentAndRootSymlinksAreRejected) {
    const std::string firstRoot = makeTempDirectory();
    const std::string secondRoot = makeTempDirectory();
    ASSERT_FALSE(firstRoot.empty());
    ASSERT_FALSE(secondRoot.empty());
    {
        zmc::LocalVfs first(firstRoot);
        zmc::LocalVfs second(secondRoot);
        ASSERT_EQ(zmc::VfsResult::Ok, first.initialize());
        ASSERT_EQ(zmc::VfsResult::Ok, second.initialize());
        ASSERT_EQ(zmc::VfsResult::Ok,
                  first.writeFile("private", "data", 4, zmc::VfsWriteMode::CreateOnly));
        zmc::VfsMetadata metadata;
        EXPECT_EQ(zmc::VfsResult::NotFound, second.stat("private", metadata));
    }
#ifndef _WIN32
    const std::string rootLink = firstRoot + "-link";
    ASSERT_EQ(0, symlink(firstRoot.c_str(), rootLink.c_str()));
    zmc::LocalVfs linkedRoot(rootLink);
    EXPECT_EQ(zmc::VfsResult::InvalidPath, linkedRoot.initialize());
    unlink(rootLink.c_str());
#endif
    removeTestDirectory(firstRoot);
    removeTestDirectory(secondRoot);
}
#endif

}  // namespace
