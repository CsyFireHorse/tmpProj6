#ifndef _ZMC_CUBIC_POLY_H_
#define _ZMC_CUBIC_POLY_H_

#include "vel/VelWrapCommon.h"
#include "common/zmc/fbt_errorcode_def.h"


namespace zmc {

struct CubicPolyOrg {
    inline double val(double x) const {
        return ((dat[0] * x + dat[1]) * x + dat[2]) * x + dat[3];
    };
    inline double diff(double x) const { return ((3 * dat[0] * x + 2 * dat[1]) * x + dat[2]); };
    inline double ddiff(double x) const { return (6 * dat[0] * x + 2 * dat[1]); };
    double dat[4];
};

struct CubicPoly : public CubicPolyOrg {
    void init(double p_0, double p_1, double v_0, double v_1) {
        dat[2] = v_0;
        dat[3] = p_0;
        dat[0] = (v_1 - v_0) - (p_1 - p_0 - v_0) * 2;
        dat[1] = (p_1 - p_0 - v_0) * 3 - (v_1 - v_0);
    }
    void init(double t0, double t1, double p0, double p1, double v0, double v1) {
        ofs = t0;
        sc = 1.0 / (t1 - t0);
        init(p0, p1, v0 / sc, v1 / sc);
    }
    inline double val(double t) const {
        double x = (t - ofs) * sc;
        return CubicPolyOrg::val(x);
    };
    inline double diff(double t) const {
        double x = (t - ofs) * sc;
        return CubicPolyOrg::diff(x) * sc;
    };
    inline double ddiff(double t) const {
        double x = (t - ofs) * sc;
        return CubicPolyOrg::ddiff(x) * sc * sc;
    };
    double sc = 1;
    double ofs = 0;
};

// use a0 instead v1
struct CubicPoly2 : public CubicPolyOrg {
    void init(double p_0, double p_1, double v_0, double a_0) {
        dat[2] = v_0;
        dat[3] = p_0;
        dat[0] = p_1 - p_0 - v_0 - a_0/2;
        dat[1] = a_0/2;
    }
    void init(double t0, double t1, double p0, double p1, double v0, double a0) {
        ofs = t0;
        sc = 1.0 / (t1 - t0);
        init(p0, p1, v0 / sc, a0 / sc / sc);
    }
    inline double val(double t) const {
        double x = (t - ofs) * sc;
        return CubicPolyOrg::val(x);
    };
    inline double diff(double t) const {
        double x = (t - ofs) * sc;
        return CubicPolyOrg::diff(x) * sc;
    };
    inline double ddiff(double t) const {
        double x = (t - ofs) * sc;
        return CubicPolyOrg::ddiff(x) * sc * sc;
    };
    double sc = 1;
    double ofs = 0;
};




struct FifthOrderPolyOrg {
    inline double val(double x) const {
        return ((((dat[0] * x + dat[1]) * x + dat[2]) * x + dat[3]) * x + dat[4]) * x + dat[5];
    };
    inline double diff(double x) const {
        return (((5 * dat[0] * x + 4 * dat[1]) * x + 3 * dat[2]) * x + 2 * dat[3]) * x + dat[4];
    };
    inline double ddiff(double x) const {
        return ((20 * dat[0] * x + 12 * dat[1]) * x + 6 * dat[2]) * x + 2 * dat[3];
    };
    inline double dddiff(double x) const {
        return (60 * dat[0] * x + 24 * dat[1]) * x + 6 * dat[2];
    };
    inline void get_pva(double x, PosVelAcc& pva) const {
        pva = PosVelAcc{val(x), diff(x), ddiff(x)}; 
    }
    inline PosVelAcc get_pva(double x) const {
        return PosVelAcc{val(x), diff(x), ddiff(x)}; 
    }

    double dat[6];
};

struct FifthOrderPoly : public FifthOrderPolyOrg {
    void init(double p_0, double p_1, double v_0, double v_1, double a_0, double a_1) {
        dat[3] = a_0 / 2;
        dat[4] = v_0;
        dat[5] = p_0;
        double qq[3] = {p_1 - dat[3] - dat[4] - dat[5], v_1 - 2 * dat[3] - dat[4],
                        a_1 / 2 - dat[3]};

        dat[0] = 6 * qq[0] - 3 * qq[1] + 1 * qq[2];
        dat[1] = -15 * qq[0] + 7 * qq[1] - 2 * qq[2];
        dat[2] = 10 * qq[0] - 4 * qq[1] + 1 * qq[2];
    }
    // depreciated
    void init(double t0, double t1, double p0, double p1, double v0, double v1, double a_0,
              double a_1) {
        ofs = t0;
        sc = 1.0 / (t1 - t0);
        init(p0, p1, v0 / sc, v1 / sc, a_0 / sc / sc, a_1 / sc / sc);
    }
    int init2(double t0, double t1, double p0, double p1, double v0, double v1, double a0,
              double a1) {
        ofs = t0;
        sc = 1.0 / (t1 - t0);
        double T = t1 - t0;
        if (T == 0) {
            return ZMC_ERR_TRAJ_INVALID_SPLINE_DATA;
        }
        // sc = T;


        double c0 = p0;
        double c1 = v0 * T;
        double c2 = a0 * T * T / 2.0;


        double D = p1 - c0 - c1 - c2;
        double E = v1 * T - c1 - 2 * c2;
        double F = (a1- a0) * T * T;


        double temp1 = E - 3 * D;
        double temp2 = F - 6 * D;
        double c5 = (temp2 - 6 * temp1) / 2.0;
        double c4 = temp1 - 2 * c5;
        double c3 = D - c4 - c5;

        dat[0] = c5;
        dat[1] = c4;
        dat[2] = c3;
        dat[3] = c2;
        dat[4] = c1;
        dat[5] = c0;
        return 0;
    }

    // inline double val(double x) const {
    //     return FifthOrderPolyOrg::val((x - ofs) * sc);
    // }
    // inline double diff(double x) const {
    //     return FifthOrderPolyOrg::diff((x - ofs) * sc) * sc;
    // }
    // inline double ddiff(double x) const {
    //     return FifthOrderPolyOrg::ddiff((x - ofs) * sc) * sc * sc;
    // }

    inline PosVelAcc get_pva(double x) const {
        double t = (x - ofs) * sc;
        return PosVelAcc{val(t), diff(t) * sc, ddiff(t) * sc * sc}; 
    }
    inline void get_pva(double x, PosVelAcc& pva) const {
        double t = (x - ofs) * sc;
        pva = PosVelAcc{val(t), diff(t) * sc, ddiff(t) * sc * sc}; 
    }
    inline void get_pvaj(double x, PosVelAccJerk& pvaj) const {
        double t = (x - ofs) * sc;
        pvaj = PosVelAccJerk{val(t), diff(t) * sc, ddiff(t) * sc * sc, dddiff(t) * sc * sc * sc}; 
    }
    inline void get_pva_normal(double t, PosVelAcc& pva) const {
        pva = PosVelAcc{val(t), diff(t) * sc, ddiff(t) * sc * sc}; 
    }
    inline void get_pvaj_normal(double t, PosVelAccJerk& pvaj) const {
        pvaj = PosVelAccJerk{val(t), diff(t) * sc, ddiff(t) * sc * sc, dddiff(t) * sc * sc * sc}; 
    }
    double unnormalize(double x)const{ return x/sc + ofs;}
    double sc = 1;
    double ofs = 0;
};

}
#endif
