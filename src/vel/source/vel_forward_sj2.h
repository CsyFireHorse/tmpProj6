/*
 * @Author: ChenShengyong
 * @Date: 2024-07-04 16:07:38
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-12-12 09:46:52
 */
#ifndef ZS_VELOCITY_FORWARD_SJ2_H_
#define ZS_VELOCITY_FORWARD_SJ2_H_

#include "VelBase.h"
#include "common/misc/polyval.h"
#include "scurve_base.h"
#include "stdint.h"
#include "vel_forward_common.h"
#include "vel_par_sj.h"


namespace zmc {

using VelForwarderSTmpl = VelForwarderCommon<7>;

template <bool IsNext = false>
inline double next_vel(const VelForwarderSTmpl& vel) {
    return vel.pPar->V_0;
};
template <>
inline double next_vel<true>(const VelForwarderSTmpl& vel) {
    return vel.vm;
};

template <bool IsNext = false>
inline double next_vel_diff(const VelForwarderSTmpl& vel) {
    return vel.vm - vel.pPar->V_0;
};
template <>
inline double next_vel_diff<true>(const VelForwarderSTmpl& vel) {
    return vel.pPar->V_d - vel.vm;
};

struct TimeVelPos3 {
    double time[3];
    double vel[3];
    double pos[3];
};

template <bool IsNext = false>
struct NoperS : public VelForwarderSTmpl {
    enum { IsValid = false };
    double get_distance(double dt) const { return 0; }
    double get_rate(double dt) const { return next_vel<IsNext>(*this); }
    double get_acc(double dt) const { return 0; }
    TimeVelPos3 build_TimeSegment() const {
        double v0 = next_vel<IsNext>(*this);
        return TimeVelPos3{0, 0, 0, v0, v0, v0, 0, 0, 0};
    }
};

/*
  acc
              ___________
            /             \
          /                \
      ___/                  \__
      s  l  s      l     s l

*/
template <bool IsNext = false>
struct AccerST : public VelForwarderSTmpl {
    enum { IsValid = true, IDX = 4 * IsNext };
    // enum { IsNext = true};
    double get_distance(double tm) const {
        double sc;
        const double J = pPar->get_signed_jerk();
        if (tm <= cTarr[IDX + 1]) {
            sc = polyval3(tm - cTarr[IDX], J / 6, pPar->A_0 / 2, Varr[IDX], cSarr[IDX]);
        } else if (tm <= cTarr[IDX + 2]) {
            sc = polyval2(tm - cTarr[IDX + 1], am / 2, Varr[IDX + 1], cSarr[1 + IDX]);
        } else if (tm <= cTarr[IDX + 3]) {
            sc = polyval3(tm - cTarr[IDX + 2], -J / 6, am / 2, Varr[IDX + 2], cSarr[IDX + 2]);
        }
        return sc;
    }
    double get_rate(double tm) const {
        double vc;
        const double J = pPar->get_signed_jerk();
        if (tm <= cTarr[IDX + 1]) {
            vc = polyval2(tm - cTarr[IDX], J / 2, pPar->A_0, Varr[IDX]);
        } else if (tm <= cTarr[IDX + 2]) {
            vc = polyval1(tm - cTarr[IDX + 1], am, Varr[IDX + 1]);
        } else if (tm <= cTarr[IDX + 3]) {
            vc = polyval2(tm - cTarr[IDX + 2], -J / 2, am, Varr[IDX + 2]);
        }
        return vc;
    }
    double get_acc(double tm) const {
        double ac;
        const double J = pPar->get_signed_jerk();
        if (tm <= cTarr[IDX + 1]) {
            ac = polyval1(tm - cTarr[IDX], J, pPar->A_0);
        } else if (tm <= cTarr[IDX + 2]) {
            ac = am;
        } else if (tm <= cTarr[IDX + 3]) {
            ac = polyval1(tm - cTarr[IDX + 2], -J, am);
        }
        return ac;
    }
    TimeVelPos3 build_TimeSegment() const {
        const double& A_0 = pPar->A_0;
        const double J = pPar->get_signed_jerk();
        double dt = (am - A_0) / J;
        double dt2 =
            static_cast<SLadderParams*>(pPar)->get_T2(vm, am, an, pPar->get_signed_jerk());
        //  (SQUARE(A_0) - 2*SQUARE(am) - 2*J*V_0 + 2*J*vm)/(2*J*am);
        double dt3 = am / J;

        double v0 = next_vel<IsNext>(*this);
        double v1 = polyval2(dt, J / 2, A_0, v0);
        double v2 = polyval1(dt2, am, v1);          // Varr[1] +am * Tarr[2];
        double v3 = polyval2(dt3, -J / 2, am, v2);  // Varr[2] +am * Tarr[3] - J*SQUARE(Tarr[3])/2;

        double s1 = polyval3(dt, J / 6, A_0 / 2, v0);
        double s2 = polyval2(dt2, am / 2, v1);
        double s3 = polyval3(dt3, -J / 6, am / 2, v2);  // am*SQUARE(Tarr[3])/2 -
                                                        // J*CUBE(Tarr[3])/6;
        return TimeVelPos3{dt, dt2, dt3, v1, v2, v3, s1, s2, s3};
    }
};

/*
  acc

          /\
         /  \
       _/    \_
      s  l    s l
*/
template <bool IsNext = false>
struct AccerS : public VelForwarderSTmpl {
    enum { IsValid = true, IDX = 4 * IsNext };
    // enum { IsNext = true};
    double get_distance(double tm) const {
        double sc;
        const double J = pPar->get_signed_jerk();
        if (tm <= cTarr[IDX + 1]) {
            sc = polyval3(tm - cTarr[IDX], J / 6, pPar->A_0 / 2, Varr[IDX], cSarr[IDX]);
        } else if (tm <= cTarr[IDX + 2]) {
            sc = polyval2(tm - cTarr[IDX + 1], am / 2, Varr[IDX + 1], cSarr[1 + IDX]);
        } else if (tm <= cTarr[IDX + 3]) {
            sc = polyval3(tm - cTarr[IDX + 2], -J / 6, am / 2, Varr[IDX + 2], cSarr[IDX + 2]);
        }
        return sc;
    }
    double get_rate(double tm) const {
        double vc;
        const double J = pPar->get_signed_jerk();
        if (tm <= cTarr[IDX + 1]) {
            vc = polyval2(tm - cTarr[IDX], J / 2, pPar->A_0, Varr[IDX]);
        } else if (tm <= cTarr[IDX + 2]) {
            vc = polyval1(tm - cTarr[IDX + 1], am, Varr[IDX + 1]);
        } else if (tm <= cTarr[IDX + 3]) {
            vc = polyval2(tm - cTarr[IDX + 2], -J / 2, am, Varr[IDX + 2]);
        }
        return vc;
    }
    double get_acc(double tm) const {
        double ac;
        const double J = pPar->get_signed_jerk();
        if (tm <= cTarr[IDX + 1]) {
            ac = polyval1(tm - cTarr[IDX], J, pPar->A_0);
        } else if (tm <= cTarr[IDX + 2]) {
            ac = am;
        } else if (tm <= cTarr[IDX + 3]) {
            ac = polyval1(tm - cTarr[IDX + 2], -J, am);
        }
        return ac;
    }
    TimeVelPos3 build_TimeSegment() const {
        const double& A_0 = pPar->A_0;
        const double J = pPar->get_signed_jerk();
        double dt = (am - A_0) / J;
        double dt2 = 0;
        double dt3 = am / J;

        double v0 = next_vel<IsNext>(*this);
        double v1 = polyval2(dt, J / 2, A_0, v0);
        double v2 = polyval1(dt2, am, v1);
        double v3 = polyval2(dt3, -J / 2, am, v2);

        double s1 = polyval3(dt, J / 6, A_0 / 2, v0);
        double s2 = polyval2(dt2, am / 2, v1);
        double s3 = polyval3(dt3, -J / 6, am / 2, v2);
        return TimeVelPos3{dt, dt2, dt3, v1, v2, v3, s1, s2, s3};
    }
};

/*
  acc
    _                 _
     \               /
      \             /
       \___________/
*/
template <bool IsNext = true>
struct DecerST : public VelForwarderSTmpl {
    enum { IsValid = true, IDX = 4 * IsNext };

    double get_distance(double tm) const {
        double sc;
        const double J = pPar->get_signed_jerk();
        if (tm <= cTarr[IDX + 1]) {
            sc = polyval3(tm - cTarr[IDX], -J / 6, 0, Varr[IDX], cSarr[IDX]);
        } else if (tm <= cTarr[IDX + 2]) {
            sc = polyval2(tm - cTarr[IDX + 1], -an / 2, Varr[IDX + 1], cSarr[1 + IDX]);
        } else if (tm <= cTarr[IDX + 3]) {
            sc = polyval3(tm - cTarr[IDX + 2], J / 6, -an / 2, Varr[IDX + 2], cSarr[IDX + 2]);
        }
        return sc;
    }
    double get_rate(double tm) const {
        double vc;
        const double J = pPar->get_signed_jerk();
        if (tm <= cTarr[IDX + 1]) {
            vc = polyval2(tm - cTarr[IDX], -J / 2, 0, Varr[IDX]);
        } else if (tm <= cTarr[IDX + 2]) {
            vc = polyval1(tm - cTarr[IDX + 1], -an, Varr[IDX + 1]);
        } else if (tm <= cTarr[IDX + 3]) {
            vc = polyval2(tm - cTarr[IDX + 2], J / 2, -an, Varr[IDX + 2]);
        }
        return vc;
    }
    double get_acc(double tm) const {
        double ac;
        const double J = pPar->get_signed_jerk();
        if (tm <= cTarr[IDX + 1]) {
            ac = polyval1(tm - cTarr[IDX], -J, 0);
        } else if (tm <= cTarr[IDX + 2]) {
            ac = -an;
        } else if (tm <= cTarr[IDX + 3]) {
            ac = polyval1(tm - cTarr[IDX + 2], J, -an);
        }
        return ac;
    }
    TimeVelPos3 build_TimeSegment() const {
        const double& A_0 = pPar->A_0;
        const double J = pPar->get_signed_jerk();
        double dt = an / J;
        double dt2 =
            static_cast<SLadderParams*>(pPar)->get_T6(vm, am, an, pPar->get_signed_jerk());
        double dt3 = an / J;

        double v0 = next_vel<IsNext>(*this);
        double v1 = polyval2(dt, -J / 2, 0, v0);
        double v2 = polyval1(dt2, -an, v1);
        double v3 = polyval2(dt3, J / 2, -an, v2);

        double s1 = polyval3(dt, -J / 6, 0, v0);
        double s2 = polyval2(dt2, -an / 2, v1);
        double s3 = polyval3(dt3, J / 6, -an / 2, v2);
        return TimeVelPos3{dt, dt2, dt3, v1, v2, v3, s1, s2, s3};
    }
};

/*
  acc
    _      _
     \    /
      \  /
       \/
*/
template <bool IsNext = true>
struct DecerS : public VelForwarderSTmpl {
    enum { IsValid = true, IDX = 4 * IsNext };

    double get_distance(double tm) const {
        double sc;
        const double J = pPar->get_signed_jerk();
        if (tm <= cTarr[IDX + 1]) {
            sc = polyval3(tm - cTarr[IDX], -J / 6, 0, Varr[IDX], cSarr[IDX]);
        } else if (tm <= cTarr[IDX + 2]) {
            sc = polyval2(tm - cTarr[IDX + 1], -an / 2, Varr[IDX + 1], cSarr[1 + IDX]);
        } else if (tm <= cTarr[IDX + 3]) {
            sc = polyval3(tm - cTarr[IDX + 2], J / 6, -an / 2, Varr[IDX + 2], cSarr[IDX + 2]);
        }
        return sc;
    }
    double get_rate(double tm) const {
        double vc;
        const double J = pPar->get_signed_jerk();
        if (tm <= cTarr[IDX + 1]) {
            vc = polyval2(tm - cTarr[IDX], -J / 2, 0, Varr[IDX]);
        } else if (tm <= cTarr[IDX + 2]) {
            vc = polyval1(tm - cTarr[IDX + 1], -an, Varr[IDX + 1]);
        } else if (tm <= cTarr[IDX + 3]) {
            vc = polyval2(tm - cTarr[IDX + 2], J / 2, -an, Varr[IDX + 2]);
        }
        return vc;
    }
    double get_acc(double tm) const {
        double ac;
        const double J = pPar->get_signed_jerk();
        if (tm <= cTarr[IDX + 1]) {
            ac = polyval1(tm - cTarr[IDX], -J, 0);
        } else if (tm <= cTarr[IDX + 2]) {
            ac = -an;
        } else if (tm <= cTarr[IDX + 3]) {
            ac = polyval1(tm - cTarr[IDX + 2], J, -an);
        }
        return ac;
    }
    TimeVelPos3 build_TimeSegment() const {
        const double& A_0 = pPar->A_0;
        const double J = pPar->get_signed_jerk();
        double dt = an / J;
        double dt2 = 0;
        double dt3 = an / J;

        double v0 = next_vel<IsNext>(*this);
        double v1 = polyval2(dt, -J / 2, 0, v0);
        double v2 = polyval1(dt2, -an, v1);
        double v3 = polyval2(dt3, J / 2, -an, v2);

        double s1 = polyval3(dt, -J / 6, 0, v0);
        double s2 = polyval2(dt2, -an / 2, v1);
        double s3 = polyval3(dt3, J / 6, -an / 2, v2);
        return TimeVelPos3{dt, dt2, dt3, v1, v2, v3, s1, s2, s3};
    }
};

/*
  acc
              ___________
            /             \
          /                \
        /                   \
      s  l  s      l     s l \___________
                                         \               /
                                          \             /
                                           \___________/
*/
template <typename R, typename T, bool HasMean>
class VelForwarderSMixer : public VelForwarderSTmpl {
   public:
    template <typename TT>
    TT& forwarder() {
        return *((TT*)this);
    }
    template <typename TT>
    const TT& forwarder() const {
        return *((const TT*)this);
    }
    int get_slice(double slice, double* position, double* velocity, double* acc) const {
        int ret = E_VEL_UNCOVERED;
        if (slice < get_begin_time())
            slice = 0;
        else if (slice >= get_end_time()) {
            // slice = get_end_time() + 1e-6;
            ret = E_VEL_COVERED;
        }
        if (position) *position = get_distance(slice);
        if (velocity) *velocity = get_rate(slice);
        if (acc) *acc = get_acc(slice);
        return ret;
    };
    double get_distance(double tm) const {
        double sc;
        if (tm <= cTarr[0]) {
            sc = cSarr[0];
        } else if (tm <= cTarr[3]) {
            sc = forwarder<R>().get_distance(tm);
        } else if (HasMean && tm <= cTarr[4]) {
            sc = cSarr[3] + this->vm * (tm - cTarr[3]);
        } else if (tm <= cTarr[7]) {
            sc = forwarder<T>().get_distance(tm);
        } else {
            if (pPar->is_continuous_vel())
                sc = cSarr[7] + (tm - cTarr[7]) * this->Varr[7];
            else
                sc = cSarr[7];
        }
        return sc;
    }
    double get_rate(double tm) const {
        double vc;
        const double& V_0 = this->Varr[0];
        if (tm < cTarr[0])
            vc = this->Varr[0];
        else if (tm <= cTarr[3]) {
            vc = forwarder<R>().get_rate(tm);
        } else if (HasMean && tm <= cTarr[4])
            vc = this->vm;
        else if (tm <= cTarr[7])
            vc = forwarder<T>().get_rate(tm);
        else
            vc = this->Varr[7];
        return vc;
    }

    double get_acc(double tm) const {
        double ac;
        if (tm < cTarr[0]) {
            ac = 0;
        } else if (tm <= cTarr[3])
            ac = forwarder<R>().get_acc(tm);
        else if (HasMean && tm <= cTarr[4])
            ac = 0;
        else if (tm <= cTarr[7])
            ac = forwarder<T>().get_acc(tm);
        else
            ac = 0;
        return ac;
    }

    void _buildTimeSegment() {
        auto& Tarr = this->Tarr;
        const auto& vm = this->vm;
        Tarr[0] = 0;
        TimeVelPos3 st = forwarder<R>().build_TimeSegment();  // (vm - pPar->V_0) / am;
        Tarr[1] = st.time[0];
        Tarr[2] = st.time[1];
        Tarr[3] = st.time[2];
        // double s13 = ((TrapzParams*) pPar)->nseg20_vm2s(vm);
        TimeVelPos3 st3 = forwarder<T>().build_TimeSegment();  // (vm - pPar->V_d) / an;
        Tarr[5] = st3.time[0];
        Tarr[6] = st3.time[1];
        Tarr[7] = st3.time[2];

        double& V_0 = this->pPar->V_0;
        Varr[0] = this->pPar->V_0;
        Varr[1] = st.vel[0];
        Varr[2] = st.vel[1];
        Varr[3] = st.vel[2];
        Varr[4] = vm;
        Varr[5] = st3.vel[0];
        Varr[6] = st3.vel[1];
        Varr[7] = st3.vel[2];

        Sarr[0] = this->pPar->S_0;
        Sarr[1] = st.pos[0];
        Sarr[2] = st.pos[1];
        Sarr[3] = st.pos[2];

        Sarr[5] = st3.pos[0];
        Sarr[6] = st3.pos[1];
        Sarr[7] = st3.pos[2];
        if (HasMean)
            Tarr[4] =
                (this->pPar->get_S() - Sarr[1] - Sarr[2] - Sarr[3] - Sarr[5] - Sarr[6] - Sarr[7]) /
                vm;
        else
            Tarr[4] = 0;
        Sarr[4] = vm * Tarr[4];
    }
    void _buildPosVecSegment() {}
    int check() const { return vel_check(*this); }
};

using VelForwarderS70 = VelForwarderSMixer<AccerST<false>, DecerST<true>, true>;
using VelForwarderS60 = VelForwarderSMixer<AccerST<false>, DecerST<true>, false>;
using VelForwarderS61 = VelForwarderSMixer<AccerS<false>, DecerST<true>, true>;
using VelForwarderS62 = VelForwarderSMixer<AccerST<false>, DecerS<true>, true>;
using VelForwarderS50 = VelForwarderSMixer<AccerS<false>, DecerS<true>, true>;
using VelForwarderS51 = VelForwarderSMixer<AccerS<false>, DecerST<true>, false>;
using VelForwarderS52 = VelForwarderSMixer<AccerST<false>, DecerS<true>, false>;
using VelForwarderS40 = VelForwarderSMixer<AccerS<false>, DecerS<true>, false>;
using VelForwarderS41 = VelForwarderSMixer<AccerST<false>, NoperS<true>, true>;
using VelForwarderS42 = VelForwarderSMixer<NoperS<false>, DecerST<true>, true>;
using VelForwarderS44 = VelForwarderSMixer<DecerS<false>, AccerS<true>, false>;
using VelForwarderS31 = VelForwarderSMixer<AccerST<false>, NoperS<true>, false>;
using VelForwarderS32 = VelForwarderSMixer<NoperS<false>, DecerST<true>, false>;
using VelForwarderS21 = VelForwarderSMixer<AccerS<false>, NoperS<true>, false>;
using VelForwarderS22 = VelForwarderSMixer<NoperS<false>, DecerS<true>, false>;
using VelForwarderS10 = VelForwarderSMixer<NoperS<false>, NoperS<true>, true>;
using VelForwarderS72 = VelForwarderSMixer<DecerS<false>, AccerS<true>, true>;

struct VelForwardSj2 : public VelForwarderS70 {
    template <typename T>
    T& forwarder() {
        return *(T*)(this);
    }
    template <typename T>
    const T& forwarder() const {
        return *(const T*)(this);
    }
    template <typename F, typename R = void>
    inline R dosth() {
        switch (get_nseg()) {
            case SLADDER_70_SEGMENT:
                return F()(forwarder<VelForwarderS70>());
            case SLADDER_60_SEGMENT:
                return F()(forwarder<VelForwarderS60>());
            case SLADDER_61_SEGMENT:
                return F()(forwarder<VelForwarderS61>());
            case SLADDER_62_SEGMENT:
                return F()(forwarder<VelForwarderS62>());
            case SLADDER_50_SEGMENT:
                return F()(forwarder<VelForwarderS50>());
            case SLADDER_51_SEGMENT:
                return F()(forwarder<VelForwarderS51>());
            case SLADDER_52_SEGMENT:
                return F()(forwarder<VelForwarderS52>());
            case SLADDER_40_SEGMENT:
                return F()(forwarder<VelForwarderS40>());
            case SLADDER_41_SEGMENT:
                return F()(forwarder<VelForwarderS41>());
            case SLADDER_42_SEGMENT:
                return F()(forwarder<VelForwarderS42>());
            case SLADDER_31_SEGMENT:
                return F()(forwarder<VelForwarderS31>());
            case SLADDER_32_SEGMENT:
                return F()(forwarder<VelForwarderS32>());
            case SLADDER_21_SEGMENT:
                return F()(forwarder<VelForwarderS21>());
            case SLADDER_22_SEGMENT:
                return F()(forwarder<VelForwarderS22>());
            case SLADDER_10_SEGMENT:
                return F()(forwarder<VelForwarderS10>());
            default:
                return F()(forwarder<VelForwarderS70>());
        }
    }
    template <typename F, typename R, typename... T>
    inline R get_val(T&&... args) const {
        switch (get_nseg()) {
            case SLADDER_70_SEGMENT:
                return F()(forwarder<VelForwarderS70>(), args...);
            case SLADDER_60_SEGMENT:
                return F()(forwarder<VelForwarderS60>(), args...);
            case SLADDER_61_SEGMENT:
                return F()(forwarder<VelForwarderS61>(), args...);
            case SLADDER_62_SEGMENT:
                return F()(forwarder<VelForwarderS62>(), args...);
            case SLADDER_50_SEGMENT:
                return F()(forwarder<VelForwarderS50>(), args...);
            case SLADDER_51_SEGMENT:
                return F()(forwarder<VelForwarderS51>(), args...);
            case SLADDER_52_SEGMENT:
                return F()(forwarder<VelForwarderS52>(), args...);
            case SLADDER_40_SEGMENT:
                return F()(forwarder<VelForwarderS40>(), args...);
            case SLADDER_41_SEGMENT:
                return F()(forwarder<VelForwarderS41>(), args...);
            case SLADDER_42_SEGMENT:
                return F()(forwarder<VelForwarderS42>(), args...);
            case SLADDER_10_SEGMENT:
                return F()(forwarder<VelForwarderS10>(), args...);
            default:
                return F()(forwarder<VelForwarderS70>(), args...);
        }
    }

   public:
    void _buildTimeSegment() { return dosth<Vel_buildTimeSegment>(); }
    void build(SjRelaxParam& rp, const VelParam& par) {
        pPar = (VelParam*)&par;
        nseg = rp.seg_type;
        vm = rp.vMax;
        am = rp.aMax;
        an = rp.aMin;
        _buildTimeSegment();
        _buildPosVecSegment();
        _cumsum();
    }
    void _buildPosVecSegment() { return dosth<Vel_buildPosVecSegment>(); }

    int get_slice(double time, double* pos, double* vel, double* acc) const {
        return get_val<Vel_get_slice, int>(time, pos, vel, acc);
    }
    double get_distance(double tm) const { return get_val<Vel_get_distance, double>(tm); }
    double get_rate(double tm) const { return get_val<Vel_get_rate, double>(tm); }
    double get_acc(double tm) const { return get_val<Vel_get_acc, double>(tm); }
    inline int check() const { return get_val<Vel_check, int>(); }
};

}  // namespace zmc

#endif