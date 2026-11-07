#ifndef _ZMC_AX_LOG_SPD_H_
#define _ZMC_AX_LOG_SPD_H_
#include <map>
#include <memory>
#include <string>

#include "axLogBase.hpp"
#include "stdio.h"

namespace zmc {
class SpdlogAxLogger : public AxLogger {
    public:
     void write(const AxisFrame& posFrm) override {
         // _INFO_("{},{},{},{},{},{},{},{},{}", posFrm.cnt, posFrm._m, posFrm.axis_state_,
         // posFrm.posActual, posFrm.velActual, posFrm.accActual, posFrm.pos, posFrm.vel, posFrm.acc);
     }
 };
}
#endif