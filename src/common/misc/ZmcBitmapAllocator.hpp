/*
 * @Author: ChenShengyong
 * @Date: 2024-08-09 17:55:24
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-02-12 10:52:07
 */
#ifndef _ZMC_BITMAP_ALLOCATOR_HPP_
#define _ZMC_BITMAP_ALLOCATOR_HPP_

#include "stdint.h"
#include "ZmcAllocator.hpp"
#include "ZmcBitmap.hpp"

namespace zmc {
    
template <int LEN, int SZ = 1>
class AllocatorBitmap : public PmAllocator {
   public:
    void* allocate(int) override {
        int ofs = _bitmap.allocate();
        if (ofs == -1) return nullptr;
        auto ptr = pBuf + ofs * SZ;
        return ptr;
    }

    void deallocate(void* p, int) override {
        int ofs = ((uint8_t*)p - pBuf) / SZ;
        _bitmap.deallocate(ofs);
    }

   private:
    BitmapAllocatorTmpl<LEN> _bitmap;
    uint8_t pBuf[SZ * LEN];
};

}  // namespace zmc
#endif