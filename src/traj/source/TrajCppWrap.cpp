
#include "TrajCppWrap.h"
#include "TrajUn.h"
#include <new>

namespace zmc {

static inline TrajUn& toTrajUn(TrajWrap& self) {
    return ((TrajUn&)self);
}
static inline const TrajUn& toTrajUn(const TrajWrap& self) {
    return ((const TrajUn&)self);
}
TrajWrap::TrajWrap(uint8_t ax) {
    new (storage_) TrajUn(ax);
    static_assert(sizeof(TrajUn) <= STORAGE_SIZE, "TrajUn is too large");
}

TrajWrap::TrajWrap(const TrajWrap& other) {
    new (storage_) TrajUn(toTrajUn(other));
}

TrajWrap& TrajWrap::operator=(const TrajWrap& other) {
    if (this != &other) {
        toTrajUn(*this) = toTrajUn(other);
    }
    return *this;
}

TrajWrap::~TrajWrap() {
    toTrajUn(*this).~TrajUn();
}

int TrajWrap::init_axis_num(uint8_t axisNum) {
    return toTrajUn(*this).init_axis_num(axisNum);
}

void TrajWrap::set_allocator(void* pAlloc) {
    toTrajUn(*this).set_allocator(pAlloc);
}

void TrajWrap::set_default_allocator() {
    toTrajUn(*this).set_default_allocator();
}

void TrajWrap::set_begin_pos(const double *pos) { return toTrajUn(*this).set_begin_pos(pos); };

void TrajWrap::set_sg_param(double pos_or_dist, bool is_relative, bool no_end) {
    toTrajUn(*this).set_sg_param(pos_or_dist, is_relative, no_end);
}

void TrajWrap::set_line_param(const double *pos_or_diff_arr, bool is_relative) {
    toTrajUn(*this).set_line_param(pos_or_diff_arr, is_relative);
}
void TrajWrap::set_circ_param(const double *AuxPoint, const double *EndPoint, bool PathChoice,
                              bool is_relative, SMC_CIRC_MODE mode) {
    toTrajUn(*this).set_circ_param(AuxPoint, EndPoint, PathChoice, is_relative, mode);
}

int TrajWrap::build() { return toTrajUn(*this).build(); }

uint8_t TrajWrap::get_axis_num() const { return toTrajUn(*this).get_axis_num(); };
uint8_t TrajWrap::get_traj_type() const { return toTrajUn(*this).get_traj_type(); };

void TrajWrap::get_begin_pos(double *pos) const { return toTrajUn(*this).get_begin_pos(pos); };
void TrajWrap::get_end_pos(double *pos) const { return toTrajUn(*this).get_end_pos(pos); }
double TrajWrap::get_plan() const { return toTrajUn(*this).get_plan(); };
void TrajWrap::get_slice(double d, double *absPos, double *difPos) const {
    return toTrajUn(*this).get_slice(d, absPos, difPos);
}
void TrajWrap::get_slice2(double d, double v, double *absPos, double *difPos) const {
    return toTrajUn(*this).get_slice2(d, v, absPos, difPos);
}
void TrajWrap::get_slice3(double d, double v, double a,  double *pos, double *difPos, double *dif2Pos) const{
    return toTrajUn(*this).get_slice3(d, v, a, pos, difPos, dif2Pos);
}

double TrajWrap::get_sg_direct() const{
    return toTrajUn(*this).get_sg_direct();
}

}  // namespace zmc