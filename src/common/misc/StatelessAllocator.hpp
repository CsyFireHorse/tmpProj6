#ifndef _ZMC_STATELESS_ALLOCATOR_HPP_
#define _ZMC_STATELESS_ALLOCATOR_HPP_

#include "common/misc/MemoryAllocator.hpp"
#include <cstddef>
#include <limits>
#include <new>
#include <stdexcept>
#include <utility>

namespace zmc {

// ==========================================================
// Unified allocator access template
// ==========================================================

// Generic allocator access template (supports default construction)
template<typename Alloc>
struct SharedAllocator {
    static Alloc& getAllocator() {
        static Alloc allocator;
        return allocator;
    }
};

// MemoryAllocator specialization (supports custom heap size)
template<>
struct SharedAllocator<MemoryAllocator> {
    // Get allocator with custom heap size
    static MemoryAllocator& getAllocator(size_t heap_size) {
        static MemoryAllocator allocator(heap_size);
        return allocator;
    }
    
    // Default: use 1MB heap
    static MemoryAllocator& getAllocator() {
        return getAllocator(1024 * 1024);
    }
};

// ==========================================================
// Stateless allocator using MemoryAllocator as global pool
// ==========================================================

// Stateless allocator using MemoryAllocator as global pool
// No member variables, sizeof == 1 (optimized to 0 via empty base optimization)
template <typename T>
class StatelessAllocatorMA {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    // C++11 allocator traits
    using propagate_on_container_copy_assignment = std::false_type;
    using propagate_on_container_move_assignment = std::false_type;
    using propagate_on_container_swap = std::false_type;
    using is_always_equal = std::true_type;  // All instances are equal (stateless)

    // Default constructor (no parameters needed)
    StatelessAllocatorMA() noexcept = default;

    // Copy constructor
    template <typename U>
    StatelessAllocatorMA(const StatelessAllocatorMA<U>&) noexcept {}

    // Destructor
    ~StatelessAllocatorMA() = default;

    // Allocate memory for n objects
    // Uses SharedAllocator<MemoryAllocator> for unified access
    pointer allocate(size_type n) {
        if (n > max_size()) {
            throw std::bad_alloc();
        }
        void* ptr = SharedAllocator<MemoryAllocator>::getAllocator().allocate(n * sizeof(T));
        if (!ptr) {
            throw std::bad_alloc();
        }
        return static_cast<pointer>(ptr);
    }

    // Deallocate memory
    // Uses SharedAllocator<MemoryAllocator> for unified access
    void deallocate(pointer p, size_type n) noexcept {
        if (p) {
            SharedAllocator<MemoryAllocator>::getAllocator().deallocate(p);
        }
    }

    // Maximum number of elements
    size_type max_size() const noexcept {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    // Construct object
    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
    }

    // Destroy object
    template <typename U>
    void destroy(U* p) {
        p->~U();
    }

    // Rebind to other type (required for containers)
    template <typename U>
    struct rebind {
        using other = StatelessAllocatorMA<U>;
    };
};

// Equality comparison (all instances are equal)
template <typename T, typename U>
bool operator==(const StatelessAllocatorMA<T>&, const StatelessAllocatorMA<U>&) noexcept {
    return true;  // All stateless allocators are equal
}

template <typename T, typename U>
bool operator!=(const StatelessAllocatorMA<T>&, const StatelessAllocatorMA<U>&) noexcept {
    return false;
}

}  // namespace zmc

#endif  // _ZMC_STATELESS_ALLOCATOR_HPP_
