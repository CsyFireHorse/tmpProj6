#ifndef SIMPLE_POOL_ALLOCATOR_HPP
#define SIMPLE_POOL_ALLOCATOR_HPP

#include <cstddef>
#include <cstdint>
#include <new>
#include <algorithm>

/**
 * @brief Fixed-size block free list allocator (Pool Allocator)
 * 
 * @tparam BlockSize Byte size of each memory block
 * @tparam BlockCount Total number of memory blocks in the pool
 * @tparam Alignment Alignment requirement, default is 8 bytes
 */
template <std::size_t BlockSize, std::size_t BlockCount, std::size_t Alignment = 8>
class SimplePoolAllocator {
private:
    // Ensure block size is at least large enough to hold a pointer and meets alignment requirements
    static constexpr std::size_t ActualBlockSize = (BlockSize > sizeof(void*) ? BlockSize : sizeof(void*));
    static constexpr std::size_t AlignedBlockSize = (ActualBlockSize + Alignment - 1) & ~(Alignment - 1);

    // Intrusive linked list node: directly uses the first few bytes of free blocks to store pointer
    struct Node {
        Node* next;
    };

public:
    SimplePoolAllocator() {
        reset();
    }

    // Disable copy to prevent memory management confusion
    SimplePoolAllocator(const SimplePoolAllocator&) = delete;
    SimplePoolAllocator& operator=(const SimplePoolAllocator&) = delete;

    /**
     * @brief Allocate a memory block
     * @return void* Pointer to allocated block, returns nullptr if pool is full
     */
    void* allocate(std::size_t n = 0, std::size_t align = 0) {
        // If requested size exceeds block size, allocation fails
        if (n > BlockSize) return nullptr;
        
        if (!free_list_head_) return nullptr;

        Node* node = free_list_head_;
        free_list_head_ = node->next;
        return static_cast<void*>(node);
    }

    /**
     * @brief Release a memory block back to free list
     * @param p Pointer to the block to be released
     */
    void deallocate(void* p, std::size_t n = 0, std::size_t align = 0) {
        if (!p) return;

        Node* node = static_cast<Node*>(p);
        node->next = free_list_head_;
        free_list_head_ = node;
    }

    /**
     * @brief Reset pool, mark all blocks as free again
     */
    void reset() {
        free_list_head_ = reinterpret_cast<Node*>(data_);
        Node* curr = free_list_head_;
        
        for (std::size_t i = 0; i < BlockCount - 1; ++i) {
            uint8_t* next_addr = data_ + (i + 1) * AlignedBlockSize;
            curr->next = reinterpret_cast<Node*>(next_addr);
            curr = curr->next;
        }
        curr->next = nullptr;
    }

    // Query interface
    static constexpr std::size_t block_size() { return BlockSize; }
    static constexpr std::size_t capacity() { return BlockCount; }

private:
    // Use alignas to ensure the start alignment of the entire buffer
    alignas(Alignment) uint8_t data_[AlignedBlockSize * BlockCount];
    Node* free_list_head_ = nullptr;
};

#endif // SIMPLE_POOL_ALLOCATOR_HPP