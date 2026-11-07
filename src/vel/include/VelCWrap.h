/*
 * @Author: ChenShengyong 
 * @Date: 2024-04-16 17:48:36 
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-03-27 18:22:02
 */
#ifndef VEL_C_WRAP_H_
#define VEL_C_WRAP_H_

#include "zmc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void * vel_handle;
// #define ZMC_DLLPORT

ZMC_DLLPORT vel_handle vel_new(void);
ZMC_DLLPORT void vel_delete(vel_handle handle);
ZMC_DLLPORT vel_handle vel_copy(vel_handle handle);

ZMC_DLLPORT void vel_set_config(vel_handle vel, double vm, double acc, double dec, double jerk);
ZMC_DLLPORT void vel_set_begin_pos_vel(vel_handle handle, double s0, double v0);
ZMC_DLLPORT void vel_set_end_pos_vel(vel_handle handle, double sd, double vd);
// ZMC_DLLPORT void vel_set_begin_end_acc(vel_handle handle, double a0, double ad);

ZMC_DLLPORT int vel_build(vel_handle handle);
ZMC_DLLPORT double vel_get_end_time(vel_handle handle);
// ZMC_DLLPORT void vel_shift_pos(vel_handle handle, double ofs);
// ZMC_DLLPORT int vel_get_plan(vel_handle handle, double* total_time, double* rVmax, double* rAcc, double* rDec);
ZMC_DLLPORT int vel_get_slice(vel_handle handle, double time, double* pos, double* vel, double* acc);

#ifdef __cplusplus
}
#endif

#endif  // VEL_C_WRAP_H_
/* end of file */