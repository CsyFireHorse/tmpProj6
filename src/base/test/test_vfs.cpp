#include "MemoryVfs.hpp"
#include "vfs_contract.hpp"

// Exercises the common contract against the in-memory backend. AI modification signature: GPT-6 Codex.

TEST(MemoryVfsTest, SharedContract) {
    zmc::MemoryVfs vfs;
    zmc::test::runVfsContract(vfs);
}

TEST(MemoryVfsTest, RootsAreIndependent) {
    zmc::MemoryVfs first;
    zmc::MemoryVfs second;
    ASSERT_EQ(zmc::VfsResult::Ok,
              first.writeFile("private", "data", 4, zmc::VfsWriteMode::CreateOnly));
    zmc::VfsMetadata metadata;
    EXPECT_EQ(zmc::VfsResult::NotFound, second.stat("private", metadata));
}
