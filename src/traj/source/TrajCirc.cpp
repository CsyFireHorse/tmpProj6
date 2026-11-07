/*
 * @Author: ChenShengyong
 * @Date: 2024-11-27 16:27:36
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-11-28 10:31:42
 */

#include "TrajCirc.h"
#include <new>

namespace zmc {

struct TrajCirc::CircCache {
    // bool isRelative = false;
    bool pathChoice = false;  // ignored for BORDER
    // uint8_t circMode = 0;
    double auxIn[3] = {0, 0, 0};
    double endIn[3] = {0, 0, 0};

    double radius = 0;
    double center2[2] = {0, 0};
    double theta0 = 0;
    double angleSpan = 0;
    int sign = 1;

    double ex[3] = {1, 0, 0};
    double ey[3] = {0, 1, 0};
    double ez[3] = {0, 0, 1};
};

void TrajCirc::set_circ_param(const double *AuxPoint, const double *EndPoint, bool PathChoice,
                              bool is_relative, SMC_CIRC_MODE mode) {
    _mode = is_relative ? TRAJ_TYPE_CIRC_REL : TRAJ_TYPE_CIRC_ABS;

    if (cc_() == nullptr || get_cache_size() < sizeof(TrajCirc::CircCache)) {
        allocate(sizeof(TrajCirc::CircCache));
    }
    if (cc_()) {
        cc_()->pathChoice = PathChoice;
        _sub_mode = mode;
        for (int i = 0; i < 3; i++) {
            cc_()->auxIn[i] = (AuxPoint && i < get_axis_num()) ? AuxPoint[i] : 0;
            cc_()->endIn[i] = (EndPoint && i < get_axis_num()) ? EndPoint[i] : 0;
        }
    }
    double *cmd = cmd_pos_data();
    if (cmd) {
        for (int i = 0; i < get_axis_num(); i++) {
            cmd[i] = EndPoint ? EndPoint[i] : 0;
        }
    }
}

namespace {

static inline double dot3(const double a[3], const double b[3]) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static inline void sub3(const double a[3], const double b[3], double out[3]) {
    out[0] = a[0] - b[0];
    out[1] = a[1] - b[1];
    out[2] = a[2] - b[2];
}

static inline void add3(const double a[3], const double b[3], double out[3]) {
    out[0] = a[0] + b[0];
    out[1] = a[1] + b[1];
    out[2] = a[2] + b[2];
}

static inline void mul3(const double a[3], double s, double out[3]) {
    out[0] = a[0] * s;
    out[1] = a[1] * s;
    out[2] = a[2] * s;
}

static inline void cross3(const double a[3], const double b[3], double out[3]) {
    out[0] = a[1] * b[2] - a[2] * b[1];
    out[1] = a[2] * b[0] - a[0] * b[2];
    out[2] = a[0] * b[1] - a[1] * b[0];
}

static inline double norm3(const double a[3]) {
    return sqrt(dot3(a, a));
}

static inline bool unit3(const double a[3], double out[3]) {
    double n = norm3(a);
    if (n < 1e-12)
        return false;
    out[0] = a[0] / n;
    out[1] = a[1] / n;
    out[2] = a[2] / n;
    return true;
}

static inline double angle_02pi(double x, double y) {
    double a = atan2(y, x);
    if (a < 0)
        a += 2.0 * (3.14159265358979323846);
    return a;
}

static inline double angle_diff_ccw(double start, double end) {
    const double two_pi = 2.0 * (3.14159265358979323846);
    double d = fmod(end - start, two_pi);
    if (d < 0)
        d += two_pi;
    if (d <= 0)
        d += two_pi;
    return d;
}

static inline bool angle_in_arc_ccw(double angle, double start, double arc_span) {
    const double two_pi = 2.0 * (3.14159265358979323846);
    double t = fmod(angle - start, two_pi);
    if (t < 0)
        t += two_pi;
    return (t > 1e-12) && (t < arc_span - 1e-12);
}

static inline bool circumcircle_2d(const double p1[2], const double p2[2], const double p3[2], double &cx, double &cy,
                                  double &r) {
    double x1 = p1[0], y1 = p1[1];
    double x2 = p2[0], y2 = p2[1];
    double x3 = p3[0], y3 = p3[1];
    double d = 2.0 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));
    if (fabs(d) < 1e-12)
        return false;
    double ux = ((x1 * x1 + y1 * y1) * (y2 - y3) + (x2 * x2 + y2 * y2) * (y3 - y1) +
                 (x3 * x3 + y3 * y3) * (y1 - y2)) /
                d;
    double uy = ((x1 * x1 + y1 * y1) * (x3 - x2) + (x2 * x2 + y2 * y2) * (x1 - x3) +
                 (x3 * x3 + y3 * y3) * (x2 - x1)) /
                d;
    cx = ux;
    cy = uy;
    r = sqrt((x1 - cx) * (x1 - cx) + (y1 - cy) * (y1 - cy));
    return r > 1e-12;
}

}  // namespace

int TrajCirc::_build_center() {
    if (cc_() == nullptr) {
        return ZMC_ERR_TRAJ_CACHE_INVALID;
    }
    if (CIRC_MODE_CENTER != _sub_mode) {
        return ZMC_ERR_TRAJ_INVALID_MODE;
    }

    const int n = get_axis_num();
    if (n != 2 && n != 3) {
        return ZMC_ERR_TRAJ_INVALID_AXIS_NUM;
    }

    const double eps = 1e-12;
    const double pi = 3.14159265358979323846;
    const double two_pi = 2.0 * pi;

    // Build absolute center and end positions.
    double ps[3] = {_beginPos[0], _beginPos[1], _beginPos[2]};
    double pc[3] = {cc_()->auxIn[0], cc_()->auxIn[1], cc_()->auxIn[2]};
    double pe[3] = {cc_()->endIn[0], cc_()->endIn[1], cc_()->endIn[2]};
    if (TRAJ_TYPE_CIRC_REL == _mode) {
        add3(ps, pc, pc);
        add3(ps, pe, pe);
    }

    // Project the programmed center onto the perpendicular-bisector plane.
    double chord[3];
    sub3(pe, ps, chord);
    const double chord_len = norm3(chord);
    if (chord_len < eps) {
        return ZMC_ERR_TRAJ_BUILD_FAILED;
    }
    double chord_dir[3];
    if (!unit3(chord, chord_dir)) {
        return ZMC_ERR_TRAJ_BUILD_FAILED;
    }
    double midpoint[3];
    add3(ps, pe, midpoint);
    mul3(midpoint, 0.5, midpoint);
    double center_offset[3];
    sub3(pc, midpoint, center_offset);
    const double projection_offset = dot3(center_offset, chord_dir);
    double projection[3];
    mul3(chord_dir, projection_offset, projection);
    sub3(pc, projection, pc);

    double start_radius[3];
    sub3(ps, pc, start_radius);
    const double radius = norm3(start_radius);
    if (radius < eps || fabs(projection_offset) > radius * 0.01) {
        return ZMC_ERR_TRAJ_BUILD_FAILED;
    }

    // Build the plane basis used by the shared circular interpolation code.
    if (n == 2) {
        cc_()->ex[0] = 1;
        cc_()->ex[1] = 0;
        cc_()->ex[2] = 0;
        cc_()->ey[0] = 0;
        cc_()->ey[1] = 1;
        cc_()->ey[2] = 0;
        cc_()->ez[0] = 0;
        cc_()->ez[1] = 0;
        cc_()->ez[2] = 1;
    } else {
        double end_radius[3];
        double normal[3];
        sub3(pe, pc, end_radius);
        cross3(start_radius, end_radius, normal);
        if (!unit3(normal, cc_()->ez) || !unit3(start_radius, cc_()->ex)) {
            return ZMC_ERR_TRAJ_BUILD_FAILED;
        }
        double tmp[3];
        cross3(cc_()->ez, cc_()->ex, tmp);
        if (!unit3(tmp, cc_()->ey)) {
            return ZMC_ERR_TRAJ_BUILD_FAILED;
        }
    }

    auto to2 = [&](const double p[3], double out2[2]) {
        double d3[3];
        sub3(p, ps, d3);
        out2[0] = dot3(d3, cc_()->ex);
        out2[1] = dot3(d3, cc_()->ey);
    };

    double center2[2];
    double end2[2];
    to2(pc, center2);
    to2(pe, end2);
    const double theta_s = angle_02pi(-center2[0], -center2[1]);
    const double theta_e = angle_02pi(end2[0] - center2[0], end2[1] - center2[1]);
    const double d_ccw = angle_diff_ccw(theta_s, theta_e);
    if (fabs(d_ccw - pi) < eps) {
        return ZMC_ERR_TRAJ_BUILD_FAILED;
    }

    cc_()->sign = cc_()->pathChoice ? 1 : -1;
    cc_()->angleSpan = cc_()->pathChoice ? d_ccw : two_pi - d_ccw;
    cc_()->radius = radius;
    cc_()->center2[0] = center2[0];
    cc_()->center2[1] = center2[1];
    cc_()->theta0 = theta_s;
    _distance = cc_()->radius * cc_()->angleSpan;
    return 0;
}

int TrajCirc::_build_radius() {
    // TODO: implement CIRC_MODE_RADIUS
    return ZMC_ERR_TRAJ_NOT_IMPLEMENTED;
}

int TrajCirc::_build_border() {
    if (cc_() == nullptr) {
        return ZMC_ERR_TRAJ_CACHE_INVALID;
    }
    if (CIRC_MODE_BORDER != _sub_mode) {
        return ZMC_ERR_TRAJ_INVALID_MODE;
    }

    const int n = get_axis_num();
    if (n != 2 && n != 3) {
        return ZMC_ERR_TRAJ_INVALID_AXIS_NUM;
    }

    // Build absolute Aux/End
    double ps[3] = {_beginPos[0], _beginPos[1], _beginPos[2]};
    double pa[3] = {cc_()->auxIn[0], cc_()->auxIn[1], cc_()->auxIn[2]};
    double pe[3] = {cc_()->endIn[0], cc_()->endIn[1], cc_()->endIn[2]};
    if (TRAJ_TYPE_CIRC_REL == _mode) {
        add3(ps, pa, pa);
        add3(ps, pe, pe);
    }

    // Build plane basis
    if (n == 2) {
        cc_()->ex[0] = 1;
        cc_()->ex[1] = 0;
        cc_()->ex[2] = 0;
        cc_()->ey[0] = 0;
        cc_()->ey[1] = 1;
        cc_()->ey[2] = 0;
        cc_()->ez[0] = 0;
        cc_()->ez[1] = 0;
        cc_()->ez[2] = 1;
    } else {
        double u[3], v[3], nn[3];
        sub3(pa, ps, u);
        sub3(pe, ps, v);
        cross3(u, v, nn);
        if (!unit3(nn, cc_()->ez)) {
            return ZMC_ERR_TRAJ_BUILD_FAILED;
        }
        if (!unit3(u, cc_()->ex)) {
            if (!unit3(v, cc_()->ex)) {
                return ZMC_ERR_TRAJ_BUILD_FAILED;
            }
        }
        double tmp[3];
        cross3(cc_()->ez, cc_()->ex, tmp);
        if (!unit3(tmp, cc_()->ey)) {
            return ZMC_ERR_TRAJ_BUILD_FAILED;
        }
    }

    auto to2 = [&](const double p[3], double out2[2]) {
        double d3[3];
        sub3(p, ps, d3);
        out2[0] = dot3(d3, cc_()->ex);
        out2[1] = dot3(d3, cc_()->ey);
    };

    double p1[2] = {0, 0};
    double p2[2], p3[2];
    to2(pa, p2);
    to2(pe, p3);

    double cx, cy, r;
    if (!circumcircle_2d(p1, p2, p3, cx, cy, r)) {
        return ZMC_ERR_TRAJ_BUILD_FAILED;
    }

    double theta_s = angle_02pi(p1[0] - cx, p1[1] - cy);
    double theta_a = angle_02pi(p2[0] - cx, p2[1] - cy);
    double theta_e = angle_02pi(p3[0] - cx, p3[1] - cy);
    double d_ccw = angle_diff_ccw(theta_s, theta_e);
    const double two_pi = 2.0 * (3.14159265358979323846);
    double d_cw = two_pi - d_ccw;

    if (angle_in_arc_ccw(theta_a, theta_s, d_ccw)) {
        cc_()->sign = 1;
        cc_()->angleSpan = d_ccw;
    } else {
        cc_()->sign = -1;
        cc_()->angleSpan = d_cw;
    }

    cc_()->radius = r;
    cc_()->center2[0] = cx;
    cc_()->center2[1] = cy;
    cc_()->theta0 = theta_s;
    _distance = cc_()->radius * cc_()->angleSpan;
    return 0;
}

int TrajCirc::build() {
    // reuse TrajBase3d member: _sub_mode = SMC_CIRC_MODE
    switch (_sub_mode) {
        case CIRC_MODE_BORDER:
            return _build_border();
        case CIRC_MODE_CENTER:
            return _build_center();
        case CIRC_MODE_RADIUS:
            return _build_radius();
        default:
            break;
    }
    return ZMC_ERR_TRAJ_INVALID_MODE;
}

void TrajCirc::get_slice(double d, double *absPos, double *difPos) const {
    if (d < 0)
        d = 0;
    else if (d > _distance)
        d = _distance;
    if (cc_() == nullptr || cc_()->radius <= 1e-12) {
        if (absPos) {
            for (uint32_t i = 0; i < get_axis_num(); i++) {
                absPos[i] = _beginPos[i];
            }
        }
        if (difPos) {
            for (uint32_t i = 0; i < get_axis_num(); i++) {
                difPos[i] = 0;
            }
        }
        return;
    }

    double theta = cc_()->theta0 + cc_()->sign * (d / cc_()->radius);
    double cs = cos(theta);
    double sn = sin(theta);
    double px = cc_()->center2[0] + cc_()->radius * cs;
    double py = cc_()->center2[1] + cc_()->radius * sn;

    double tx, ty;
    if (cc_()->sign == 1) {
        tx = -sn;
        ty = cs;
    } else {
        tx = sn;
        ty = -cs;
    }

    if (absPos) {
        // pos = begin + ex*px + ey*py
        absPos[0] = _beginPos[0] + cc_()->ex[0] * px + cc_()->ey[0] * py;
        absPos[1] = _beginPos[1] + cc_()->ex[1] * px + cc_()->ey[1] * py;
        if (get_axis_num() == 3) {
            absPos[2] = _beginPos[2] + cc_()->ex[2] * px + cc_()->ey[2] * py;
        }
    }
    if (difPos) {
        // unit tangent
        difPos[0] = cc_()->ex[0] * tx + cc_()->ey[0] * ty;
        difPos[1] = cc_()->ex[1] * tx + cc_()->ey[1] * ty;
        if (get_axis_num() == 3) {
            difPos[2] = cc_()->ex[2] * tx + cc_()->ey[2] * ty;
        }
    }
}

void TrajCirc::get_slice2(double d, double v, double *absPos, double *difPos) const {
    get_slice(d, absPos, difPos);
    if (difPos) {
        for (uint32_t i = 0; i < get_axis_num(); i++) {
            difPos[i] *= v;
        }
    }
}
void TrajCirc::get_slice3(double d, double v, double a, double *pos, double *difPos, double *dif2Pos) const {
    get_slice(d, pos, difPos);
    if (difPos) {
        for (uint32_t i = 0; i < get_axis_num(); i++) {
            difPos[i] *= v;
        }
    }
    if (!dif2Pos) {
        return;
    }
    if (cc_() == nullptr || cc_()->radius <= 1e-12) {
        for (uint32_t i = 0; i < get_axis_num(); i++) {
            dif2Pos[i] = 0;
        }
        return;
    }

    // Recompute theta cheaply from current d
    if (d < 0)
        d = 0;
    else if (d > _distance)
        d = _distance;
    double theta = cc_()->theta0 + cc_()->sign * (d / cc_()->radius);
    double cs = cos(theta);
    double sn = sin(theta);

    double tx, ty;
    if (cc_()->sign == 1) {
        tx = -sn;
        ty = cs;
    } else {
        tx = sn;
        ty = -cs;
    }

    // Inward normal (towards center): -[cos, sin]
    double nx = -cs;
    double ny = -sn;

    double T0 = cc_()->ex[0] * tx + cc_()->ey[0] * ty;
    double T1 = cc_()->ex[1] * tx + cc_()->ey[1] * ty;
    double N0 = cc_()->ex[0] * nx + cc_()->ey[0] * ny;
    double N1 = cc_()->ex[1] * nx + cc_()->ey[1] * ny;
    dif2Pos[0] = a * T0 + (v * v / cc_()->radius) * N0;
    dif2Pos[1] = a * T1 + (v * v / cc_()->radius) * N1;
    if (get_axis_num() == 3) {
        double T2 = cc_()->ex[2] * tx + cc_()->ey[2] * ty;
        double N2 = cc_()->ex[2] * nx + cc_()->ey[2] * ny;
        dif2Pos[2] = a * T2 + (v * v / cc_()->radius) * N2;
    }
}

}  // namespace zmc
