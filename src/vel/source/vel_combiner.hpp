#ifndef _ZMC_VEL_COMBINER_HPP_
#define _ZMC_VEL_COMBINER_HPP_

#include <deque>
#include <list>
#include <vector>

#include "common/misc/zal_utils.h"

namespace zmc {

template <typename T>
class VelCombiner {
   public:
    void set_config(double vm, double acc, double dec, double jerk) {
        V_m = vm;
        A_m = acc;
        A_n = dec;
        J = jerk;
    }
    inline int size() const { return _vels.size(); }
    // 复制并尾部添加
    inline void append(const T& v) { _vels.push_back(v); }

    T& operator[](int idx) { return _vels[idx]; };
    const T& operator[](int idx) const { return _vels[idx]; };
    inline const T& get_vel(int idx) const { return _vels[idx]; };
    inline T& get_vel(int idx) { return _vels[idx]; };

    int find_lower_index(double tm) const { return findGroups(ctArr, tm); }
    int abstime_to_reltime(double& tm) const {
        int fd = find_lower_index(tm);
        tm -= ctArr[fd];
        return fd;
    }

    inline int build_from(int idx) {
        int errCode = 0;
        for (int i = 0; i < size(); i++) {
            errCode |= get_vel(i).build();
        }
        _build_idx(0);
        return errCode;
    }
    void lookahead_vel() {
        int N = size();
        for (int i = 0; i < N; i++) {
            // get_vel(i)
        }
    }
    inline int build() { return build_from(_cur_idx); }
    // 更新第 idx个点
    void _build_idx(int idx = 0) {
        int N = size();
        ctArr.resize(N + 1);
        ctArr[0] = _cur_abstime;
        double ct = _cur_abstime;
        for (int i = 0; i < N; i++) {
            ct += get_vel(i).get_end_time();
            ctArr[i + 1] = ct;
        }
    }
    void _build_idx0(int idx = 0) {
        int N = _vels.size();
        ctArr.resize(N + 1);
        int i = 0;
        double ct = 0.;
        if (idx <= 0) {
            ctArr[0] = 0.;
        } else {
            i = idx - 1;
            ct = ctArr[idx - 1];
        }
        for (; i < N; i++) {
            ct += _vels[i].get_end_time();
            ctArr[i + 1] = ct;
        }
    }

    inline double get_end_time() const { return ctArr[ctArr.size() - 1]; };

    int get_slice(double tm, double* position, double* velocity, double* acc) const {
        if (tm <= 0) {
            _vels[0].get_slice(tm, position, velocity, acc);
            return 0;
        }
        int fd = abstime_to_reltime(tm);
        _vels[fd].get_slice(tm, position, velocity, acc);
        // if (position) position[0] += csArr[fd];
        return fd;
    }

    // int chop_left_time(int fd, double tm) { _vels[fd].chop_left_time(tm); }
    void timeTo(double tm) {
        _cur_abstime = tm;
        _cur_idx = abstime_to_reltime(tm);
        _vels[_cur_idx].chop_left_time(tm);
        for (int i = 0; i < _cur_idx; i++) {
            _vels.pop_front();
        }
        _cur_idx = 0;
    }
    void append_tail(double dist) {
        int N = _vels.size();
        _vels.resize(_vels.size() + 1);
        T& vel = _vels[_vels.size() - 1];
        vel.set_config(V_m, A_m, A_n, J);
        vel.set_begin_end_pos(0, dist);
    }
    int insert_stop() {
        _vels.resize(_vels.size() + 1);
        // _vels(_vels.size() - 1).set_begin_end_pos(0, dist);
    }

   private:
    double V_m;
    double A_m;
    double A_n;
    double J;
    double _cur_abstime = 0;
    int _cur_idx = 0;
    double _cur_reltime = 0;
    std::deque<T> _vels;
    std::vector<double> ctArr;
    std::vector<double> csArr;
};

}  // namespace zmc

#endif  // _ZMC_VELOCITY_COMBINER_H_
/* end of file */
