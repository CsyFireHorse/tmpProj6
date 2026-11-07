
#include "vel_sin_square.hpp"
#include "vel_relax_param.hpp"
#include "vel_sin_square_forwarder.h"
#include "VelSinSquare.h"
#include "VelBase.h"
#include <cmath>

#define M_PI (3.141592653589793)
#define EPS (1e-12)
#define V_EPS (1e-12)
namespace zmc {


struct SinSquareParam0{
    double S0, Sd;
    double V0, Vd; 
    double A0, Ad;
    double Vmax; 
    double Amax;
    double Amin;
    double Jerk;
    double Tc;
    int policy;
};

void VelSinSquareForwarder::build(SinSquareRelaxParam& rParam, const SinSquareParam& param ){
    nseg = rParam.get_seg_num();
    vMax = rParam.vMax;
    _build(rParam, param);
    _cumsum_time_pos();
}

void VelSinSquareForwarder::_build(SinSquareRelaxParam& rParam, const SinSquareParam& param ){
        int seg_num = rParam.get_seg_num();
        sArr[0] = param.S_0;
        vArr[0] = param.V_0;

        if (seg_num == 1){
            tArr[1] = 0;
            sArr[1] = 0;
            vArr[1] = param.V_0;
            aArr[0] = 0;

            tArr[3] = 0;
            sArr[3] = 0;
            vArr[3] = param.V_d;

            sArr[2] = param.get_S() - sArr[1] - sArr[3];
            tArr[2] = sArr[2] / rParam.vMax;
            vArr[1] = rParam.vMax;
            vArr[2] = rParam.vMax;
            aArr[1] = 0;
            return;
        }
        tArr[1] = M_PI * (rParam.vMax - param.V_0) / 2 / rParam.aMax;
        sArr[1] = (vArr[0] + rParam.vMax) * tArr[1] / 2.0;
        aArr[0] = rParam.aMax;

        vArr[3] = param.V_d;        
        tArr[3] = M_PI * (vArr[3] - rParam.vMax) / 2 / rParam.aMin;
        sArr[3] = (param.V_d + rParam.vMax) * tArr[3] / 2.0;
        aArr[2] = rParam.aMin;
        aArr[3] = 0;

        if (seg_num == 3){
            sArr[2] = param.get_S() - sArr[1] - sArr[3];
            tArr[2] = sArr[2] / rParam.vMax;
            vArr[1] = rParam.vMax;
            vArr[2] = rParam.vMax;
            aArr[1] = 0;
        }else{
            tArr[2] = 0;
            sArr[2] = 0;
            vArr[1] = rParam.vMax;
            vArr[2] = rParam.vMax;
            aArr[1] = 0;
        }
    }

void VelSinSquareForwarder::_cumsum_time_pos(){
    // cumsum time
    tArr[0] = 0;
    tArr[1] = tArr[0] + tArr[1];
    tArr[2] = tArr[1] + tArr[2];
    tArr[3] = tArr[2] + tArr[3];
    // cumsum pos

    sArr[1] = sArr[0] + sArr[1];
    sArr[2] = sArr[1] + sArr[2];
    sArr[3] = sArr[2] + sArr[3];
}

PosVelAcc VelSinSquareForwarder::get_slice(double t) const {
    PosVelAcc pva;
    this->get_slice(t, pva);
    return pva;
}

int VelSinSquareForwarder::get_slice(double t, PosVelAcc& state) const{
        if (t <= tArr[0]) { 
            state.pos = sArr[0];
            state.vel = vArr[0];
            state.acc = 0; // aArr[0];
            return 0;
        }
        if (t >= tArr[3]) {
            state.pos = sArr[3];
            state.vel = vArr[3];
            state.acc = aArr[3];
            return 2;
        }

        if (t < tArr[1]) {
            double dt = t - tArr[0];
            double t1 = tArr[1] - tArr[0]; // M_PI * (Vmax - V_0) / 2 / A_m;
            double theta = M_PI * dt  / t1;
            double sin_theta = std::sin(theta);

            double dV = t1 *2 * aArr[0] / M_PI;
            // double A_m = A0 ; // M_PI * (V1 - V_0) / 2 / T1;
            // double jerk = cos(theta) * aArr[0] * M_PI/t1;
            state.acc = aArr[0] * sin_theta;
            state.vel = vArr[0] + dV * (1-std::cos(theta))/2;
            state.pos = sArr[0] + vArr[0] * dt + dV/ 2 * (dt - t1 / M_PI * sin_theta);
        } else if (t < tArr[2]) {
            double dt = t - tArr[1];
            state.vel = vArr[1];
            state.acc = 0;
            state.pos = sArr[1] + vArr[1] * dt;
        } else {
            double dt = t - tArr[2]; 
            double t3 = tArr[3] - tArr[2];
            double tau = t3 - dt;   
            double theta = M_PI * tau / t3;
            double sin_theta = std::sin(theta);
            double dV = t3 *2 * aArr[2] / M_PI; // (V2 - V3)
            state.vel = vArr[3] - dV * (1-std::cos(theta))/2;
            // double A_n = A2; // M_PI * (V2 - V3) / 2 / T3;
            state.acc = aArr[2] * sin_theta;
            state.pos = sArr[2] + vArr[2] * dt + dV / 2 * (dt - t3 / M_PI * sin_theta);
        }

        return 1;
    }

int VelSinSquareForwarder::get_slice_continuous(double tm, PosVelAcc& pva) const {
    if (tm >= get_end_time()) {
        double dt = tm - tArr[3];
        pva.vel = vArr[3];
        pva.acc = 0;
        pva.pos = sArr[3] + vArr[3] * dt;
        // get_pva(3, tm, pva);
        return 2;
    }else{
        return get_slice(tm, pva);
    }
}


struct SinSquareRelaxerBase:public SinSquareParam{
    inline double coef()const{
        return M_PI/4;
    }
    double nseg20_s2vm(double S)const{
        double vm = ((S * A_m *A_n)/coef() + V_0*V_0 *A_n + V_d*V_d *A_m)/(A_m + A_n);
        return sqrt(vm);
    }
    double nseg20_vm2s(double vm)const{
        return ((vm*vm - V_0*V_0 )/A_m + (vm*vm - V_d*V_d )/A_n) *coef();
    }
    // assume V_0 < V_d
    inline double nseg11_vm2s() const { return (V_d*V_d - V_0*V_0) *coef()/ A_m; }
    // assume V_0 > V_d
    inline double nseg12_vm2s() const { return (V_0*V_0 - V_d*V_d) *coef() / A_n; }
    double nseg1x_vm2s()const { 
        if (V_0 < V_d){
            return nseg11_vm2s();
        }else if(V_0 > V_d){
            return nseg12_vm2s();
        }else
            return 0;
    }   
    
    // Helper functions for relaxation_sym logic
    double nseg_h_t_s() const {
        double sEx = 0;
        if (V_0 < 0)
            sEx -= (V_0 * V_0 * coef() / A_n);
        else
            sEx += (V_0 * V_0 * coef() / A_n);
        if (V_d < 0)
            sEx -= (V_d * V_d * coef() / A_m);
        else
            sEx += (V_d * V_d * coef() / A_m);
        return sEx;
    }
    
    double nseg20z_vm2s(double vm) const {
        // assume V_0 = 0 && V_d = 0
        double S =vm * vm * (A_m +  A_n)/ (A_m* A_n) * coef();
        return S;
    }
    
    double nseg20z_s2vm(double S) const {
        return sqrt((S * A_m * A_n) / (coef() * (A_m + A_n)));
    }
    
    double nseg23_vm2s(double vm) const {
        // {-, +} : decelerate then accelerate
        // Note: A_n should be negative (deceleration)
        // When V_0 > vm, we decelerate from V_0 to vm using A_n (negative)
        // When V_d > vm, we accelerate from vm to V_d using A_m (positive)
        double vm2 = vm * vm;
        // For deceleration: (V_0^2 - vm^2) / (2 * |A_n|) = (V_0^2 - vm^2) * M_PI / 4 / (-A_n)
        // For acceleration: (V_d^2 - vm^2) / (2 * A_m) = (V_d^2 - vm^2) * M_PI / 4 / A_m
        double S = ((V_0*V_0 - vm2) / A_n + (V_d*V_d - vm2)/ A_m) * coef();
        return S;
    }
    
    double nseg23_s2vn(double S) const {
        // Solve for vm in: S = (V_0^2 - vm^2) * M_PI / 4 / (-A_n) + (V_d^2 - vm^2) * M_PI / 4 / A_m
        // Rearranging: 4*S/π = (V_0^2 - vm^2) / (-A_n) + (V_d^2 - vm^2) / A_m
        // vm^2 = (V_0^2 * A_m + V_d^2 * (-A_n) - 4*S*A_m*(-A_n)/π) / (A_m + (-A_n))
        return sqrt((V_0*V_0 * A_m + V_d*V_d * A_n -  S * A_m * A_n / coef()) / (A_m + A_n));
    }
};

struct SinSquareRelaxer:public SinSquareRelaxerBase{
    // int build(VelSinSquareForwarder&);
    int relaxation(SinSquareRelaxParam& fw);
private:
    int _relaxation(SinSquareRelaxParam& fw);
    int _relaxation_sym(SinSquareRelaxParam& fw);
    int _relaxation_sym_overlimit(SinSquareRelaxParam& fw);
    int _relaxation_sym_inv_db(SinSquareRelaxParam& fw);
    int _relaxation_sym_main(SinSquareRelaxParam& fw);
    int _relaxation_asym(SinSquareRelaxParam& fw);
    int _relaxation_asym_main(SinSquareRelaxParam& fw);
    int _relaxation_asym_overlimit_db(SinSquareRelaxParam& fw);
    int _relaxation_asym_overlimit_sg(SinSquareRelaxParam& fw);
    int _relaxation_asym_main_inlimit(SinSquareRelaxParam& fw);
    int _relaxation_asym_inv_db(SinSquareRelaxParam& fw);
    int _relaxation_asym_inv_sg(SinSquareRelaxParam& fw);
    int _relaxation_unreachable(SinSquareRelaxParam& fw);
};



int SinSquareRelaxer::_relaxation_sym_overlimit(SinSquareRelaxParam& fw){
    double _S = get_S();
    double S23 = nseg23_vm2s(V_m);
    if (_S >= S23 - EPS) {  // LADDER_34_SEGMENT
        SinSquareRelaxParam rParam = {3, V_m, -A_n, A_m};
        fw = rParam;
    } else {  // LADDER_23_SEGMENT
        double vm = nseg23_s2vn(_S);
        SinSquareRelaxParam rParam = {2, vm, -A_n, A_m};
        fw = rParam;
    }
    return 0;
}

int SinSquareRelaxer::_relaxation_sym_inv_db(SinSquareRelaxParam& fw){
    double _S = get_S();
    // if initial vel is inverse, first decel to zero, then make trapz/triangle velocity plan
    double S20 = nseg20_vm2s(V_m); 
    double sRes = _S;
    if (sRes >= S20 + EPS) {
        SinSquareRelaxParam rParam = {3, V_m, A_m, -A_n};
        fw = rParam;
    } else {
        double vm = nseg20_s2vm(sRes);
        SinSquareRelaxParam rParam = {2, vm, A_m, -A_n};
        fw = rParam;
    }
    return 0;
}

int SinSquareRelaxer::_relaxation_sym_main(SinSquareRelaxParam& fw){
    double _S = get_S();
    double S20 = nseg20_vm2s(V_m);
    if (_S >= S20 + EPS) {
        if (V_0 <= V_m - V_EPS) {  // LADDER_30_SEGMENT
            SinSquareRelaxParam rParam = {3, V_m, A_m, -A_n};
            fw = rParam;
        } else if (V_0 <= V_m + V_EPS) {  // LADDER_10_SEGMENT
            SinSquareRelaxParam rParam = {1, V_0, 0, 0};
            fw = rParam;
        }
    } else {
        double vm = nseg20_s2vm(get_S()); // LADDER_20_SEGMENT
        SinSquareRelaxParam rParam = {2, vm, A_m, -A_n};
        fw = rParam;
    }
    return 0;
}

int SinSquareRelaxer::_relaxation_sym(SinSquareRelaxParam& fw){
    if (V_0 > V_m + V_EPS) {
        return _relaxation_sym_overlimit(fw);
    } else if (V_0 < -V_EPS) {
        return _relaxation_sym_inv_db(fw);
    } else {
        return _relaxation_sym_main(fw);
    }
}


int SinSquareRelaxer::relaxation(SinSquareRelaxParam& fw) {
    if (S_0 > S_d){
        auto relaxer = *this;
        relaxer.invert();
        int ret = relaxer._relaxation(fw);
        // fw.setInvertFlag(true);
        fw.invert();
        return ret;
    }
    return _relaxation(fw);
}

int SinSquareRelaxer::_relaxation(SinSquareRelaxParam& fw) {
    int ret;
    if (is_vel_symmetry()) {
        ret = _relaxation_sym(fw);
    } else {
        ret = _relaxation_asym(fw);
    }
    return ret;
}

int SinSquareRelaxer::_relaxation_asym(SinSquareRelaxParam& fw) {
    if (V_0 > -V_EPS && V_d > -V_EPS) {
        if (V_0 < V_m + V_EPS && V_d < V_m + V_EPS) {
            return _relaxation_asym_main_inlimit(fw);
        }else if (V_0 >= V_m + V_EPS && V_d >= V_m + V_EPS) {
            return _relaxation_asym_overlimit_db(fw);
        }else {
            return _relaxation_asym_overlimit_sg(fw);
        }
    }else if (V_0 < 0 && V_d < 0) {
        return _relaxation_asym_inv_db(fw);
    }else { // (V_0 < 0 && V_d >= 0) || (V_0 >= 0 && V_d < 0)
        return _relaxation_asym_inv_sg(fw);
    }
}

int SinSquareRelaxer::_relaxation_asym_overlimit_db(SinSquareRelaxParam& fw){
    double _S = get_S();
    double S23 = nseg23_vm2s(V_m);
    if (_S >= S23 - EPS) {  // LADDER_34_SEGMENT
        SinSquareRelaxParam rParam = {3, V_m, -A_n, -A_n};
        fw = rParam;
        return 0;
    } else {
        double sEx = nseg23_vm2s(0);
        if (_S > sEx - EPS){
            double vn = nseg23_s2vn(_S);
            SinSquareRelaxParam rParam = {2, vn, -A_n, -A_n};
            fw = rParam;
            return 0;
        }else
            return _relaxation_unreachable(fw);
    }
}

int SinSquareRelaxer::_relaxation_asym_overlimit_sg(SinSquareRelaxParam& fw){
    double _S = get_S();
    if (V_0 >= V_m + V_EPS) {
        double S12 = nseg12_vm2s();
        if (_S > S12 - EPS) {  // LADDER_31_SEGMENT
            SinSquareRelaxParam rParam = {3, V_m, -A_n, -A_n};
            fw = rParam;
            return 0;
        // } else if (_S >= S12 - EPS) {  // LADDER_12_SEGMENT
        //     SinSquareRelaxParam rParam = {1, V_0, 0, 0};
        //     fw = rParam;
        //     return 0;
        } else {
            return _relaxation_unreachable(fw);
        }
    } else if (V_d >= V_m + V_EPS) {
        double S11 = nseg11_vm2s();
        if (_S > S11 - EPS) {  // LADDER_32_SEGMENT
            SinSquareRelaxParam rParam = {3, V_m, A_m, A_m};
            fw = rParam;
            return 0;
        // } else if (_S >= S11 - EPS) {  // LADDER_11_SEGMENT
        //     VelAccArr<1> velAccArr = {{V_0, V_d}, {A_m, 0}};
        //     fillForwardParam(fw, velAccArr);
        //     return 0;
        } else {
            return _relaxation_unreachable(fw);
        }
    }
    return -1;
}

int SinSquareRelaxer::_relaxation_asym_main(SinSquareRelaxParam& fw) {
    return _relaxation_asym_main_inlimit(fw);
}

int SinSquareRelaxer::_relaxation_asym_main_inlimit(SinSquareRelaxParam& fw) {
    double S20 = nseg20_vm2s(V_m);
    double S = get_S();
    if (S > S20){
        SinSquareRelaxParam rParam = {3, V_m, A_m, -A_n};
        fw = rParam;
    }else{
        double S1x = nseg1x_vm2s();
        if (S > S1x){
            double vm = nseg20_s2vm(get_S());
            SinSquareRelaxParam rParam = {2, vm, A_m, -A_n};
            fw = rParam;
        }else{
            return _relaxation_unreachable(fw);
        }
    }
    return 0;
}

int SinSquareRelaxer::_relaxation_unreachable(SinSquareRelaxParam& fw) {
    double _S = get_S();
    double sEx = nseg_h_t_s();
    double sRes = _S - sEx;
    double S23 = -nseg20z_vm2s(V_m);
    if (-sRes >= -S23  + EPS) {
        SinSquareRelaxParam rParam = {3, -V_m, -A_n, A_m};
        fw = rParam;
    } else {
        double vm = -nseg20z_s2vm(-sRes);
        SinSquareRelaxParam rParam = {2, vm, -A_n, A_m};
        fw = rParam;
    }
    return 0;
}


int SinSquareRelaxer::_relaxation_asym_inv_db(SinSquareRelaxParam& fw){
    // velocity will cross zero twice
    double _S = get_S();
    double sEx = nseg20_vm2s(0);
    double S20 = nseg20z_vm2s(V_m); 
    double sRes = _S - sEx;
    if (sRes >= S20 + EPS) {
        SinSquareRelaxParam rParam = {3, V_m, A_m, -A_n};
        fw = rParam;
    } else {
        double vm = nseg20z_s2vm(sRes);
        SinSquareRelaxParam rParam = {2, vm, A_m, -A_n};
        fw = rParam;
    }
    return 0;
}

int SinSquareRelaxer::_relaxation_asym_inv_sg(SinSquareRelaxParam& fw){
    // velocity will cross zero once
    double _S = get_S();
    double sEx = nseg1x_vm2s();

    double sRes = _S - sEx;
    if (sRes > -EPS){
        if (V_0 > V_m + V_EPS) { // LADDER_32_SEGMENT
            SinSquareRelaxParam rParam = {3, V_m, -A_n, -A_n};
            fw = rParam;
        } else if (V_d > V_m + V_EPS) { // LADDER_31_SEGMENT
            SinSquareRelaxParam rParam = {3, V_m, A_m, A_m};
            fw = rParam;
        }else{
        double S20 = nseg20_vm2s(V_m);
        if (_S >= S20 + EPS) { // LADDER_30_SEGMENT
            SinSquareRelaxParam rParam = {3, V_m, A_m, -A_n};
            fw = rParam;
        } else {
            double vm = nseg20_s2vm(_S);
            SinSquareRelaxParam rParam = {2, vm, A_m, -A_n};
            fw = rParam;
        }
        }
    }else{
        // core direction is inverse to S, so use negative vm
        if (V_0 < -V_m) { 
            SinSquareRelaxParam rParam = {3, -V_m, A_n, A_n};
            fw = rParam;
        } else if (V_d < -V_m) { 
            SinSquareRelaxParam rParam = {3, -V_m, -A_m, -A_m};
            fw = rParam;
        }else{
        double S23 = nseg23_vm2s(V_m);
        if (_S <= S23 + EPS) {
            SinSquareRelaxParam rParam = {3, -V_m, -A_n, A_m};
            fw = rParam;
        } else {
            double vm = -nseg23_s2vn(_S);
            SinSquareRelaxParam rParam = {2, vm,  -A_n, A_m};
            fw = rParam;
        }
    }
    }
    return 0;
}


VelSinSquarePlanner::VelSinSquarePlanner(){
    pImpl = new VelSinSquareForwarder();
    policy = 0;
}

VelSinSquarePlanner::~VelSinSquarePlanner(){
    if (pImpl){
        delete pImpl;
        pImpl = nullptr;
    }
}

void VelSinSquarePlanner::set_config(double vmax, double acc, double dec, double jerk) {
    Vmax = vmax;
    Amax = acc;
    Amin = dec;
    Jerk = jerk;
}

void VelSinSquarePlanner::set_begin_pva(double s0, double v0, double a0) {
    S0 = s0;
    V0 = v0;
    A0 = a0;
}

void VelSinSquarePlanner::set_end_pva(double s, double vd, double ad) {
    Sd = s;
    Vd = vd;
    Ad = ad;
}

void VelSinSquarePlanner::build_by_phasing(double v0, double vd, double vm, double t1, double t2,
                                      double t3) {
    V0 = v0;
    Vd = vd;
    Vmax = vm;
    auto& impl = *(VelSinSquareForwarder*)pImpl;
    impl.tArr[0] = 0;
    impl.tArr[1] = t1, impl.tArr[2] = t2, impl.tArr[3] = t3;

    impl.sArr[0] = 0;
    impl.sArr[1] = (V0 + Vmax) * impl.tArr[1] / 2.0;
    impl.sArr[2] = Vmax * impl.tArr[2];
    impl.sArr[3] = (Vmax + Vd) * impl.tArr[3] / 2.0;

    impl.vArr[0] = V0;
    impl.vArr[1] = Vmax;
    impl.vArr[2] = Vmax;
    impl.vArr[3] = Vd;

    impl.aArr[0] = Amax;
    impl.aArr[1] = 0;
    impl.aArr[2] = -Amin;
    impl.aArr[3] = 0;

    impl._cumsum_time_pos();
}

int VelSinSquarePlanner::build_by_s(double s0, double sd, double v0, double vd, double Vmax, double A_m, double A_n) {
    V0 = v0;
    S0 = s0;
    Sd = sd;
    Vd = vd;
    this->Vmax = Vmax;
    this->Amax = A_m;
    this->Amin = A_n;

    return build();
}



int VelSinSquarePlanner::build() {
    auto param = *((SinSquareRelaxer*) &this->S0);
    // return param.build(*(VelSinSquareForwarder*)pImpl); 
    return velRelaxBuild<SinSquareRelaxParam>(param, *static_cast<VelSinSquareForwarder*>(pImpl));
    // SinSquareRelaxParam rParam;
    // int ret = param.relaxation(rParam);
    // static_cast<VelSinSquareForwarder*>(pImpl)->build(rParam, param);
    // return ret;
}


double VelSinSquarePlanner::get_end_time() const {
    return static_cast<VelSinSquareForwarder*>(pImpl)->get_end_time();
}

PosVelAcc VelSinSquarePlanner::get_slice(double t) const {
    return static_cast<VelSinSquareForwarder*>(pImpl)->get_slice(t);
}
int VelSinSquarePlanner::get_slice(double tm, PosVelAcc& pva) const{
    return static_cast<VelSinSquareForwarder*>(pImpl)->get_slice(tm, pva);
}

int VelSinSquarePlanner::get_slice(double tm, double* pos, double* vel, double* acc) const{
    PosVelAcc pva;
    int ret = this->get_slice(tm, pva);
    *pos = pva.pos;
    *vel = pva.vel;
    *acc = pva.acc;
    return ret;
}

// inline
int FuncVelSinSquare::operator()(const VelParam& param, VelSinSquareForwarder& fwUn){
    auto& par = * (SinSquareRelaxer*) &param;
    // return 0;
    return velRelaxBuild<SinSquareRelaxParam>(par, fwUn);

}



#if 0

PosVelAcc VelSinSquarePlanner::get_slice0(double t) const {
    PosVelAcc state;

    if (t <= 0) { 
        state.pos = 0;
        state.vel = V0;
        state.acc = 0;
        return state;
    }
    if (t >= T_total) {
        state.pos = S1 + S2 + S3;
        state.vel = Vd;
        state.acc = 0;
        return state;
    }


    if (t < T1) {
        double phase_t = t;
        double theta = M_PI * phase_t / 2 / T1;
        double sin2_theta = std::sin(2*theta);
        state.pos = V0 * phase_t + (Vmax - V0) / 2 * (phase_t - T1 / M_PI * sin2_theta);

        state.vel = V0 + (Vmax - V0) * (1-std::cos(2*theta))/2;
        double A_m = M_PI * (Vmax - V0) / 2 / T1;
        state.acc = A_m * sin2_theta;
    } else if (t < T1 + T2) {
        double phase_t = t - T1;
        state.vel = Vmax;
        state.acc = 0;
        state.pos = S1 + Vmax * phase_t;
    } else {
        double phase_t = t - T1 - T2; 
        double tau = T3 - phase_t;   
        double theta = M_PI * tau / T3 / 2;


        double sin2_theta = std::sin(2*theta);
        state.vel = Vd + (Vmax - Vd) * (1-std::cos(2*theta))/2;
        double A_n = M_PI * (Vmax - Vd) / 2 / T3;

        state.acc = -A_n * sin2_theta;


        state.pos = S1 + S2 + Vmax * phase_t + (Vd - Vmax) / 2 * (phase_t - T3 / M_PI * sin2_theta);
    }

    return state;
}

#endif
}  // namespace zmc