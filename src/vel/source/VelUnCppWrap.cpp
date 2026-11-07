#include "VelUnCppWrap.h"
#include "VelWrapImpl.h"
#include "Vel4Wrap.h"
#include <new>

namespace zmc{

static inline VelocityWrapImpl& toVelImpl(VelocityWrap& self) {
    return ((VelocityWrapImpl&)self);
}
static inline const VelocityWrapImpl& toVelImpl(const VelocityWrap& self) {
    return ((const VelocityWrapImpl&)self);
}

VelocityWrap::VelocityWrap(){
    static_assert(sizeof(VelocityWrapImpl) <= STORAGE_SIZE, "VelocityWrapImpl is too large");
    new (storage_) VelocityWrapImpl();
};

VelocityWrap::VelocityWrap(const VelocityWrap &other) {
    new (storage_) VelocityWrapImpl(toVelImpl(other));
}

VelocityWrap &VelocityWrap::operator=(const VelocityWrap &other) {
    if (this != &other) {
        toVelImpl(*this) = toVelImpl(other);
    }
    return *this;
}

VelocityWrap::~VelocityWrap(){ 
    toVelImpl(*this).~VelocityWrapImpl();
}
// VelocityWrap* VelocityWrap::clone()const{
//     return new VelocityWrap(new VelocityWrapImpl(*this->impl));
// };
void VelocityWrap::set_config(double vm, double acc, double dec, double jerk){
    return toVelImpl(*this).set_config(vm, acc, dec, jerk);
}
void VelocityWrap::set_begin_end_pos_vel(double s0, double sd, double v0, double vd){
    return toVelImpl(*this).set_begin_end_pos_vel(s0, sd, v0, vd);
}
void VelocityWrap::set_begin_end_pos(double s0, double sd){
    return toVelImpl(*this).set_begin_end_pos(s0, sd);
}
void VelocityWrap::set_begin_end_vel(double v0, double vd){
    return toVelImpl(*this).set_begin_end_vel(v0, vd);
}

void VelocityWrap::set_begin_end_acc(double a0, double ad){
    return toVelImpl(*this).set_begin_end_acc(a0, ad);
}

void VelocityWrap::set_begin_pva(double s0, double v0, double a0){
    return toVelImpl(*this).set_begin_pva(s0, v0, a0);
}
void VelocityWrap::set_end_pva(double sd, double vd, double ad){
    return toVelImpl(*this).set_end_pva(sd, vd, ad);
}

void VelocityWrap::set_begin_pos(double s0){
    return toVelImpl(*this).set_begin_pos(s0);
}
void VelocityWrap::set_begin_vel(double v0){
    return toVelImpl(*this).set_begin_vel(v0);
}
void VelocityWrap::set_end_vel(double vd){
    return toVelImpl(*this).set_end_vel(vd);
}
void VelocityWrap::set_total_time(double Tc){
    return toVelImpl(*this).set_total_time(Tc);
}
void VelocityWrap::init(){
        return  toVelImpl(*this).init();
}
void VelocityWrap::set_num_den(int n, int d){
        return  toVelImpl(*this).set_num_den(n, d);
}
void VelocityWrap::set_time_sample(double v){
    return  toVelImpl(*this).set_time_sample(v);
}
bool VelocityWrap::is_scurve()const{
    return toVelImpl(*this).is_scurve();
}
bool VelocityWrap::is_continuous_vel()const{
    return toVelImpl(*this).is_continuous_vel();
}
bool VelocityWrap::is_no_dest()const{
    return toVelImpl(*this).is_no_dest();
}
bool VelocityWrap::is_slave()const{
    return toVelImpl(*this).is_slave();
}

void VelocityWrap::set_scurve(bool v){
    return toVelImpl(*this).set_scurve(v);
}
void VelocityWrap::set_continuous_vel(bool v){
    return toVelImpl(*this).set_continuous_vel(v);
}
void VelocityWrap::set_inversable(bool v){
    return toVelImpl(*this).set_inversable(v);
}
void VelocityWrap::set_no_dest(bool v){
    return toVelImpl(*this).set_no_dest(v);
}
void VelocityWrap::set_slave(bool v){
    return toVelImpl(*this).set_slave(v);
}
void VelocityWrap::set_sinsquare(bool v){
    return toVelImpl(*this).set_sinsquare(v);
}
void VelocityWrap::set_mode(int mode){
    return toVelImpl(*this).set_mode(mode);
}
int VelocityWrap::get_nseg()const{
    return toVelImpl(*this).get_nseg();
}
double VelocityWrap::cal_reachable_pre_vel(double vd)const{
    return toVelImpl(*this).cal_reachable_pre_vel(vd);
}
double VelocityWrap::cal_reachable_pro_vel(double v0)const{
    return toVelImpl(*this).cal_reachable_pro_vel(v0);
}

double VelocityWrap::cal_reachable_backward_vel(double vd)const{
    return toVelImpl(*this).cal_reachable_backward_vel(vd);
}
double VelocityWrap::cal_reachable_forward_vel(double v0)const{
    return toVelImpl(*this).cal_reachable_forward_vel(v0);
}

double VelocityWrap::get_delta_pos()const{
    return toVelImpl(*this).get_delta_pos();
}
double VelocityWrap::get_end_pos()const{
    return toVelImpl(*this).get_end_pos();
}
void VelocityWrap::set_end_pos(double sd){
    return toVelImpl(*this).set_end_pos(sd);
}
int VelocityWrap::check()const{
    return toVelImpl(*this).check();
}
void VelocityWrap::follow(double sc, double vc, double ac){
    return toVelImpl(*this).follow(sc, vc, ac);
}
void VelocityWrap::shift_time(double tm){
    return toVelImpl(*this).shift_time(tm);
}
void VelocityWrap::chop_left_time(double tm){
    return toVelImpl(*this).chop_left_time(tm);
}

int VelocityWrap::build(){
    return toVelImpl(*this).build();
}
void VelocityWrap::shift_pos(double ofs){
    return toVelImpl(*this).shift_pos(ofs);
}
double VelocityWrap::get_begin_rate()const{
    return toVelImpl(*this).get_begin_rate();
}
double VelocityWrap::get_end_rate()const{
    return toVelImpl(*this).get_end_rate();
}
double VelocityWrap::get_begin_acc()const{
    return toVelImpl(*this).get_begin_acc();
}
double VelocityWrap::get_end_acc()const{
    return toVelImpl(*this).get_end_acc();
}
double VelocityWrap::get_rend_time()const{
    return toVelImpl(*this).get_end_time();
}
double VelocityWrap::get_max_vel()const{
    return toVelImpl(*this).get_max_vel();
}
double VelocityWrap::get_rmax_vel()const{
    return toVelImpl(*this).get_rmax_vel();
}

double VelocityWrap::get_end_time()const{
    return toVelImpl(*this).get_end_time();
}
int VelocityWrap::get_plan(double* total_time, double* rVmax, double* rAcc, double* rDec)const{
    return toVelImpl(*this).get_plan(total_time, rVmax, rAcc, rDec);
}
int VelocityWrap::get_slice(double time, PosVelAcc& pva)const{
    return toVelImpl(*this).get_slice(time, pva);
}
int VelocityWrap::get_slice(double time, double* pos, double* vel, double* acc)const{
    return toVelImpl(*this).get_slice(time, pos, vel, acc);
}
int VelocityWrap::get_slice_continuous(double time, PosVelAcc& pva)const{
    return toVelImpl(*this).get_slice_continuous(time, pva);
}
int VelocityWrap::get_plan2(double& total_time, double& rVmax, double& rAcc, double& rDec)const{
    return toVelImpl(*this).get_plan(&total_time, &rVmax, &rAcc, &rDec);
}
int VelocityWrap::get_slice2(double time, double& pos, double& vel, double& acc)const{
    return toVelImpl(*this).get_slice(time, &pos, &vel, &acc);
}
int VelocityWrap::get_slice4(double time, double* pva)const{
    return toVelImpl(*this).get_slice(time, &pva[0], &pva[1], &pva[2]);
}
PosVelAcc VelocityWrap::get_slice3(double time)const{
    PosVelAcc pva;
    toVelImpl(*this).get_slice(time, &pva.pos, &pva.vel, &pva.acc);
    return pva;
}

double VelocityWrap::get_param(VelInnerParam par)const{
    return toVelImpl(*this).get_param(par);
}
void VelocityWrap::get_config(double& vm, double& acc, double& dec, double& jerk){
    return toVelImpl(*this).get_config(vm, acc, dec, jerk);
}
double VelocityWrap::calc_stop_dist(double v0, double a0, double dec, double jerk){
    return VelocityWrapImpl::calc_stop_dist(v0, a0, dec, jerk);
}
double VelocityWrap::calc_stop_dist(double v0, double dec){
    return VelocityWrapImpl::calc_stop_dist(v0, dec);
}
double VelocityWrap::calc_vel_dist(double v0, double vd, double acc, double dec){
    return VelocityWrapImpl::calc_vel_dist(v0, vd, acc, dec);
}

double VelocityWrap::cal_stop_pos()const{
    return toVelImpl(*this).cal_stop_pos();
}

void Vel4Wrap::switchVelMode(int velMode){
    return toVelImpl(*this).set_mode(velMode);
}

}
