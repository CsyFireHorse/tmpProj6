#include <gtest/gtest.h>
#include "common/misc/ZmcBitmap.hpp"

using namespace zmc;

// Test BitmapAllocatorTmpl basic functionality
TEST(bitmap, basic_alloc_dealloc) {
    BitmapAllocatorTmpl<32> bitmap;
    
    // Test initial state
    EXPECT_TRUE(bitmap.isEmpty());
    EXPECT_FALSE(bitmap.isFull());
    
    // Test allocation
    int idx = bitmap.allocate();
    EXPECT_EQ(idx, 0);
    EXPECT_FALSE(bitmap.isEmpty());
    EXPECT_TRUE(bitmap.isAllocated(0));
    
    // Test deallocation
    bitmap.deallocate(0);
    EXPECT_TRUE(bitmap.isEmpty());
    EXPECT_FALSE(bitmap.isAllocated(0));
}

// Test consecutive allocation
TEST(bitmap, consecutive_allocation) {
    BitmapAllocatorTmpl<32> bitmap;
    
    // Allocate 3 consecutive bits
    int idx = bitmap.allocate_consecutive(3);
    EXPECT_EQ(idx, 0);
    
    // Check that first 3 bits are allocated
    for (int i = 0; i < 3; i++) {
        EXPECT_TRUE(bitmap.isAllocated(i));
    }
    EXPECT_FALSE(bitmap.isAllocated(3));
}

// Test findFirstKConsecutiveBits_Naive function
TEST(bitmap, find_consecutive_bits) {
    // Test with pattern 0b11001100 (0xCC)
    uint32_t n = 0xCC;
    EXPECT_EQ(findFirstKConsecutiveBits_Naive(n, 2), 2);  // First 2 consecutive bits at position 0
    EXPECT_EQ(findFirstKConsecutiveBits_Naive(n, 1), 2);  // First 1 consecutive bit at position 0
    EXPECT_EQ(findFirstKConsecutiveBits_Naive(n, 3), UINT32_MAX);  // No 3 consecutive bits
    
    // Test with pattern 0b11110000 (0xF0)
    EXPECT_EQ(findFirstKConsecutiveBits_Naive(0xF0, 4), 4);  // First 4 consecutive bits at position 4
    EXPECT_EQ(findFirstKConsecutiveBits_Naive(0x1e0, 4), 5);  // First 4 consecutive bits at position 4
}

// Test edge cases
TEST(bitmap, edge_cases) {
    BitmapAllocatorTmpl<8> bitmap;
    
    // Test invalid allocation
    int idx = bitmap.allocate_consecutive(0);
    EXPECT_EQ(idx, -1);
    
    idx = bitmap.allocate_consecutive(9);  // More than bitmap size
    EXPECT_EQ(idx, -1);
    
    // Test invalid deallocation
    bitmap.deallocate(-1);  // Invalid index
    bitmap.deallocate(10);  // Out of bounds
    
    // Test isAllocated with invalid indices
    EXPECT_FALSE(bitmap.isAllocated(-1));
    EXPECT_FALSE(bitmap.isAllocated(10));
}
