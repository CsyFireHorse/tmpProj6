#ifndef MC_VEL_API_H
#define MC_VEL_API_H

#include "zmc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *mc_vel_handle;

ZMC_DLLPORT mc_vel_handle mc_vel_new(void);
ZMC_DLLPORT void mc_vel_delete(mc_vel_handle handle);

ZMC_DLLPORT void mc_vel_set_config(mc_vel_handle handle, double vm, double acc, double dec,
                                   double jerk);
ZMC_DLLPORT void mc_vel_set_begin_pva(mc_vel_handle handle, double s0, double v0, double a0);
ZMC_DLLPORT void mc_vel_set_end_pva(mc_vel_handle handle, double sd, double vd, double ad);

ZMC_DLLPORT int mc_vel_build(mc_vel_handle handle);
ZMC_DLLPORT double mc_vel_get_end_time(mc_vel_handle handle);
ZMC_DLLPORT int mc_vel_get_slice(mc_vel_handle handle, double time, double *pos, double *vel,
                                 double *acc);

#ifdef __cplusplus
}
#endif

#endif  // MC_VEL_API_H
