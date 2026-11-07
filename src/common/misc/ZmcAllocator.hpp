/*
 * @Author: ChenShengyong
 * @Date: 2024-08-09 17:55:24
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-04-27 11:11:56
 */
#ifndef _ZMC_ALLOCATOR_HPP_
#define _ZMC_ALLOCATOR_HPP_

#include <string>

#include "stdint.h"

namespace zmc {
// std::pmr::polymorphic_allocator
class PmAllocator {
   public:
    virtual void* allocate(int n) = 0;
    virtual void deallocate(void* p, int n) = 0;
    virtual ~PmAllocator(){}
};

class AllocatorDefault : public PmAllocator {
   public:
    void* allocate(int n) override { return malloc(n); }
    void deallocate(void* p, int n) override { free(p); }
};

// only allocate once
// always deallocate all memory before allocate memory, so return the beginning of memory
template <int SZ = 512>
class AllocatorArray : public PmAllocator {
   public:
    void* allocate(int n) override { 
        if (n <= SZ) return buf;
        else return nullptr; }
    void deallocate(void* p, int n) override {}

   private:
    // Keep typed objects stored in the byte buffer correctly aligned.
    alignas(double) uint8_t buf[SZ];
};
// only allocate once
class AllocatorOneshot : public PmAllocator {
   public:
    AllocatorOneshot(unsigned int sz = 512):_sz(sz) { pBuf = new uint8_t[sz]; }
    ~AllocatorOneshot() { delete[] pBuf; }

    void* allocate(int n) override {         
        if (n <= _sz) return pBuf;
        else return nullptr; 
    }
    void deallocate(void* p, int n) override {}

   private:
    uint8_t* pBuf;
    unsigned int _sz;
};

typedef void (*VoidFunc)(void*);
// only allocate once
class AllocatorSecondaryOneshot : public PmAllocator {
   public:
    AllocatorSecondaryOneshot(void* ptr, unsigned int sz, VoidFunc pFree):_sz(sz) { pBuf = ptr; _pFree = pFree; }
    ~AllocatorSecondaryOneshot() { 
        if (_pFree)
            _pFree(pBuf);
    }

    void* allocate(int n) override {         
        if (n <= _sz) return pBuf;
        else return nullptr; 
    }
    void deallocate(void* p, int n) override {}

   private:
    void* pBuf;
    unsigned int _sz;
    VoidFunc _pFree = nullptr;
};

template <unsigned int M = 4>
inline unsigned int align_upper(unsigned int n) {
    return ((int)(n + M - 1) / M) * M;
}

template <int SZ>
class AllocatorArena : public PmAllocator {
   public:
   AllocatorArena() { pBuf = new uint8_t[SZ]; }
    ~AllocatorArena() { delete[] pBuf; }
    void* baseAddr() const {
        return pBuf;
    }
    void* allocate(int n) override {
        auto aligned_n = align_upper(n);
        if (ofs + aligned_n > SZ) return nullptr;  
        auto ptr = pBuf + ofs;
        ofs += align_upper(n);
        return ptr;
    }
    void deallocate(void* p, int n) override { ofs = 0; }

   private:
    uint32_t ofs = 0;
    uint8_t* pBuf;
};

template <int SZ>
using AllocatorAreno = AllocatorArena<SZ>;

}  // namespace zmc
#endif
