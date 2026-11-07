#include "TrajCppWrap.h"
#include "mc_traj_api.h"

using namespace zmc;

mc_traj_handle mc_traj_new(void) { return new TrajWrap(); }

void mc_traj_delete(mc_traj_handle handle) { delete (TrajWrap *)handle; }

uint8_t mc_traj_get_axis_num(mc_traj_handle handle) {
    return ((TrajWrap *)handle)->get_axis_num();
}

int mc_traj_init_axis_num(mc_traj_handle handle, uint8_t axisNum) {
    return ((TrajWrap *)handle)->init_axis_num(axisNum);
}

uint8_t mc_traj_get_traj_type(mc_traj_handle handle) {
    return ((TrajWrap *)handle)->get_traj_type();
}

void mc_traj_set_begin_pos(mc_traj_handle handle, const double *pos) {
    ((TrajWrap *)handle)->set_begin_pos(pos);
}

void mc_traj_set_sg_param(mc_traj_handle handle, double pos_or_dist, bool is_relative,
                          bool no_end) {
    ((TrajWrap *)handle)->set_sg_param(pos_or_dist, is_relative, no_end);
}

void mc_traj_set_line_param(mc_traj_handle handle, const double *pos_or_diff_arr,
                            bool is_relative) {
    ((TrajWrap *)handle)->set_line_param(pos_or_diff_arr, is_relative);
}

void mc_traj_set_circ_param(mc_traj_handle handle, const double *aux_point,
                            const double *end_point, bool path_choice, bool is_relative,
                            SMC_CIRC_MODE mode) {
    ((TrajWrap *)handle)->set_circ_param(aux_point, end_point, path_choice, is_relative, mode);
}

int mc_traj_build(mc_traj_handle handle) { return ((TrajWrap *)handle)->build(); }

void mc_traj_get_begin_pos(mc_traj_handle handle, double *pos) {
    ((TrajWrap *)handle)->get_begin_pos(pos);
}

void mc_traj_get_end_pos(mc_traj_handle handle, double *pos) {
    ((TrajWrap *)handle)->get_end_pos(pos);
}

double mc_traj_get_plan(mc_traj_handle handle) { return ((TrajWrap *)handle)->get_plan(); }

void mc_traj_get_slice(mc_traj_handle handle, double d, double *abs_pos, double *dif_pos) {
    ((TrajWrap *)handle)->get_slice(d, abs_pos, dif_pos);
}

void mc_traj_get_slice2(mc_traj_handle handle, double d, double v, double *abs_pos,
                        double *dif_pos) {
    ((TrajWrap *)handle)->get_slice2(d, v, abs_pos, dif_pos);
}

void mc_traj_get_slice3(mc_traj_handle handle, double d, double v, double a, double *abs_pos,
                        double *dif_pos, double *dif2_pos) {
    ((TrajWrap *)handle)->get_slice3(d, v, a, abs_pos, dif_pos, dif2_pos);
}

// helper functions for double arrays
double *mc_new_doubleArray(uint32_t nelements) {
    return (new double[nelements]());
}

void mc_delete_doubleArray(double *ary) {
    delete[] ary;
}

double mc_doubleArray_getitem(double *ary, uint32_t index) {
    return ary[index];
}

void mc_doubleArray_setitem(double *ary, uint32_t index, double value) {
    ary[index] = value;
}