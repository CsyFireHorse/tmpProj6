#ifndef _ZMC_FBT_AXIS_LOG_H_
#define _ZMC_FBT_AXIS_LOG_H_
#include "axLogFactory.h"
#include "fbt_axis_base.h"

namespace zmc {

template <typename T>
class AxisImplLog : public T {
   public:
    void installLogger(AxLoggerType type, const std::string& filename = "") {
        this->setLogger(AxLoggerFactory::createLogger(type, filename));
    }
    void runCycleLog() {
        if (this->is_ready_to_print() && this->getLogger()) {
            AxisFrame posFrm = {T::m_cycleCounter, T::wAxisStructID,  T::axisStat,
                                 T::get_current_time(),       T::fActPosition,  T::fActVelocity,  T::fActAcceleration,  T::fSetPosition,
                                T::fSetVelocity,   T::fSetAcceleration};
            this->getLogger()->write(posFrm);
        }
    }
    void log(int level, const char* fmt) {
        if (this->getLogger()) {
            this->getLogger()->log(level, fmt);
        }
    }
};

}  // namespace zmc
#endif