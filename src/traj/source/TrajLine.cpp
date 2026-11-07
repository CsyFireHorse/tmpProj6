/*
 * @Author: ChenShengyong
 * @Date: 2024-11-27 16:27:36
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2026-03-11 18:22:53
 */

#include "TrajLine.h"

namespace zmc {

void TrajLine::set_line_param(const double *pos_or_diff_arr, bool is_relative) {
    _mode = is_relative ? TRAJ_TYPE_LINE_REL : TRAJ_TYPE_LINE_ABS;
    if (!pos_or_diff_arr) return;

    double *cmd = cmd_pos_data();
    if (!cmd) return;
    for (int i = 0; i < get_axis_num(); i++) {
        cmd[i] = pos_or_diff_arr[i];
    }
}

int TrajLine::build() {
    if (_mode != TRAJ_TYPE_LINE_REL && _mode != TRAJ_TYPE_LINE_ABS) {
        return ZMC_ERR_TRAJ_INVALID_MODE;
    }
    if (get_axis_num() == 0 || get_axis_num() > MC_MOTION_MAX_AXES) {
        return ZMC_ERR_TRAJ_INVALID_AXIS_NUM;
    }

    double *begin = begin_pos_data();
    double *direction = aux_pos_data();
    const double *cmd = cmd_pos_data();
    if (!begin || !direction || !cmd) {
        return ZMC_ERR_TRAJ_CACHE_INVALID;
    }

    double cum = 0;
    if (_mode == TRAJ_TYPE_LINE_REL) {
        for (int i = 0; i < get_axis_num(); i++) {
            direction[i] = cmd[i];
            cum += direction[i] * direction[i];
        }
    } else {
        for (int i = 0; i < get_axis_num(); i++) {
            direction[i] = cmd[i] - begin[i];
            cum += direction[i] * direction[i];
        }
    }
    _distance = sqrt(cum);
    if (_distance > 1e-6) {
        for (int i = 0; i < get_axis_num(); i++) {
            direction[i] /= _distance;
        }
    }
    return 0;
}

void TrajLine::get_slice(double d, double *absPos, double *difPos) const {
    const double *begin = begin_pos_data();
    const double *direction = aux_pos_data();
    if (!begin || !direction) return;

    if (d < 0)
        d = 0;
    else if (d > _distance)
        d = _distance;
    if (absPos) {
        for (uint32_t i = 0; i < get_axis_num(); i++) {
            absPos[i] = begin[i] + direction[i] * d;
        }
    }
    if (difPos) {
        for (uint32_t i = 0; i < get_axis_num(); i++) {
            difPos[i] = direction[i];
        }
    }
}

void TrajLine::get_slice2(double d, double v, double *absPos, double *difPos) const {
    const double *begin = begin_pos_data();
    const double *direction = aux_pos_data();
    if (!begin || !direction) return;

    if (d < 0)
        d = 0;
    else if (d > _distance)
        d = _distance;
    if (absPos) {
        for (uint32_t i = 0; i < get_axis_num(); i++) {
            absPos[i] = begin[i] + direction[i] * d;
        }
    }
    if (difPos) {
        for (uint32_t i = 0; i < get_axis_num(); i++) {
            difPos[i] = direction[i] * v;
        }
    }
}

void TrajLine::get_slice3(double d, double v, double a, double *pos, double *difPos,
                          double *dif2Pos) const {
    const double *begin = begin_pos_data();
    const double *direction = aux_pos_data();
    if (!begin || !direction) return;

    if (d < 0)
        d = 0;
    else if (d > _distance)
        d = _distance;
    if (pos) {
        for (uint32_t i = 0; i < get_axis_num(); i++) {
            pos[i] = begin[i] + direction[i] * d;
        }
    }
    if (difPos) {
        for (uint32_t i = 0; i < get_axis_num(); i++) {
            difPos[i] = direction[i] * v;
        }
    }
    if (dif2Pos) {
        for (uint32_t i = 0; i < get_axis_num(); i++) {
            dif2Pos[i] = direction[i] * a;
        }
    }
}

}  // namespace zmc
