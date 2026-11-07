#include "SplineWrap.h"
#include "SplineCppWrap.h"

using namespace zmc;

zmc_spline_t zmc_spline_new() {
    return reinterpret_cast<zmc_spline_t>(new zmc::SplineCppWrap());
}

void zmc_spline_delete(zmc_spline_t handle) {
    delete reinterpret_cast<zmc::SplineCppWrap*>(handle);
}

void zmc_spline_set_xx_yy(zmc_spline_t handle, int len, 
                         const double* tArr, const double* pArr) {
    auto obj = reinterpret_cast<zmc::SplineCppWrap*>(handle);
    obj->set_xx_yy(len, tArr, pArr);
}

void zmc_spline_set_xx_yy_vv(zmc_spline_t handle, int len,
                            const double* tArr, const double* pArr, 
                            const double* vArr) {
    auto obj = reinterpret_cast<zmc::SplineCppWrap*>(handle);
    obj->set_xx_yy_vv(len, tArr, pArr, vArr);
}

int zmc_spline_build(zmc_spline_t handle) {
    auto obj = reinterpret_cast<zmc::SplineCppWrap*>(handle);
    return obj->build();
}

double zmc_spline_get_pos(zmc_spline_t handle, double t) {
    auto obj = reinterpret_cast<zmc::SplineCppWrap*>(handle);
    return obj->get_pos(t);
}

void zmc_spline_set_default_allocator(zmc_spline_t handle) {
    auto obj = reinterpret_cast<zmc::SplineCppWrap*>(handle);
    obj->set_default_allocator();
}

void zmc_spline_set_condition_value(zmc_spline_t handle, 
    const double* values, int len) 
{
    auto obj = reinterpret_cast<zmc::SplineCppWrap*>(handle);
    obj->set_condition_value(values, len);
}

void zmc_spline_set_condition_type(zmc_spline_t handle, int cond_type) {
    auto obj = reinterpret_cast<zmc::SplineCppWrap*>(handle);
    obj->set_condition_type(cond_type);
}

void zmc_spline_set_ptype(zmc_spline_t handle, int spline_type) {
    auto obj = reinterpret_cast<zmc::SplineCppWrap*>(handle);
    obj->set_ptype(static_cast<SPLINE_TYPE>(spline_type));
}

void zmc_spline_get_pos_vel_acc(zmc_spline_t handle, double tm,
    double* pos, double* vel, double* acc) 
{
    auto obj = reinterpret_cast<zmc::SplineCppWrap*>(handle);
    obj->get_pos_vel_acc(tm, pos, vel, acc);
}