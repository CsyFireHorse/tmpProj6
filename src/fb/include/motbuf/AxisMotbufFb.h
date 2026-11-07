/*
 * Axis function-block motbuf declarations.
 * AI modification: GPT-5 Codex
 */
#ifndef ZMC_AXIS_MOTBUF_FB_H
#define ZMC_AXIS_MOTBUF_FB_H

#include <cstddef>
#include <vector>

#include "AxisMotbufBase.h"
#include "AxisWithFb.hpp"

namespace zmc {

class Ctx;
class FbMovePool;
class FbMoveWrapWithAxis;

class AxisMotbufFb : public AxisMotbufBase {
   public:
    // Create the axis motbuf owned by one POU wrapper.
    AxisMotbufFb(FbMoveWrapWithAxis* parent, int axNum = 3);

    // Clear all axis FBs before releasing the shared FB object pool.
    ~AxisMotbufFb();

    // Store the runtime context and rebind all configured axis slots.
    void setCtx(const Ctx* ctx);

    // Reserve axis storage without changing the configured axis count.
    void reserve(int cap);

    // Resize and rebind the axis slots; return an error code for invalid input.
    int resize(int axNum);

    // Reset all slots and return the fixed motion-FB pools to their initial state.
    void reset_runtime();

    // Return the number of configured axis slots.
    std::size_t size() const;

    // Return mutable access to one axis FB slot.
    AxisWithFb& axis(int index);

    // Return read-only access to one axis FB slot.
    const AxisWithFb& axis(int index) const;

    // Return the bound axis reference for one slot.
    AXIS_REF axisRef(int index) const;

    // Bind an explicit axis reference to one slot.
    void bindAxis(AXIS_REF axisRef, int index);

    // Process the command queues of all axis FB slots.
    void runCycleTop();

    // Execute the function blocks of all axis FB slots.
    void runCycleBottom();

    // Execute both phases of the axis motbuf.
    void runCycle();

   private:
    friend class AxisWithFb;

    // Set the wrapper owner, logical index, and runtime axis of one slot.
    void bindAxisSlot(int index);

    // Allocate and bind a motion FB from the shared object pool.
    FbMoveErasedBase* allocateFb(int cmdType, AXIS_REF axisRef);

    // Return one motion FB to the pool recorded by setPool during allocateFb.
    void releaseFb(FbMoveErasedBase* fb, void* pool);

    FbMoveWrapWithAxis* parent_;
    std::vector<AxisWithFb> axFbArr;
    FbMovePool* pMovePool;
};

}  // namespace zmc

#endif
