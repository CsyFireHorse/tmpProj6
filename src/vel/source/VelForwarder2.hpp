#include "stdint.h"
#include "common/misc/polyval.h"
#include "vel/VelWrapCommon.h"
#include "VelBase.h"
#include "vel_par_sj.h"


namespace zmc{

enum EnumVelSeg:uint8_t{
	VEL_SEG_NOP,
	VEL_SEG_MEAN,
	VEL_SEG_Acc,
	VEL_SEG_DEC,
	VEL_SEG_JERK_POS,
	VEL_SEG_JERK_NEG
};


struct TimePosVelAcc {
    double time;
    double pos;
    double vel;
    double acc;

    void invert(){
        pos = -pos;
        vel = -vel;
        acc = -acc;
    }
    void get_default_slice(PosVelAcc& pva) const {
        pva.acc = acc;
        pva.vel = vel;
        pva.pos = pos;
    }
    void get_pva0(double dt, PosVelAcc& pva, double J=0) const {
        pva.acc = acc + J * dt;
        pva.vel = vel + (acc + J * dt / 2) * dt;
        pva.pos = ((J * dt / 6 + acc / 2) * dt + vel) * dt + pos;
    }
    inline void get_pva(double tm, PosVelAcc& pva, double J=0) const {
        double dt = tm - time;
        pva = PosVelAcc{polyval3(dt, J/6, 0.5 * acc, vel, pos),
                        polyval2(dt, J/2, acc, vel), 
                        polyval1(dt, J, acc)};
    }
    inline void get_pvaj(double tm, PosVelAccJerk& pva, double J=0) const {
        double dt = tm - time;
        pva = PosVelAccJerk{polyval3(dt, J/6, 0.5 * acc, vel, pos),
                        polyval2(dt, J/2, acc, vel), 
                        polyval1(dt, J, acc), J};
    }
};

template<int N=7>
struct VelForwarderGeneric{
    // bool bUseJerk = false; 
    uint8_t nSegNum = 0; 
    uint8_t nsegType = 0;
    EnumVelSeg seg_type[N];
    // uint32_t nseg;
    double J = 0;
    TimePosVelAcc tpvaArr[N+1];
    inline int get_nseg() const { return nsegType; }
    inline double get_end_pos() const { return tpvaArr[nSegNum].pos; }
    inline double get_end_time() const { return tpvaArr[nSegNum].time; }
    inline double get_rmax_vel() const { return 0; }

    inline void push_tpva(TimePosVelAcc tpva) {
        nSegNum++;
        tpvaArr[nSegNum] = tpva;
    }
    inline void push_tpva_jsg(TimePosVelAcc tpva, EnumVelSeg jType) {
        seg_type[nSegNum] = jType;
        nSegNum++;
        tpvaArr[nSegNum] = tpva;
    }

    void shift_pos(double ofs){
        for (unsigned int i=0; i<nSegNum; i++)
            tpvaArr[i].pos += ofs;
    }
    void shift_time(double ofs){
        for (unsigned int i=0; i<nSegNum; i++)
            tpvaArr[i].time += ofs;
    }
    void _cumsum(){
        for (int i = 1; i < nSegNum + 1; i++){
            tpvaArr[i].pos += tpvaArr[i - 1].pos;
            // if (tpvaArr[i-1].time < 0)
                // printf("time[%d] < 0: %f\n", i, tpvaArr[i-1].time);
            tpvaArr[i].time += tpvaArr[i - 1].time;
        }
    }
    int build(SjRelaxParam& param, VelParam& vParam) {
        nsegType = param.seg_type;
        bool has_acc_plain = nsegType == 70 || nsegType == 60 || nsegType == 62 || nsegType == 52 || nsegType == 41|| nsegType == 77 || nsegType == 79; 
        bool has_dec_plain = nsegType == 70 || nsegType == 60 || nsegType == 61 || nsegType == 51 || nsegType == 42|| nsegType == 78 || nsegType == 79; 
        bool has_vel_plain = nsegType == 70 || nsegType == 61 || nsegType == 62 || nsegType == 50 || nsegType == 10 || nsegType == 41 || nsegType == 42; 
        bool has_acc_mid = nsegType == 79;

        double aMid = has_acc_mid? param.aMid: 0;

        double A_0 = vParam.A_0;
        double V_0 = vParam.V_0;
        J = vParam.J;
        double S_0 = vParam.S_0;
        double vm = param.vMax;

        nSegNum = 0;
        tpvaArr[0] = TimePosVelAcc{0, S_0, V_0, A_0};

        auto pPar = (SLadderParams*)&vParam;
        if (param.aMax != 0){
            if (has_acc_plain){
                double J1 = J, J3 = -J;
                EnumVelSeg segTypeArr[3] = {VEL_SEG_JERK_POS, VEL_SEG_MEAN, VEL_SEG_JERK_NEG};
                if (param.aMax < A_0){
                    J1 = -J;
                    segTypeArr[0] = VEL_SEG_JERK_NEG;
                }
                if (param.aMax < 0){
                    J3 = J;
                    segTypeArr[2] = VEL_SEG_JERK_POS;
                }
                double dt = (param.aMax - A_0) / J1;
                double dt2 = SLadderParams::get_acc_plain_time_2(vm - V_0, param.aMax, J, A_0, aMid);
                double dt3 = (aMid - param.aMax) / J3;
                
                double aM = param.aMax;
                double v1 = polyval2(dt, J1/2, A_0, V_0);
                double v2 = polyval1(dt2, aM, v1); 
                double v3 = polyval2(dt3, J3/2, aM, v2); 

                double s1 = polyval3(dt, J1/6, A_0/2, V_0);
                double s2 = polyval2(dt2, aM/2, v1);
                double s3 = polyval3(dt3, J3/6, aM/2, v2);

                push_tpva_jsg(TimePosVelAcc{dt, s1, v1, aM}, segTypeArr[0]);
                push_tpva_jsg(TimePosVelAcc{dt2, s2, v2, aM}, segTypeArr[1]);
                push_tpva_jsg(TimePosVelAcc{dt3, s3, v3, aMid}, segTypeArr[2]);
            }else{
                double J1 = J, J3 = -J;
                EnumVelSeg segTypeArr[] = {VEL_SEG_JERK_POS, VEL_SEG_JERK_NEG};
                if (param.aMax < A_0){
                    J1 = -J;
                    segTypeArr[0] = VEL_SEG_JERK_NEG;
                }
                if (param.aMax < 0){
                    J3 = J;
                    segTypeArr[1] = VEL_SEG_JERK_POS;
                }
                double dt = (param.aMax - A_0) / J1;
                double dt2 = 0;
                double aM = param.aMax;
                double dt3 = -aM / J3;

                double v1 = polyval2(dt, J1/2, A_0, V_0);
                double v2 = polyval1(dt2, aM, v1); 
                double v3 = polyval2(dt3, J3/2, aM, v2); 
                double s1 = polyval3(dt, J1/6, A_0/2, V_0);
                double s3 = polyval3(dt3, J3/6, aM/2, v2);

                push_tpva_jsg(TimePosVelAcc{dt, s1, v1, aM}, segTypeArr[0]);
                // push_tpva(TimePosVelAcc{dt2, s2, v2, aM});
                push_tpva_jsg(TimePosVelAcc{dt3, s3, v3, aMid}, segTypeArr[1]);

            }
        }
        if (has_vel_plain){
            double dt4 = param.plainTime;
            double s4 = dt4 * vm;
            push_tpva_jsg(TimePosVelAcc{dt4, s4, vm, 0}, VEL_SEG_MEAN);
        }else if(has_acc_mid){
            // TODO
        }
        if (param.aMin != 0){
            if (has_dec_plain){
                double J5 = -J, J7 = J;
                EnumVelSeg segTypeArr[] = {VEL_SEG_JERK_NEG, VEL_SEG_MEAN, VEL_SEG_JERK_POS};
                if (param.aMin > 0){
                    J5 = J;
                    J7 = -J;
                    segTypeArr[0] = VEL_SEG_JERK_POS;
                    segTypeArr[2] = VEL_SEG_JERK_NEG;
                }
                    
                double dt = (param.aMin - aMid)  / J5;
                // double dt2 = static_cast<SLadderParams*>(pPar)->get_dec_plain_time(vm, vParam.V_d,  param.aMin, J);
                double dt2 = SLadderParams::get_acc_plain_time_2(vParam.V_d - vm, param.aMin, J, aMid, 0);
                double dt3 = -param.aMin / J7;
                
                double aM = param.aMin;
                double v1 = polyval2(dt, J5/2, aMid, vm);
                double v2 = polyval1(dt2, aM, v1); 
                double v3 = polyval2(dt3, J7/2, aM, v2); 

                double s1 = polyval3(dt, J5/6, aMid/2, vm);
                double s2 = polyval2(dt2, aM/2, v1);
                double s3 = polyval3(dt3, J7/6, aM/2, v2);

                push_tpva_jsg(TimePosVelAcc{dt, s1, v1, aM}, segTypeArr[0]);
                push_tpva_jsg(TimePosVelAcc{dt2, s2, v2, aM}, segTypeArr[1]);
                push_tpva_jsg(TimePosVelAcc{dt3, s3, v3, 0}, segTypeArr[2]);
            }else{
                double J5 = -J, J7 = J;
                EnumVelSeg segTypeArr[] = {VEL_SEG_JERK_NEG, VEL_SEG_JERK_POS};

                if (param.aMin > 0){
                    J5 = J;
                    J7 = -J;
                    segTypeArr[0] = VEL_SEG_JERK_POS;
                    segTypeArr[1] = VEL_SEG_JERK_NEG;
                }

                double dt = (param.aMin - aMid) / J5;
                double dt2 = 0;
                double aM = param.aMin;
                double dt3 = -aM / J7;

                double v1 = polyval2(dt, J5/2, aMid, vm);
                double v2 = polyval1(dt2, aM, v1); 
                double v3 = polyval2(dt3, J7/2, aM, v2); 
                double s1 = polyval3(dt, J5/6, aMid/2, vm);
                double s3 = polyval3(dt3, J7/6, aM/2, v2);

                push_tpva_jsg(TimePosVelAcc{dt, s1, v1, aM}, segTypeArr[0]);
                // push_tpva(TimePosVelAcc{dt2, s2, v2, aM});
                push_tpva_jsg(TimePosVelAcc{dt3, s3, v3, 0}, segTypeArr[1]);
            }
        }
        _cumsum();
        return 0;
    }
    void invert() {
        for (int i=0; i<nSegNum+1; i++){
            tpvaArr[i].invert();
        }
    }
    int get_slice_continuous(double tm, PosVelAcc& pva)const{
        if (tm < get_end_time()){
            return get_slice(tm, pva);
        }else{
            get_pva(nSegNum, tm, pva);
            return 2;
        }
    }

    int get_slice(double tm, PosVelAcc& pva) const {
        if (nSegNum == 0) {
            return -1;
        }
        if (tm <= tpvaArr[0].time) {
            tpvaArr[0].get_default_slice(pva);
            return 0;
        }
        if (tm >= tpvaArr[nSegNum].time - 1e-12) {
            tpvaArr[nSegNum].get_default_slice(pva);
            return 2;
        }
        for (int i = 0; i < nSegNum; i++) {
            if (tm < tpvaArr[i + 1].time) {
                // double dt = tm - tpvaArr[i].time;
                get_pva(i, tm, pva);
                break;
            }
        }
        return 1;
    }
    int get_slice(double time, double* pos, double* vel, double* acc) const {
        PosVelAcc pva;
        int ret = get_slice(time, pva);
        *pos = pva.pos;
        *vel = pva.vel;
        *acc = pva.acc;
        return ret;
    }

    inline void get_pva(int idx, double tm, PosVelAcc& pva) const {
        const auto& tpva = tpvaArr[idx];
        double jerk = seg_type[idx] == VEL_SEG_JERK_POS ? J : seg_type[idx] == VEL_SEG_JERK_NEG ? -J: 0;
        tpva.get_pva(tm, pva, jerk);
    }
    int check()const{
        return 0;
    }
};

}