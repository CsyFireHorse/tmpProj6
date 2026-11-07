/*
 * Axis function-block motbuf implementation.
 * AI modification: GPT-5 Codex
 */
#include "motbuf/AxisMotbufFb.h"

#include <cstdio>

#include "common/misc/ObjectPoolAllocator.hpp"
#include "common/zmc/zmc_predef.h"
#include "motbuf/fbmEnum.h"
#include "pou/Ctx.h"

namespace zmc {

class FbMovePool {
   public:
    static constexpr std::size_t POOL_SIZE = ZMC_FB_MOVE_POOL_SIZE;
    ObjectPoolAllocator<FbMoveErasedTmpl<FbtMoveRelative>, POOL_SIZE> pool_move_relative_;
    ObjectPoolAllocator<FbMoveErasedTmpl<FbtMoveAbsolute>, POOL_SIZE> pool_move_absolute_;
    ObjectPoolAllocator<FbMoveErasedTmpl<FbtMoveAdditive>, POOL_SIZE> pool_move_additive_;
    ObjectPoolAllocator<FbMoveErasedTmpl<FbtMoveVelocity>, POOL_SIZE> pool_move_velocity_;
};

AxisMotbufFb::AxisMotbufFb(FbMoveWrapWithAxis* parent, int axNum)
    : parent_(parent), pMovePool(new FbMovePool()) {
    resize(axNum);
}

AxisMotbufFb::~AxisMotbufFb() {
    // Release active FB nodes before their shared object pool.
    axFbArr.clear();
    delete pMovePool;
}

void AxisMotbufFb::setCtx(const Ctx* ctx) {
    AxisMotbufBase::setCtx(ctx);
    for (std::size_t i = 0; i < axFbArr.size(); ++i) {
        bindAxisSlot(static_cast<int>(i));
    }
}

void AxisMotbufFb::reserve(int cap) {
    if (cap < 0) {
        return;
    }
    axFbArr.reserve(static_cast<std::size_t>(cap));
}

int AxisMotbufFb::resize(int axNum) {
    if (axNum < 0) {
        return -1;
    }
    axFbArr.resize(static_cast<std::size_t>(axNum));
    for (int i = 0; i < axNum; ++i) {
        bindAxisSlot(i);
    }
    return 0;
}

void AxisMotbufFb::reset_runtime() {
    // Reset slot-owned FBs before returning every fixed-pool node to its free list.
    for (std::size_t i = 0; i < axFbArr.size(); ++i) {
        axFbArr[i].reset_runtime();
    }
    pMovePool->pool_move_relative_.reset();
    pMovePool->pool_move_absolute_.reset();
    pMovePool->pool_move_additive_.reset();
    pMovePool->pool_move_velocity_.reset();
}

std::size_t AxisMotbufFb::size() const { return axFbArr.size(); }

AxisWithFb& AxisMotbufFb::axis(int index) {
    return axFbArr[static_cast<std::size_t>(index)];
}

const AxisWithFb& AxisMotbufFb::axis(int index) const {
    return axFbArr[static_cast<std::size_t>(index)];
}

AXIS_REF AxisMotbufFb::axisRef(int index) const {
    return axFbArr[static_cast<std::size_t>(index)].ax;
}

void AxisMotbufFb::bindAxis(AXIS_REF axisRef, int index) {
    axFbArr[static_cast<std::size_t>(index)].bindAxis(axisRef);
}

void AxisMotbufFb::runCycleTop() {
    for (std::size_t i = 0; i < axFbArr.size(); ++i) {
        axFbArr[i].runCycleTop();
    }
}

void AxisMotbufFb::runCycleBottom() {
    for (std::size_t i = 0; i < axFbArr.size(); ++i) {
        axFbArr[i].runCycleBottom();
    }
}

void AxisMotbufFb::runCycle() {
    runCycleTop();
    runCycleBottom();
}

void AxisMotbufFb::bindAxisSlot(int index) {
    AxisWithFb& axisFb = axFbArr[static_cast<std::size_t>(index)];
    axisFb.parent_ = parent_;
    axisFb.axisId = index;

    AXIS_REF axisRef = nullptr;
    if (ctx()) {
        axisRef = ctx()->getAxisByIndex(static_cast<uint32_t>(index));
    }
    axisFb.bindAxis(axisRef);
}

FbMoveErasedBase* AxisMotbufFb::allocateFb(int cmdType, AXIS_REF axisRef) {
    FbMoveErasedBase* fb = nullptr;
    switch (cmdType) {
        case FBM_MOVE_RELATIVE:
            fb = pMovePool->pool_move_relative_.pop();
            if (fb) {
                fb->setPool(&pMovePool->pool_move_relative_);
                static_cast<FbMoveErasedTmpl<FbtMoveRelative>*>(fb)->get()->Axis = axisRef;
            }
            break;
        case FBM_MOVE_ABSOLUTE:
            fb = pMovePool->pool_move_absolute_.pop();
            if (fb) {
                fb->setPool(&pMovePool->pool_move_absolute_);
                static_cast<FbMoveErasedTmpl<FbtMoveAbsolute>*>(fb)->get()->Axis = axisRef;
            }
            break;
        case FBM_MOVE_ADDITIVE:
            fb = pMovePool->pool_move_additive_.pop();
            if (fb) {
                fb->setPool(&pMovePool->pool_move_additive_);
                static_cast<FbMoveErasedTmpl<FbtMoveAdditive>*>(fb)->get()->Axis = axisRef;
            }
            break;
        case FBM_MOVE_VELOCITY:
            fb = pMovePool->pool_move_velocity_.pop();
            if (fb) {
                fb->setPool(&pMovePool->pool_move_velocity_);
                static_cast<FbMoveErasedTmpl<FbtMoveVelocity>*>(fb)->get()->Axis = axisRef;
            }
            break;
        default:
            printf("AxisMotbufFb: allocateFb: unknown command type: %d\n", cmdType);
            return nullptr;
    }
    if (!fb) {
        printf("AxisMotbufFb: allocateFb: pool is empty for command type: %d\n", cmdType);
    }
    return fb;
}

void AxisMotbufFb::releaseFb(FbMoveErasedBase* fb, void* pool) {
    if (!fb || !pool) {
        return;
    }
    // Match AxisWithFb reclaim cast: FbMoveErasedWrapper + fixed pool capacity.
    typedef FbMoveErasedTmpl<FbtMoveRelative> FbMoveErasedWrapper;
    auto* typedPool =
        static_cast<ObjectPoolAllocator<FbMoveErasedWrapper, FbMovePool::POOL_SIZE>*>(pool);
    typedPool->push((FbMoveErasedWrapper*)fb);
}

}  // namespace zmc
