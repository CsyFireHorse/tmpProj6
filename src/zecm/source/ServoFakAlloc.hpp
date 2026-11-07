#ifndef _ZMC_SERVO_FAK_ALLOC_HPP_
#define _ZMC_SERVO_FAK_ALLOC_HPP_

#include "base/include/ServoFakerToy.h"
#include "common/misc/ZmcBitmap.hpp"

namespace zmc {
    class ServoSgFaker: public ServoFakerToy{};
    class ServoSgFakerEager {
    public:
        ServoSgFakerEager(const ServoSgFakerEager&) = delete;
        ServoSgFakerEager& operator=(const ServoSgFakerEager&) = delete;
        static ServoSgFaker& Instance(unsigned int idx) {
            static ServoSgFaker pc[32];
            return pc[idx];
        }
        inline static int max_num() { return 32; }

        inline static zmc::BitmapAllocatorTmpl<32>& allocator() {
            static zmc::BitmapAllocatorTmpl<32> allocator;
            return allocator;
        }
        // static int s_cnt;
        static int allocate_consecutive(int SlaveCount){
            return allocator().allocate_consecutive(SlaveCount);
        }
    
        static int deallocate_consecutive(int allocatedStartIndex, int SlaveCount){
            return allocator().deallocate_consecutive(allocatedStartIndex, SlaveCount);
        }
    };
}
#endif