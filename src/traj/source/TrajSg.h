/*
 * @Author: ChenShengyong
 * @Date: 2024-11-27 16:27:36
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-12-12 17:22:49
 */
#ifndef _TRAJ_SG_H_
#define _TRAJ_SG_H_
#include "TrajBase.h"
#include "math.h"
// #include "mc_motion_def.h"
#include "stdint.h"

namespace zmc {

class TrajSg : public TrajBase3d {
   public:
    void set_sg_param(double pos_or_dist, bool is_relative, bool no_end=false);
    int build();
    void get_slice(double d, double *pos, double *difPos) const;
    void get_slice2(double d, double v, double *pos, double *difPos) const;
    void get_slice3(double d, double v, double a, double *absPos, double *difPos, double *dif2Pos) const;
    double get_sg_direct() const;
};
}  // namespace zmc
#endif
