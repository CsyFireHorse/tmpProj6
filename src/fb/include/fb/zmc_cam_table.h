#pragma once

#include "stdint.h"
#include "cam_tappet.h"
#include "mc_cam_table.h"
#include "vel/VelWrapCommon.h"
#include "mc_ref_def.h"
#include "traj/include/FifthOrderPolyWrap.h"

namespace zmc {

class CamAffineTransform;
// https://content.helpme-codesys.com/en/libs/SM3_Basic/Current/SM3_Basic/DataTypes/CAM/MC_CAM_REF.html
class CamTable : public MC_CAM_TABLE {
public:
    CamTable();
    CamTable(const CamXyva* pXyva, int len);
    CamTable(const CamXyva* pXyva, int len, const Tappet* pTappet, int tappetLen);
    ~CamTable();
    void init();
    unsigned short validate()const;
    int fillXyva(const CamXyva* pXyva, int len);
    int fillTappet(const Tappet* pTappet, int tappetLen);
    int findIntervalIndex(double x) const;
    int findDoubledIndex(double x)const;
    int build(int idx, FifthOrderPolyWrap& poly)const;
    void getPosVelAcc(double x, PosVelAcc& pva)const;
    int get_pva_affine(const PosVelAcc& masterPva, PosVelAcc &slavePva, const CamAffineTransform& affline)const;

    double getXRange()const;
    double getYRange()const;
    double calc_modulo(double x)const;
    PosVelAcc getPosVelAcc(double x)const;
};

struct CamRelStruct{
    bool masterAbsolute; // true: absolute position, false: relative position
    bool slaveAbsolute;
    double masterEngagePos; // position of master axis when cam table starts to cam in
    double slaveEngagePos; // position of slave axis when cam table starts to cam in
};

struct CamAffineParam{
    double MasterScaling;
    double SlaveScaling;
    double MasterOffset;
    double SlaveOffset;
};

struct CamAffineTransform {
    double masterScale = 1; 
    double masterIntercept = 0; 
    double slaveScale = 1; 
    double slaveIntercept = 0; 
    template<typename T>
    void fromCamRel(const CamRelStruct& camRel, const T& param){
        masterScale = param.MasterScaling;
        slaveScale = param.SlaveScaling;
        // double B =  - (camRel.masterAbsolute?camRel.masterEngagePos:0)* A  + param.MasterOffset
        // double D = param.SlaveOffset + (camRel.slaveAbsolute? camRel.slaveEngagePos:0)
        this->masterIntercept = - (camRel.masterAbsolute?0:camRel.masterEngagePos)* masterScale  + param.MasterOffset;
        this->slaveIntercept = param.SlaveOffset + (camRel.slaveAbsolute? 0: camRel.slaveEngagePos);
    }
};


class CamTableFitter{
public:
    int findIntervalIndex2(double x) const; // is quicker than findIntervalIndex
    int findDoubledIndex2(double x)const;
    //  pt[0], pt[1], pt[2], ..., pt[N-1]
    // DoubledIndex [ 0, 1, 2, 3, 4, 2*N-1]
    // 0 for begin point pt[0], 2*N-1 for end point pt[N-1], 2*i for pt[i], 
    // 2*i+1 for between pt[i] and pt[i+1]
    int get_pva(double x, PosVelAcc &pva)const;
    // return resident of doubled index
    int get_pva_or_build(double x, PosVelAcc &pva);
    int get_pva_or_build_affine(const PosVelAcc& masterPva, PosVelAcc& slavePva, const CamAffineTransform& affline);

    double getXRange()const;
    double getYRange()const;
    double calc_modulo(double x)const;
    double calc_modulo_v1(double x)const;
    double findLeftPt(double x)const;
    int detect_cross_bound(double x, double xPrev)const;
    CamTable* pCamTable;
    bool Periodic = false;
protected:
    bool _is_built = false;
    int _idx = 0;
    double _curX = 0;
    double _prevX = 0;
    FifthOrderPolyWrap _polyArr[1];
};

class CamTappetChecker : public CamTableFitter{
public:
    struct TappetTriggeredData{
        uint8_t triggerNum = 0;
        uint8_t triggerIdxArr[3] = {0};
        inline void push(uint8_t idx){
            triggerIdxArr[triggerNum++] = idx;
        }
    };
    TappetTriggeredData check_tappet()const;
    int checkProcessTappet(SMC_TappetData* pTappetData)const;
};


inline const char* getTappetActionName(int action){
    switch (action)
    {
    case TAPPETACTION_on:
        return "on";
    case TAPPETACTION_off:
        return "off";
    case TAPPETACTION_inv:
        return "inv";
    case TAPPETACTION_time:
        return "time";
    default:
        return "unknown";
    }
}

ZMC_STRING_80 make_ZMC_STRING_80(const char* s);
void assign_ZMC_STRING_80(ZMC_STRING_80*, const char* s);
}  // namespace zmc
