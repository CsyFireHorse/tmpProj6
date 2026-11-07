#ifndef __SERVOFAK_TO_JSON_H_
#define __SERVOFAK_TO_JSON_H_

#include "common/ecat/ServoSgFaker.hpp"
#include "common/thirdlib/nlohmann/json.hpp"
#include "stdio.h"

using json = nlohmann::json;

namespace zmc {
template <typename T>
void to_dict(T& j, const AxisChecker& p) {
    j = T{{"enable_val", p.enable_val},       {"_firstFlag", p._firstFlag},
          {"cycle_freq", p.cycle_freq},       {"vel_limit", p.vel_limit},
          {"acc_limit", p.acc_limit},         {"dec_limit", p.dec_limit},
          {"jerk_limit", p.jerk_limit},       {"pos_max_limit", p.pos_max_limit},
          {"pos_min_limit", p.pos_min_limit}, {"_pos", p._pos}};  //
}

template <typename T>
void from_dict(const T& j, AxisChecker& p) {
    j.at("enable_val").get_to(p.enable_val);
    j.at("_firstFlag").get_to(p._firstFlag);
    j.at("cycle_freq").get_to(p.cycle_freq);
    j.at("vel_limit").get_to(p.vel_limit);
    j.at("acc_limit").get_to(p.acc_limit);
    j.at("dec_limit").get_to(p.dec_limit);
    j.at("jerk_limit").get_to(p.jerk_limit);
    j.at("pos_max_limit").get_to(p.pos_max_limit);
    j.at("pos_min_limit").get_to(p.pos_min_limit);
    j.at("_pos").get_to(p._pos);
}

template <typename T>
void to_dict(T& j, const ServoSgFaker& p) {
    j = T{
        {"_cnt", p._cnt},
        {"_cycle_freq", p._cycle_freq},
        {"_cycle_time", p._cycle_time},
        {"_enable_checker", p._enable_checker},
        {"_mode", p._mode},
        {"errCode", p.errCode},
        {"_statw", p._statw},
        {"_ctrl", p._ctrl},
        {"_pos_target", p._pos_target.val},
        {"_pos", p._pos.val},
        {"_pos_bak", p._pos_bak.val},
        {"_decel", p._decel},
    };
    to_dict(j["chk"], p.chk);
}
template <typename T>
void from_dict(const T& j, ServoSgFaker& p) {
    j.at("_cnt").get_to(p._cnt);
    j.at("_cycle_freq").get_to(p._cycle_freq);
    j.at("_cycle_time").get_to(p._cycle_time);
    j.at("_enable_checker").get_to(p._enable_checker);
    j.at("_mode").get_to(p._mode);
    j.at("errCode").get_to(p.errCode);
    j.at("_statw").get_to(p._statw);
    j.at("_ctrl").get_to(p._ctrl);
    j.at("_pos_target").get_to(p._pos_target.val);
    j.at("_pos").get_to(p._pos.val);
    j.at("_pos_bak").get_to(p._pos_bak.val);
    j.at("_decel").get_to(p._decel);

    from_dict<T>(j["chk"], p.chk);
}

inline void to_json(json& j, const AxisChecker& p) { to_dict(j, p); }
inline void from_json(const json& j, AxisChecker& p) { from_dict(j, p); }
inline void to_json(json& j, const ServoSgFaker& p) { to_dict(j, p); }
inline void from_json(const json& j, ServoSgFaker& p) { from_dict(j, p); }
}  // namespace zmc

#endif