#ifndef _ZMC_FBT_AXIS_TRANSFORM_H_
#define _ZMC_FBT_AXIS_TRANSFORM_H_

#include "fbt_common.h"
#include <cmath>
#include "zmc_fb_predef.h"

namespace zmc {

template<typename Base>
class AxTransformLayer : public Base {
public:
    LREAL fPositionPeriod = ZMC_AXIS_POSITION_PERIOD_DEFAULT;
    int iRatioTechUnitsNum = 1;
    uint32_t dwRatioTechUnitsDenom = 1;
    double fOffsetPosition = 0;
    int32_t dwPosOffsetForResiduals = 0;
    float fPosRes = 0;
    uint32_t dwLastPosition = 0;
    int iTurn = 0;

    int32_t diSetPosition = 0;
    int32_t diActPosition = 0;
    int32_t diSetVelocity = 0;
    int32_t diActVelocity = 0;
    int32_t diSetAcceleration = 0;
    int32_t diActAcceleration = 0;
    int32_t diSetCurrent = 0;
    int32_t diActCurrent = 0;
    int32_t diSetTorque = 0;
    int32_t diActTorque = 0;

    inline double getPosRes() const {
        #if !ZMC_AXIS_USE_POS_RES
        // if constexpr(!ZMC_AXIS_USE_POS_RES)
            return 0;
        #endif
        return (diActPosition == diSetPosition) ? fPosRes:0;
    }
    template<typename T>
    inline double scale_di2set(T vel) const {
        return ((double)vel) / iRatioTechUnitsNum * dwRatioTechUnitsDenom;
    }
    template<typename T>
    inline double scale_set2di(T vel) const {
        return ((double)vel) * iRatioTechUnitsNum / dwRatioTechUnitsDenom;
    }

    inline double calc_modulo(double v) const {
        if (!this->is_modulo_type()) return v;
        double v2 = std::fmod(v, fPositionPeriod);
        return v2 < 0 ? (v2 + fPositionPeriod) : v2;
    }
    inline double calc_zero_point(double v) const {
        if (!this->is_modulo_type()) return 0;
        return calc_modulo(v) - v;
    }
};

} // namespace zmc

#endif // _ZMC_FBT_AXIS_TRANSFORM_H_ 