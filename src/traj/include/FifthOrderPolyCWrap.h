#pragma once

#include "vel/VelWrapCommon.h"
#include "zmc_common.h"
#ifdef __cplusplus
extern "C" {
#endif


// Opaque handle for FifthOrderPoly
typedef struct FifthOrderPoly_Handle* FifthOrderPoly_Handle_t;

// Create a new FifthOrderPoly instance
ZMC_DLLPORT FifthOrderPoly_Handle_t FifthOrderPoly_create(void);

// Destroy a FifthOrderPoly instance
ZMC_DLLPORT void FifthOrderPoly_destroy(FifthOrderPoly_Handle_t handle);

// Initialize with position, velocity and acceleration at start and end points
ZMC_DLLPORT int FifthOrderPoly_init(FifthOrderPoly_Handle_t handle, 
                       double t0, double t1, double p0, double p1, 
                       double v0, double v1, double a0, double a1);

// Get position, velocity and acceleration at time t
ZMC_DLLPORT PosVelAcc FifthOrderPoly_get_pva(FifthOrderPoly_Handle_t handle, double x);

// Get position, velocity and acceleration at time t (output parameter version)
ZMC_DLLPORT void FifthOrderPoly_get_pva_out(FifthOrderPoly_Handle_t handle, double x, PosVelAcc* pva);

// Get position, velocity, acceleration and jerk at time t
ZMC_DLLPORT void FifthOrderPoly_get_pvaj(FifthOrderPoly_Handle_t handle, double x, PosVelAccJerk* pvaj);

#ifdef __cplusplus
}
#endif
