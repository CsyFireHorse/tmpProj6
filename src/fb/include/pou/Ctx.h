#ifndef _ZMC_CTX_H_
#define _ZMC_CTX_H_

#include <vector>

#include "AxisRef.h"
#include "io/IoManager.hpp"
#include "pou/AxisManager.h"

namespace zmc {

// Resource context for motbuf/runtime (axes, IO, and later resources).
// POU subclass owns Ctx; consumers hold a non-owning pointer.
class Ctx {
   public:
    Ctx() : axArr_(&SgAxisArr::getInstance()) {}

    // Non-owning. nullptr resets to SgAxisArr (axis manager).
    void set_axis_arr(const std::vector<AXIS_REF>* arr) {
        axArr_ = arr ? arr : &SgAxisArr::getInstance();
    }

    AXIS_REF getAxisByIndex(uint32_t index) const {
        if (!axArr_ || index >= axArr_->size()) {
            return nullptr;
        }
        return (*axArr_)[index];
    }

    uint32_t getAxisCount() const {
        return axArr_ ? static_cast<uint32_t>(axArr_->size()) : 0u;
    }

    IoManager& io() { return io_; }
    const IoManager& io() const { return io_; }

   private:
    const std::vector<AXIS_REF>* axArr_;
    IoManager io_;
};

}  // namespace zmc

#endif  // _ZMC_CTX_H_
