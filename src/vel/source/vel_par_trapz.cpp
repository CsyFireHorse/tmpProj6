#include "VelToyTrapz.h"
#include "vel_par_trapz.h"
#include "base/include/solver.h"


namespace zmc{

double TrapzParams::nseg30_t2vm(double T) const{
    double lst[] = {-(1/A_m + 1/A_n), 2*(V_d /A_n + V_0/A_m + T), -(2*get_S() + V_0*V_0/A_m + V_d*V_d/A_n)};
    // assert  v0, vd <= vm
    double vm = quadtric_equation_solver(lst);
    return vm;
}

double TrapzParams::nseg31_t2vm(double T) const{
    double T2 = T - (V_0 - V_d)/ A_n;
    // assert  v0 >= vm  >= vd
    return (get_S() - nseg12_vm2s())/ T2;
}
double TrapzParams::nseg32_t2vm(double T) const{
    double T2 = T - (V_d - V_0)/ A_m;
    // assert  v0 <= vm  <= vd
    return (get_S() - nseg11_vm2s())/ T2;
}
double TrapzParams::nseg34_t2vm(double T) const{
    // assert  vm  <= v0, vd
    double lst[] = {(1/A_m + 1/A_n), -2*(V_d /A_m + V_0/A_n - T), (-2*get_S() + V_0*V_0/A_n + V_d*V_d/A_m)};
    double vm = quadtric_equation_solver(lst);
    return vm;
}

double TrapzParams::nseg30_vm2t(double vm) const{
    double S1 = (vm * vm - V_0 * V_0)/2 / A_m;
    double S3 = (vm * vm - V_d * V_d)/2 / A_n;
    return (get_S() - S1 - S3)/ vm + (vm - V_0) / A_m + (vm - V_d) / A_n;
}
double TrapzParams::nseg31_vm2t(double vm) const{
    double S1 = (V_0 * V_0 - vm * vm )/2 / A_n;
    double S3 = (vm * vm - V_d * V_d)/2 / A_n;
    return (get_S() - S1 - S3)/ vm + (V_0 - vm) / A_n + (vm - V_d) / A_n;
}
double TrapzParams::nseg32_vm2t(double vm) const{
    double S1 = (vm * vm - V_0 * V_0)/2 / A_m;
    double S3 = (V_d * V_d - vm * vm)/2 / A_m;
    return (get_S() - S1 - S3)/ vm + (vm - V_0) / A_m + (V_d - vm) / A_m;
}

double TrapzParams::nseg34_vm2t(double vm) const{
    double S1 = (V_0 * V_0 - vm * vm )/2 / A_n;
    double S3 = (V_d * V_d - vm * vm )/2 / A_m;
    return (get_S() - S1 - S3)/ vm + (V_0 - vm) / A_n + (V_d - vm) / A_m;
}

int TrapzParams::relaxation(double& _vm, double& _acc, double& _dec) const{
    double vd2 = V_d;
    int nseg = relaxation2(_vm, _acc, _dec, vd2);
    if (nseg && Tc > 0){
        int nseg2 = time_relaxation(_vm, _acc, _dec, vd2);
        if (nseg2) 
            return nseg2;
        else
            return nseg;
    }
    return nseg;
}

int TrapzParams::relaxation(TrapzRelaxParam& rParam) const{
    if (get_S() < 0){
        // int ret = relaxation(fw.vm, fw.am, fw.an);
        TrapzParams param2 = *this;
        param2.invert();
        param2.A_m = A_n;
        param2.A_n = A_m;
        int ret = param2.relaxation(rParam); 
        rParam.invert();
        return ret;
    }
    int ret = relaxation(rParam.vMax, rParam.aMax, rParam.aMin);
    rParam.seg_num = ret;
    return ret;
}

int TrapzParams::relaxation2(double& _vm, double& _acc, double& _dec, double& V_d2) const{
    LadderRelaxParam lrp = relaxation3(_vm, _acc, _dec, V_d2);
    _vm = lrp._vm;
    return lrp.nseg;
}

#define EPS 1e-6
struct TrapzParamsRelaxer:public TrapzParams{
    inline LadderRelaxParam relaxation(double& _vm, double& _acc, double& _dec, double& V_d2) const{
        if (is_symmetry()){
            V_d2 = V_d;
            return _relaxation_sym(_vm, _acc, _dec);
        }else{
            return _relaxation_asym(_vm, _acc, _dec, V_d2);
        }
    }
    LadderRelaxParam _relaxation_sym(double& _vm, double& _acc, double& _dec) const;
    LadderRelaxParam _relaxation_asym(double& _vm, double& _acc, double& _dec, double& V_d2) const;

    inline LadderRelaxParam node_t0() const{
        return gen_LadderRelaxParam(LADDER_0_SEGMENT, V_0);
    };
    template<LadderSegmentType...  T>
    inline LadderRelaxParam node() const{
        return gen_LadderRelaxParam(LADDER_0_SEGMENT, V_m);
    };
    
    inline LadderRelaxParam node_t30_e() const;
    inline LadderRelaxParam node_t20_e() const;

    inline LadderRelaxParam node_t30_t20_m() const{
        double S20 = nseg20_vm2s(V_m);
        double _S = get_S();
        if (_S >= S20 + EPS){
            return node_t30_e();
        }else{
            double S1x = nseg1_vm2s();
            if (get_S() >= S1x - EPS){
                return node_t20_e();
            }else{
                return node_t0x_m();
            }
        }
    }

    inline LadderRelaxParam node_t0x_m() const{
        LadderSegmentType nseg = LADDER_0_SEGMENT;
        double vm = 0;
        if ((V_0 < 0 && V_d >= 0) || (V_0 >= 0 && V_d < 0 )){
            vm = nseg23_s2vn(get_S(), V_d);
            return gen_LadderRelaxParam(LADDER_23_SEGMENT, vm); 
        }
        if (V_0 < V_d && (policy & E_VEL_POLICY_DISABLE_VD_RELAX)){
            nseg = LADDER_11_SEGMENT;
            vm = nseg11_s2vd(get_S());
            // V_d2 = _vm;
        }else if (V_0 > V_d && (policy & E_VEL_POLICY_DISABLE_VD_INC)){
            // V_d2 = nseg12_s2vd(get_S());
            vm = V_0;
            nseg = LADDER_12_SEGMENT;
        }else if (policy &  E_VEL_POLICY_DISABLE_INVERT){
            vm = nseg23_s2vn(get_S(), V_d);
            nseg = LADDER_23_SEGMENT;
        }else
            nseg = LADDER_0_SEGMENT;
        return gen_LadderRelaxParam(nseg, vm);
    }

    inline LadderRelaxParam node_t30(double vm) const{
        return gen_LadderRelaxParam(LADDER_30_SEGMENT, vm);
    };

    inline LadderRelaxParam node_t20(double vm) const{
        return gen_LadderRelaxParam(LADDER_20_SEGMENT, vm);
    };
};


    template<>
    inline LadderRelaxParam TrapzParamsRelaxer::node<LADDER_30_SEGMENT>() const{
        return gen_LadderRelaxParam(LADDER_30_SEGMENT, V_m);
    };
    template<>
    inline LadderRelaxParam TrapzParamsRelaxer::node<LADDER_21_SEGMENT>() const{
        return gen_LadderRelaxParam(LADDER_21_SEGMENT, V_d);
    };
    template<>
    inline LadderRelaxParam TrapzParamsRelaxer::node<LADDER_22_SEGMENT>() const{
        return gen_LadderRelaxParam(LADDER_22_SEGMENT, V_0);
    };
    template<>
    inline LadderRelaxParam TrapzParamsRelaxer::node<LADDER_10_SEGMENT>() const{
        return gen_LadderRelaxParam(LADDER_10_SEGMENT, V_m);
    };
    template<>
    inline LadderRelaxParam TrapzParamsRelaxer::node<LADDER_11_SEGMENT>() const{
        return gen_LadderRelaxParam(LADDER_11_SEGMENT, V_d);
    };
    template<>
    inline LadderRelaxParam TrapzParamsRelaxer::node<LADDER_12_SEGMENT>() const{
        return gen_LadderRelaxParam(LADDER_12_SEGMENT, V_0);
    };

    template<>
    inline LadderRelaxParam TrapzParamsRelaxer::node<LADDER_32_SEGMENT>() const{
        return gen_LadderRelaxParam(LADDER_32_SEGMENT, V_m);
    };
    template<>
    inline LadderRelaxParam TrapzParamsRelaxer::node<LADDER_25_SEGMENT>() const{
        return gen_LadderRelaxParam(LADDER_25_SEGMENT, V_m);
    };
    template<>
    inline LadderRelaxParam TrapzParamsRelaxer::node<LADDER_32_SEGMENT, LADDER_25_SEGMENT>() const{
        if (V_0 < V_m - EPS)
            return node<LADDER_32_SEGMENT>();
        else 
            return node<LADDER_25_SEGMENT>();
    };

    template<>
    inline LadderRelaxParam TrapzParamsRelaxer::node<LADDER_31_SEGMENT>()const{
        return gen_LadderRelaxParam(LADDER_31_SEGMENT, V_m);
    };
    template<>
    inline LadderRelaxParam TrapzParamsRelaxer::node<LADDER_24_SEGMENT>() const{
        return gen_LadderRelaxParam(LADDER_24_SEGMENT, V_m);
    };
    template<>
    inline LadderRelaxParam TrapzParamsRelaxer::node<LADDER_31_SEGMENT, LADDER_24_SEGMENT>() const{
        if (V_d < V_m - EPS)
            return node<LADDER_31_SEGMENT>();
        else 
            return node<LADDER_24_SEGMENT>();
    };
    template<>
    inline LadderRelaxParam TrapzParamsRelaxer::node<LADDER_34_SEGMENT>() const{
        return gen_LadderRelaxParam(LADDER_34_SEGMENT, V_m);
    };

inline LadderRelaxParam TrapzParamsRelaxer::node_t30_e() const{
    if (V_0 == V_m)
        return node<LADDER_22_SEGMENT>();
    else if (V_d == V_m)
        return node<LADDER_21_SEGMENT>();
    else
        return node<LADDER_30_SEGMENT>();
};
inline LadderRelaxParam TrapzParamsRelaxer::node_t20_e() const{
    double _vm = nseg20_s2vm(get_S());
    if (_vm > V_0 && _vm > V_d)
        return node_t20(_vm);
    else if (V_0 > V_d)
        return node<LADDER_12_SEGMENT>();
    else
        return node<LADDER_11_SEGMENT>();
};

LadderRelaxParam TrapzParamsRelaxer::_relaxation_sym(double& _vm, double& _acc, double& _dec) const{
    // [  T20, T3x (T30, T10, T34)]
    _vm = V_m;
    _acc = A_m;
    _dec = A_n;
    double S20 = nseg20_vm2s(V_m);
    double _S = get_S();
    if (_S >= S20 + EPS){
        if (V_0 >= V_m + EPS)
            return node<LADDER_34_SEGMENT>();
        else if (V_0 >= V_m - EPS)
            return node<LADDER_10_SEGMENT>();
        else
            return node<LADDER_30_SEGMENT>();
    }else{
        double _vm = nseg20_s2vm(_S);
        return node_t20(_vm);
    }
}

/**
 *  |v0| > vm, |vd| > vm
 *  |v0| > vm, |vd| < vm
 *      v0 > vm , vd > 0 // branch
 *      v0 > vm , vd < 0 // branch
 *      v0 < -vm , // reachable !
 *  |v0| < vm, |vd| > vm
 *      vd > vm , v0 > 0 // branch
 *      vd > vm , v0 < 0 // branch
 *      vd < -vm , // reachable !
 *  |v0| < vm, |vd| < vm
 *      v0 > 0 , vd > 0 // branch
 *      v0 < 0 , vd > 0 // branch
 *      v0 > 0 , vd < 0 // branch
 *      v0 < 0 , vd < 0 // reachable !
 * 
*/
LadderRelaxParam TrapzParamsRelaxer::_relaxation_asym(double& _vm, double& _acc, double& _dec, double& V_d2) const{
    _vm = V_m;
    _acc = A_m;
    _dec = A_n;

    double _S = get_S();
    LadderSegmentType nseg = LADDER_0_SEGMENT;

    if (V_d2 > V_m){
        if (policy & E_VEL_POLICY_DISABLE_VD_MAX_RELAX)
            V_d2 = V_m;
    }else if (V_d2 < -V_m){
        if (policy & E_VEL_POLICY_DISABLE_VD_MAX_RELAX)
            V_d2 = -V_m;
    }
    if (V_0 > V_m && V_d > V_m){
        // TODO
        return node_t0();
    }else if (V_0 > V_m && V_d >= -V_m){
        double S12 = nseg12_vm2s();
        if (_S > S12 + EPS){
            return node<LADDER_31_SEGMENT, LADDER_24_SEGMENT>();
        }else if (_S >= S12 - EPS)
            return node<LADDER_12_SEGMENT>();
        else{
            return node_t0x_m();
        }
    }else if (V_d > V_m && V_0 >= -V_m){
        double S11 = nseg11_vm2s();
        double _S = get_S();
        if (_S > S11 + EPS){
            return node<LADDER_32_SEGMENT, LADDER_25_SEGMENT>();
        }else if (_S >= S11 - EPS)
            return node<LADDER_11_SEGMENT>();
        else{
            return node_t0x_m();
        }
    }else if (V_0 < - V_m - EPS || V_d < - V_m - EPS || (V_0 <= 0 && V_d2 <= 0)){
        double S20 = nseg20_vm2s(V_m);
        if (_S >= S20 + EPS){
            return node_t30_e();
        }else{
            return node_t20_e();
        }
    }else {
        return node_t30_t20_m();
    }
}


LadderRelaxParam TrapzParams::relaxation3(double& _vm, double& _acc, double& _dec, double& V_d2) const{
    return static_cast<const TrapzParamsRelaxer*>(this)->relaxation(_vm, _acc, _dec, V_d2);
}


struct TrapzParamsTimeRelaxer:public TrapzParamsRelaxer{
    inline LadderRelaxParam relaxation(double& _vm, double& _acc, double& _dec, double& V_d2) const{
        if (is_symmetry()){
            V_d2 = V_d;
            return _relaxation_sym(_vm, _acc, _dec);
        }else{
            return _relaxation_asym(_vm, _acc, _dec, V_d2);
        }
    }
    LadderRelaxParam _relaxation_sym(double& _vm, double& _acc, double& _dec) const;
    LadderRelaxParam _relaxation_asym(double& _vm, double& _acc, double& _dec, double& V_d2) const;

    LadderRelaxParam node_time_t30_t2x() const;
    LadderRelaxParam node_time_t24_t22() const;
    LadderRelaxParam node_time_t25_t21() const;

    LadderRelaxParam node_time_t23_t34()const;

    inline LadderRelaxParam node_t34(double vm) const{
        return gen_LadderRelaxParam(LADDER_34_SEGMENT, vm);
    };
};


LadderRelaxParam TrapzParamsTimeRelaxer::_relaxation_sym(double& _vm, double& _acc, double& _dec) const{
    // [  T30(T20)  < T10 < T34 (T23) ]
    if (V_0 < 1e-6 && V_0 > -1e-6){
        _vm = nseg30_t2vm(Tc);
        return node_t30(_vm);
    }else if (V_0 < 0){
        _vm = nseg30_t2vm(Tc);
        return node_t30(_vm);
    }
    double T10 = get_S() / V_0;
    if (Tc > T10){
        double T34 = nseg34_vm2t(0);
        if (Tc > T34)
            return node_t0();
        else{
            _vm = nseg34_t2vm(Tc);
            return node_t34(_vm);
        }
    }else{
        double T30 = nseg34_vm2t(V_m);
        if (Tc < T30)
            return node_t0();
        else{
            _vm = nseg30_t2vm(Tc);
            return node_t30(_vm);
        }
    }
}


// assume distance is feasible
LadderRelaxParam TrapzParamsTimeRelaxer::_relaxation_asym(double& _vm, double& _acc, double& _dec, double& V_d2) const{
    int nseg = 0;
    if (abs(V_0) < 1e-6 || abs(V_d) < 1e-6){
        return node_time_t30_t2x();
    }else if (V_0 > V_d){
        return node_time_t24_t22();
    }else if (V_0 < V_d){
        return node_time_t25_t21();
    }
    
    if (nseg && isnan(_vm)){
        // warning
        _vm = V_m;
    }
    return node_t0();
}

LadderRelaxParam TrapzParamsTimeRelaxer::node_time_t30_t2x() const{
    double _vm;
    if (V_0 > V_d + 1e-6){
        if (V_0 < EPS){
            _vm = nseg30_t2vm(Tc);
            return LadderRelaxParam{LADDER_30_SEGMENT, _vm};
        }
        double T22 = nseg22_s2t();
        if (Tc >= T22 - 1e-6){
            _vm = nseg31_t2vm(Tc);
            return LadderRelaxParam{LADDER_31_SEGMENT, _vm};
        }else{
            _vm = nseg30_t2vm(Tc);
            return LadderRelaxParam{LADDER_30_SEGMENT, _vm};
        }
    }else if (V_0 < V_d - 1e-6){
        if (V_d < EPS){
            _vm = nseg30_t2vm(Tc);
            return LadderRelaxParam{LADDER_30_SEGMENT, _vm};
        }
        double T21 = nseg21_s2t();
        if (Tc >= T21 - 1e-6){
            _vm = nseg32_t2vm(Tc);
            return LadderRelaxParam{LADDER_32_SEGMENT, _vm};
        }else{
            _vm = nseg30_t2vm(Tc);
            return LadderRelaxParam{LADDER_30_SEGMENT, _vm};
        }
    }else {
        _vm = nseg30_t2vm(Tc);
        return LadderRelaxParam{LADDER_30_SEGMENT, _vm};
    } 
}


LadderRelaxParam TrapzParamsTimeRelaxer::node_time_t23_t34() const{
    double vn = nseg23_s2vm(get_S());
    double _vm;
    if (vn < 1e-6 ){
        _vm = nseg34_t2vm(Tc);
        return node_t34(_vm);
    }else {
        double T23 = nseg23_vm2t(vn);
        if (Tc < T23 - 1e-6){
            _vm = nseg34_t2vm(Tc);
            return node_t34(_vm);
        }else{
            // warning
            return node_t0();
        } 
    }
}
LadderRelaxParam TrapzParamsTimeRelaxer::node_time_t24_t22() const{
    // [ T30 <= T20 < T31 <= T22 < T24 < T23 < T34]
    double _vm = V_m;
    double T24 = nseg24_s2t();
    if (Tc > T24){
        return node_time_t23_t34();
    }else if (Tc > T24 - 1e-6){
        _vm = V_0;
        return LadderRelaxParam{LADDER_24_SEGMENT, _vm};
    }else{
        double T22 = nseg22_s2t();
        if (Tc >= T22 - 1e-6){
            _vm = nseg31_t2vm(Tc);
            return LadderRelaxParam{LADDER_31_SEGMENT, _vm};
        }else{
            double T20 = nseg20_s2t(get_S()); // nseg20_vm2t(_vm);
            if (Tc >= T20 - 1e-6){
                _vm = nseg30_t2vm(Tc);
                return LadderRelaxParam{LADDER_30_SEGMENT, _vm};
            }else{
                return node_t0();
            }
        }
    }
}

LadderRelaxParam TrapzParamsTimeRelaxer::node_time_t25_t21() const{
    // [ T30 <= T20 < T32 <= T21 < T25 < T23 < T34]
    double _vm = V_m;
    double T25 = nseg25_s2t();
    if (Tc > T25){
        return node_time_t23_t34();
    }else if (Tc > T25 - 1e-6){
        _vm = V_d;
        return {LADDER_25_SEGMENT, V_d};
    }else{
        double T21 = nseg21_s2t();
        if (Tc >= T21 - 1e-6){
            _vm = nseg32_t2vm(Tc);
            return LadderRelaxParam{LADDER_32_SEGMENT, _vm};
        }else{
            double T20 = nseg20_s2t(get_S()); // nseg20_vm2t(_vm);
            if (Tc >= T20 - 1e-6){
                _vm = nseg30_t2vm(Tc);
                return LadderRelaxParam{LADDER_30_SEGMENT, _vm};
            }else{
                return node_t0();;
            }
        }
    }
}

int TrapzParams::time_relaxation(double& _vm, double& _acc, double& _dec, double& V_d2) const{
    LadderRelaxParam lrp = static_cast<const TrapzParamsTimeRelaxer*>(this)->relaxation(_vm, _acc, _dec, V_d2);
    if (lrp.nseg)
        _vm = lrp._vm;
    return lrp.nseg;
}

// v0 
// + |T | J | J|
// 0 |T | T | J|
// - |T | T | T|
//    -   0   + vd
bool TrapzParams::is_feasible()const{
    if (V_0 * V_d < 0){
        return true;
    }else{
        if (V_0 > V_d){
            return get_S() >= nseg12_vm2s();
        }else if (V_0 < V_d){
            return get_S() >= nseg11_vm2s();
        }else{
            return true;
        }
    }
}
bool TrapzParams::is_symmetry()const{
    const double vDif = V_0 - V_d;
    if (vDif < EPS && vDif > -EPS)
        return true;
    else 
        return false;
}

// assume s is feasible
double TrapzParams::get_max_duration()const{
    double S = get_S();
    const double tol = 1e-6;
    double vDiff = abs(V_0 - V_d);
    if (V_0 * V_d <= 0){
        return 1e9;
    }else if (vDiff < tol && abs(V_0) < tol){
        return 1e9;
    }else if (vDiff < tol){ // T10
        return S / V_0;
    }else if (V_0 > V_d){
        return nseg24_s2t(); 
    }else{  // // T11 T25 T32
        return nseg25_s2t();
    }
}

int TrapzParams::posttune_duration(int nseg, double T, double& _vm, double& _acc, double& _dec) const{
    double vm;
    if (nseg == LadderSegmentType::LADDER_30_SEGMENT){
        _vm = nseg30_t2vm(T);
        return 0;
    }else{
        if (T <= get_max_duration()){
            _vm = nseg30_t2vm(T);
            return 0;        
        }
        return -1;
    }
}

}
