/*
 * @Author: ChenShengyong
 * @Date: 2024-11-27 16:27:36
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2025-04-29 15:26:54
 */

#include "TrajUn.h"
#include <new>

namespace zmc {



TrajUn::~TrajUn() {
    trajLine.~TrajLine();
}

int TrajUn::init_axis_num(uint8_t axisNum) {
    return trajLine.init_axis_num(axisNum);
}

void TrajUn::set_sg_param(double pos_or_dist, bool is_relative, bool no_end) {
    trajSg.set_sg_param(pos_or_dist, is_relative, no_end);
}
void TrajUn::set_line_param(const double *pos_or_diff_arr, bool is_relative) {
    trajLine.set_line_param(pos_or_diff_arr, is_relative);
}
void TrajUn::set_circ_param(const double *AuxPoint, const double *EndPoint, bool PathChoice,
                            bool is_relative, SMC_CIRC_MODE mode) {
    trajCirc.set_circ_param(AuxPoint, EndPoint, PathChoice, is_relative, mode);
}

int TrajUn::build() {
    switch (trajLine.get_traj_type()) {
        case TRAJ_TYPE_SG_ABS:
        case TRAJ_TYPE_SG_REL:
            return trajSg.build();
        case TRAJ_TYPE_LINE_ABS:
        case TRAJ_TYPE_LINE_REL:
            return trajLine.build();
        case TRAJ_TYPE_CIRC_ABS:
        case TRAJ_TYPE_CIRC_REL:
            return trajCirc.build();
        default:
            break;
    }
    return ZMC_ERR_TRAJ_INVALID_MODE;
}
void TrajUn::get_end_pos(double *pos) const {
    return trajLine.get_end_pos(pos);
}

void TrajUn::get_slice(double d, double *absPos, double *difPos) const {
    switch (trajLine.get_traj_type()) {
        case TRAJ_TYPE_SG_ABS:
        case TRAJ_TYPE_SG_REL:
            return trajSg.get_slice(d, absPos, difPos);
        case TRAJ_TYPE_LINE_ABS:
        case TRAJ_TYPE_LINE_REL:
            return trajLine.get_slice(d, absPos, difPos);
        case TRAJ_TYPE_CIRC_ABS:
        case TRAJ_TYPE_CIRC_REL:
            return trajCirc.get_slice(d, absPos, difPos);
        default:
            break;
    }
}

void TrajUn::get_slice2(double d, double v, double *absPos, double *difPos) const {
    switch (trajLine.get_traj_type()) {
        case TRAJ_TYPE_SG_ABS:
        case TRAJ_TYPE_SG_REL:
            return trajSg.get_slice2(d, v, absPos, difPos);
        case TRAJ_TYPE_LINE_ABS:
        case TRAJ_TYPE_LINE_REL:
            return trajLine.get_slice2(d, v, absPos, difPos);
        case TRAJ_TYPE_CIRC_ABS:
        case TRAJ_TYPE_CIRC_REL:
            return trajCirc.get_slice2(d, v, absPos, difPos);
        default:
            break;
    }
}

void TrajUn::get_slice3(double d, double v, double a, double *absPos, double *difPos, double *dif2Pos) const {
    switch (trajLine.get_traj_type()) {
        case TRAJ_TYPE_SG_ABS:
        case TRAJ_TYPE_SG_REL:
            return trajSg.get_slice3(d, v, a, absPos, difPos, dif2Pos);
        case TRAJ_TYPE_LINE_ABS:
        case TRAJ_TYPE_LINE_REL:
            return trajLine.get_slice3(d, v, a, absPos, difPos, dif2Pos);
        case TRAJ_TYPE_CIRC_ABS:
        case TRAJ_TYPE_CIRC_REL:
            return trajCirc.get_slice3(d, v, a, absPos, difPos, dif2Pos);
        default:
            break;
    }
}

double TrajUn::get_sg_direct() const{
    switch (trajLine.get_traj_type()) {
        case TRAJ_TYPE_SG_ABS:
        case TRAJ_TYPE_SG_REL:
            return trajSg.get_sg_direct();
        case TRAJ_TYPE_LINE_ABS:
        case TRAJ_TYPE_LINE_REL:
        case TRAJ_TYPE_CIRC_ABS:
        case TRAJ_TYPE_CIRC_REL:
        default:
            break;
    }
    return 0;
}

}  // namespace zmc
