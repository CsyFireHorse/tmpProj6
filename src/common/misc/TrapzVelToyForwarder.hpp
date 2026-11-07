#ifndef _TRAPZ_VEL_TOY_FORWARDER_HPP_
#define _TRAPZ_VEL_TOY_FORWARDER_HPP_

#include "math.h"
#include "polyval.h"
#include "vel/VelWrapCommon.h"

namespace zmc{

struct TrapzToyForwarder{
    double s0 = 0;
    double v0 = 3;
    double acc = 4;
    double dec = 5;
    double _vm;
    double ctArr[3] = {5, 20, 24};
    double csArr[3];
    void set_begin_pos_vel(double s_0, double v_0, double a_0 = 0){
        s0 = s_0;
        v0 = v_0;
    }
    void set_config(double vm, double acc, double dec, double ){
        _vm = vm;
        this->acc = acc;
        this->dec = dec;
    }
    inline void build(){
        _vm = polyval1(ctArr[0], acc, v0);
        csArr[0] = s0 + polyval2(ctArr[0], 0.5 * acc, v0);
        csArr[1] = csArr[0] + _vm * (ctArr[1]-ctArr[0]);
        csArr[2] = csArr[1] + polyval2(ctArr[2]-ctArr[1], -0.5 * dec, _vm);
    }
    double get_max_acc()const{
        return fabs(acc);
    }
    double get_max_dec()const{
        return fabs(dec);
    }
    double get_max_vel()const{
        return (fabs(_vm) < fabs(v0))? fabs(v0) : fabs(_vm);
    }
    inline double get_end_time()const { return ctArr[2];}
    inline double get_pos(double tm)const{ 
        if (tm < 0)   
            return s0;
        if (tm < ctArr[0]){
            return polyval2(tm, 0.5 * acc, v0, s0);
        }else if(tm  < ctArr[1]){
            return polyval1(tm-ctArr[0], _vm, csArr[0]);
        }else if(tm < ctArr[2]){
            return polyval2(tm-ctArr[1], -0.5 * dec, _vm, csArr[1]);
        }else{
            return csArr[2];
        }
    }
};


struct ToyTrapzVelPlanner{
    double s0 = 0;
    double sd = 0;
    double v0 = 3;
    double vd = 3;
    double acc = 4;
    double dec = 5;
    double V_m;
    double _vm;
    double ctArr[3] = {5, 20, 24};
    double csArr[3];
    void set_begin_pos_vel(double s_0, double v_0, double a_0 = 0){
        set_begin_pva(s_0, v_0, a_0);
    }
    void set_begin_pva(double s_0, double v_0, double a_0 = 0){
        s0 = s_0;
        v0 = v_0;
    }
    void set_end_pva(double s_d, double v_d, double a_d = 0){
        sd = s_d;
        vd = v_d;
    }
    void set_end_pos_vel(double s_d, double v_d, double a_d = 0){
        set_end_pva(s_d, v_d, a_d);
    }
    void set_config(double vm, double acc, double dec, double ){
        V_m = vm;
        this->acc = acc;
        this->dec = dec;
    }
    inline int build(){
        if (s0 <= sd){
            return _build();
        }else{
            s0 = -s0;
            sd = -sd;
            v0 = -v0;
            vd = -vd;
            int ret = _build();
            v0 = -v0;
            vd = -vd;
            s0 = -s0;
            sd = -sd;
            _vm = -_vm;
            csArr[0] = -csArr[0];
            csArr[1] = -csArr[1];
            csArr[2] = -csArr[2];
            return ret;
        }
    }
    bool is_symmetry()const{
        return v0 == vd;
    }
    inline int _build_sym(){
        double S20 = nseg20_vm2s(V_m);
        double _sDif = sd - s0;
        if (_sDif >= S20 + 1e-6){
            _vm = V_m;
            double sArr[3];

            sArr[0]= (_vm*_vm - v0*v0)/2/acc; 
            sArr[1] = _sDif - S20;
            sArr[2] = S20 - sArr[0];  

            ctArr[0] = (_vm - v0) / acc;
            ctArr[1] = ctArr[0] + (_sDif - S20) / _vm;
            ctArr[2] = ctArr[1] + (_vm - vd) / dec;
            
            csArr[0] = s0 + sArr[0];
            csArr[1] = csArr[0] + sArr[1];
            csArr[2] = csArr[1] + sArr[2];
            // csArr[2] = sd;
            return 0;
        }else {
            _vm = nseg20_s2vm(_sDif);
            double sArr[3];

            sArr[0]= (_vm*_vm - v0*v0)/2/acc; 
            sArr[1] = 0;
            sArr[2] = _sDif - sArr[0];  

            ctArr[0] = (_vm - v0) / acc;
            ctArr[1] = ctArr[0];
            ctArr[2] = ctArr[1] + (_vm - vd) / dec;
            
            csArr[0] = s0 + sArr[0];
            csArr[1] = csArr[0];
            csArr[2] = csArr[1] + sArr[2];
        }
        return 0;
    }

    inline int _build(){
        if (is_symmetry()){
            return _build_sym();
        }else{
            return _build_asym();
        }
    }
    inline int _build_asym(){    
        double S20 = nseg20_vm2s(V_m);
        double _sDif = sd - s0;
        if (_sDif >= S20 + 1e-6){
            _vm = V_m;
            double sArr[3];

            sArr[0]= (_vm*_vm - v0*v0)/2/acc; 
            sArr[1] = _sDif - S20;
            sArr[2] = S20 - sArr[0];  

            ctArr[0] = (_vm - v0) / acc;
            ctArr[1] = ctArr[0] + (_sDif - S20) / _vm;
            ctArr[2] = ctArr[1] + (_vm - vd) / dec;
            
            csArr[0] = s0 + sArr[0];
            csArr[1] = csArr[0] + sArr[1];
            csArr[2] = csArr[1] + sArr[2];
            // csArr[2] = sd;
            return 0;
        }else if (isFeasible()){
            _vm = nseg20_s2vm(_sDif);
            double sArr[3];

            sArr[0]= (_vm*_vm - v0*v0)/2/acc; 
            sArr[1] = 0;
            sArr[2] = _sDif - sArr[0];  

            ctArr[0] = (_vm - v0) / acc;
            ctArr[1] = ctArr[0];
            ctArr[2] = ctArr[1] + (_vm - vd) / dec;
            
            csArr[0] = s0 + sArr[0];
            csArr[1] = csArr[0];
            csArr[2] = csArr[1] + sArr[2];
        }else{
            return -1;
        }
        return 0;
    }
    bool isFeasible()const{
        if (v0 > vd){
            return nseg12_vm2s() >= sd - s0;
        }else if (v0 < vd){
            return nseg11_vm2s() >= sd - s0;;
        }else return true;
    }

    double nseg20_s2vm(double S) const{
        return sqrt((2*S*acc*dec +  v0*v0*dec +  vd*vd*acc)/(acc + dec));
    }
    double nseg20_vm2s(double vm) const{
        double vm2 = vm * vm;
        double S = (vm2 - v0*v0)/2/acc + (vm2 - vd*vd)/2/dec;
        return S;
    }
    double nseg11_vm2s() const{
        return (vd*vd - v0*v0) / 2 / acc;
    }
    double nseg12_vm2s() const{
        return (v0*v0 - vd*vd) / 2 / dec;
    }

    inline double get_end_time()const { return ctArr[2];}

    inline PosVelAcc get_slice(double tm)const{ 
        if (tm < 0)   
            return PosVelAcc{s0, v0, 0};
        if (tm <= ctArr[0]){
            double dt = tm;
            double acc = (s0 < sd)? this->acc: -this->acc;
            return PosVelAcc{polyval2(tm, 0.5 * acc, v0, s0), v0 + acc*dt, acc};
        }else if(tm  < ctArr[1]){
            return PosVelAcc{polyval1(tm-ctArr[0], _vm, csArr[0]), _vm, 0};
        }else if(tm <= ctArr[2]){
            double dt = tm-ctArr[1];
            double dec = (s0 < sd)? this->dec: -this->dec;
            return PosVelAcc{polyval2(dt, -0.5 * dec, _vm, csArr[1]), _vm - dec*dt, -dec};
        }else{
            return PosVelAcc{sd, vd, 0};
        }
    }

    inline int get_slice(double tm, PosVelAcc& pva)const{ 
        if (tm < 0)   
            pva = PosVelAcc{s0, v0, 0};
        if (tm <= ctArr[0]){
            double dt = tm;
            double acc = (s0 < sd)? this->acc: -this->acc;
            pva =  PosVelAcc{polyval2(tm, 0.5 * acc, v0, s0), v0 + acc*dt, acc};
        }else if(tm  < ctArr[1]){
            pva =  PosVelAcc{polyval1(tm-ctArr[0], _vm, csArr[0]), _vm, 0};
        }else if(tm <= ctArr[2]){
            double dt = tm-ctArr[1];
            double dec = (s0 < sd)? this->dec: -this->dec;
            pva =  PosVelAcc{polyval2(dt, -0.5 * dec, _vm, csArr[1]), _vm - dec*dt, -dec};
        }else{
            pva =  PosVelAcc{sd, vd, 0};
        }
        return 0;
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
struct SCurveToyForwarder{
    double s0 = 0;
    double v0 = 3;
    double a0 = 3;
    double acc = 4;
    double dec = 5;
    double J = 10;
    double _vm;
    double ctArr[7] = { 3, 5, 8, 20, 24, 25, 27};
    double csArr[7];
    double vArr[7];
    inline void build(){
        vArr[0] = polyval2(ctArr[0], J/2, a0, v0);
        vArr[1] = vArr[0] + polyval1(ctArr[1]-ctArr[0], acc);
        vArr[2] = vArr[1] + polyval2(ctArr[2]-ctArr[1], -J/2, acc);
        vArr[3] = vArr[2];
        vArr[4] = vArr[3] + polyval2(ctArr[4]-ctArr[3], -J/2, -dec);
        vArr[5] = vArr[4] + polyval1(ctArr[5]-ctArr[4], -dec);
        vArr[6] = vArr[5] + polyval2(ctArr[6]-ctArr[5], J/2, - dec);
        _vm = vArr[3];
        // TODO
        csArr[0] = polyval3(ctArr[0], J/6, 0.5 * a0, v0, s0);
        csArr[1] = csArr[0] + polyval2(ctArr[1] - ctArr[0], 0.5 * acc, v0);
        csArr[2] = csArr[1] + polyval3(ctArr[2]-ctArr[1], -J/6, 0.5 * acc, _vm);
        csArr[3] = csArr[2] + _vm * (ctArr[3] - ctArr[2]);
        csArr[4] = csArr[3] + polyval3(ctArr[4]-ctArr[3], -J/6, -0.5 * dec, _vm);
        csArr[5] = csArr[4] + polyval2(ctArr[5] - ctArr[4], -0.5 * dec, v0);
        csArr[6] = csArr[5] + polyval3(ctArr[6]-ctArr[5], J/6, -0.5 * dec, _vm);
    }

    inline double get_pos(double tm)const{ 
        if (tm < 0)   
            return s0;
        if (tm < ctArr[0]){
            return polyval3(tm, J/6, 0.5 * a0, v0);
        }else if(tm  < ctArr[1]){
            return polyval2(tm-ctArr[0], acc/2, vArr[0], csArr[0]);
        }else if(tm < ctArr[2]){
            return polyval3(tm-ctArr[1], -J/6, 0.5 * acc, vArr[1], csArr[1]);
        }else if (tm < ctArr[3]){
            return polyval1(tm-ctArr[2], _vm,  csArr[2]);
        }else if(tm  < ctArr[4]){
            return polyval3(tm-ctArr[3], -J/6, 0, vArr[3], csArr[3]);
        }else if(tm < ctArr[5]){
            return polyval2(tm-ctArr[4], -0.5 * dec, vArr[4], csArr[4]);
        }else if(tm < ctArr[6]){
            return polyval3(tm-ctArr[5], J/6, -0.5 * dec, vArr[5], csArr[5]);
        }else{
            return csArr[6];
        }
    }
};


}

#endif