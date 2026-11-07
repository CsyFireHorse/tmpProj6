#include "mc_sdo_api.h"

#include <new>

#include "sdo/sdo_rt.hpp"

struct McSdoRuntime {
    zmc::SdoRt sdo;
};

mc_sdo_handle_t mc_sdo_create(void) {
    McSdoRuntime* runtime = new (std::nothrow) McSdoRuntime();
    if (runtime == nullptr) {
        return nullptr;
    }
    return runtime;
}

void mc_sdo_destroy(mc_sdo_handle_t handle) {
    if (handle == nullptr) {
        return;
    }
    delete handle;
}

void mc_sdo_pause(mc_sdo_handle_t handle) {
    if (handle != nullptr) {
        handle->sdo.pause();
    }
}

void mc_sdo_resume(mc_sdo_handle_t handle) {
    if (handle != nullptr) {
        handle->sdo.resume();
    }
}

void mc_sdo_clear_watches(mc_sdo_handle_t handle) {
    if (handle != nullptr) {
        handle->sdo.clear_watches();
    }
}

int mc_sdo_bind_adapter(mc_sdo_handle_t handle, const McSdoAdapter* adapter) {
    if (handle == nullptr) {
        return -1;
    }
    return handle->sdo.bind_adapter(adapter);
}

void* mc_sdo_native(mc_sdo_handle_t handle) {
    if (handle == nullptr) {
        return nullptr;
    }
    return &handle->sdo;
}
