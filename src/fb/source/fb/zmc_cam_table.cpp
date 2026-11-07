#include "zmc_cam_table.h"
#include "fbt_errorcode_def.h"
#include <math.h>
#include <stddef.h>
#include "stdio.h"
#include "find_db_index.hpp"
#include "string.h"

namespace zmc {

// enum EnumTappetActionImpl{
//     TAPPET_ACTION_INV = 0,
//     TAPPET_ACTION_ON = 1,
//     TAPPET_ACTION_OFF = 2,
//     TAPPET_ACTION_NOP = 3,
// };

union UnTappetActive{
    uint32_t value = 0xffffffff;
    struct {
        // because of initial value is all high, so we has to save value by reverse bit 
        uint8_t _padding: 5;
        bool bNotHasInit: 1;
        bool bRevRestartCamIn: 1;
        // [bRevOn, bRevOff]: 00-> inverse bit, 01-> on, 10-> off, 11-> no change
        // uint8_t bAction: 2;
        // need to update by init value
        // bool bRevTappet : 1;

        // bool bRevInitValue : 1;
        // bool bRevSetInitValueAtReset : 1;
    };
    // inline bool getTappet()const {
    //     return !bRevTappet;
    // }
    // void setInitValueAndApply(bool bInitValue){
    //     if (bAction == TAPPET_ACTION_INV){
    //         bAction = TAPPET_ACTION_NOP;
    //         bRevTappet = bInitValue;
    //     }else{
    //         // no need to do anything
    //     }
    // }
    // void apply(uint16_t cta){
    //     switch (cta){
    //         case TAPPETACTION_on:
    //             bAction = TAPPET_ACTION_ON;
    //             bRevTappet = !true;
    //             break;
    //         case TAPPETACTION_off:
    //             bAction = TAPPET_ACTION_OFF;
    //             bRevTappet = !false;
    //             break;
    //         case TAPPETACTION_inv:
    //             if (bAction == TAPPET_ACTION_INV){
    //                 bAction = TAPPET_ACTION_NOP;
    //                 bRevTappet = !bRevTappet;
    //             }else if (bAction == TAPPET_ACTION_ON){
    //                 bAction = TAPPET_ACTION_OFF;
    //                 bRevTappet = !bRevTappet;
    //             }else if (bAction == TAPPET_ACTION_OFF){
    //                 bAction = TAPPET_ACTION_ON;
    //                 bRevTappet = !bRevTappet;
    //             }else {
    //                 bAction = TAPPET_ACTION_INV;
    //                 bRevTappet = !bRevTappet;
    //             }
    //             break;
    //         case TAPPETACTION_time:
    //             bRevTappet = true;
    //             break;
    //         default:
    //             break;
    //     }
    // }
};
#define MAX_TRIGGERED_IDX 3

CamTable::CamTable() {
    init();  
}
CamTable::~CamTable() {
    // delete[] pce;
    // delete[] pt;
}

void CamTable::init(){
    wCamStructID = 0xDC34;
    byType = mcCamTypeXYVA;
    nElements = 0;
    pce = (uint8_t*) nullptr;
    pt = nullptr;
    nTappets = 0;
    xStart = 0.;  
    xEnd = 360.;  

    strCAMName = nullptr; // ZMC_STRING
    byVarType = 0;  

    dwTappetActiveBits = 0;  //    Internal variable
    byInterpolationQuality = 5;  // 1 1: Linear interpolation, 3: Cubic interpolation
    byCompatibilityMode = 0; 
    bChangedOnline = false;          //   Internal variable
    xPartofLM = false;
}

CamTable::CamTable(const CamXyva* pXyva, int len){
    init();
    nElements = len;
    pce = (uint8_t*) pXyva;
    pt = nullptr;
    nTappets = 0;
    xStart = pXyva[0].dX;  
    xEnd = pXyva[len-1].dX;  
}

CamTable::CamTable(const CamXyva* pXyva, int len, const Tappet* pTappet, int tappetLen){
    init();
    nElements = len;
    pce = (uint8_t*) pXyva;
    pt = (SMC_CAMTappet*) pTappet;
    nTappets = tappetLen;
    xStart = pXyva[0].dX;  
    xEnd = pXyva[len-1].dX;  
}
int CamTable::fillXyva(const CamXyva* pXyva, int len){
    if (pXyva == nullptr || len <= 0) {
        return -1;
    }
    pce = (uint8_t*) pXyva;
    nElements = len;
    // xStart = pXyva[0].dX;  
    // xEnd = pXyva[len-1].dX;  
    return 0;
}
int CamTable::fillTappet(const Tappet* pTappet, int tappetLen){
    if (pTappet == nullptr || tappetLen <= 0) {
        return -1;
    }
    pt = (SMC_CAMTappet*) pTappet;
    nTappets = tappetLen;
    return 0;
}

unsigned short CamTable::validate()const{
// 1. Basic pointer and type validation
    if (this == NULL) {
        return ZMC_NO_CAM_REF_TYPE;
    }
    
    if (wCamStructID != 0xDC34) {
        return ZMC_NO_CAM_REF_TYPE;
    }
    
    if (byType != mcCamTypeXYVA) {
        return ZMC_NO_CAM_REF_TYPE;
    }
    
    // 2. Master range validation
    if (xStart >= xEnd) {
        return SMC_CAM_TABLE_EMPTY_MASTER_RANGE;
    }
    
    // Check for invalid master values (NaN or infinity)
    if (isnan(xStart) || isnan(xEnd) || 
        isinf(xStart) || isinf(xEnd)) {
        return SMC_CAM_TABLE_INVALID_MASTER_MINMAX;
    }
    
    // 3. Data validation
    if (nElements > 0) {
        // Data pointer must exist if element count > 0
        if (pce == NULL) {
            return ZMC_CAM_INVALID_ELEMENT_COUNT;
        }
        
        // Access the XYVA data
        const CamXyva* xyvaData = (const CamXyva*)pce;
        
        // Check first and last points against range
        if (fabs(xyvaData[0].dX - xStart) > 1e-10 || 
            fabs(xyvaData[nElements-1].dX - xEnd) > 1e-10) {
            return SMC_CAM_TABLE_DOES_NOT_COVER_MASTER_SCALE;
        }
        
        // Validate data values
        double prevX = xyvaData[0].dX;
        
        for (int i = 0; i < nElements; i++) {
            // Check for NaN or infinity in XYVA values
            if (isnan(xyvaData[i].dX) || isnan(xyvaData[i].dY) || 
                isnan(xyvaData[i].dV) || isnan(xyvaData[i].dA) ||
                isinf(xyvaData[i].dX) || isinf(xyvaData[i].dY) || 
                isinf(xyvaData[i].dV) || isinf(xyvaData[i].dA)) {
                return SMC_CAM_TABLE_INVALID_MASTER_MINMAX;
            }
            
            // Check for monotonically increasing X values (master position)
            if (i > 0) {
                if (xyvaData[i].dX <= prevX) {
                    return SMC_CAM_TABLE_INVALID_MASTER_MINMAX;
                }
                prevX = xyvaData[i].dX;
            }
        }
        
        // Find min and max Y values for slave range validation
        double yMin = xyvaData[0].dY;
        double yMax = xyvaData[0].dY;
        
        for (int i = 1; i < nElements; i++) {
            if (xyvaData[i].dY < yMin) yMin = xyvaData[i].dY;
            if (xyvaData[i].dY > yMax) yMax = xyvaData[i].dY;
        }
        
        // Slave range must have difference
        if (yMin == yMax) {
            return SMC_CAM_TABLE_INVALID_SLAVE_MINMAX;
        }
    }
    else {
        // No elements is only valid if we're initializing
        // If we have master range outside of the standard initialization range,
        // we should have elements
        if (xStart != 0.0 || xEnd != 360.0) {
            return ZMC_CAM_TABLE_INVALID_DATA;
        }
    }
    
    // 4. Tappet validation
    if (nTappets > 0) {
        if (pt == NULL) {
            return ZMC_CAM_TABLE_INVALID_DATA;
        }
        
        // Validate each tappet
        const SMC_CAMTappet* tappets = pt;
        for (int i = 0; i < nTappets; i++) {
            // Check tappet position is within master range
            if (tappets[i].x < xStart || tappets[i].x > xEnd) {
                return SMC_CI_MASTER_OUT_OF_SCALE;
            }
        }
    }
    
    return ZMC_NO_ERROR;
}

int CamTable::findIntervalIndex(double x) const{
    auto pce = (CamXyva*)this->pce;
    int nPoints = nElements;
    
    // Check if x is outside the entire cam profile range
    if (x < pce[0].dX) return 0;  // Return first interval if before start
    if (x > pce[nPoints-1].dX) return nPoints-2;  // Return last interval if beyond end
    
    // Find the interval containing x
    for (int i = 0; i < nPoints-1; i++) {
        if (x >= pce[i].dX && x <= pce[i+1].dX) {
            return i;
        }
    }
    return -1;
}
int CamTable::findDoubledIndex(double x)const {
    if ( !pce || nElements < 2) {
        return -1;
    }
    return zmc::findDoubledIndex(x, (Xyva*)pce, nElements);
}

void CamTable::getPosVelAcc(double x, PosVelAcc& pva)const{
    const auto& pce = (const CamXyva*)this->pce;
    if (x <= xStart ) {
        pva.pos = pce[0].dY;
        pva.vel = pce[0].dV;
        pva.acc = pce[0].dA;
        return;
    }else if (x >= xEnd) {
        pva.pos = pce[nElements-1].dY;
        pva.vel = pce[nElements-1].dV;
        pva.acc = pce[nElements-1].dA;
        return;
    }
    int idx;
    for (int i = 0; i < nElements-1; i++) {
        if (x >= pce[i].dX && x <= pce[i+1].dX) {
            idx = i;
            break;
        }
    }
    FifthOrderPolyWrap poly;
    build(idx, poly);
    poly.get_pva(x, &pva);
}

int CamTable::build(int idx, FifthOrderPolyWrap& poly)const{
    if (idx < 0 || idx >= nElements - 1) {
        return -1;
    }
    auto pCam = (CamXyva*)  pce;
    if (pCam[idx].dX > pCam[idx+1].dX) {
        return -1;
    }
    poly.init(pCam[idx].dX, pCam[idx+1].dX, pCam[idx].dY, pCam[idx+1].dY, pCam[idx].dV, pCam[idx+1].dV, pCam[idx].dA, pCam[idx+1].dA);
    return 0;
}

double CamTable::getXRange()const{
    auto pXyva = (const CamXyva*)pce;
    return pXyva[nElements-1].dX - pXyva[0].dX;
}
double CamTable::getYRange()const{
    auto pXyva = (const CamXyva*)pce;
    return pXyva[nElements-1].dY - pXyva[0].dY;
}
double CamTable::calc_modulo(double x)const{
    auto pXyva = (const CamXyva*)pce;
    double range = pXyva[nElements-1].dX - pXyva[0].dX;
    double xM = fmod(x - pXyva[0].dX, range);
    return xM < 0 ? (xM + range) : xM;
}

int CamTable::get_pva_affine(const PosVelAcc& masterPva, PosVelAcc &slavePva, const CamAffineTransform& affline)const{
    const double& A = affline.masterScale;
    const double& B = affline.masterIntercept;
    const double& C = affline.slaveScale;
    const double& D = affline.slaveIntercept;

    PosVelAcc pvaCam;
    double x = masterPva.pos* A + B;

    double tableLookupPos = calc_modulo(x);
    int iTurn = (x - tableLookupPos)/getXRange();
    getPosVelAcc(tableLookupPos, pvaCam);
    pvaCam.pos += iTurn * getYRange();

    slavePva.pos = pvaCam.pos * C + D;
    slavePva.vel = pvaCam.vel* C * masterPva.vel ;
    slavePva.acc = ( C * pvaCam.acc * A* A* masterPva.vel *masterPva.vel) + 
                 ( C * pvaCam.vel * A* masterPva.acc);
    return 0;
}

int CamTableFitter::findIntervalIndex2(double x) const{
    auto pce = (CamXyva*)pCamTable->pce;
    int nPoints = pCamTable->nElements;
    
    if (x < pce[0].dX) return 0;
    if (x > pce[nPoints-1].dX) return nPoints-2;
    
    // Find the interval containing x
    for (int i = _idx; i < nPoints-1; i++) {
        if (x >= pce[i].dX && x <= pce[i+1].dX) {
            return i;
        }
    }
    for (int i = 0; i < _idx; i++) {
        if (x >= pce[i].dX && x <= pce[i+1].dX) {
            return i;
        }
    }
    return -1;
}

int CamTableFitter::findDoubledIndex2(double x)const {
    auto pce = (const CamXyva*)pCamTable->pce;
    int nPoints = pCamTable->nElements;
    // Find the interval containing x
    if ( x >= pce[_idx].dX){
        if (x == pce[nPoints-1].dX) 
            return 2*_idx;
        else if (x >= pce[nPoints-1].dX) 
            return 2*nPoints-2;
        else{
            for (int i = _idx; i < nPoints-1; i++) {
                double dx = x - pce[i+1].dX;
                if (dx < 1e-6) {
                    return 2*i - 1;
                }else if (dx < 0) {
                    return 2*i;
                }
            }
        }
    }else{
        if (x <= pce[0].dX) 
            return 0;
        else {
            for (int i = 0; i < _idx; i++) {
                double dx = x - pce[i+1].dX;
                if (dx < 1e-6) {
                    return 2*i - 1;
                }else if (dx < 0) {
                    return 2*i;
                }
            }
        }
    }
}



int CamTableFitter::get_pva(double x, PosVelAcc &pva)const{
    auto pce = (CamXyva*)  pCamTable->pce;
    if (x < pce[_idx].dX || x > pce[_idx+1].dX) {
        return -1;
    }
    _polyArr[0].get_pva(x, &pva);
    if (x <= pce[_idx].dX + 1e-6) {
        return 2*_idx;
    }else if (x >= pce[_idx+1].dX - 1e-6) {
        return 2*_idx + 2;
    }else { // between pt[idx] and pt[idx+1]
        return 2*_idx + 1;
    }
}

int CamTableFitter::get_pva_or_build(double x, PosVelAcc &pva){
    auto pce = (const CamXyva*)pCamTable->pce;
    int nPoints = pCamTable->nElements;
    int idx;

    if (x < pce[0].dX) {
        idx = 0;
        x = pce[0].dX;
    }else if (x > pce[nPoints-1].dX) {
        idx = nPoints-2;
        x = pce[nPoints-1].dX;
    }else {
        // Find the interval containing x
        for (int i = _idx; i < nPoints-1; i++) {
            if (x >= pce[i].dX && x <= pce[i+1].dX) {
                idx = i;
                break;
            }
        }
        for (int i = 0; i < _idx; i++) {
            if (x >= pce[i].dX && x <= pce[i+1].dX) {
                idx = i;
                break;
            }
        }
    }
    if (_idx != idx || !_is_built) {
        if (pce[idx].dX > pce[idx+1].dX) {
            return -2;
        } 
        pCamTable->build(idx, _polyArr[0]);
        // _polyArr[0].init(pce[idx].dX, pce[idx+1].dX, pce[idx].dY, pce[idx+1].dY, pce[idx].dV, pce[idx+1].dV, pce[idx].dA, pce[idx+1].dA);
        _idx = idx;
        _is_built = true;
    }
    _polyArr[0].get_pva(x, &pva);
    _prevX = _curX;
    _curX = x;
    if (x <= pce[_idx].dX + 1e-6) {
        return 2* nPoints - 2 - 2*_idx;
    }else if (x >= pce[_idx+1].dX - 1e-6) {
        return 2* nPoints - 2- (2*_idx + 2);
    }else { // between pt[idx] and pt[idx+1]
        return 2* nPoints - 2 - (2*_idx + 1);
    }
}

int CamTableFitter::get_pva_or_build_affine(const PosVelAcc& masterPva, PosVelAcc &slavePva, const CamAffineTransform& affline){
    const double& A = affline.masterScale;
    const double& B = affline.masterIntercept;
    const double& C = affline.slaveScale;
    const double& D = affline.slaveIntercept;


    PosVelAcc pvaCam;
    double x = masterPva.pos* A + B;

    double tableLookupPos = calc_modulo(x);
    int iTurn = (x - tableLookupPos)/getXRange();
    int ret = get_pva_or_build(tableLookupPos, pvaCam);
    pvaCam.pos += iTurn * getYRange();

    slavePva.pos = pvaCam.pos * C + D;
    slavePva.vel = pvaCam.vel* C * masterPva.vel ;
    slavePva.acc = ( C * pvaCam.acc * A* A* masterPva.vel *masterPva.vel) + 
                 ( C * pvaCam.vel * A* masterPva.acc);
    return ret;
}
double CamTableFitter::getXRange()const{
    auto pce = (const CamXyva*)pCamTable->pce;
    return pce[pCamTable->nElements-1].dX - pce[0].dX;
}
double CamTableFitter::getYRange()const{
    auto pce = (const CamXyva*)pCamTable->pce;
    return pce[pCamTable->nElements-1].dY - pce[0].dY;
}
double CamTableFitter::calc_modulo_v1(double x)const{
    if (!Periodic) return x;
    auto pce = (const CamXyva*)pCamTable->pce;
    double modulo = pce[pCamTable->nElements-1].dX - pce[0].dX;
    while (x < pce[0].dX) x += modulo;
    while (x > pce[pCamTable->nElements-1].dX) x -= modulo;
    return x;
}
double CamTableFitter::calc_modulo(double x)const{
    auto pce = (const CamXyva*)pCamTable->pce;
    double range = pce[pCamTable->nElements-1].dX - pce[0].dX;
    double xM = fmod(x - pce[0].dX, range);
    return xM < 0 ? (xM + range) : xM;
}

double CamTableFitter::findLeftPt(double x)const{
    auto pce = (const CamXyva*)pCamTable->pce;
    double range = pce[pCamTable->nElements-1].dX - pce[0].dX;
    double xM = fmod(x - pce[0].dX, range);
    return x - (xM < 0 ? (xM + range) : xM);
}
// x & xPrev is project to linear position
int CamTableFitter::detect_cross_bound(double x, double xPrev)const{
    if (x > xPrev){
        double nearestBoundPos = x - calc_modulo(x);
        if (xPrev < nearestBoundPos && (x-xPrev)*2 < getXRange()){
            return 1;
        }
    }else if (x < xPrev){
        double nearestBoundPos = xPrev - calc_modulo(xPrev);
        if (x < nearestBoundPos && (xPrev-x)*2 < getXRange()){
            return -1;
        }
    }
    return 0;
}

// void CamTableFitter::process_tappet(int idx){
    // if (idx < 0 || idx >= pCamTable->nTappets) return;

    // UnTappetActive& impl = (UnTappetActive&) pCamTable->pt[idx].dwActive;
    // impl.apply(pCamTable->pt[idx].cta);
    // pCamTable->pt[idx].dwActive = impl.value;
    // case TAPPETACTION_time:
// }
// bool CamTableFitter::get_tappet_value(int idx)const{
//     if (idx < 0 || idx >= pCamTable->nTappets) return false;
//     return ((UnTappetActive&) pCamTable->pt[idx].dwActive).getTappet();
// }

CamTappetChecker::TappetTriggeredData CamTappetChecker::check_tappet()const{
    const double& x = _curX;
    bool isForward = x > _prevX;
    TappetTriggeredData tapTrg;
    if (isForward){
        for (int i = 0; i < pCamTable->nTappets; i++) {
            if (pCamTable->pt[i].x > _prevX && pCamTable->pt[i].x <= x + 1e-6) {
                if (pCamTable->pt[i].ctt != TAPPET_neg){
                    tapTrg.push(i);
                    // printf("check_tappet::_triggeredTappetNum = %d, j = %d, pCamTable->pt[i].x = %f, x = %f, _prevX = %f\n", _triggeredTappetNum, j, pCamTable->pt[i].x, x, _prevX);
                    if (tapTrg.triggerNum >= MAX_TRIGGERED_IDX) break;
                }
            }
        }
    }else{
        for (int i = 0; i < pCamTable->nTappets; i++) {
            if (pCamTable->pt[i].x < _prevX && pCamTable->pt[i].x >= x - 1e-6) {
                if (pCamTable->pt[i].ctt != TAPPET_pos){
                    tapTrg.push(i);
                    if (tapTrg.triggerNum >= MAX_TRIGGERED_IDX) break;
                }
            }
        }
    }
    return tapTrg;
}


int CamTappetChecker::checkProcessTappet(SMC_TappetData* pTappetData)const{
    auto tapTrg = check_tappet();
    for (int i = 0; i < MAX_TRIGGERED_IDX; i++){
        if (tapTrg.triggerNum > i){
            int tappet_idx = tapTrg.triggerIdxArr[i];
            pTappetData->pTaps[i] = (SMC_CAMTappet*) &pCamTable->pt[tappet_idx]; 
        }else{
            pTappetData->pTaps[i] = nullptr;
        }
        // printf("pTappetData->pTaps[%d] = %p, _triggeredTappetNum = %d\n", i, pTappetData->pTaps[i], _triggeredTappetNum);
    }
    pTappetData->byChannels = MAX_TRIGGERED_IDX;
    return tapTrg.triggerNum;
}

ZMC_STRING_80 make_ZMC_STRING_80(const char* s){
    ZMC_STRING_80 ret;
    strncpy(ret.value, s, sizeof(ret.value));
    return ret;
}

void assign_ZMC_STRING_80(ZMC_STRING_80* dst, const char* s){
    strncpy(dst->value, s, sizeof(dst->value));
}
}  // namespace zmc
