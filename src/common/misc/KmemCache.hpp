#include <cstddef>
#include <cstdint>
#include <new>
#include <vector>
#include <cassert>
#include <iostream>

// Helper: find the first zero bit in the bitmap (an available slot).
static inline int find_first_free(uint64_t bitmap, int limit) {
    if (bitmap == ~0ULL) return -1;
    // Use the built-in instruction to count trailing zeros (__builtin_ctzll).
    int bit = __builtin_ctzll(~bitmap);
    return (bit < limit) ? bit : -1;
}

/**
 * @brief Slab structure definition.
 * 
 * Manage a contiguous memory block divided into multiple slots.
 */
struct Slab {
    uint8_t* memory;      // Storage area for objects.
    uint64_t usage_mask;  // Bitmap (supports up to 64 objects per slab).
    uint8_t  active_count; // Number of allocated objects.
    uint8_t  total_slots;  // Total number of slots.
    Slab*    next;
    Slab*    prev;

    Slab(void* mem, size_t slot_size, int count) 
        : memory(static_cast<uint8_t*>(mem)), usage_mask(0), 
          active_count(0), total_slots(count), next(nullptr), prev(nullptr) {}

    bool is_full() const { return active_count == total_slots; }
    bool is_empty() const { return active_count == 0; }

    void* allocate_slot(size_t slot_size) {
        int idx = find_first_free(usage_mask, total_slots);
        if (idx == -1) return nullptr;
        
        usage_mask |= (1ULL << idx);
        active_count++;
        return memory + (idx * slot_size);
    }

    bool deallocate_slot(void* ptr, size_t slot_size) {
        size_t offset = static_cast<uint8_t*>(ptr) - memory;
        int idx = offset / slot_size;
        if (!(usage_mask & (1ULL << idx))) return false; // Double free check

        usage_mask &= ~(1ULL << idx);
        active_count--;
        return true;
    }
};

/**
 * @brief kmem_cache
 * 
 * Manage a cache for a specific object type.
 */
template <typename T>
class KmemCache {
public:
    KmemCache(size_t slab_capacity = 64) 
        : obj_size(sizeof(T)), 
          slots_per_slab(slab_capacity > 64 ? 64 : slab_capacity),
          full_slabs(nullptr), partial_slabs(nullptr), empty_slabs(nullptr) {
        // Ensure the object size meets pointer or alignment requirements.
        if (obj_size < sizeof(void*)) obj_size = sizeof(void*);
    }

    T* alloc() {
        Slab* target = nullptr;

        // 1. Allocate from a partially free slab first (fastest path).
        if (partial_slabs) {
            target = partial_slabs;
        } 
        // 2. Otherwise, allocate from an empty slab.
        else if (empty_slabs) {
            target = empty_slabs;
            move_slab(&empty_slabs, &partial_slabs, target);
        }
        // 3. Create a new slab if needed.
        else {
            target = create_new_slab();
            add_to_list(&partial_slabs, target);
        }

        void* ptr = target->allocate_slot(obj_size);
        
        // Move the slab to the full list when it is full.
        if (target->is_full()) {
            move_slab(&partial_slabs, &full_slabs, target);
        }

        return new (ptr) T(); // Call the constructor (object caching).
    }

    void free(T* ptr) {
        ptr->~T(); // Call the destructor.
        
        // Find which slab owns this pointer (normally found through the page header; simplified here).
        Slab* s = find_slab_by_ptr(ptr);
        assert(s != nullptr);

        bool was_full = s->is_full();
        s->deallocate_slot(ptr, obj_size);

        if (s->is_empty()) {
            // Move the slab from the current list to the empty list.
            Slab** current_list = was_full ? &full_slabs : &partial_slabs;
            move_slab(current_list, &empty_slabs, s);
            
            // Policy: excess empty slabs could be freed; retain them here for reuse.
        } else if (was_full) {
            move_slab(&full_slabs, &partial_slabs, s);
        }
    }
    /**
     * @brief Preallocate the specified number of slabs.
     * @param slab_count Number of slabs to preallocate.
     */
    void reserve_slabs(size_t slab_count) {
        for (size_t i = 0; i < slab_count; ++i) {
            Slab* s = create_new_slab();
            add_to_list(&empty_slabs, s);
        }
    }

    /**
     * @brief Ensure the cache can hold at least the specified number of objects.
     * @param object_count Target total object capacity.
     */
    void reserve_objects(size_t object_count) {
        // Calculate current capacity (counting free and partially free slots is complex; total slots are used here).
        // In production, calculate how many slabs are needed from the target capacity.
        size_t current_slabs = count_all_slabs(); // Requires a helper function.
        size_t target_slabs = (object_count + slots_per_slab - 1) / slots_per_slab;
        
        if (target_slabs > current_slabs) {
            reserve_slabs(target_slabs - current_slabs);
        }
    }

    private:
    // Helper: count the slabs currently in use.
    size_t count_all_slabs() const {
        auto count_list = [](Slab* head) {
            size_t c = 0;
            while (head) { c++; head = head->next; }
            return c;
        };
        return count_list(full_slabs) + count_list(partial_slabs) + count_list(empty_slabs);
    }
private:
    size_t obj_size;
    int slots_per_slab;

    Slab* full_slabs;
    Slab* partial_slabs;
    Slab* empty_slabs;

    // Simulate low-level operating system page allocation.
    Slab* create_new_slab() {
        size_t total_mem = obj_size * slots_per_slab;
        void* raw_mem = ::operator new(total_mem); 
        return new Slab(raw_mem, obj_size, slots_per_slab);
    }

    // Linked-list management helpers.
    void add_to_list(Slab** list, Slab* s) {
        s->next = *list;
        if (*list) (*list)->prev = s;
        *list = s;
        s->prev = nullptr;
    }

    void remove_from_list(Slab** list, Slab* s) {
        if (s->prev) s->prev->next = s->next;
        else *list = s->next;
        if (s->next) s->next->prev = s->prev;
    }

    void move_slab(Slab** from, Slab** to, Slab* s) {
        remove_from_list(from, s);
        add_to_list(to, s);
    }

    // A production implementation can calculate the page index from the pointer's high bits in O(1).
    // This demo uses a simple global record for illustration.
    Slab* find_slab_by_ptr(void* ptr) {
        // This traversal is only for completeness; a real OS implementation does not need it.
        auto check = [&](Slab* head) -> Slab* {
            for (Slab* c = head; c; c = c->next) {
                if (ptr >= c->memory && ptr < (c->memory + obj_size * slots_per_slab))
                    return c;
            }
            return nullptr;
        };
        Slab* res = check(partial_slabs);
        if (!res) res = check(full_slabs);
        return res;
    }
};
