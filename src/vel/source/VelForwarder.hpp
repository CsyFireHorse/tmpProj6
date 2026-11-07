#include "stdint.h"

struct PosVelAcc {
    double pos;
    double vel;
    double acc;
};

struct VelSegment {
    double Tarr;
    double Sarr;
    double Varr;
    double Aarr;
    double Jarr;
    void get_slice(double dt, PosVelAcc& pva) const {
        pva.acc = Aarr + Jarr * dt;
        pva.vel = Varr + (Aarr + Jarr * dt / 2) * dt;
        pva.pos = ((Jarr * dt / 6 + Aarr / 2) * dt + Varr) * dt + Sarr;
    }
    void get_default_slice(PosVelAcc& pva) const {
        pva.acc = Aarr;
        pva.vel = Varr;
        pva.pos = Sarr;
    }
};
struct VelForwarder2 {
    enum { SZ = 9 };
    uint8_t flag;
    uint8_t seg_num;
    VelSegment segArr[SZ];

    int get_slice(double tm, PosVelAcc& pva) const {
        if (seg_num == 0) {
            return -1;
        }
        if (tm < segArr[0].Tarr) {
            segArr[0].get_default_slice(pva);
            return 0;
        }
        for (int i = 0; i < seg_num; i++) {
            if (tm < segArr[i + 1].Tarr) {
                double dt = tm - segArr[i].Tarr;
                segArr[i].get_slice(dt, pva);
                return 0;
            }
        }
        segArr[seg_num + 1].get_default_slice(pva);
        return 0;
    }
};
