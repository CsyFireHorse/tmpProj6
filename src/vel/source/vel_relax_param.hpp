#ifndef _ZMC_VEL_RELAX_PARAM_HPP_
#define _ZMC_VEL_RELAX_PARAM_HPP_

namespace zmc{



struct TrapzRelaxParam{
    int seg_num;
    double vMax;
    double aMax;
    double aMin;
    inline int get_seg_num()const{
        return seg_num & 0xf;
    }
    inline void setInvertFlag(bool b){
        if (b)
            seg_num |= (1 << 31);
        else
            seg_num &= ~(1 << 31);
    }
    inline bool getInvertFlag()const{
        return seg_num & (1 << 31);
    }
    void invert(){
        vMax = -vMax;
        aMax = -aMax;
        aMin = -aMin;
    }
};


struct TrapzRelaxParam2{
    int seg_num;
    double vMax;
    double aMax;
    double aMin;
    inline int get_seg_num()const{
        return seg_num & 0xf;
    }
    inline void setInvertFlag(bool b){
        if (b)
            seg_num |= (1 << 31);
        else
            seg_num &= ~(1 << 31);
    }
    inline bool getInvertFlag()const{
        return seg_num & (1 << 31);
    }
    double plainTime; 
    double aHead;
    double aTail;
    void invert(){
        vMax = -vMax;
        aMax = -aMax;
        aMin = -aMin;
        aHead = -aHead;
        aTail = -aTail;
    }
};

struct SjRelaxParam {
    int seg_type;  // SLadderSegmentType output
    int err;       // error code
    double vMax;   // V_t
    double aMax;   // A_t
    double aMin;   // An_t
    double plainTime; // T_4, vel plain time
    double aMid; // default value is zero, new add, use with plainTime & vMax for T_4, 
    void invert(){
        vMax = - vMax;
        aMax = - aMax;
        aMin = - aMin;
        aMid = - aMid;
    }
};


typedef TrapzRelaxParam SinSquareRelaxParam;

template<typename T, typename P, typename F>
inline int velBuild(P& par, F& fw){
    T rParam;
    int ret = par.relaxation(rParam);
    if (ret <= 0){
        return -1;
    }
    fw.build(rParam, par);
    return 0;
}

template<typename T, typename P, typename F>
inline int velRelaxBuild(P& par, F& fw){
    T rParam;
    int ret = par.relaxation(rParam);
    if (ret){
        return ret;
    }
    fw.build(rParam, par);
    return 0;
}

}
#endif