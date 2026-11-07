#ifndef _ZMC_BITMAP_HPP_
#define _ZMC_BITMAP_HPP_

#include "stdint.h"
namespace zmc {
inline
uint32_t findFirstKConsecutiveBits_Naive(uint32_t n, uint32_t k) {
    if (k == 0 || k > 32) return UINT32_MAX;
    
    uint32_t mask = (1U << k) - 1;
    for (uint32_t i = 0; i <= 32 - k; ++i) {
        if (((n>>i) & mask) == mask) {
            return i;
        }
    }
    return UINT32_MAX; // Not found
}

template <int SZ = 32>
class BitmapAllocatorTmpl {
   public:
    enum { 
        LEN4 = (SZ + 31) / 32,
        LEN = (SZ + 7) / 8 
    };
    bool isEmpty() const {
        for (int i = 0; i < LEN4; ++i) {
            if (_bitmap4[i] != 0) return false;
        }
        return true;
    }
    bool isFull() const {
        for (int i = 0; i < LEN4; ++i) {
            if (_bitmap4[i] != 0xFFFFFFFF) return false;
        }
        return true;
    }

    int allocate() {
        for (int j = 0; j < LEN4; ++j) {
            for (int i = 0; i < 32; i++){
                if ((_bitmap4[j] & (1UL << i)) == 0) {
                    _bitmap4[j] |= (1UL << i);
                    return i + 32 * j;
                }
            }
        }
        return -1;
    }
    int allocate_consecutive(int count) {
        if (count <= 0 || count > SZ) return -1;
        int idx = find_first_fit(count);
        if (idx == -1) return -1;
        
        // Set the consecutive bits to 1
        for (int i = 0; i < count; i++) {
            int j = (idx + i) / 32;
            int bit_pos = (idx + i) % 32;
            _bitmap4[j] |= (1UL << bit_pos);
        }
        return idx;
    }

    int find_first_fit(uint32_t required_units) {
        uint32_t run_len = 0;   
        int start_candidate;     
        for (uint32_t i = 0; i < SZ; ++i) {
            if (isNotAllocated(i)) {
                if (run_len == 0) {
                    start_candidate = i;
                }
                run_len++;
                
                if (run_len >= required_units) {
                    return start_candidate;
                }
            } else {
                run_len = 0;
            }
        }
        return -1;
    }

    void deallocate(int index) {
        if (index < 0 || index >= SZ) return;
        int j = index / 32;
        int i = index - 32 * j;
        _bitmap4[j] &= ~(1UL << i);
    }

    int deallocate_consecutive(int index, int count) {
        if (count <= 0 || count > SZ) return -1;
        for (int i=0; i<count; i++){
            int j = (index+i) & 7;
            // need to check this bit?
            _bitmap[index / 8] &= ~(1 << j);
        }
        return 0;
    }

    inline bool isAllocated(int index) const {
        if (index < 0 || index >= SZ) return false;
        return (_bitmap[index / 8] & (1 << (index & 7))) != 0;
    }
    inline bool isNotAllocated(int index) const {
        if (index < 0 || index >= SZ) return false;
        return (_bitmap[index / 8] & (1 << (index & 7))) == 0;
    }

   private:
   union{
    uint8_t _bitmap[LEN] = {0};
    uint32_t _bitmap4[LEN4];
   };
};
}  // namespace zmc
#endif