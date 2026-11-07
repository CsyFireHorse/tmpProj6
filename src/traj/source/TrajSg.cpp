/*
 * @Author: ChenShengyong
 * @Date: 2024-11-27 16:27:36
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-12-12 17:22:33
 */

#include "TrajSg.h"

namespace zmc {

enum { TRAJ_SUBMODE_NOP = 0, TRAJ_SUBMODE_NO_DEST = 1 };

void TrajSg::set_sg_param(double pos_or_dist, bool is_relative, bool no_end) {
    if (no_end) {
        _mode = is_relative ? TRAJ_TYPE_SG_REL : TRAJ_TYPE_SG_ABS;
        _sub_mode = TRAJ_SUBMODE_NO_DEST;
    } else
        _mode = is_relative ? TRAJ_TYPE_SG_REL : TRAJ_TYPE_SG_ABS;
    double *cmd = cmd_pos_data();
    if (cmd) {
        cmd[0] = pos_or_dist;
    }
}
int TrajSg::build() {
    const double *cmd = cmd_pos_data();
    if (!cmd) {
        return ZMC_ERR_TRAJ_CACHE_INVALID;
    }
    if (_mode != TRAJ_TYPE_SG_REL) {
        _auxPos[0] = cmd[0] - _beginPos[0];
    } else {
        _auxPos[0] = cmd[0];
    }
    _distance = _auxPos[0] >= 0 ? _auxPos[0] : -_auxPos[0];
    if (_distance > 1e-6) {
        _auxPos[0] /= _distance;
    }
    return 0;
}

void TrajSg::get_slice(double d, double *absPos, double *difPos) const {
    if (d < 0 && !(_sub_mode & TRAJ_SUBMODE_NO_DEST))
        d = 0;
    else if (!(_sub_mode & TRAJ_SUBMODE_NO_DEST) && d > _distance)
        d = _distance;
    int i = 0;
    if (absPos) {
        absPos[0] = _beginPos[0] + _auxPos[0] * d;
    }
    if (difPos) {
        difPos[0] = _auxPos[0];
    }
}

void TrajSg::get_slice2(double d, double v, double *absPos, double *difPos) const {
    if (d < 0 && !(_sub_mode & TRAJ_SUBMODE_NO_DEST))
        d = 0;
    else if (!(_sub_mode & TRAJ_SUBMODE_NO_DEST) && d > _distance)
        d = _distance;
    int i = 0;
    if (absPos) {
        absPos[0] = _beginPos[0] + _auxPos[0] * d;
    }
    if (difPos) {
        difPos[0] = _auxPos[0] * v;
    }
}

void TrajSg::get_slice3(double d, double v, double a, double *absPos, double *difPos, double *dif2Pos) const {
    if (d < 0 && !(_sub_mode & TRAJ_SUBMODE_NO_DEST))
        d = 0;
    else if (!(_sub_mode & TRAJ_SUBMODE_NO_DEST) && d > _distance)
        d = _distance;
    int i = 0;
    if (absPos) {
        absPos[0] = _beginPos[0] + _auxPos[0] * d;
    }
    if (difPos) {
        difPos[0] = _auxPos[0] * v;
    }
    if (dif2Pos) {
        dif2Pos[0] = 0;
    }
}

double TrajSg::get_sg_direct() const{
    return _auxPos[0];
}

}  // namespace zmc
