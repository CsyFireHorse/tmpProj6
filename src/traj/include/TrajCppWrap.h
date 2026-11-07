/*
 * @Author: ChenShengyong
 * @Date: 2024-05-16 17:49:06
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2026-08-19 17:30:00
 */
#ifndef ZS_TRAJ_CPP_WRAP_H_
#define ZS_TRAJ_CPP_WRAP_H_

#include "stdint.h"
#include "TrajEnumDef.h"
namespace zmc {

class TrajWrap {
   public:
    TrajWrap(uint8_t axisNum=1);
    TrajWrap(const TrajWrap& other);
    TrajWrap& operator=(const TrajWrap& other);
    ~TrajWrap();
    int init_axis_num(uint8_t axisNum);
    void set_allocator(void* pAlloc);
    void set_default_allocator();
    void set_sg_param(double pos_or_dist, bool is_relative, bool no_end=false);
    void set_line_param(const double *pos_or_diff_arr, bool is_relative);
    void set_circ_param(const double *AuxPoint, const double *EndPoint, bool PathChoice,
                        bool is_relative, SMC_CIRC_MODE mode);
    void set_begin_pos(const double *pos);
    int build();

    uint8_t get_axis_num() const;
    uint8_t get_traj_type() const;
    void get_begin_pos(double *pos) const;
    void get_end_pos(double *) const;
    double get_plan() const;
    void get_slice(double d, double *pos, double *difPos) const;
    void get_slice2(double d, double v, double *pos, double *difPos) const;
    void get_slice3(double d, double v, double a,  double *pos, double *difPos, double *dif2Pos) const;

    double get_sg_direct() const;
private:
    // TrajUn includes trajectory implementations; keep this large enough for future extensions.
    static constexpr int STORAGE_SIZE = 160;
    char storage_[STORAGE_SIZE];
};

}  // namespace zmc

#endif  // ZS_TRAJ_CPP_WRAP_H_
        /* end of file */