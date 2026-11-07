#ifndef _ZMC_FB_BASE_H_
#define _ZMC_FB_BASE_H_

// #include "McCommon.h"
#include "stdint.h"

namespace zmc {

struct FbBase {
    uint16_t get_fb_ctrl() const;
    uint16_t get_fb_stat() const;
    uint16_t get_fb_type() const;
    uint32_t get_fb_error_id() const;
    void* get_impl_ptr() const;
};
struct FbBaseReconstruct{
    void* pImpl;
};
inline FbBaseReconstruct ptr2FbBasePtr(void* ptr){
    return FbBaseReconstruct { ptr };
}

struct FbReaderBase : public FbBase {};
struct FbWriterBase : public FbBase {};

struct FbBMotionBase : public FbBase {};


// only FbPower, FbStop, FbCamOut, FbGearOut
struct FbImediatelyBase : public FbBase {};

struct FbBufferableBase : public FbBMotionBase {};

struct FbDisconcreteBase : public FbBufferableBase {};
struct FbContinuousBase : public FbBufferableBase {};
struct FbSlaveBase : public FbBufferableBase {};

// only FbPhase
struct FbEndlessBase : public FbBMotionBase {};
// only FbMoveSuperimposed
struct FbSpecialBase : public FbBMotionBase {};
uint16_t get_fb_stat(FbBase* fb);

} /* namespace zmc */

#endif