#ifndef _ZMC_PDO_PREPARER_H_
#define _ZMC_PDO_PREPARER_H_

#include "stdint.h"
typedef int (*PrepareHandler)(void*, uint8_t*, uint8_t*);

struct EniPdoPrepareCtx;

struct PdoPreparerConfig {
    PrepareHandler handler;      // PDO handler, null for IO-only ENI
    EniPdoPrepareCtx* ctx;       // ENI parse context, owned by caller
    int slaveCount;              // servo count for simulation
    uint32_t rxpdoLen;           // PDO output size (master -> slave), bytes
    uint32_t txpdoLen;           // PDO input size (slave -> master), bytes
    uint32_t servoSize;          // active servo count (<=32)
    uint32_t servoMask;          // bit[i]=1 means slave i is CiA402 axis
};

// Parse ENI file and build PDO preparer configuration
PdoPreparerConfig guess_preparer(const char* ENI);

// Release context allocated by guess_preparer
void freeEniPdoPrepareCtx(void* p);

// Opaque context accessors
uint32_t eni_pdo_ctx_get_cycle_time(EniPdoPrepareCtx* ctx);
void eni_pdo_ctx_set_sv_arr(EniPdoPrepareCtx* ctx, void* svArr);

#endif
