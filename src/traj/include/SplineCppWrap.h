/*
 * @Author: ChenSy
 * @Date: 2024-08-12 9:42:25
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-04-29 16:17:56
 */
#ifndef _SPLINE_CPP_WRAP_H_
#define _SPLINE_CPP_WRAP_H_

#include "stdint.h"
#include "vel/VelWrapCommon.h"
#include "spline_def.h"

namespace zmc {

class PoliFitterWrap {
   public:
    PoliFitterWrap();
    ~PoliFitterWrap();

    void set_allocator(void*);
    void set_default_allocator();
    void* get_allocator();
    
    void set_xx_yy(int len, const double* tArr, const double* pArr);
    void set_xx_yy_vv(int len, const double* tArr, const double* pArr, const double* vArr);
    void set_xx_yy_vv_aa(int len, const double* tArr, const double* pArr, const double* vArr,
                         const double* aArr);
    void set_xy(int len, const double* tArr);
    void set_xyva(int len, const double*);
    void set_ptype(SPLINE_TYPE);
    void set_condition_type(int); // ENUM_SPLINE_COND
    void set_condition_value(const double*, int len);
    void set_condition_type();
    
    int build();
    double get_pos(double) const;
    double get_vel(double) const;
    double get_acc(double) const;
    void get_pos_vel_acc(double tm, double* pos, double* vel, double* acc) const;
    void get_pva(double tm, PosVelAcc*) const;
   private:
    static constexpr int STORAGE_SIZE = 256;
    char storage_[STORAGE_SIZE]; // 
};

typedef PoliFitterWrap SplineCppWrap;
}  // namespace zmc
#endif