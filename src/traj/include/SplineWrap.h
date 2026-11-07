#ifndef SPLINE_CPP_WRAP_API_H
#define SPLINE_CPP_WRAP_API_H

#include "zmc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// 不透明指针类型
typedef struct ZmcSplineStruct ZmcSplineStruct;
typedef ZmcSplineStruct* zmc_spline_t;

// 对象生命周期管理
ZMC_DLLPORT zmc_spline_t zmc_spline_new();
ZMC_DLLPORT void zmc_spline_delete(zmc_spline_t handle);

// 核心功能接口
ZMC_DLLPORT void zmc_spline_set_xx_yy(zmc_spline_t handle, 
    int len, const double* tArr, const double* pArr);
    
ZMC_DLLPORT void zmc_spline_set_xx_yy_vv(zmc_spline_t handle, 
    int len, const double* tArr, const double* pArr, const double* vArr);

ZMC_DLLPORT void zmc_spline_set_xyva(zmc_spline_t handle,
    int len, const double* xyva);


ZMC_DLLPORT void zmc_spline_set_condition(zmc_spline_t handle,
    int cond_type, const double* values, int len);

ZMC_DLLPORT int zmc_spline_build(zmc_spline_t handle);

ZMC_DLLPORT double zmc_spline_get_pos(zmc_spline_t handle, double t);
ZMC_DLLPORT double zmc_spline_get_vel(zmc_spline_t handle, double t);
ZMC_DLLPORT double zmc_spline_get_acc(zmc_spline_t handle, double t);

// 新增接口声明
ZMC_DLLPORT void zmc_spline_set_default_allocator(zmc_spline_t handle);
ZMC_DLLPORT void zmc_spline_set_condition_value(zmc_spline_t handle, 
    const double* values, int len);
ZMC_DLLPORT void zmc_spline_set_condition_type(zmc_spline_t handle, int cond_type);
ZMC_DLLPORT void zmc_spline_set_ptype(zmc_spline_t handle, int spline_type);
ZMC_DLLPORT void zmc_spline_get_pos_vel_acc(zmc_spline_t handle, double tm,
    double* pos, double* vel, double* acc);

#ifdef __cplusplus
}
#endif

#endif // SPLINE_CPP_WRAP_API_H