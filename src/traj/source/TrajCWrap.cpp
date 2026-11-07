#if 1
/*
 * @Author: ChenShengyong
 * @Date: 2024-12-09 17:16:38
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-04-29 15:21:10
 */

#include "TrajCppWrap.h"
#include "TrajCWrap.h"

using namespace zmc;

traj_handle traj_new(void) { return new TrajWrap(); }
void traj_delete(traj_handle impl) { delete (TrajWrap *)impl; }
// traj_handle traj_copy(traj_handle handle);

uint8_t traj_get_axis_num(traj_handle impl) { return ((TrajWrap *)impl)->get_axis_num(); };

uint8_t traj_get_traj_type(traj_handle impl) { return ((TrajWrap *)impl)->get_traj_type(); };

int traj_init_axis_num(traj_handle impl, uint8_t axisNum) {
    return ((TrajWrap *)impl)->init_axis_num(axisNum);
}

void traj_set_begin_pos(traj_handle impl, const double *pos) {
    ((TrajWrap *)impl)->set_begin_pos(pos);
}

void traj_set_sg_param(traj_handle impl, double pos_or_dist, bool is_relative, bool no_end) {
    ((TrajWrap *)impl)->set_sg_param(pos_or_dist, is_relative, no_end);
}
void traj_set_line_param(traj_handle impl, const double *pos_or_diff_arr, bool is_relative) {
    ((TrajWrap *)impl)->set_line_param(pos_or_diff_arr, is_relative);
}

void traj_set_circ_param(traj_handle impl, const double *AuxPoint, const double *EndPoint,
                         bool PathChoice, bool is_relative, SMC_CIRC_MODE mode) {
    ((TrajWrap *)impl)->set_circ_param(AuxPoint, EndPoint, PathChoice, is_relative, mode);
}

int traj_build(traj_handle impl) { return ((TrajWrap *)impl)->build(); }

void traj_get_begin_pos(traj_handle impl, double *pos) {
    return ((TrajWrap *)impl)->get_begin_pos(pos);
};
void traj_get_end_pos(traj_handle impl, double *pos) { return ((TrajWrap *)impl)->get_end_pos(pos); }

double traj_get_plan(traj_handle impl) { return ((TrajWrap *)impl)->get_plan(); };

void traj_get_slice(traj_handle impl, double d, double *absPos, double *difPos) {
    return ((TrajWrap *)impl)->get_slice(d, absPos, difPos);
}

void traj_get_slice2(traj_handle impl, double d, double v, double *absPos, double *difPos) {
    return ((TrajWrap *)impl)->get_slice2(d, v, absPos, difPos);
}
#endif