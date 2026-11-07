#ifndef DISABLE_ZECM_FAK
/*
 * @Author: ChenShengyong
 * @Date: 2024-03-26 11:08:15
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2026-02-26 17:22:33
 *
 */
#include "../zecm_zh.h"
#include "../zecm_ex.h"
#include <thread>
#include <chrono>
#include "stdio.h"
#include "string.h"
// #include "ServoSgFaker.hpp"
// #include "ServoFakAlloc.hpp"
// #include "EcatSlaveSimulatorClient.h"
// #include "EcatSlaveSimulatorService.h"

#include "EcatFaker.hpp"
using namespace zmc;

using std::thread;
using std::chrono::milliseconds;
using std::this_thread::sleep_for;
using std::this_thread::sleep_until;
// using std::chrono::steady_clock;

// static EcatFaker* get_ecat(){
//     static EcatFaker* pEcat = new EcatFaker();
//     return pEcat;
// }
inline EcatFaker* get_ecat(ECAT_HANDLE hMaster) { return (EcatFaker*)hMaster; }

// Consecutive bitmap allocator for ServoSgFaker array management
// Uses the existing ZmcBitmap as base and adds consecutive allocation support

ECAT_RESULT EcatMasterLibInit(ECAT_CSTR pszIniFileName) {
    // while(1){
    printf("EcatMasterLibInit\n");
    // }
    return ECAT_S_OK;
};

/**
Release master library resources
*/
ECAT_RESULT EcatMasterLibExit() {
    printf("EcatMasterLibExit\n");
    return ECAT_S_OK;
};

#ifndef DISABLE_PCI_ZECM_FAK
/**
 */
ECAT_RESULT /* ECAT_S_OK-success, ECAT_XXX-errorCode */
EcatRequestMasterState(ECAT_HANDLE hMaster, ECAT_BYTE byState /* [IN] see also EcatState */
) {
    if (!hMaster) return ECAT_E_INVALIDARG;
    return get_ecat(hMaster)->requestMasterState(byState);
}

ECAT_RESULT /* ECAT_S_OK-success, ECAT_XXX-errorCode */
EcatGetMasterState(ECAT_HANDLE hMaster, ECAT_BYTE* pbyState) {
    if (!hMaster) return ECAT_E_INVALIDARG;
    return get_ecat(hMaster)->getMasterState(pbyState);
}

int32_t EcatGetSlaveCount(ECAT_HANDLE hHandle, ECAT_WORD* pu16Count) {
    EcatFaker* pEcat = get_ecat(hHandle);
    if (!pEcat || !pu16Count) return -1;
    *pu16Count = pEcat->SlaveCount;
    return ECAT_S_OK;
}

ECAT_RESULT EcatGetActiveSlaveCount(ECAT_HANDLE hMaster, ECAT_WORD* pwSlaveCount) {
    EcatFaker* pEcat = get_ecat(hMaster);
    if (!pEcat || !pwSlaveCount) return -1;
    *pwSlaveCount = pEcat->SlaveCount;
    return ECAT_S_OK;
}

ECAT_RESULT EcatGetSlaveState(ECAT_HANDLE hMaster, ECAT_WORD wSlaveId, ECAT_BYTE* pbyState) {
    if (!hMaster) {
        return ECAT_E_INVALIDARG;
    }
    return get_ecat(hMaster)->getSlaveState(wSlaveId, pbyState);
}

ECAT_RESULT EcatGetSlaveReqState(ECAT_HANDLE hMaster, ECAT_WORD wSlaveId, ECAT_BYTE* pbyState) {
    if (!hMaster) {
        return ECAT_E_INVALIDARG;
    }
    return get_ecat(hMaster)->getSlaveReqState(wSlaveId, pbyState);
}

ECAT_RESULT EcatClearSlaveStateError(ECAT_HANDLE hMaster, ECAT_WORD wSlaveId) {
    if (!hMaster) {
        return ECAT_E_INVALIDARG;
    }
    return get_ecat(hMaster)->clearSlaveStateError(wSlaveId);
}
#endif

/**

*/
ECAT_RESULT /* ECAT_S_OK-success, ECAT_XXX-errorCode */
EcatSetExtCtrlTaskHandler(
    ECAT_HANDLE hMaster,
    EcatExtCtrlTaskHandler pfnHandler, /* [IN] user callback function pointer */
    ECAT_LPVOID pvCtrlTaskArg          /* [IN] user callback function argument */
) {
    return get_ecat(hMaster)->setTaskHandler(pfnHandler, pvCtrlTaskArg);
}

ECAT_RESULT /* ECAT_S_OK-success, ECAT_XXX-errorCode */
EcatResetExtCtrlTaskHandler(ECAT_HANDLE hMaster) {
    return get_ecat(hMaster)->setTaskHandler(nullptr, nullptr);
}

/**
Read slave SDO
*/
ECAT_RESULT                            /* ECAT_S_OK-success, ECAT_XXX-errorCode */
EcatCoeSDOUpload(ECAT_HANDLE hMaster,  /* [IN] master handle */
                 ECAT_WORD wSlaveId,   /* slave id */
                 ECAT_WORD wIndex,     /* SDO index */
                 ECAT_BYTE bySubIndex, /* SDO subindex */
                 ECAT_BOOL bCompleteAccess, ECAT_DWORD* pdwDataLen, ECAT_BYTE* pbyData,
                 ECAT_DWORD dwMaxDataLen, ECAT_DWORD dwTimeOut /* timeout */
) {
    sleep_for(milliseconds(1));

    if (pdwDataLen && pbyData) {
        int mixIdx = get_ecat(hMaster)->index_to_mix(wSlaveId, wIndex, bySubIndex);
        get_ecat(hMaster)->read_reg_len(mixIdx, *pdwDataLen, pbyData);
    }
    // *pdwDataLen = 1;
    // *pbyData = get_ecat(hMaster)->read_reg(wIndex);
    // printf("EcatCoeSDOUpload: wSlaveId:%d, wIndex:%d, bySubIndex:%d, bCompleteAccess:%d,
    // dwMaxDataLen:%d, data:%d, rd_len:%d\r\n", (int)wSlaveId, (int)wIndex, (int)bySubIndex,
    // (int)bCompleteAccess, (int)dwMaxDataLen, (int)*pbyData, (int)*pdwDataLen);
    return ECAT_S_OK;
}

/**
Write slave SDO
*/
ECAT_RESULT                              /* ECAT_S_OK-success, ECAT_XXX-errorCode */
EcatCoeSDODownload(ECAT_HANDLE hMaster,  /* [IN] master handle */
                   ECAT_WORD wSlaveId,   /* slave id */
                   ECAT_WORD wIndex,     /* SDO index */
                   ECAT_BYTE bySubIndex, /* SDO subindex */
                   ECAT_BOOL bCompleteAccess, ECAT_DWORD* pdwDataLen, ECAT_BYTE* pbyData,
                   ECAT_DWORD dwMaxDataLen, ECAT_DWORD dwTimeOut /* timeout */
) {
    sleep_for(milliseconds(1));
    // Fix for length=1,2,4 bytes
    if (pdwDataLen && pbyData) {
        int mixIdx = get_ecat(hMaster)->index_to_mix(wSlaveId, wIndex, bySubIndex);
        get_ecat(hMaster)->write_reg_len(mixIdx, *pdwDataLen, pbyData);
    }
    // get_ecat(hMaster)->write_reg(wIndex, *pbyData);
    // printf("EcatCoeSDODownload: wSlaveId:%d, wIndex:%d, bySubIndex:%d, bCompleteAccess:%d,
    // dwMaxDataLen:%d, data:%d, rd_len:%d\r\n", (int)wSlaveId, (int)wIndex, (int)bySubIndex,
    // (int)bCompleteAccess, (int)dwMaxDataLen, (int)*pbyData, (int)*pdwDataLen);
    return ECAT_S_OK;
}

void ecatFakeRunOnce(ECAT_HANDLE hMaster) { get_ecat(hMaster)->runOnce(); }

ECAT_RESULT
EcatSimpleStart(ECAT_SIMPLE_START_CONTEXT* ctx) {
    auto pEcat = new EcatFaker();
    pEcat->ctx = *ctx;
    ctx->Master = pEcat;
    std::thread thd(ecatFakeRun, ctx->Master);
    thd.detach();
    return ECAT_S_OK;
}

/**
Stop master
*/
ECAT_RESULT
EcatSimpleStop(ECAT_SIMPLE_START_CONTEXT* ctx) {
    int ret = get_ecat(ctx->Master)->stop();
    get_ecat(ctx->Master)->unbind();
    return ret;
}

ECAT_RESULT EcatRequestSlaveState(ECAT_HANDLE hMaster, ECAT_WORD wSlaveId, ECAT_BYTE byState) {
    if (!hMaster) {
        return ECAT_E_INVALIDARG;
    }
    return get_ecat(hMaster)->requestSlaveState(wSlaveId, byState);
}

ECAT_RESULT EcatGetSlaveStateByPosIndex(ECAT_HANDLE hMaster, ECAT_WORD wSlavePosIndexArrLength,
                                        ECAT_WORD* pwSlavePosIndexArr, ECAT_BYTE* pbyState) {
    if (!hMaster) {
        return ECAT_E_INVALIDARG;
    }
    return get_ecat(hMaster)->getSlaveStatesByPosIndex(wSlavePosIndexArrLength, pwSlavePosIndexArr,
                                                       pbyState);
}

ECAT_RESULT EcatGetDLStatus(ECAT_HANDLE hMaster, ECAT_WORD wSlavePosAddrArrLength,
                            ECAT_WORD* pwSlavePosAddrArr, ECAT_REG_DL_STATUS* pDLStatus) {
    if (!hMaster || (wSlavePosAddrArrLength != 0u && (!pwSlavePosAddrArr || !pDLStatus))) {
        return ECAT_E_INVALIDARG;
    }
    for (ECAT_WORD i = 0; i < wSlavePosAddrArrLength; ++i) {
        ECAT_WORD wDLStatus = 0;
        int ret = get_ecat(hMaster)->getSlaveDLStatus(pwSlavePosAddrArr[i], &wDLStatus);
        if (ret != ECAT_S_OK) {
            return ret;
        }
        pDLStatus[i].RawData[0] = static_cast<ECAT_BYTE>(wDLStatus);
        pDLStatus[i].RawData[1] = static_cast<ECAT_BYTE>(wDLStatus >> 8u);
    }
    return ECAT_S_OK;
}

int32_t EcatGetDiagnostics(ECAT_HANDLE hHandle, ECAT_MASTER_DIAGNOSTIC_INFO* pMasterDiagInfo,
                           uint16_t u16SlaveCount, uint16_t* pu16SlaveIds,
                           ECAT_SLAVE_DIAGNOSTIC_INFO* pSlaveDiagInfo) {
    EcatFaker* pEcat = get_ecat(hHandle);
    if (!pEcat || !pMasterDiagInfo ||
        (u16SlaveCount != 0u && (!pu16SlaveIds || !pSlaveDiagInfo))) {
        return -1;
    }

    memset(pMasterDiagInfo, 0, sizeof(*pMasterDiagInfo));
    for (uint16_t i = 0; i < u16SlaveCount; ++i) {
        if (pu16SlaveIds[i] >= pEcat->SlaveCount) return -1;
        memset(&pSlaveDiagInfo[i], 0, sizeof(pSlaveDiagInfo[i]));
        if (!pEcat->isOperational()) {
            pSlaveDiagInfo[i].dwDiagState = EcatSlaveDiagnosticStateErrorEcatState;
        }
    }
    return ECAT_S_OK;
}

#endif
