/*
 * @Author: ChenShengyong 
 * @Date: 2024-06-05 17:18:36 
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-12-24 16:18:01
 */
#ifndef _POS_CHECKER_H_
#define _POS_CHECKER_H_

#include "stdint.h"
#include "math.h"


namespace zmc{
struct AxisCheckerConfig{
    double freq;
    double max_vel;
    double max_acc;
    double max_dec;
};
class AxisChecker{
public:
    enum {
        E_AXIS_ERROR_NONE,
        E_AXIS_ERROR_LIMIT_MAX,
        E_AXIS_ERROR_LIMIT_MIN,
        E_AXIS_ERROR_DIRECT_POS,
        E_AXIS_ERROR_DIRECT_NEG,
        E_AXIS_ERROR_LIMIT_VEL,
        E_AXIS_ERROR_LIMIT_ACC,
        E_AXIS_ERROR_LIMIT_DEC,
        E_AXIS_ERROR_LIMIT_JERK,
    };
    AxisChecker(){
        enable_val = 0x0f;
        init_pos(0);
    } 
    void set_config(const AxisCheckerConfig& pCfg){
        cycle_freq = pCfg.freq;
        if (pCfg.max_vel > 0)
            set_max_vel(pCfg.max_vel);
        if (pCfg.max_acc > 0)
            set_max_acc(pCfg.max_acc);
        if (pCfg.max_dec > 0)
            set_max_dec(pCfg.max_dec);
    }
    void set_cycle_freq(double freq){ cycle_freq = freq;} 
    // f(54321, 1000) = 55000;
    //  f(145, 20)=160; 
    // if limit is small while freq is big, f(33, 1000) = 1000;
    inline static double get_nearest_value(double val, double freq){
        return (0.001 + ceil(val / freq)) * freq;
    }
    void set_max_vel(double Vl){ 
        if (cycle_freq >= 1)
            vel_limit = get_nearest_value(Vl, cycle_freq);
        else
            vel_limit = (0.001 + ceil(Vl * cycle_freq)) / cycle_freq;
    }
    void set_max_acc(double ma){ 
        // acc_limit = (0.001 + ceil((0.001 + ceil(ma * cycle_freq))* cycle_freq)) / cycle_freq / cycle_freq;
        acc_limit = get_nearest_value(get_nearest_value(ma, cycle_freq), cycle_freq);
    }
    void set_max_dec(double ma){ 
        acc_limit = get_nearest_value(get_nearest_value(ma, cycle_freq), cycle_freq);
    }
    void init_pos(double pos, uint8_t flag=4){ _pos[0] = _pos[1] = _pos[2] = _pos[3] = pos; _firstFlag = flag;}
    void reset(){ _firstFlag = 4;}
    int step(double pos){
        if (_firstFlag){
            _push(pos);
            _firstFlag -= 1;
            return 0;
        }
        int ret = _check(pos);
        _push(pos);
        return ret;
    }
    inline double get_vel()const { return get_vel_diff()* cycle_freq;}
    inline double get_acc()const { return get_acc_diff()* cycle_freq* cycle_freq;}
    inline double get_jerk()const { return get_jerk_diff()* cycle_freq* cycle_freq* cycle_freq;}
    inline double get_vel_diff()const {
        return _pos[0] - _pos[1];
    }
    inline double get_acc_diff()const {
        return _pos[0] - 2*_pos[1] +  _pos[2];
    }
    inline double get_jerk_diff()const {
        return _pos[0] - 3*_pos[1] +  3 * _pos[2] - _pos[3];
    }
private:
    int _check(double pos)const{
        double vel =  (pos - _pos[0]) * cycle_freq;
        double acc = (vel - get_vel_diff()* cycle_freq) * cycle_freq;
        double acc0 = get_acc_diff() * cycle_freq * cycle_freq ;
        double jerk = (acc - acc0) * cycle_freq;
        if (enable_pos_limit && pos > pos_max_limit)
            return E_AXIS_ERROR_LIMIT_MAX;
        if (enable_pos_limit && pos < pos_min_limit)
            return E_AXIS_ERROR_LIMIT_MIN;
        if (!enable_pos_direct && vel > 0)
            return E_AXIS_ERROR_DIRECT_POS;
        if (!enable_neg_direct && vel < 0)
            return E_AXIS_ERROR_DIRECT_NEG;
        if (enable_vel_limit && fabs(vel) > vel_limit)
            return E_AXIS_ERROR_LIMIT_VEL;
        if (enable_acc_limit && fabs(acc) > acc_limit && acc0 > acc_limit)
            return E_AXIS_ERROR_LIMIT_ACC;
        if (enable_dec_limit && fabs(acc) < -dec_limit && acc0 < -dec_limit)
            return E_AXIS_ERROR_LIMIT_DEC;
        if (enable_jerk_limit && fabs(jerk) > jerk_limit)
            return E_AXIS_ERROR_LIMIT_JERK;
        return E_AXIS_ERROR_NONE;
    }
    inline void _push(double pos){
        _pos[3] = _pos[2];
        _pos[2] = _pos[1];
        _pos[1] = _pos[0];
        _pos[0] = pos;
    }
    uint8_t _firstFlag = 4;
public:
    union{
    struct {
        uint8_t enable_pos_direct:1;
        uint8_t enable_neg_direct:1;
        uint8_t enable_pos_limit:1;
        uint8_t enable_vel_limit:1;
        uint8_t enable_acc_limit:1;
        uint8_t enable_dec_limit:1;
        uint8_t enable_jerk_limit:1;
    };
    uint8_t enable_val;
    };
    // double cycle_time = 1;
    double cycle_freq = 1;
    double vel_limit = 1e9;
    double acc_limit = 1e9;
    double dec_limit = 1e9;
    double jerk_limit = 1e19;
    double pos_max_limit = 1e99;
    double pos_min_limit = -1e99;
private:
    double _pos[4];

    template<typename T>
    friend void to_dict(T& j, const AxisChecker&);
    template<typename T>
    friend void from_dict(const T& j, AxisChecker&);
};
}

#endif