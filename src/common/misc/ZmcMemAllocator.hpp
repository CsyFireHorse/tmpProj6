/*
 * @Author: ChenShengyong
 * @Date: 2024-08-09 17:55:24
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-07-22 14:57:36
 */
#ifndef _ZMC_MEM_ALLOCATOR_HPP_
#define _ZMC_MEM_ALLOCATOR_HPP_

#include <string>
#include "stdint.h"
#include "ZmcAllocator.hpp"
#include "MemoryAllocator.hpp"

namespace zmc {
    
// Adapter: Make MemoryAllocator compatible with PmAllocator interface
class AllocatorMemoryAdapter : public PmAllocator {
public:
    // Construct with custom heap size (default 1MB)
    explicit AllocatorMemoryAdapter(size_t heap_size = 1024 * 1024)
        : allocator_(heap_size) {}

    // Allocate memory (n bytes)
    void* allocate(int n) override {
        return allocator_.allocate(static_cast<size_t>(n));
    }

    // Deallocate memory
    void deallocate(void* p, int /*n*/) override {
        allocator_.deallocate(p);
    }

    // Optional: expose pointer validity check
    bool is_pointer_valid(void* p) const {
        return allocator_.is_pointer_valid(p);
    }

private:
    MemoryAllocator allocator_;
};

}  // namespace zmc
#endif