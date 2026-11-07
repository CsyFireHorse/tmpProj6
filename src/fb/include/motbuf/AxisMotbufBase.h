#ifndef ZMC_AXIS_MOTBUF_BASE_H
#define ZMC_AXIS_MOTBUF_BASE_H

#include "MotCmd.hpp"

namespace zmc {

class Ctx;

class AxisMotbufBase {
   public:
    // Hold a read-only runtime context owned by the POU layer.
    void setCtx(const Ctx* ctx) { ctx_ = ctx; }
    const Ctx* ctx() const { return ctx_; }

    int pushCommand(const CommandStructDefault& cmd) { return 0; };
    // process command queue
    void runCycleTop() {};
    // execute fb.runCycle()
    void runCycleBottom() {};
    void reset_runtime() {};

   private:
    const Ctx* ctx_ = nullptr;
};

}  // namespace zmc

#endif
