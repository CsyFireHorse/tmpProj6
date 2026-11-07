#ifndef _AXIS_LOG_BASE_HPP_
#define _AXIS_LOG_BASE_HPP_
#include <map>
#include <memory>
#include <string>

#include "stdio.h"

namespace zmc {

struct AxisFrame {
    uint16_t cnt;
    uint8_t axis_id;
    uint8_t axis_state_;
    double time;
    double posActual;
    double velActual;
    double accActual;
    double pos;
    double vel;
    double acc;
};

class AxLogger {
   public:
    virtual ~AxLogger() = default;
    virtual void write(const AxisFrame& posFrm) {}
    virtual void log(int level, const char* fmt) {}
};

}  // namespace zmc
#endif