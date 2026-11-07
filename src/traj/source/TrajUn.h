/*
 * @Author: ChenShengyong
 * @Date: 2024-03-28 14:41:18
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-12-12 17:24:35
 */
#ifndef _TRAJ_UN_H_
#define _TRAJ_UN_H_

#include "TrajCirc.h"
#include "TrajLine.h"
#include "TrajSg.h"
#include "math.h"

namespace zmc {

union TrajUn {
    TrajLine trajLine;
    TrajCirc trajCirc;
    TrajSg trajSg;

    TrajUn(uint8_t axisNum=1) : trajLine(axisNum) { }
    TrajUn(const TrajUn& other) : trajLine(other.trajLine) {}
    TrajUn& operator=(const TrajUn& other) {
        if (this != &other) {
            trajLine = other.trajLine;
        }
        return *this;
    }
    ~TrajUn();

    int init_axis_num(uint8_t axisNum);
    // TrajBase3d(uint32_t axisNum = 1) { _axisNum = axisNum; };
    inline uint8_t get_axis_num() const { return trajLine.get_axis_num(); };
    inline uint8_t get_traj_type() const { return trajLine.get_traj_type(); };

    inline void set_allocator(void* pAlloc) { trajLine.set_allocator(static_cast<PmAllocator*>(pAlloc)); }
    inline void set_default_allocator() { trajLine.set_default_allocator(); }

    void set_sg_param(double pos_or_dist, bool is_relative, bool no_end=false);
    void set_line_param(const double *pos_or_diff_arr, bool is_relative);
    void set_circ_param(const double *AuxPoint, const double *EndPoint, bool PathChoice,
                        bool is_relative, SMC_CIRC_MODE mode);
    inline void set_begin_pos(const double *pos) { return trajLine.set_begin_pos(pos); };

    int build();
    inline void get_begin_pos(double *pos) const { return trajLine.get_begin_pos(pos); };
    void get_end_pos(double *) const;
    inline double get_plan() const { return trajLine.get_plan(); };
    void get_slice(double d, double *pos, double *difPos) const;
    void get_slice2(double d, double v, double *pos, double *difPos) const;
    void get_slice3(double d, double v, double a,  double *pos, double *difPos, double *dif2Pos) const;

    double get_sg_direct() const;

    
};

}  // namespace zmc
#endif
