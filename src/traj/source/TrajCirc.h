/*
 * @Author: ChenShengyong
 * @Date: 2024-11-27 16:27:36
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-11-28 10:36:51
 */
#ifndef _TRAJ_CIRC_H_
#define _TRAJ_CIRC_H_

#include "TrajBase.h"
#include "math.h"
#include "stdint.h"

namespace zmc {

    
class TrajCirc : public TrajBase3d {
   public:
    TrajCirc(uint8_t axisNum = 1) : TrajBase3d(axisNum) {};
    ~TrajCirc() = default;
    void set_circ_param(const double *AuxPoint, const double *EndPoint, bool PathChoice,
                        bool is_relative, SMC_CIRC_MODE mode);
    int build();
    void get_slice(double d, double *pos, double *difPos) const;
    void get_slice2(double d, double v, double *pos, double *difPos) const;
    void get_slice3(double d, double v, double a, double *pos, double *difPos, double *dif2Pos) const;

   private:
    int _build_border();
    int _build_center();  // TODO
    int _build_radius();  // TODO

    // Keep TrajUn small: store heavy cache on heap.
    struct CircCache;
    inline CircCache* cc_() const { return reinterpret_cast<CircCache*>(get_cache()); }
};
}  // namespace zmc
#endif
