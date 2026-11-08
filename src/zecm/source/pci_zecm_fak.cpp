#ifndef DISABLE_PCI_ZECM_FAK
/*
 * PCIe ZECM Faker - Async-only implementation using EcatFaker
 * Uses lock-free 3-buffer mechanism for PI data exchange
 */

#include "../zecm_zh.h"
#include "pci_zecm.h"
#include "EcatFaker.hpp"

#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cstring>
#include <cstdio>

using std::condition_variable;
using std::lock_guard;
using std::mutex;
using std::thread;
using std::unique_lock;
using std::chrono::milliseconds;

using namespace zmc;

// Forward declaration from zecm_fak.cpp
int ecatFakeRun(ECAT_HANDLE hMaster);
// Lock-free 3-buffer for async PI exchange
struct Buf3 {
    uint8_t slot[3][4096];
    std::atomic<int> latest{0};
    int w = 1;
    int r = 2;

    void publish(const uint8_t* data, uint32_t len) {
        memcpy(slot[w], data, len);
        w = latest.exchange(w);
    }

    void consume(uint8_t* data, uint32_t len) {
        r = latest.exchange(r);
        memcpy(data, slot[r], len);
    }
};

// Context for PCIe faker
struct PcieContext {
    Buf3 tbOutput;  // app -> cycle
    Buf3 tbInput;   // cycle -> app
    bool piEnabled = false;
    EcatFaker* pEcat = nullptr;

    // for EcatPIInputWaitNotify
    mutex notifyMtx;
    condition_variable notifyCv;
    uint64_t inputCycleCount = 0;
};

// Main callback - called every cycle by EcatFaker
ECAT_RESULT pcieExchange(const ECAT_LPVOID arg, ECAT_BYTE* pPdoPi, ECAT_BYTE* pPdoPo) {
    PcieContext* ctx = (PcieContext*)arg;
    if (!ctx || !ctx->piEnabled) return 0;

    // 1. Publish last cycle's slave response for app to read
    ctx->tbInput.publish(pPdoPi, ctx->pEcat->txpdoLen);

    // 2. Notify waiters (EcatPIInputWaitNotify)
    {
        lock_guard<mutex> lk(ctx->notifyMtx);
        ctx->inputCycleCount++;
    }
    ctx->notifyCv.notify_all();

    // 3. Load app's latest output for slave sim this cycle
    ctx->tbOutput.consume(pPdoPo, ctx->pEcat->rxpdoLen);

    return ECAT_S_OK;
}

// Helper to get EcatFaker from handle
inline EcatFaker* get_ecat(ECAT_HANDLE h) { return (EcatFaker*)h; }

// Helper to get PcieContext from EcatFaker
inline PcieContext* get_ctx_from_ecat(EcatFaker* pEcat) {
    return pEcat ? (PcieContext*)pEcat->pvCtrlTaskArg : nullptr;
}

// Bus run thread function - reuse zecm_fak implementation
int pcieBusRunThread(EcatFaker* pEcat) {
    if (!pEcat) return -1;
    return ecatFakeRun((ECAT_HANDLE)pEcat);
}

// API implementations
extern "C" {

int32_t EcatOpen(ECAT_HANDLE* phHandle, const char* sBoardIf, uint32_t u32EcatChannel) {
    if (!phHandle) return -1;

    EcatFaker* pEcat = new EcatFaker();
    PcieContext* ctx = new PcieContext();
    ctx->pEcat = pEcat;
    pEcat->setTaskHandler(pcieExchange, ctx);

    *phHandle = pEcat;
    return ECAT_S_OK;
}

int32_t EcatClose(ECAT_HANDLE hHandle) {
    EcatFaker* pEcat = get_ecat(hHandle);
    if (!pEcat) return -1;

    pEcat->stop();
    PcieContext* ctx = get_ctx_from_ecat(pEcat);
    pEcat->pFunc = nullptr;
    pEcat->pvCtrlTaskArg = nullptr;
    pEcat->clearUserCtx();
    delete ctx;
    delete pEcat;
    return ECAT_S_OK;
}

int32_t EcatBusRun(ECAT_HANDLE hHandle, const char* cEniFileName) {
    EcatFaker* pEcat = get_ecat(hHandle);
    if (!pEcat || !cEniFileName) return -1;
    PcieContext* ctx = get_ctx_from_ecat(pEcat);
    if (!ctx) return -1;

    // Store ENI path into EcatFaker context; cycle time can be tuned by caller if needed
    pEcat->ctx.ENI = cEniFileName;
    pEcat->ctx.MainCycleTimeUs = 1000;  // 1ms default
    pEcat->ctx.SubCycleTimeUs = 1000;

    // PDO lengths will be filled by guess_use_preparer inside ecatFakeRun

    pEcat->initializeState(EcatStateI);

    // Launch bus cycle thread using shared implementation
    std::thread thd(pcieBusRunThread, pEcat);
    thd.detach();

    return ECAT_S_OK;
}

int32_t EcatBusStop(ECAT_HANDLE hHandle) {
    EcatFaker* pEcat = get_ecat(hHandle);
    if (!pEcat) return -1;
    return pEcat->stop();
}

int32_t EcatGetEniInfo(ECAT_HANDLE hHandle, ECAT_ENI_INFO* ptInfo) {
    EcatFaker* pEcat = get_ecat(hHandle);
    if (!pEcat || !ptInfo) return -1;
    ptInfo->u32MainCycle = pEcat->ctx.MainCycleTimeUs;
    ptInfo->u32MailboxCycle = pEcat->ctx.SubCycleTimeUs;
    ptInfo->u32PIOutputByteSize = pEcat->rxpdoLen;
    ptInfo->u32PIInputByteSize = pEcat->txpdoLen;
    return ECAT_S_OK;
}

int32_t PcieEcatCoeSDODownload(ECAT_HANDLE hHandle, uint16_t u16SlaveId, uint16_t u16Index,
                               uint8_t u8SubIndex, uint32_t u32DataLen, const void* pvData,
                               uint32_t u32TimeOutMs) {
    (void)u16SlaveId;
    (void)u32TimeOutMs;
    EcatFaker* pEcat = get_ecat(hHandle);
    if (!pEcat || !pvData || u32DataLen == 0) return -1;

    // Keep toy behavior: store only first byte into faker register.
    int regIdx = pEcat->index_to_mix(u16SlaveId, u16Index, u8SubIndex);
    pEcat->write_reg_len(regIdx, u32DataLen, pvData);
    return ECAT_S_OK;
}

int32_t PcieEcatCoeSDOUpload(ECAT_HANDLE hHandle, uint16_t u16SlaveId, uint16_t u16Index,
                             uint8_t u8SubIndex, uint32_t* pu32DataLen, void* pvData,
                             uint32_t u32TimeOutMs) {
    (void)u16SlaveId;
    (void)u32TimeOutMs;
    EcatFaker* pEcat = get_ecat(hHandle);
    if (!pEcat || !pu32DataLen || !pvData || *pu32DataLen == 0) return -1;

    int regIdx = pEcat->index_to_mix(u16SlaveId, u16Index, u8SubIndex);
    pEcat->read_reg_len(regIdx, *pu32DataLen, pvData);
    // *pu32DataLen = 1;
    return ECAT_S_OK;
}

int32_t EcatCoeSDODownloadEx(ECAT_HANDLE hHandle, uint16_t u16SlaveId, uint16_t u16Index,
                             uint8_t u8SubIndex, uint8_t bCompleteAccess, uint32_t u32DataLen,
                             const void* pvData, uint32_t u32TimeOutMs) {
    (void)u16SlaveId;
    (void)u32TimeOutMs;
    (void)bCompleteAccess;
    EcatFaker* pEcat = get_ecat(hHandle);
    if (!pEcat || !pvData || u32DataLen == 0) return -1;

    int regIdx = pEcat->index_to_mix(u16SlaveId, u16Index, u8SubIndex);
    pEcat->write_reg_len(regIdx, u32DataLen, pvData);
    return ECAT_S_OK;
}

int32_t EcatCoeSDOUploadEx(ECAT_HANDLE hHandle, uint16_t u16SlaveId, uint16_t u16Index,
                           uint8_t u8SubIndex, uint8_t bCompleteAccess, uint32_t* pu32DataLen,
                           void* pvData, uint32_t u32TimeOutMs) {
    (void)u16SlaveId;
    (void)u32TimeOutMs;
    (void)bCompleteAccess;
    EcatFaker* pEcat = get_ecat(hHandle);
    if (!pEcat || !pu32DataLen || !pvData || *pu32DataLen == 0) return -1;

    int regIdx = pEcat->index_to_mix(u16SlaveId, u16Index, u8SubIndex);
    pEcat->read_reg_len(regIdx, *pu32DataLen, pvData);
    return ECAT_S_OK;
}

int32_t EcatPIEnable(ECAT_HANDLE hHandle) {
    EcatFaker* pEcat = get_ecat(hHandle);
    PcieContext* ctx = get_ctx_from_ecat(pEcat);
    if (!pEcat || !ctx) return -1;
    ctx->piEnabled = true;
    return ECAT_S_OK;
}

int32_t EcatPIRead(ECAT_HANDLE hHandle, uint32_t u32PiArea, uint32_t u32Offset,
                   uint32_t u32DataLen, void* pvData) {
    EcatFaker* pEcat = get_ecat(hHandle);
    PcieContext* ctx = get_ctx_from_ecat(pEcat);
    if (!pEcat || !ctx || !pvData) return -1;

    if (u32PiArea == PI_AREA_SHADOW_INPUT) {
        if (u32Offset >= pEcat->txpdoLen) return -1;
        uint32_t copyLen = u32DataLen;
        if (u32Offset + copyLen > pEcat->txpdoLen) copyLen = pEcat->txpdoLen - u32Offset;
        memset(pvData, 0, u32DataLen);
        ctx->tbInput.consume((uint8_t*)pvData, copyLen);
        return ECAT_S_OK;
    }
    return -1;
}

int32_t EcatPIWrite(ECAT_HANDLE hHandle, uint32_t u32PiArea, uint32_t u32Offset,
                    uint32_t u32DataLen, const void* pvData) {
    EcatFaker* pEcat = get_ecat(hHandle);
    PcieContext* ctx = get_ctx_from_ecat(pEcat);
    if (!pEcat || !ctx || !pvData) return -1;

    if (u32PiArea == PI_AREA_FAST_OUTPUT) {
        if (u32Offset >= pEcat->rxpdoLen) return -1;
        uint32_t copyLen = u32DataLen;
        if (u32Offset + copyLen > pEcat->rxpdoLen) copyLen = pEcat->rxpdoLen - u32Offset;
        ctx->tbOutput.publish((const uint8_t*)pvData, copyLen);
        return ECAT_S_OK;
    }
    return -1;
}

int32_t EcatPIInputWaitNotify(ECAT_HANDLE hHandle, uint32_t u32TimeOutMs) {
    EcatFaker* pEcat = get_ecat(hHandle);
    PcieContext* ctx = get_ctx_from_ecat(pEcat);
    if (!pEcat || !ctx) return -1;

    uint64_t prev = ctx->inputCycleCount;
    unique_lock<mutex> lk(ctx->notifyMtx);
    if (ctx->notifyCv.wait_for(lk, milliseconds(u32TimeOutMs),
                               [&] { return ctx->inputCycleCount > prev; })) {
        return ECAT_S_OK;
    }
    return ECAT_S_TIMEOUT;
}

const char* EcatErrorGetErrorMessage(ECAT_RESULT nError) {
    // Simple error message lookup
    switch (nError) {
        case ECAT_S_OK:
            return "Success";
        case ECAT_S_TIMEOUT:
            return "Timeout";
        default:
            return "Unknown error";
    }
}

int32_t EcatGetSlaveDLStatus(ECAT_HANDLE hHandle, uint16_t u16SlaveId, uint16_t* pu16DLStatus) {
    EcatFaker* pEcat = get_ecat(hHandle);
    if (!pEcat) return -1;
    return pEcat->getSlaveDLStatus(u16SlaveId, pu16DLStatus);
}

}  // extern "C"
#endif
