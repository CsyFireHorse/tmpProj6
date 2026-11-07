#include "FifthOrderPolyCWrap.h"

#include <new>

#include "vel/VelWrapCommon.h"
#include "cubic_poly.h"
#include "common/zmc/fbt_errorcode_def.h"

// Forward declaration of the actual handle structure
struct FifthOrderPoly_Handle {
    zmc::FifthOrderPoly poly;
};

extern "C" {

FifthOrderPoly_Handle_t FifthOrderPoly_create(void) {
    FifthOrderPoly_Handle* handle = new FifthOrderPoly_Handle();
    return handle;
}

void FifthOrderPoly_destroy(FifthOrderPoly_Handle_t handle) {
    if (handle) {
        delete static_cast<FifthOrderPoly_Handle*>(handle);
    }
}

int FifthOrderPoly_init(FifthOrderPoly_Handle_t handle, double t0, double t1, double p0, double p1,
                        double v0, double v1, double a0, double a1) {
    if (!handle) {
        return ZMC_ERR_TRAJ_INVALID_ARG;
    }

    FifthOrderPoly_Handle* h = static_cast<FifthOrderPoly_Handle*>(handle);
    return h->poly.init2(t0, t1, p0, p1, v0, v1, a0, a1);
}

PosVelAcc FifthOrderPoly_get_pva(FifthOrderPoly_Handle_t handle, double x) {
    PosVelAcc result = {0.0, 0.0, 0.0};

    if (!handle) {
        return result;
    }

    FifthOrderPoly_Handle* h = static_cast<FifthOrderPoly_Handle*>(handle);
    return h->poly.get_pva(x);
}

void FifthOrderPoly_get_pva_out(FifthOrderPoly_Handle_t handle, double x, PosVelAcc* pva) {
    if (!handle || !pva) {
        return;
    }
    FifthOrderPoly_Handle* h = static_cast<FifthOrderPoly_Handle*>(handle);
    h->poly.get_pva(x, *pva);
}

void FifthOrderPoly_get_pvaj(FifthOrderPoly_Handle_t handle, double x, PosVelAccJerk* pvaj) {
    if (!handle || !pvaj) {
        return;
    }

    FifthOrderPoly_Handle* h = static_cast<FifthOrderPoly_Handle*>(handle);
    h->poly.get_pvaj(x, *pvaj);
}

}  // extern "C"
