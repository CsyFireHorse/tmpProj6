#ifndef _ZMC_MEMORY_ALLOCATOR_HPP_
#define _ZMC_MEMORY_ALLOCATOR_HPP_

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace zmc{
class MemoryAllocator {
public:
    // Initialize the memory allocator.
    explicit MemoryAllocator(size_t heap_size = 1024 * 1024) 
        : heap_size_(heap_size) {
        initialize();
    }

    // Release the entire heap in the destructor.
    ~MemoryAllocator() {
        std::free(heap_start_);
    }

    // Allocate memory.
    void* allocate(size_t size) {
        if (size == 0) return nullptr;

        // Calculate the block size, including alignment.
        size_t total_size = align_size(size + HEADER_SIZE + FOOTER_SIZE);
        if (total_size < MIN_BLOCK_SIZE) {
            #if 0
            throw std::bad_alloc();
            #else
            total_size = MIN_BLOCK_SIZE;
            #endif
        }

        // Traverse the free list using the first-fit algorithm.
        for (auto* current = free_list_.next; current != &free_list_; current = current->next) {
            BlockHeader* header = get_header_from_node(current);
            const size_t block_size = header->get_size();

            if (block_size >= total_size) {
                // Calculate the remaining size after splitting.
                const size_t remaining_size = block_size - total_size;
                
                if (remaining_size >= MIN_BLOCK_SIZE) {
                    // Split the block into an allocated block and a new free block.
                    header->set_size_and_allocated(total_size, true);
                    
                    // Create a new free block.
                    BlockHeader* new_free_header = get_next_block(header);
                    new_free_header->set_size_and_allocated(remaining_size, false);
                    
                    // Update the free list.
                    insert_free_block(new_free_header);
                } else {
                    // Allocate the entire block without splitting.
                    header->set_allocated(true);
                }
                
                // Remove the current block from the free list.
                remove_free_block(current);
                
                // Return the address of the user data area.
                return header->user_ptr();
            }
        }
        return nullptr;
        // throw std::bad_alloc(); // Not enough memory.
    }

    // Free memory.
    void deallocate(void* ptr) {
        if (!ptr) return;
        
        // Check whether the pointer belongs to this allocator.
        // if (!is_pointer_valid(ptr)) {
            // throw std::runtime_error("Attempt to free invalid pointer");
        // }
        // Get the block header.
        BlockHeader* header = BlockHeader::from_user_ptr(ptr);
        header->set_allocated(false);
        
        // Try to coalesce adjacent free blocks.
        BlockHeader* new_header = coalesce_block(header);
        
        // Add the coalesced block to the free list.
        insert_free_block(new_header);
    }
bool is_pointer_valid(void* ptr) const {
    // 1. Check for a null pointer.
    if (!ptr) return false;
    
    // 2. Check that the pointer is within the heap.
    char* char_ptr = static_cast<char*>(ptr);
    if (char_ptr < heap_start_ || char_ptr >= heapEnd()) {
        return false;
    }
    
    // 3. Get the header pointer.
    BlockHeader* header = BlockHeader::from_user_ptr(ptr);
    
    // 4. Check that the header address is valid.
    if (reinterpret_cast<char*>(header) < heap_start_ || 
        HEADER_SIZE >= ((char*)heapEnd()) - reinterpret_cast<char*>(header) ) {
        return false;
    }
    
    // 5. Check that the block size is valid.
    if (header->get_size() < MIN_BLOCK_SIZE) {
        return false;
    }
    
    // 6. Check that the header and footer match.
    if (!header->is_footer_valid()) {
        return false;
    }
    
    // 7. Check the allocation state.
    return header->is_allocated();
}
private:
    // Block header/footer structure.
    class BlockHeader {
    public:
        // Set the block size and allocation state.
        void set_size_and_allocated(size_t size, bool allocated) {
            size_ = size | (allocated ? 1 : 0);
            get_footer()->size_ = size_;
        }

        // Set the allocation state.
        void set_allocated(bool allocated) {
            if (allocated) {
                size_ |= 1;
            } else {
                size_ &= ~1;
            }
            get_footer()->size_ = size_;
        }

        // Get the block size with the allocation bit cleared.
        size_t get_size() const { 
            return size_ & ~1; 
        }
        // Check whether the block is allocated.
        bool is_allocated() const { 
            return !is_free(); 
        }
        // Check whether the block is free.
        bool is_free() const { 
            return (size_ & 1) == 0; 
        }

        // Get the user pointer.
        void* user_ptr() { 
            return reinterpret_cast<char*>(this) + HEADER_SIZE; 
        }

        // Get the header pointer from the user pointer.
        static BlockHeader* from_user_ptr(void* ptr) {
            return reinterpret_cast<BlockHeader*>(static_cast<char*>(ptr) - HEADER_SIZE);
        }

        // Get the footer pointer.
        BlockHeader* get_footer() {
            return reinterpret_cast<BlockHeader*>(reinterpret_cast<char*>(this) + get_size() - FOOTER_SIZE);
        }
        bool is_footer_valid() {
            return get_footer()->size_ == size_;
        }
    private:
        size_t size_;  // Block size (the lowest bit indicates the allocation state).
    };

    // Free-list node structure, stored in free blocks.
    struct FreeListNode {
        FreeListNode* prev;
        FreeListNode* next;
    };

    // Constant definitions.
    static constexpr size_t ALIGNMENT = 8;         // Alignment requirement.
    static constexpr size_t MIN_BLOCK_SIZE = 32;   // Minimum block size.
    static constexpr size_t HEADER_SIZE = sizeof(BlockHeader);
    static constexpr size_t FOOTER_SIZE = sizeof(BlockHeader);
    static constexpr size_t MIN_BLOCK_DATA_SIZE = sizeof(FreeListNode);

    // Member variables.
    size_t heap_size_;
    void* heap_start_ = nullptr;
    FreeListNode free_list_;  // Free-list sentinel node.

    inline void* heapEnd()const{
        return static_cast<char*>(heap_start_) + heap_size_;
    }
    // Initialize the memory allocator.
    void initialize() {
        // Allocate heap memory.
        heap_start_ = std::malloc(heap_size_);
        if (!heap_start_) {
            // throw std::bad_alloc();
            return;
        }

        // Initialize the free list.
        free_list_.prev = &free_list_;
        free_list_.next = &free_list_;

        // Set the prologue block (allocated).
        BlockHeader* prologue = static_cast<BlockHeader*>(heap_start_);
        prologue->set_size_and_allocated(HEADER_SIZE, true);

        // Set the epilogue block at the end of the heap (allocated).
        BlockHeader* epilogue = reinterpret_cast<BlockHeader*>(
            static_cast<char*>(heap_start_) + heap_size_ - HEADER_SIZE);
        epilogue->set_size_and_allocated(0, true);

        // Create the initial free block from the remaining space.
        const size_t free_size = heap_size_ - HEADER_SIZE - HEADER_SIZE;
        BlockHeader* free_header = reinterpret_cast<BlockHeader*>(
            static_cast<char*>(heap_start_) + HEADER_SIZE);
        free_header->set_size_and_allocated(free_size, false);
        
        // Add the initial free block to the list.
        insert_free_block(free_header);
    }

    // Handle alignment.
    static size_t align_size(size_t size) {
        return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    }

    // Get the next block's header.
    BlockHeader* get_next_block(BlockHeader* header) {
        return reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(header) + header->get_size());
    }

    // Get the previous block's header.
    BlockHeader* get_prev_block(BlockHeader* header) {
        // The previous block's footer is immediately before the current header.
        BlockHeader* prev_footer = reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(header) - FOOTER_SIZE);
        return reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(header) - prev_footer->get_size());
    }

    // Get the header from a free-list node.
    BlockHeader* get_header_from_node(FreeListNode* node) {
        return reinterpret_cast<BlockHeader*>(
            reinterpret_cast<char*>(node) - HEADER_SIZE);
    }

    // Get the free-list node from a header.
    FreeListNode* get_node_from_header(BlockHeader* header) {
        return reinterpret_cast<FreeListNode*>(
            reinterpret_cast<char*>(header) + HEADER_SIZE);
    }

    // Insert a free block into the list.
    void insert_free_block(BlockHeader* header) {
        FreeListNode* node = get_node_from_header(header);
        
        node->prev = &free_list_;
        node->next = free_list_.next;
        free_list_.next->prev = node;
        free_list_.next = node;
    }

    // Remove a block from the free list.
    void remove_free_block(FreeListNode* node) {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    // Coalesce adjacent free blocks.
    BlockHeader* coalesce_block(BlockHeader* header) {
        size_t total_size = header->get_size();
        BlockHeader* current = header;

        // Check and coalesce with the next block.
        BlockHeader* next = get_next_block(header);
        if (next->is_free() && next->get_size() > 0) {
            total_size += next->get_size();
            remove_free_block(get_node_from_header(next));
        }

        // Check and coalesce with the previous block.
        if (reinterpret_cast<char*>(header) > static_cast<char*>(heap_start_) + HEADER_SIZE) {
            BlockHeader* prev = get_prev_block(header);
            if (prev->is_free()) {
                total_size += prev->get_size();
                current = prev;
                remove_free_block(get_node_from_header(prev));
            }
        }

        // Set the header and footer of the coalesced block.
        current->set_size_and_allocated(total_size, false);
        return current;
    }
};

}
#endif
// // Test code.
// int main() {
//     try {
//         MemoryAllocator allocator;
        
//         // Allocation test.
//         void* p1 = allocator.allocate(100);
//         void* p2 = allocator.allocate(200);
//         void* p3 = allocator.allocate(300);
        
//         std::cout << "Allocated: p1=" << p1 << ", p2=" << p2 << ", p3=" << p3 << "\n";
        
//         // Pointer validation test.
//         std::cout << "p1 valid: " << allocator.is_pointer_valid(p1) << "\n";
//         std::cout << "p2 valid: " << allocator.is_pointer_valid(p2) << "\n";
//         std::cout << "p3 valid: " << allocator.is_pointer_valid(p3) << "\n";
        
//         // Invalid pointer test.
//         int x;
//         std::cout << "Local variable valid: " << allocator.is_pointer_valid(&x) << "\n";
//         std::cout << "Null pointer valid: " << allocator.is_pointer_valid(nullptr) << "\n";
          
//         // Freeing test.
//         allocator.deallocate(p2);
//         std::cout << "Freed p2\n";
        
//         allocator.deallocate(p1);
//         std::cout << "Freed p1\n";
        
//         // Reallocation.
//         void* p4 = allocator.allocate(150);
//         std::cout << "Allocated p4=" << p4 << "\n";
        
//         allocator.deallocate(p3);
//         allocator.deallocate(p4);
        
//     } catch (const std::exception& e) {
//         std::cerr << "Error: " << e.what() << "\n";
//         return 1;
//     }
    
//     return 0;
// }
