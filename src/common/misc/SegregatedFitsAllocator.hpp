#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cassert>

class SegregatedFitsAllocator {
private:
    static constexpr size_t NUM_BUCKETS = 32; // Supports sizes from 16 B to 4 GB.
    static constexpr size_t MIN_BLOCK_SIZE = 32; // Minimum block (header + footer + 2 pointers).
    static constexpr size_t ALIGNMENT = 8;

    struct Block {
        size_t header; // The lowest bit indicates allocated (1) or free (0); the remaining bits store the size.
        Block* next;   // Valid only when free.
        Block* prev;   // Valid only when free.

        size_t size() const { return header & ~1UL; }
        bool is_allocated() const { return header & 1; }
        void set_header(size_t s, bool alloc) { header = s | (alloc ? 1 : 0); }

        // Boundary tag: get the size_t at the end of the block (footer).
        size_t* footer() {
            return (size_t*)((uint8_t*)this + size() - sizeof(size_t));
        }
        void update_footer() { *footer() = header; }
    };

    Block* buckets[NUM_BUCKETS];
    void* heap_start;
    size_t heap_size;

public:
    SegregatedFitsAllocator(size_t total_size) {
        for (int i = 0; i < NUM_BUCKETS; ++i) buckets[i] = nullptr;
        
        heap_size = align(total_size);
        heap_start = ::operator new(heap_size);
        
        // Initialize the entire heap as one large free block.
        Block* initial = (Block*)heap_start;
        initial->set_header(heap_size, false);
        initial->update_footer();
        insert_to_bucket(initial);
    }

    void* allocate(size_t size) {
        size_t total_size = align(size + sizeof(size_t) * 2); // Data + header + footer.
        if (total_size < MIN_BLOCK_SIZE) total_size = MIN_BLOCK_SIZE;

        int bucket_idx = get_bucket_index(total_size);
        Block* found = nullptr;

        // Search upward from the current bucket for a suitable free block.
        for (int i = bucket_idx; i < NUM_BUCKETS; ++i) {
            found = find_fit_in_bucket(buckets[i], total_size);
            if (found) break;
        }

        if (!found) return nullptr; // Out of memory.

        remove_from_bucket(found);
        
        // Check whether the block needs to be split.
        size_t remaining = found->size() - total_size;
        if (remaining >= MIN_BLOCK_SIZE) {
            Block* next_block = (Block*)((uint8_t*)found + total_size);
            next_block->set_header(remaining, false);
            next_block->update_footer();
            insert_to_bucket(next_block);
            
            found->set_header(total_size, true);
            found->update_footer();
        } else {
            found->set_header(found->size(), true);
            found->update_footer();
        }

        return (uint8_t*)found + sizeof(size_t); // Return the data area.
    }

    void deallocate(void* ptr) {
        if (!ptr) return;
        Block* block = (Block*)((uint8_t*)ptr - sizeof(size_t));
        
        // Try to coalesce with the previous and next blocks.
        block = coalesce(block);
        
        block->set_header(block->size(), false);
        block->update_footer();
        insert_to_bucket(block);
    }

private:
    size_t align(size_t s) { return (s + ALIGNMENT - 1) & ~(ALIGNMENT - 1); }

    int get_bucket_index(size_t size) {
        // Use simple logarithmic size classes (powers of two).
        if (size <= 32) return 0;
        int idx = 0;
        size >>= 6; // Start size classes at 64 B.
        while (size > 0 && idx < NUM_BUCKETS - 1) {
            size >>= 1;
            idx++;
        }
        return idx;
    }

    void insert_to_bucket(Block* b) {
        int idx = get_bucket_index(b->size());
        b->next = buckets[idx];
        b->prev = nullptr;
        if (buckets[idx]) buckets[idx]->prev = b;
        buckets[idx] = b;
    }

    void remove_from_bucket(Block* b) {
        int idx = get_bucket_index(b->size());
        if (b->prev) b->prev->next = b->next;
        else buckets[idx] = b->next;
        if (b->next) b->next->prev = b->prev;
    }

    Block* find_fit_in_bucket(Block* head, size_t size) {
        Block* curr = head;
        while (curr) {
            if (curr->size() >= size) return curr;
            curr = curr->next;
        }
        return nullptr;
    }

    Block* coalesce(Block* b) {
        size_t size = b->size();
        
        // 1. Check the physically next block.
        Block* next = (Block*)((uint8_t*)b + size);
        if ((uint8_t*)next < (uint8_t*)heap_start + heap_size && !next->is_allocated()) {
            remove_from_bucket(next);
            size += next->size();
        }

        // 2. Check the physically previous block using the footer.
        if ((uint8_t*)b > (uint8_t*)heap_start) {
            size_t prev_footer = *((size_t*)((uint8_t*)b - sizeof(size_t)));
            if (!(prev_footer & 1)) {
                Block* prev = (Block*)((uint8_t*)b - (prev_footer & ~1UL));
                remove_from_bucket(prev);
                size += prev->size();
                b = prev;
            }
        }

        b->set_header(size, false);
        return b;
    }
};
