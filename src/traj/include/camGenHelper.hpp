#ifndef _ZMC_PROFILE_GEN_HELPER_H_
#define _ZMC_PROFILE_GEN_HELPER_H_

#include "math.h"
#include "vel/VelWrapCommon.h"

// #include "cam_tappet.h"

namespace zmc{

#define PI (3.14159265358979)
#define DEG2RAD (2*PI/360.0)
inline void fillXyvaToy0(Xyva* dat, int len, double scale=1, int slices=1, double xmax=360){
    for (int i=0; i<len; i++){
        dat[i].x = i*xmax/(len-1)/slices;
        double th = dat[i].x * DEG2RAD;
        dat[i].y = scale*sin(th);
        dat[i].v = scale*cos(th)*DEG2RAD;
        dat[i].a = (i==0 || i==len-1)? 0 :scale*-sin(th)*DEG2RAD*DEG2RAD;
    }
}
inline void fillXyvaToy1(Xyva* dat, int len, double scale=1, int slices=1, double xmax=360){
    for (int i=0; i<len; i++){
        dat[i].x = i*xmax/(len-1)/slices;
        double th = dat[i].x * DEG2RAD;
        dat[i].y = scale*(1-cos(th));
        dat[i].v = scale*sin(th)*DEG2RAD;
        dat[i].a = (i==0 || i==len-1)? 0 :scale*cos(th)*DEG2RAD*DEG2RAD;
    }
}


inline void fillXyvaToy2(Xyva* dat, int len, double scale=1, int slices=1, double xmax=360){
    for (int i=0; i<len; i++){
        dat[i].x = i*xmax/(len-1)/slices;
        double th = dat[i].x * DEG2RAD;
        dat[i].y = scale * (dat[i].x -  1/DEG2RAD * sin(th))*DEG2RAD;
        dat[i].v = scale * (1 - cos(th))*DEG2RAD;
        dat[i].a = scale * sin(th)*DEG2RAD*DEG2RAD;
    }
}

inline void fillXyvaToy(Xyva* dat, int len, double scale=1, int slices=1, double xmax=360, int order=2){
    if (order == 2)
        return fillXyvaToy2(dat, len, scale,  slices, xmax);
    else if (order == 1)
        return fillXyvaToy1(dat, len, scale,  slices, xmax);
    else
        return fillXyvaToy0(dat, len, scale,  slices, xmax);
}

inline void fillXyvaLinear(Xyva* dat, int len, double amp=1, double xmax=360){
    double scale = amp/xmax;
    for (int i=0; i<len; i++){
        dat[i].x = i*xmax/(len-1);
        dat[i].y = dat[i].x * scale;
        dat[i].v = scale;
        dat[i].a = 0;
    }
}
}
#endif