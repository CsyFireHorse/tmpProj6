#include "VelCppWrap.h"
#include "source/VelWrapImpl.h"

namespace zmc {
template <typename T>
void to_dict(T& j, const VelParam& p) {
    j = T{{"S_0", p.S_0},      {"S_d", p.S_d},
          {"V_0", p.V_0},      {"V_d", p.V_d},
          {"A_0", p.A_0},      {"A_d", p.A_d},
          {"V_m", p.V_m},      {"A_m", p.A_m},
          {"A_n", p.A_n},      {"J", p.J},
          {"J2", p.J2},        {"Tc", p.Tc},
          {"policy", p.policy}};
}
template <typename T>
void from_dict(const T& j, VelParam& p) {
    j.at("S_0").get_to(p.S_0);
    j.at("S_d").get_to(p.S_d);
    j.at("V_0").get_to(p.V_0);
    j.at("V_d").get_to(p.V_d);
    j.at("A_0").get_to(p.A_0);
    j.at("A_d").get_to(p.A_d);
    j.at("V_m").get_to(p.V_m);
    j.at("A_m").get_to(p.A_m);
    j.at("A_n").get_to(p.A_n);
    j.at("J").get_to(p.J);
    j.at("J2").get_to(p.J2);
    j.at("Tc").get_to(p.Tc);
    j.at("policy").get_to(p.policy);
}
template <typename T>
void to_dict(T& j, const VelForwardSj2& p) {
    j = T{{"vm", p.vm},     {"am", p.am},       {"an", p.an},      {"_vs", p._vs},
          {"Varr", p.Varr}, {"cTarr", p.cTarr}, {"cSarr", p.cSarr}, {"nseg", p.nseg}
          , {"niche", p.niche}};
}

template <typename T>
void from_dict(const T& j, VelForwardSj2& p) {
    j.at("nseg").get_to(p.nseg);
    j.at("niche").get_to(p.niche);
    j.at("vm").get_to(p.vm);
    j.at("am").get_to(p.am);
    j.at("an").get_to(p.an);
    j.at("_vs").get_to(p._vs);
    j.at("Varr").get_to(p.Varr);
    j.at("cTarr").get_to(p.cTarr);
    j.at("cSarr").get_to(p.cSarr);
}

template <typename T>
void to_dict(T& j, const VelocitySj& p) {
    to_dict(j["param"], p.param);
    to_dict(j["fw"], p.fw);
}
template <typename T>
void from_dict(const T& j, VelocitySj& p) {
    from_dict<T>(j["param"], p.param);
    from_dict<T>(j["fw"], p.fw);
}

template <typename T>
void to_dict(T& j, const VelocityWrapImpl& p) {
    to_dict(j, p.velSj);
}
template <typename T>
void from_dict(const T& j, VelocityWrapImpl& p) {
    from_dict<T>(j, p.velSj);
}

template <typename T>
void to_dict(T& j, const VelocityWrap& vw) {
    auto& p = ((const VelocityWrapImpl&)vw);
    to_dict(j["impl"], p);
}
template <typename T>
void from_dict(const T& j, VelocityWrap& vw) {
    auto& p = ((VelocityWrapImpl&)vw);
    from_dict<T>(j["impl"], p);
}

}  // namespace zmc