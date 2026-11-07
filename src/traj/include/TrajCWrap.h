/*
 * @Author: ChenShengyong
 * @Date: 2024-11-28 10:40:05
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-04-29 15:22:00
 */
#ifndef ZMC_TRAJ_C_WRAP_H_
#define ZMC_TRAJ_C_WRAP_H_

#include "TrajEnumDef.h"
#include "zmc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *traj_handle;

ZMC_DLLPORT traj_handle traj_new(void);
ZMC_DLLPORT void traj_delete(traj_handle handle);
// ZMC_DLLPORT traj_handle traj_copy(traj_handle handle);
ZMC_DLLPORT uint8_t traj_get_axis_num(traj_handle handle);
ZMC_DLLPORT uint8_t traj_get_traj_type(traj_handle handle);
ZMC_DLLPORT int traj_init_axis_num(traj_handle handle, uint8_t axisNum);
ZMC_DLLPORT void traj_set_begin_pos(traj_handle handle, const double *pos);

ZMC_DLLPORT void traj_set_sg_param(traj_handle impl, double pos_or_dist, bool is_relative,
                                   bool no_end);

ZMC_DLLPORT void traj_set_line_param(traj_handle impl, const double *pos_or_diff_arr,
                                     bool is_relative);

ZMC_DLLPORT void traj_set_circ_param(traj_handle impl, const double *AuxPoint,
                                     const double *EndPoint, bool PathChoice, bool is_relative,
                                     SMC_CIRC_MODE mode);

ZMC_DLLPORT int traj_build(traj_handle impl);
ZMC_DLLPORT void traj_get_begin_pos(traj_handle impl, double *pos);
ZMC_DLLPORT void traj_get_end_pos(traj_handle impl, double *pos);

ZMC_DLLPORT double traj_get_plan(traj_handle impl);

ZMC_DLLPORT void traj_get_slice(traj_handle impl, double d, double *absPos, double *difPos);
ZMC_DLLPORT void traj_get_slice2(traj_handle impl, double d, double v, double *absPos,
                                 double *difPos);
ZMC_DLLPORT void traj_get_slice3(traj_handle impl, double d, double v, double a,
                                 double *absPos, double *difPos, double *dif2Pos);

#ifdef __cplusplus
}
#endif

#endif  // VEL_C_WRAP_H_
        /* end of file */