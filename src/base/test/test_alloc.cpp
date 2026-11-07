#include <gtest/gtest.h>
#include <vector>
#include <algorithm>
#include "common/misc/MemoryAllocator.hpp"
#include "common/misc/ZmcMemAllocator.hpp"

using zmc::MemoryAllocator;
using zmc::AllocatorMemoryAdapter;

// Basic allocation and deallocation
TEST(MemoryAllocator, BasicAllocFree) {
    MemoryAllocator alloc(4096);
    void* p1 = alloc.allocate(128);
    ASSERT_NE(p1, nullptr);
    alloc.deallocate(p1);
}

// Allocate all memory, then fail
TEST(MemoryAllocator, OutOfMemory) {
    MemoryAllocator alloc(1024);
    void* p1 = alloc.allocate(900); // Should succeed
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(alloc.allocate(200), nullptr);
    // EXPECT_THROW(alloc.allocate(200), std::bad_alloc);
    alloc.deallocate(p1);
}

// Free and reallocate (fragmentation/merge)
TEST(MemoryAllocator, FragmentationAndMerge) {
    MemoryAllocator alloc(2048);
    void* p1 = alloc.allocate(256);
    void* p2 = alloc.allocate(256);
    void* p3 = alloc.allocate(256);
    alloc.deallocate(p2);
    void* p4 = alloc.allocate(200); // Should reuse p2's space
    ASSERT_EQ(p2, p4);
    alloc.deallocate(p1);
    alloc.deallocate(p3);
    alloc.deallocate(p4);
    // Now all memory should be free, try a big alloc
    void* p5 = alloc.allocate(1800);
    ASSERT_NE(p5, nullptr);
    alloc.deallocate(p5);
}

// Double free and invalid free
TEST(MemoryAllocator, DoubleAndInvalidFree) {
    MemoryAllocator alloc(1024);
    void* p1 = alloc.allocate(128);
    alloc.deallocate(p1);
    // Double free: should not crash, but may not throw
    alloc.deallocate(p1);
    // Invalid free: pointer not from allocator
    int x = 0;
    // Should not throw, but is_pointer_valid should be false
    EXPECT_FALSE(alloc.is_pointer_valid(&x));
    EXPECT_FALSE(alloc.is_pointer_valid(nullptr));
}

// Zero-size allocation
TEST(MemoryAllocator, ZeroSizeAlloc) {
    MemoryAllocator alloc(1024);
    void* p = alloc.allocate(0);
    ASSERT_EQ(p, nullptr);
}

// Pointer validity check
TEST(MemoryAllocator, PointerValidity) {
    MemoryAllocator alloc(1024);
    void* p1 = alloc.allocate(128);
    EXPECT_TRUE(alloc.is_pointer_valid(p1));
    alloc.deallocate(p1);
    EXPECT_FALSE(alloc.is_pointer_valid(p1)); // After free, should be invalid
}

// Edge cases: min/max block, alignment
TEST(MemoryAllocator, MinMaxBlockAndAlignment) {
    MemoryAllocator alloc(4096);
    // Min block (should round up to MIN_BLOCK_SIZE)
    void* p1 = alloc.allocate(1);
    ASSERT_NE(p1, nullptr);
    // Max block
    void* p2 = alloc.allocate(4096 - 128); // Large, but not too large
    ASSERT_NE(p2, nullptr);
    alloc.deallocate(p1);
    alloc.deallocate(p2);
    // Alignment
    void* p3 = alloc.allocate(13);
    void* p4 = alloc.allocate(21);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(p3) % 8, 0u);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(p4) % 8, 0u);
    alloc.deallocate(p3);
    alloc.deallocate(p4);
}

// Stress test: many small allocs/frees
TEST(MemoryAllocator, StressManySmallAllocs) {
    MemoryAllocator alloc(16 * 1024);
    std::vector<void*> ptrs;
    for (int i = 0; i < 500; ++i) {
        void* p = alloc.allocate(16);
        ASSERT_NE(p, nullptr);
        ptrs.push_back(p);
    }
    std::random_shuffle(ptrs.begin(), ptrs.end());
    for (void* p : ptrs) {
        alloc.deallocate(p);
    }
    // After all frees, should be able to allocate a big block
    void* big = alloc.allocate(15 * 1024);
    ASSERT_NE(big, nullptr);
    EXPECT_TRUE(alloc.is_pointer_valid(big));
    alloc.deallocate(big);
}

// Basic allocation and deallocation for AllocatorMemoryAdapter
TEST(AllocatorMemoryAdapter, BasicAllocFree) {
    AllocatorMemoryAdapter alloc(4096);
    void* p1 = alloc.allocate(128);
    ASSERT_NE(p1, nullptr);
    alloc.deallocate(p1, 128);
}

// Allocate all memory, then fail
TEST(AllocatorMemoryAdapter, OutOfMemory) {
    AllocatorMemoryAdapter alloc(1024);
    void* p1 = alloc.allocate(900); // Should succeed
    ASSERT_NE(p1, nullptr);
    ASSERT_EQ(alloc.allocate(200), nullptr);
    // EXPECT_THROW(alloc.allocate(200), std::bad_alloc);
    alloc.deallocate(p1, 900);
}

// Free and reallocate (fragmentation/merge)
TEST(AllocatorMemoryAdapter, FragmentationAndMerge) {
    AllocatorMemoryAdapter alloc(2048);
    void* p1 = alloc.allocate(256);
    void* p2 = alloc.allocate(256);
    void* p3 = alloc.allocate(256);
    alloc.deallocate(p2, 256);
    void* p4 = alloc.allocate(200); // Should reuse p2's space
    ASSERT_EQ(p2, p4);
    alloc.deallocate(p1, 256);
    alloc.deallocate(p3, 256);
    alloc.deallocate(p4, 200);
    // Now all memory should be free, try a big alloc
    void* p5 = alloc.allocate(1800);
    ASSERT_NE(p5, nullptr);
    alloc.deallocate(p5, 1800);
}

// Zero-size allocation
TEST(AllocatorMemoryAdapter, ZeroSizeAlloc) {
    AllocatorMemoryAdapter alloc(1024);
    void* p = alloc.allocate(0);
    ASSERT_EQ(p, nullptr);
}

// Pointer validity check (if available)
TEST(AllocatorMemoryAdapter, PointerValidity) {
    AllocatorMemoryAdapter alloc(1024);
    void* p1 = alloc.allocate(128);
    EXPECT_TRUE(alloc.is_pointer_valid(p1));
    alloc.deallocate(p1, 128);
    EXPECT_FALSE(alloc.is_pointer_valid(p1)); // After free, should be invalid
}

