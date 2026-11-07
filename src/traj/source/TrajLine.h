/*
 * @Author: ChenShengyong
 * @Date: 2024-11-27 16:27:36
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-11-28 11:07:28
 */
#ifndef _TRAJ_LINE_H_
#define _TRAJ_LINE_H_

#include "TrajBase.h"
#include "math.h"
#include "stdint.h"

namespace zmc {

class TrajLine : public TrajBase3d {
   public:
    // Construct a line trajectory with the specified axis count.
    TrajLine(uint8_t axisNum = 1) : TrajBase3d(axisNum) {}

    // Set the absolute endpoint or relative displacement.
    void set_line_param(const double *pos_or_diff_arr, bool is_relative);

    // Build the unit direction and Euclidean path length.
    int build();

    // Evaluate position and derivative with respect to path distance.
    void get_slice(double d, double *pos, double *difPos) const;

    // Evaluate position and per-axis velocity.
    void get_slice2(double d, double v, double *pos, double *difPos) const;

    // Evaluate position, velocity, and acceleration.
    void get_slice3(double d, double v, double a, double *pos, double *difPos,
                    double *dif2Pos) const;
};
}  // namespace zmc
#endif
