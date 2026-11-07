#ifndef _ZMC_TRAPZ_VEL_FORWARDER_HPP_
#define _ZMC_TRAPZ_VEL_FORWARDER_HPP_

#include "common/misc/polyval.h"
#include "vel/VelWrapCommon.h"
#include "stdint.h"

namespace zmc{

#define T_EPS (1e-12)
class TrapzRelaxParam2;
class VelParam;

template <int MAX_SEG_NUM = 5>
struct ForwarderBase {
    uint8_t seg_num = 3;
    uint8_t seg_order = 2;
    // double TArr[MAX_SEG_NUM + 1];  // T0, T1 T2, T3
    // double Parr[MAX_SEG_NUM + 1];  // S0, S1, S2, Sd
    // double VArr[MAX_SEG_NUM + 1];  // V0, V1, V2, Vd
    // double AArr[MAX_SEG_NUM + 1];  // A_0, A_1, A_2, A_d
    // double JArr[N];
    PosVelAccTime tpvaArr[MAX_SEG_NUM + 1];
    int check()const{ return 0; }
    double get_rmax_vel()const { return 0;}
    void push_tpva(const PosVelAccTime& pvat){
        seg_num ++;
        tpvaArr[seg_num] = pvat;
        // imporant !
        tpvaArr[seg_num-1].acc = pvat.acc;
    }
	
    void invert() {
        for (int i=0; i<seg_num+1; i++){
            tpvaArr[i].pos = -tpvaArr[i].pos;
            tpvaArr[i].vel = -tpvaArr[i].vel;
            tpvaArr[i].acc = -tpvaArr[i].acc;
        }
    }
    void shift_pos(double dS){
        for (int i=0; i<seg_num+1; i++){
            tpvaArr[i].pos += dS;
        }
    }
    void shift_time(double dt){
        for (int i=0; i<seg_num+1; i++){
            tpvaArr[i].tm += dt;
        }
    }

    inline double get_end_pos() const { return tpvaArr[seg_num].pos; }
    inline double get_end_time() const { return tpvaArr[seg_num].tm; }
    enum {
        BEGIN_PT = 0,
        MIDDLE_PT = 1,
        END_PT = 2,
    };
    inline int get_slice(double tm, PosVelAcc& pva) const {
        if (tm <= tpvaArr[0].tm) {
            getPva(tpvaArr[0], pva);
            return BEGIN_PT;
        } else if (tm >= tpvaArr[seg_num].tm - T_EPS) {
            getPva(tpvaArr[seg_num], pva);
            return END_PT;
        }
        int idx = 0;
        for (int i=1; i <= seg_num; i++){
            if (tm < tpvaArr[i].tm){
                idx = i-1;
                break;
            }
        }
        // double dt = tm - TArr[idx];
        // pva = PosVelAcc{polyval2(dt, 0.5 * Aarr[idx], Varr[idx], Sarr[idx]),
        //                 polyval2(dt, 0, Aarr[idx], Varr[idx]), polyval1(dt, 0, Aarr[idx])};

        get_pva(idx, tm, pva);
        return MIDDLE_PT;
    }
    inline int get_slice_continuous(double tm, PosVelAcc& pva) const {
        if (tm >= tpvaArr[seg_num].tm) {
            get_pva(seg_num, tm, pva);
            return END_PT;
        }else{
            return get_slice(tm, pva);
        }
    }

    inline void get_pva(int idx, double tm, PosVelAcc& pva) const {
        auto& tpva = tpvaArr[idx];
        double dt = tm - tpva.tm;
        pva = PosVelAcc{polyval2(dt, 0.5 * tpva.acc, tpva.vel, tpva.pos),
                        polyval2(dt, 0, tpva.acc, tpva.vel), 
                        polyval1(dt, 0, tpva.acc)};
    }
    inline void get_pvaj(int idx, double tm, PosVelAcc& pva) const {
        auto& tpva = tpvaArr[idx];
        double dt = tm - tpva.tm;
        double jerk = (tpvaArr[idx + 1].acc - tpva.acc) / (tpvaArr[idx + 1].tm - tpva.tm);
        pva = PosVelAcc{polyval3(dt, jerk / 6, 0.5 * tpva.acc, tpva.vel, tpva.pos),
                        polyval2(dt, jerk / 2, tpva.acc, tpva.vel), polyval1(dt, jerk, tpva.acc)};
    }
    int get_slice(double tm, double* pos, double*  vel, double* acc)const{
        PosVelAcc pva;
        int ret = get_slice(tm, pva);
        if (pos) *pos = pva.pos;
        if (vel) *vel = pva.vel;
        if (acc) *acc = pva.acc;
        return ret;
    }
    void _shift_to_end_pos(double Sd, int idx){
        double ds = Sd - get_end_pos();
        double dt = ds / tpvaArr[idx].vel;
        for (int i = idx; i < seg_num + 1; i++){
            tpvaArr[i].tm += dt;
            tpvaArr[i].pos += ds;
        }
    }
    inline void _shift_pos_time(double dt, double ds, int index){
        for (int i = index; i < seg_num + 1; i++){
            tpvaArr[i].tm += dt;
            tpvaArr[i].pos += ds;
        }
    }
    void insert_mean(double Sd, int idx){
        double dS = Sd - get_end_pos();
        double dt = dS/tpvaArr[idx].vel;
        seg_num ++;
        for (int i = seg_num; i > idx; i--){
            tpvaArr[i].tm = tpvaArr[i - 1].tm + dt;
            tpvaArr[i].pos = tpvaArr[i - 1].pos + dS;
            tpvaArr[i].vel = tpvaArr[i - 1].vel;
            tpvaArr[i].acc = tpvaArr[i - 1].acc;
        }
        tpvaArr[idx].acc = 0;
    }
    void insert(int index, const PosVelAccTime& pva) {
        seg_num ++;
        for (int i = seg_num; i > index; i--){
            tpvaArr[i].tm = tpvaArr[i - 1].tm;
            tpvaArr[i].pos = tpvaArr[i - 1].pos;
            tpvaArr[i].vel = tpvaArr[i - 1].vel;
            tpvaArr[i].acc = tpvaArr[i - 1].acc;
        }
    }

    void append(const PosVelAccTime& pva) {
        seg_num ++;
        tpvaArr[seg_num] = pva;
        tpvaArr[seg_num-1].acc = pva.acc;
        tpvaArr[seg_num].acc = 0;
    }

    void _cumsum(){
        for (int i = 1; i < seg_num + 1; i++){
            tpvaArr[i].pos += tpvaArr[i - 1].pos;
            tpvaArr[i].tm += tpvaArr[i - 1].tm;
        }
    }

    void build(const TrapzRelaxParam2& rParam, const VelParam& param);
};

using TrapzForwarder = ForwarderBase<5>;
class TrapzForwarderContinuous:public TrapzForwarder{  
public:
    inline int get_slice(double tm, PosVelAcc& pva) const {
        return get_slice_continuous(tm, pva);
    }
};

}

#endif