#include "mc_core/mc_eni_blob.h"

#include <cstring>

namespace {

ecat_eni_info_ex g_blob;
bool g_blob_valid = false;

}  // namespace

extern "C" {

int mc_eni_load_pending_descriptor(void) {
    ecat_eni_info_ex info;
    std::memset(&info, 0, sizeof(info));
    int err = MC_EcatGetEniInfo(&info);
    if (err) return err;
    g_blob = info;
    g_blob_valid = true;
    return 0;
}

const ecat_eni_info_ex* mc_eni_blob_get(void) {
    return g_blob_valid ? &g_blob : 0;
}

void mc_eni_blob_reset(void) {
    g_blob_valid = false;
    std::memset(&g_blob, 0, sizeof(g_blob));
}

}  // extern "C"
