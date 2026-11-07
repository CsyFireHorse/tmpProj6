/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-18
@ArtifactStatusAuthority GPT-5.6 Sol
*/
#include "sdo_rt_instance.h"

#include "sdo_backend_pcie.hpp"

namespace zmc {

namespace {

int32_t pcie_sdo_upload(void* ctx, const SdoCKey* key, uint32_t* len, void* data,
                        uint32_t timeout_ms) {
    if (!key) {
        return -1;
    }
    return PcieSdoBackend::Upload(ctx, key->slave_id, key->index, key->subindex, len, data,
                                  timeout_ms);
}

int32_t pcie_sdo_download(void* ctx, const SdoCKey* key, uint32_t len, const void* data,
                          uint32_t timeout_ms) {
    if (!key) {
        return -1;
    }
    return PcieSdoBackend::Download(ctx, key->slave_id, key->index, key->subindex, len, data,
                                    timeout_ms);
}

int32_t pcie_get_slave_state(void* ctx, uint16_t slave_id, uint8_t* slave_state) {
    return EcatGetSlaveState(static_cast<ECAT_HANDLE>(ctx), slave_id, slave_state);
}

}  // namespace

PcieSdoRt::PcieSdoRt() {}
PcieSdoRt::~PcieSdoRt() {}

int PcieSdoRt::bindEcat(void* ecat_handle) {
    SdoCAdapter adapter = {};
    adapter.ctx = ecat_handle;
    adapter.upload = pcie_sdo_upload;
    adapter.download = pcie_sdo_download;
    adapter.get_slave_state = pcie_get_slave_state;
    return bind_adapter(&adapter);
}

PcieSdoRt& SdoPcieRtInstance::instance() {
    static PcieSdoRt s;
    return s;
}
int SdoPcieRtInstance::bindEcat(void* h) { return instance().bindEcat(h); }
void SdoPcieRtInstance::start() { instance().start(); }
void SdoPcieRtInstance::stop() { instance().stop(); }

}  // namespace zmc
