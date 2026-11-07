/*
 * @Author: ChenShengyong
 * @Date: 2022-10-21 13:07:43
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-04-29 11:23:49
 */


#include "spline_def.h"

struct Cubic_poly {
    void init(const double* xx);
    inline double val(double t) const {
        return ((dat[0] * t + dat[1]) * t + dat[2]) * t + dat[3];
    };
    inline double diff(double t) const { return (3 * dat[0] * t + 2 * dat[1]) * t + dat[2]; };
    inline double ddiff(double t) const { return 6 * dat[0] * t + 2 * dat[1]; };
    inline void to_bezier(double* arr) const {
        arr[0] = dat[3] - 3 * dat[2] + 3 * dat[1] - dat[0];
        arr[1] = 3 * dat[2] - 6 * dat[1] + 3 * dat[0];
        arr[2] = 3 * dat[1] - 3 * dat[0];
        arr[3] = dat[0];
    }
    inline void from_bezier(const double* arr) {
        dat[0] = arr[3];
        dat[1] = arr[2] / 3 + arr[3];
        dat[2] = (arr[1] + arr[2] * 2 + arr[3] * 3) / 3;
        dat[3] = arr[0] + arr[1] + arr[2] + arr[3];
    }
    double dat[4];
};

class CubicSpline {
    int N;
    int mode;
    double* tt;
    double* yy;
    // double pp[100][4];
    double (*pp)[4];
    double pts[2];

   public:
    CubicSpline(int len, const double* tls, const double* yls, const double* pts, int mode);
    ~CubicSpline();
    double get_pos(double tm);
    double get_velocity(double tm);
    double get_acc(double tm);
    void get_pos_vel_acc(double tm, double* pos, double* vel, double* acc) const;
    int find_idx(double tm)const;

   private:
    void _build();
};

// resample
int spline_smooth(int type, double* arr, int len, double* out, double ratio);
