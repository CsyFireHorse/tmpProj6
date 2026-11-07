#ifndef MC_TRAJ_API_H
#define MC_TRAJ_API_H

#include "TrajEnumDef.h"
#include "zmc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *mc_traj_handle;

ZMC_DLLPORT mc_traj_handle mc_traj_new(void);
ZMC_DLLPORT void mc_traj_delete(mc_traj_handle handle);

ZMC_DLLPORT uint8_t mc_traj_get_axis_num(mc_traj_handle handle);
ZMC_DLLPORT uint8_t mc_traj_get_traj_type(mc_traj_handle handle);

ZMC_DLLPORT int mc_traj_init_axis_num(mc_traj_handle handle, uint8_t axisNum);

ZMC_DLLPORT void mc_traj_set_begin_pos(mc_traj_handle handle, const double *pos);
ZMC_DLLPORT void mc_traj_set_sg_param(mc_traj_handle handle, double pos_or_dist, bool is_relative,
                                      bool no_end);
ZMC_DLLPORT void mc_traj_set_line_param(mc_traj_handle handle, const double *pos_or_diff_arr,
                                        bool is_relative);
ZMC_DLLPORT void mc_traj_set_circ_param(mc_traj_handle handle, const double *aux_point,
                                        const double *end_point, bool path_choice,
                                        bool is_relative, SMC_CIRC_MODE mode);

ZMC_DLLPORT int mc_traj_build(mc_traj_handle handle);
ZMC_DLLPORT void mc_traj_get_begin_pos(mc_traj_handle handle, double *pos);
ZMC_DLLPORT void mc_traj_get_end_pos(mc_traj_handle handle, double *pos);
ZMC_DLLPORT double mc_traj_get_plan(mc_traj_handle handle);
ZMC_DLLPORT void mc_traj_get_slice(mc_traj_handle handle, double d, double *abs_pos,
                                   double *dif_pos);
ZMC_DLLPORT void mc_traj_get_slice2(mc_traj_handle handle, double d, double v, double *abs_pos,
                                    double *dif_pos);
ZMC_DLLPORT void mc_traj_get_slice3(mc_traj_handle handle, double d, double v, double a,
                                    double *abs_pos, double *dif_pos, double *dif2_pos);

/* helper functions for double arrays (for SWIG / wrappers) */
ZMC_DLLPORT double *mc_new_doubleArray(uint32_t nelements);
ZMC_DLLPORT void mc_delete_doubleArray(double *ary);
ZMC_DLLPORT double mc_doubleArray_getitem(double *ary, uint32_t index);
ZMC_DLLPORT void mc_doubleArray_setitem(double *ary, uint32_t index, double value);

#ifdef __cplusplus
}
#endif

#endif  // MC_TRAJ_API_H
