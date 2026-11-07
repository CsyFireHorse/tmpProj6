/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-18
@ArtifactStatusAuthority GPT-5.6 Sol
*/
#include "sdo_rt_instance.h"

#include "sdo_backend_ecat.hpp"

namespace zmc {

namespace {

int32_t ecat_sdo_upload(void* ctx, const SdoCKey* key, uint32_t* len, void* data,
                        uint32_t timeout_ms) {
    if (!key) {
        return -1;
    }
    return EcatSdoBackend::Upload(ctx, key->slave_id, key->index, key->subindex, len, data,
                                  timeout_ms);
}

int32_t ecat_sdo_download(void* ctx, const SdoCKey* key, uint32_t len, const void* data,
                          uint32_t timeout_ms) {
    if (!key) {
        return -1;
    }
    return EcatSdoBackend::Download(ctx, key->slave_id, key->index, key->subindex, len, data,
                                    timeout_ms);
}

int32_t ecat_get_slave_state(void* ctx, uint16_t slave_id, uint8_t* slave_state) {
    return EcatGetSlaveState(static_cast<ECAT_HANDLE>(ctx), slave_id, slave_state);
}

}  // namespace

EcatSdoRt::EcatSdoRt() {}
EcatSdoRt::~EcatSdoRt() {}

int EcatSdoRt::bindEcat(void* ecat_handle) {
    SdoCAdapter adapter = {};
    adapter.ctx = ecat_handle;
    adapter.upload = ecat_sdo_upload;
    adapter.download = ecat_sdo_download;
    adapter.get_slave_state = ecat_get_slave_state;
    return bind_adapter(&adapter);
}

EcatSdoRt& SdoRtInstance::instance() {
    static EcatSdoRt s;
    return s;
}
int SdoRtInstance::bindEcat(void* h) { return instance().bindEcat(h); }
void SdoRtInstance::start() { instance().start(); }
void SdoRtInstance::stop() { instance().stop(); }


}  // namespace zmc
