#include "TrapzVel.h"
#include "math.h"
#include "stdint.h"
#include "VelMixerTmpl.hpp"
#include <new>
#include "stdio.h"
#include "vel_relax_param.hpp"
#include "VelBase.h"
#include "TrapzVelForwarder.hpp"
#include "VelSinSquare.h"


namespace zmc {

#define EPS (1e-12)
#define A_EPS (1e-12)
#define V_EPS (1e-12)
#define T_EPS (1e-12)
#define SQUARE(x) ((x) * (x))

enum PVAJ_CONSTANT:unsigned char {
    POS_CONSTANT,
    VEL_CONSTANT,
    ACC_CONSTANT,
    JERK_CONSTANT,
};

template <int MAX_SEG_NUM>
void ForwarderBase<MAX_SEG_NUM>::build(const TrapzRelaxParam2& rParam, const VelParam& param) {
    // TOTEST
    seg_num = 0;
    tpvaArr[seg_num] = PosVelAccTime{0, param.S_0, param.V_0, param.A_0};

    if (rParam.aHead != 0){
        double dt = (0 - param.V_0) / rParam.aHead;
        double s = (0 + param.V_0 )* dt / 2;
        push_tpva(PosVelAccTime{dt, s, 0, rParam.aHead});
    }
    double vMax = rParam.vMax; 
    if (rParam.aMax != 0){
        double v0 = rParam.aHead==0 ? param.V_0 : 0;
        double dt = (vMax - v0) / rParam.aMax;
        double s = (vMax + v0) * dt / 2;
        push_tpva(PosVelAccTime{dt, s, vMax, rParam.aMax});
    }
    if (rParam.plainTime != 0){
        double dt = rParam.plainTime;
        double s = vMax * dt;
        push_tpva(PosVelAccTime{dt, s, vMax, 0});
    }
    if (rParam.aMin != 0){
        double vd = rParam.aTail==0 ? param.V_d : 0;
        double dt = (vd - vMax) / rParam.aMin;
        double s = (vMax + vd) * dt / 2;
        push_tpva(PosVelAccTime{dt, s, vd, rParam.aMin});
    }
    if (rParam.aTail != 0){
        double dt = (param.V_d - 0) / rParam.aTail;
        double s = (param.V_d + 0) * dt / 2;
        push_tpva(PosVelAccTime{dt, s, param.V_d, rParam.aTail});
    }
    tpvaArr[seg_num].acc = 0;
    _cumsum();
}

struct VelInnerParam0 {
    double S_0, S_d;
    double V_0, V_d;
    double A_0 = 0, A_d = 0;
    double V_m;
    double A_m;
    double A_n;
    double J;
    double Tc = 0;
    int policy = 0;

    inline double get_delta_pos() const {
        return S_d - S_0;
    }
    void shift_pos(double dS){
        S_0 += dS;
        S_d += dS;
    }
    void set_config(double vm, double acc, double dec, double jerk) {
        V_m = vm;
        A_m = acc;
        A_n = dec;
        J = jerk;
    }
    inline void set_policy_mask(bool val, uint32_t mask){
        if (val)
            policy |= mask;
        else 
            policy &= ~((unsigned int) mask);
    }
    inline bool is_continuous_vel() const {
        return policy & E_VEL_POLICY_CONSTINUOUS;
    }
    inline bool is_scurve() const {
        return policy & E_VEL_POLICY_SCURVE;
    }
    inline void set_inversable(bool v) {
        set_policy_mask(v, E_VEL_POLICY_DISABLE_INVERT);
    }
    inline void set_scurve(bool v) {
        set_policy_mask(v, E_VEL_POLICY_SCURVE);
    }
    inline void set_continuous_vel(bool v) {
        set_policy_mask(v, E_VEL_POLICY_CONSTINUOUS);
    }
    void invert(){
        S_0 = -S_0;
        S_d = -S_d;
        V_0 = -V_0;
        V_d = -V_d;
        A_0 = -A_0;
        A_d = -A_d;
    }
};


struct VelInnerParam:public VelParam{
    inline double get_end_pos() const { return S_d; }
    inline double get_end_vel() const { return V_d; }
    void set_begin_pva(double s0, double v0, double a0 = 0) {
        S_0 = s0;
        V_0 = v0;
        // A_0 = a0;
    }
    void set_end_pva(double sd, double vd, double ad = 0) {
        S_d = sd;
        V_d = vd;
        // A_d = ad;
    }
    void set_begin_pos_vel(double pos, double vel) {
        S_0 = pos;
        V_0 = vel;
    }

    void set_end_pos_vel(double pos, double vel) {
        S_d = pos;
        V_d = vel;
    }
    void set_begin_pos(double pos) {
        S_0 = pos;
    }
    void set_begin_vel(double vel) {
        V_0 = vel;
    }
    void set_end_pos(double pos) {
        S_d = pos;
    }
    void set_end_vel(double vel) {
        V_d = vel;
    }
    inline double get_delta_pos() const {
        return S_d - S_0;
    }
    double cal_stop_pos() const{
        double slowDist = V_0 * V_0 / (2.0 * A_n);
        return S_0 + ((V_0 > 0) ? slowDist : -slowDist);
    }
};

struct TrapzVelInnerParam : public VelInnerParam {
    double nseg_h_t_s() const{
        double sEx = 0;
        if (V_0 < 0)
            sEx -= (V_0 * V_0 / 2 / A_n);
        else
            sEx += (V_0 * V_0 / 2 / A_n);
        if (V_d < 0)
            sEx -= (V_d * V_d / 2 / A_m);
        else
            sEx += (V_d * V_d / 2 / A_m);
        return sEx;
    }

    double nseg20_vm2s(double vm) const {
        double vm2 = SQUARE(vm);
        double S = (vm2 - SQUARE(V_0)) / 2 / A_m + (vm2 - SQUARE(V_d)) / 2 / A_n;
        return S;
    }
    double nseg20z_vm2s(double vm) const {
        // assume V_0 = 0 && V_d = 0
        double vm2 = SQUARE(vm);
        double S = vm2 / 2 / A_m + vm2  / 2 / A_n;
        return S;
    }
    double nseg20sg_vm2s(double vm, double v0d) const {
        // assume vm > V_0 = V_d >= 0 
        double S = (SQUARE(vm) - SQUARE(v0d)) *( A_m  + A_n)/ (2 * A_m * A_n) ;
        return S;
    }
    double nseg20sg_s2vm(double S, double v0d) const {
        return sqrt((2 * S * A_m * A_n + SQUARE(v0d) * (A_n + A_m)) / (A_m + A_n));
    }

    double nseg20_s2vm(double S) const {
        return sqrt((2 * S * A_m * A_n + SQUARE(V_0) * A_n + SQUARE(V_d) * A_m) / (A_m + A_n));
    }
    double nseg20z_s2vm(double S) const {
        return sqrt((2 * S * A_m * A_n) / (A_m + A_n));
    }
    // [v0, vn, vd] , vn is less than 0 
    double nseg23_s2vn(double S) const{
        return sqrt( (A_m * SQUARE(V_0) + A_n * SQUARE(V_d) - 2 * A_m * A_n * S)/ (A_m + A_n));
    }
    double nseg23_vm2s(double vm) const {
        // {-, +}
        double vm2 = SQUARE(vm);
        double S = (SQUARE(V_0) - vm2) / 2 / A_n + (SQUARE(V_d) - vm2) / 2 / A_m;
        return S;
    }
    double nseg1_vm2s() const {
        if (V_d > V_0 + V_EPS)
            return nseg11_vm2s();
        else if (V_d < V_0 - V_EPS)
            return nseg12_vm2s();
        else
            return 0;
    }
    inline
    double nseg11_vm2s() const { return (SQUARE(V_d) - SQUARE(V_0)) / 2 / A_m; }
    inline
    double nseg12_vm2s() const { return (SQUARE(V_0) - SQUARE(V_d)) / 2 / A_n; }

    double get_S() const { return S_d - S_0; }
    bool is_vel_symmetry() const {
        const double vDif = V_0 - V_d;
        if (vDif < V_EPS && vDif > -V_EPS)
            return true;
        else
            return false;
    }
};



struct VelAcc {
    double vel;
    double acc;
};

struct TrapzVelRelaxer : public TrapzVelInnerParam {
    template <int N>
    struct VelAccArr {
        // double Sd;
        // PVAJ_CONSTANT pvajFlag[N+1]; // PVAJ_CONSTANT
        double vel[N + 1];
        double acc[N + 1];
    };

    struct VelAccFlexible {
        uint8_t len;           // actual used core length (1-4)
        uint8_t flag;          // flags: hasHead | hasTail | hasMean
        uint8_t meanIdx;       // _shift_to_end_pos index position
        VelAcc extraHead;      // extra head point
        VelAcc core[4];        // core 4 points (max 4)
        VelAcc extraTail;      // extra tail point
        
        enum {
            HAS_HEAD = 0x01,
            HAS_TAIL = 0x02,
            HAS_MEAN = 0x04,
            HAS_BUILD = 0x08,
        };
        static inline uint8_t genFlag(bool hasHead, bool hasTail, bool hasMean) {
            return (hasHead ? HAS_HEAD : 0) | (hasTail ? HAS_TAIL : 0) | (hasMean ? HAS_MEAN : 0);
        }
        // get total length (number of points)
        inline uint8_t getTotalLen() const {
            return len;
        }
        inline uint8_t getMeanIdx() const {
            return meanIdx;
        }
        inline bool hasMean() const {
            return flag & HAS_MEAN;
        }
        inline bool hasHead() const {
            return flag & HAS_HEAD;
        }
        inline bool hasTail() const {
            return flag & HAS_TAIL;
        }
        void build(){
            if (flag & HAS_BUILD) return;
            if (flag & HAS_TAIL){
                if (len < 4){
                    core[len].vel = extraTail.vel;
                    core[len].acc = 0;
                    core[len-1].acc = extraTail.acc;
                }else{
                    core[3].acc = extraTail.acc;
                    extraTail.acc = 0;
                }
            }
            if (flag & HAS_HEAD) len += 1;
            if (flag & HAS_TAIL) len += 1;
            if ((flag & HAS_HEAD) && (flag & HAS_MEAN)) {
                meanIdx ++;
            }
            flag |= HAS_BUILD;
        }
        const VelAcc* getVelAccArray()const{
            if (flag & HAS_HEAD) return &extraHead;
            else return &core[0];
        }
    };

    void fillForwardParamNew(TrapzForwarder& fw, const TrapzRelaxParam2& rParam) const {
        VelParam& param = *(VelParam*) this;
        fw.build(rParam, param);
    }

    template <int N>
    void fillForwardParam(TrapzForwarder& fw, const VelAccArr<N>& param) const {
        fw.seg_num = N;
        const auto& velArr = param.vel;
        const auto& accArr = param.acc;
        PosVelAccTime* pvat = &fw.tpvaArr[0];
        pvat[0].tm = 0;
        pvat[0].pos = S_0;
        pvat[0].vel = V_0;
        pvat[0].acc = accArr[0];
        // int mean_idx = -1;
        for (int i = 1; i < N + 1; i++) {
            double dt;
            if (fabs(accArr[i-1]) > A_EPS && fabs(velArr[i] - velArr[i - 1]) > V_EPS) 
                dt = (velArr[i] - velArr[i - 1]) / accArr[i - 1]; 
            else {
                dt = 0;
                // printf("%f,%f,%f\n", accArr[i-1], velArr[i], velArr[i - 1]);
            }
            pvat[i].tm = pvat[i - 1].tm + dt;
            pvat[i].pos = polyval2(dt, 0.5 * accArr[i - 1], velArr[i - 1], pvat[i - 1].pos);
            pvat[i].vel = velArr[i];
            pvat[i].acc = accArr[i];
        }
        // if (mean_idx >= 0){
        //     double dS = param.Sd - pvat[N].pos;
        //     double dt = dS/pvat[mean_idx].vel;
        //     for (int i = mean_idx; i < N + 1; i++){
        //        pvat[i].tm += dt;
        //        pvat[i].pos += dS; 
        //     }
        // }
    }

    void fillForwardParam2(TrapzForwarder& fw, const VelAccFlexible& param) const {
        uint8_t totalLen = param.getTotalLen();
        if (totalLen == 0 || totalLen > 6) {
            // error handling: total length exceeds expected (max 6 points 5 segments)
            return;
        }
        
        const VelAcc* vaArr = param.getVelAccArray();

        // use existing fillForwardParam logic
        fw.seg_num = totalLen - 1;  // segments = points - 1
        PosVelAccTime* pvat = &fw.tpvaArr[0];

        pvat[0].tm = 0;
        pvat[0].pos = S_0;
        pvat[0].vel = V_0;  // use initial velocity
        pvat[0].acc = vaArr[0].acc;
        
        for (int i = 1; i < totalLen; i++) {
            double dt;
            if (fabs(vaArr[i-1].acc) != 0) 
                dt = (vaArr[i].vel - vaArr[i - 1].vel) / vaArr[i - 1].acc; 
            else 
                dt = 0;
            
            pvat[i].tm = pvat[i - 1].tm + dt;
            pvat[i].pos = polyval2(dt, 0.5 * vaArr[i - 1].acc, vaArr[i - 1].vel, pvat[i - 1].pos);
            pvat[i].vel = vaArr[i].vel;
            pvat[i].acc = vaArr[i].acc;
        }
        if (param.hasMean()) {
            fw._shift_to_end_pos(S_d, param.getMeanIdx());
        }
    }
    int _relaxation_sym_overlimit(TrapzRelaxParam2& fw) const {
        double _S = get_S();
        double S23 = nseg23_vm2s(V_m);
        // _S, {ACC_CONSTANT, VEL_CONSTANT, ACC_CONSTANT, POS_CONSTANT}, 
        if (_S >= S23 - EPS) {  // LADDER_34_SEGMENT
            TrapzRelaxParam2 rParam = {3, V_m, -A_n, A_m, (_S - S23) / V_m, 0, 0};
            fw = rParam;
        } else {  // LADDER_23_SEGMENT
            double vm = nseg23_s2vn(_S);
            TrapzRelaxParam2 rParam = {2, vm, -A_n, A_m, 0, 0, 0};
            fw = rParam;
        }
        return 0;
    }
    int _relaxation_sym_inv_db(TrapzRelaxParam2& fw) const {
        double _S = get_S();
        double sEx = nseg_h_t_s();
        // if initial vel is inverse, first decel to zero, then make trapz/triangle velocity plan
        // {V_0, 0, V_m, V_m, 0, V_d}, 
        // { An, A_m, 0, -A_n ,  -An , 0}
        double S20 = nseg20z_vm2s(V_m); 
        double sRes = _S- sEx;
        if (sRes >= S20 + EPS) {
            TrapzRelaxParam2 rParam = {5, V_m, A_m, -A_n, (sRes - S20) / V_m, A_n, -A_m};
            fw = rParam;
        } else {
            double vm = nseg20z_s2vm(sRes);
            TrapzRelaxParam2 rParam = {4, vm, A_m, -A_n, 0, A_n, -A_m};
            fw = rParam;
        }
        return 0;
    }

    int _relaxation_sym_main(TrapzRelaxParam2& fw) const {
        double _S = get_S();
        double S20 = nseg20_vm2s(V_m);
        if (_S >= S20 + EPS) {
            if (V_0 <= V_m - V_EPS) {  // LADDER_30_SEGMENT
                TrapzRelaxParam2 rParam = {3, V_m, A_m, -A_n, (_S - S20) / V_m, 0, 0};
                fw = rParam;
            } else if (V_0 <= V_m + V_EPS) {  // LADDER_10_SEGMENT
                TrapzRelaxParam2 rParam = {1, V_0, 0, 0, _S / V_0, 0, 0};
                fw = rParam;
            }
        } else {
            double vm = nseg20_s2vm(_S); // LADDER_20_SEGMENT
            TrapzRelaxParam2 rParam = {2, vm, A_m, -A_n, 0, 0, 0};
            fw = rParam;
        }
        return 0;
    }

    int _relaxation_sym(TrapzRelaxParam2& fw) const {
        // assume V_0 == V_d
        double _S = get_S();
        if (V_0 > V_m + V_EPS) {
            return _relaxation_sym_overlimit(fw);
        }else if (V_0 < - V_EPS){
            return _relaxation_sym_inv_db(fw);
        } else {
            return _relaxation_sym_main(fw);
        }
    }

    int _relaxation_asym_inv_db(TrapzRelaxParam2& fw)const{
        // velocity will cross zero twice
        double _S = get_S();
        double sEx = nseg_h_t_s();
        double S20 = nseg20z_vm2s(V_m); 
        double sRes = _S - sEx;
        if (sRes >= S20 + EPS) {
            TrapzRelaxParam2 rParam = {5, V_m, A_m, -A_n, (sRes - S20) / V_m,  A_n, -A_m };
            fw = rParam;
        } else {
            double vm = nseg20z_s2vm(sRes);
            TrapzRelaxParam2 rParam = {4, vm, A_m, -A_n, 0, A_n, -A_m };
            fw = rParam;
        }
        return 0;
    }

    int _relaxation_asym_inv_sg(TrapzRelaxParam2& fw)const{
        // velocity will cross zero once
        return _relaxation_asym_inv_overlimit(fw);
        // depreciated
        double _S = get_S();
        double sEx = nseg_h_t_s();

        double sRes = _S - sEx;
        if (sRes > -EPS){
            double v0d = V_0 > V_d? V_0: V_d;
            double S20 = nseg20sg_vm2s(V_m, v0d);
            if (sRes >= S20 + EPS) {
                TrapzRelaxParam2 rParam = {4, V_m, A_m, -A_n, (sRes - S20) / V_m, (V_0 < 0 ? A_n : 0), (V_d < 0 ? -A_m : 0)};
                fw = rParam;
            } else {
                double vm = nseg20sg_s2vm(sRes, v0d);
                TrapzRelaxParam2 rParam = {3, vm, A_m, -A_n, 0, (V_0 < 0 ? A_n : 0), (V_d < 0 ? -A_m : 0)};
                fw = rParam;
            }
            return 0;
        }else{
            // core direction is inverse to S, so use negative vm
            double v0d = V_0 > V_d? V_d: V_0;
            double S23 = -nseg20sg_vm2s(V_m, v0d);
            if (-sRes >= -S23 + EPS) {
                TrapzRelaxParam2 rParam = {4, -V_m, -A_m, A_n, (sRes - S23) / (-V_m), (V_0 > 0 ? -A_n : 0), (V_d > 0 ? A_m : 0)};
                fw = rParam;
            } else {
                double vm = -nseg20sg_s2vm(-sRes, v0d);
                TrapzRelaxParam2 rParam = {3, vm, -A_m, A_n, 0, (V_0 > 0 ? -A_n : 0), (V_d > 0 ? A_m : 0)};
                fw = rParam;
            }
            return 0;
        }
    }

    int _relaxation_asym(TrapzRelaxParam2& fw) const {
        // V_0, V_d, S
        // +, +, + 
        // +, -, + 
        // -, +, +
        // -, -, +
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
        }else{
            return _relaxation_asym_inv_overlimit(fw);
        }
    }

    int _relaxation_inv_unreachable(TrapzRelaxParam2& fw) const {
        // vm < MIN(V_0, V_d) && (V_0 > 0 && V_d < 0 || V_0 < 0 && V_d > 0)
        double _S = get_S();
        double sEx = nseg_h_t_s();
        double sRes = _S - sEx;
        double v0d = V_0 > V_d? V_d: V_0;
        double S23 = -nseg20sg_vm2s(V_m, v0d);
        if (-sRes >= -S23 + EPS) {
            TrapzRelaxParam2 rParam = {4, -V_m, -A_m, A_n, (sRes - S23) / (-V_m), (V_0 > 0 ? -A_n : 0), (V_d > 0 ? A_m : 0)};
            fw = rParam;
        } else {
            double vm = -nseg20sg_s2vm(-sRes, v0d);
            TrapzRelaxParam2 rParam = {3, vm, -A_m, A_n, 0, (V_0 > 0 ? -A_n : 0), (V_d > 0 ? A_m : 0)};
            fw = rParam;
        }
        return 0;
    }

    int _relaxation_unreachable(TrapzRelaxParam2& fw) const {
        // V_0 , V_d > 0
        double _S = get_S();
        double sEx = nseg_h_t_s();
        double sRes = _S - sEx;
        double S23 = -nseg20z_vm2s(V_m);
        if (-sRes >= -S23  + EPS) {
            TrapzRelaxParam2 rParam = {5, -V_m, -A_m, A_n, (sRes - S23) / (-V_m), (V_0 > 0 ? -A_n : 0), (V_d > 0 ? A_m : 0)};
            fw = rParam;
        } else {
            double vm = -nseg20z_s2vm(-sRes);
            TrapzRelaxParam2 rParam = {4, vm, -A_m, A_n, 0, (V_0 > 0 ? -A_n : 0), (V_d > 0 ? A_m : 0)};
            fw = rParam;
        }
        return 0;
    }
    // deprecated
    int _relaxation_unreachable0(TrapzForwarder& fw) const {
        double vm = 0;
        if (V_0 < 0 && V_d >= 0 || V_0 >= 0 && V_d < 0 ){
            vm = -nseg23_s2vn(get_S());
            VelAccArr<2> velAccArr = { {V_0, vm, V_d}, {-A_n, A_m, 0}};
            fillForwardParam(fw, velAccArr);
            return 0;
        }
        // if (V_0 < V_d && (policy & E_VEL_POLICY_DISABLE_VD_RELAX)){
        //     nseg = LADDER_11_SEGMENT;
        //     vm = nseg11_s2vd(get_S());
        //     // V_d2 = _vm;
        // }else if (V_0 > V_d && (policy & E_VEL_POLICY_DISABLE_VD_INC)){
        //     // V_d2 = nseg12_s2vd(get_S());
        //     vm = V_0;
        //     nseg = LADDER_12_SEGMENT;
        if (policy &  E_VEL_POLICY_DISABLE_INVERT){
            vm = -nseg23_s2vn(get_S());
            VelAccArr<2> velAccArr = { {V_0, vm, V_d}, {-A_n, A_m, 0}};
            fillForwardParam(fw, velAccArr);
            return 0;
        }else{
            // VelAccArr<2> velAccArr = { {V_0, V_0, V_0}, {-A_n, A_m, 0}};
            // fillForwardParam(fw, velAccArr);
            // return gen_LadderRelaxParam(nseg, vm);
            return -1;
        }
    }

    int _relaxation_asym_inv_overlimit(TrapzRelaxParam2& fw) const {
        // V_0 < 0 && V_d > V_m  || V_0 > V_m && V_d < 0
        double _S = get_S();
        double sEx = nseg_h_t_s();
        double sRes = _S - sEx;
        if (sRes > -EPS){
            if (V_0 > V_m + V_EPS) { // LADDER_32_SEGMENT
                TrapzRelaxParam2 rParam = {4, V_m, -A_n, -A_n, sRes / V_m, 0, -A_m};
                fw = rParam;
            } else if (V_d > V_m + V_EPS) { // LADDER_31_SEGMENT
                TrapzRelaxParam2 rParam = {4, V_m, A_m, A_m, sRes / V_m, A_n, 0};
                fw = rParam;
            }else{
                double v0d = V_0 > V_d? V_0: V_d;
                double S20 = nseg20sg_vm2s(V_m, v0d);
                if (sRes >= S20 + EPS) { // LADDER_30_SEGMENT
                    TrapzRelaxParam2 rParam = {4, V_m, A_m, -A_n, (sRes - S20) / V_m, (V_0 < 0 ? A_n : 0), (V_d < 0 ? -A_m : 0)};
                    fw = rParam;
                } else {
                    double vm = nseg20sg_s2vm(sRes, v0d);
                    TrapzRelaxParam2 rParam = {3, vm, A_m, -A_n, 0, (V_0 < 0 ? A_n : 0), (V_d < 0 ? -A_m : 0)};
                    fw = rParam;
                }
            }
        }else{
            if (V_0 < -V_m) { 
                TrapzRelaxParam2 rParam = {4, -V_m, A_n, A_n, sRes / -V_m, 0, A_m};
                fw = rParam;
            } else if (V_d < -V_m) { 
                TrapzRelaxParam2 rParam = {4, -V_m, -A_m, -A_m, sRes / -V_m, -A_n, 0};
                fw = rParam;
            }else
                return _relaxation_inv_unreachable(fw);
        }
        return 0;
    }

    int _relaxation_asym_overlimit_db(TrapzRelaxParam2& fw) const {
        double _S = get_S();
        double S23 = nseg23_vm2s(V_m);
        if (_S >= S23 - EPS) {  // LADDER_34_SEGMENT
            TrapzRelaxParam2 rParam = {3, V_m, -A_n, A_m, (_S - S23) / V_m, 0, 0};
            fw = rParam;
            return 0;
        } else {
            double sEx = nseg_h_t_s();
            if (_S > sEx - EPS){
                double vn = nseg23_s2vn(_S);
                TrapzRelaxParam2 rParam = {2, vn, -A_n, A_m, 0, 0, 0};
                fw = rParam;
                return 0;
            }else
                return _relaxation_unreachable(fw);
        }
    }
    int _relaxation_asym_overlimit_sg(TrapzRelaxParam2& fw) const {
        double _S = get_S();
        if (V_0 >= V_m + V_EPS) {
            double S12 = nseg12_vm2s();
            if (_S > S12 + EPS) {  // LADDER_32_SEGMENT
                TrapzRelaxParam2 rParam = {3, V_m, -A_n, -A_n, (_S - S12) / V_m, 0, 0};
                fw = rParam;
                return 0;
            } else if (_S >= S12 - EPS) {  // LADDER_12_SEGMENT
                TrapzRelaxParam2 rParam = {1, V_d, -A_n, 0, 0, 0, 0};
                fw = rParam;
                return 0;
            } else {
                return _relaxation_unreachable(fw);
            }
        } else if (V_d >= V_m + V_EPS) {
            double S11 = nseg11_vm2s();
            if (_S > S11 + EPS) {  // LADDER_31_SEGMENT
                TrapzRelaxParam2 rParam = {3, V_m, A_m, A_m, (_S - S11) / V_m, 0, 0};
                fw = rParam;
                return 0;
            } else if (_S >= S11 - EPS) {  // LADDER_11_SEGMENT
                TrapzRelaxParam2 rParam = {1, V_d, A_m, 0, 0, 0, 0};
                fw = rParam;
                return 0;
            } else {
                return _relaxation_unreachable(fw);
            }
        }else{
			return -1;
		}
        return 0;
    }
    int _relaxation_asym_main_inlimit(TrapzRelaxParam2& fw) const {
        double _S = get_S();
        double S20 = nseg20_vm2s(V_m);
        if (_S >= S20 + EPS) { // LADDER_30_SEGMENT
            TrapzRelaxParam2 rParam = {3, V_m, A_m, -A_n, (_S - S20) / V_m, 0, 0};
            fw = rParam;
            return 0; // LADDER_22_SEGMENT, LADDER_21_SEGMENT
        } else {
            double S1x = nseg1_vm2s();
            if (get_S() >= S1x - EPS) {
                double vm = nseg20_s2vm(_S);
                TrapzRelaxParam2 rParam = {2, vm, A_m, -A_n, 0, 0, 0};
                fw = rParam;
                return 0;
            } else {
                return _relaxation_unreachable(fw);
            }
        }
    }

    int _relaxation_asym_main(TrapzRelaxParam2& fw) const {
        return _relaxation_asym_main_inlimit(fw);
    }

    // template <typename T>
    // int build(T& fw) {
    //     return relaxation(fw);
    // }

    int relaxation(TrapzRelaxParam2& fw) {
        // velRelaxBuild treats non-zero as failure; zero acc/dec cannot form a profile.
        if (A_m <= 0 || A_n <= 0)
            return -1;
        if (S_0 > S_d){
            auto par = *this;
            par.invert();
            int ret = par._relaxation(fw);
            fw.invert();
            return ret;
        }
        return _relaxation(fw);
    }

    int _relaxation(TrapzRelaxParam2& fw) {
        // assume S_d >= S_0
        if (is_vel_symmetry()) {
            return _relaxation_sym(fw);
        } else {
            return _relaxation_asym(fw);
        }
    }
};



class TrapzBuilder{
public:
    int operator()(const VelInnerParam& param, TrapzForwarder& fw){
        TrapzVelRelaxer& impl = (TrapzVelRelaxer&)param;

        // TrapzRelaxParam2 rParam;
        // int ret = impl.relaxation(rParam);
        // if (ret < 0){
        //     return -1;
        // }
        // fw.build(rParam, (const VelParam&)impl);
        // return 0;
        return velRelaxBuild<TrapzRelaxParam2>(impl, fw);
        // return impl.build(fw);
    }
};


class TrapzVelMixer:public VelMixer2Tmpl<VelInnerParam, TrapzForwarder, TrapzBuilder>{
    public:
};


TrapzVel::TrapzVel() { 
    static_assert(sizeof(TrapzVelMixer) == sizeof(TrapzVel), "sizeof not right");
    new (this) TrapzVelMixer;
}
TrapzVel::TrapzVel(const TrapzVel& other){
    new (this) TrapzVelMixer((const TrapzVelMixer&) other);
}
TrapzVel& TrapzVel::operator=(const TrapzVel& other){
    ((TrapzVelMixer&)(*this)) = (TrapzVelMixer&) other;
    return *this;
}

TrapzVel::~TrapzVel() {
    ((TrapzVelMixer&)(*this)).~TrapzVelMixer();
}
void TrapzVel::set_inversable(bool v) { 
    ((TrapzVelMixer&)(*this)).set_inversable(v);
}
void TrapzVel::set_scurve(bool v) {
    ((TrapzVelMixer&)*this).set_scurve(v);
}
void TrapzVel::set_continuous_vel(bool v) {
    ((TrapzVelMixer&)*this).set_continuous_vel(v);
}

void TrapzVel::set_config(double vm, double acc, double dec, double jerk) {
    ((TrapzVelMixer&)*this).set_config(vm, acc, dec, jerk);
}

void TrapzVel::set_begin_pos_vel(double pos, double vel) {
    ((TrapzVelMixer&)*this).set_begin_pos_vel(pos, vel);
}
void TrapzVel::set_end_pos_vel(double pos, double vel) {
    ((TrapzVelMixer&)*this).set_end_pos_vel(pos, vel);
}

void TrapzVel::set_begin_pva(double s0, double v0, double a0) {
    ((TrapzVelMixer&)*this).set_begin_pva(s0, v0, a0);
}
void TrapzVel::set_end_pva(double sd, double vd, double ad) {
    ((TrapzVelMixer&)*this).set_end_pva(sd, vd, ad);
}

void TrapzVel::set_begin_pos(double pos) {
    ((TrapzVelMixer&)*this).set_begin_pos(pos);
}
void TrapzVel::set_begin_vel(double vel) {
    ((TrapzVelMixer&)*this).set_begin_vel(vel);
}
void TrapzVel::set_end_pos(double pos) {
    ((TrapzVelMixer&)*this).set_end_pos(pos);
}
void TrapzVel::set_end_vel(double vel) {
    ((TrapzVelMixer&)*this).set_end_vel(vel);
}
int TrapzVel::build() {
    return ((TrapzVelMixer&)*this).build();
}

double TrapzVel::get_end_time() const {
    return ((const TrapzVelMixer&)*this).get_end_time();
}
double TrapzVel::get_end_pos() const {
    return ((const TrapzVelMixer&)*this).get_end_pos();
}
double TrapzVel::get_delta_pos() const {
    return ((const TrapzVelMixer&)*this).get_delta_pos();
}
double TrapzVel::get_max_vel() const {
    return ((const TrapzVelMixer&)*this).get_max_vel();
}
int TrapzVel::get_slice(double tm, PosVelAcc& pva) const {
    return ((const TrapzVelMixer&)*this).get_slice(tm, pva);
}
int TrapzVel::get_slice(double tm, double* pos, double* vel, double* acc) const {
    return ((const TrapzVelMixer&)*this).get_slice(tm, pos, vel, acc);
}

PosVelAcc TrapzVel::get_slice(double tm) const{
    return ((const TrapzVelMixer&)*this).get_slice(tm);
}
int TrapzVel::get_slice_continuous(double tm, PosVelAcc& pva) const {
    return ((const TrapzVelMixer&)*this).get_slice_continuous(tm, pva);
}
void TrapzVel::chop_left_time(double tm) {
    ((TrapzVelMixer&)*this).chop_left_time(tm);
}
void TrapzVel::shift_time(double dt) {
    ((TrapzVelMixer&)*this).shift_time(dt);
}
void TrapzVel::shift_pos(double pos) {
    ((TrapzVelMixer&)*this).shift_pos(pos);
}
double TrapzVel::cal_reachable_backward_vel(double target_vel) const {
    return ((const TrapzVelMixer&)*this).cal_reachable_backward_vel(target_vel);
}
double TrapzVel::cal_reachable_forward_vel(double current_vel) const {
    return ((const TrapzVelMixer&)*this).cal_reachable_forward_vel(current_vel);
}
double TrapzVel::cal_stop_pos() const {
    return ((const TrapzVelMixer&)*this).cal_stop_pos();
}


int FuncVelTrapz3::operator()(const VelParam& param, TrapzForwarder& fwUn){
    auto& par = * (TrapzVelRelaxer*) &param;
    return velRelaxBuild<TrapzRelaxParam2>(par, fwUn);

}

}  // namespace zmc

// Explicit template instantiation
template void zmc::ForwarderBase<5>::build(const TrapzRelaxParam2& rParam, const VelParam& param);
