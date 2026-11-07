/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-17
@ArtifactStatusAuthority GPT-5.6 Sol
*/
#ifndef _ZMC_SDO_CONFIG_HPP_
#define _ZMC_SDO_CONFIG_HPP_

#include <cstdint>

#include "common/zmc/fbt_errorcode_def.h"

#define SDO_REQUEST_MAX_ITEMS 8u
#define SDO_RESULT_MAX_ITEMS SDO_REQUEST_MAX_ITEMS

namespace zmc {

// Default adapter timeout in milliseconds (matches ECAT_SDO_TIMEOUT_MS value).
constexpr uint16_t SDO_DEFAULT_TIMEOUT_MS = 100u;
constexpr uint16_t SDO_DEFAULT_UPLOAD_WATCH_PERIOD_MS = 50u;
constexpr uint16_t SDO_DEFAULT_DOWNLOAD_WATCH_PERIOD_MS = 1000u;
// Runtime parameters for one upload/download data direction.
struct SdoDirectionParams {
    uint16_t timeout_ms{SDO_DEFAULT_TIMEOUT_MS};
    uint16_t watch_period_ms{SDO_DEFAULT_UPLOAD_WATCH_PERIOD_MS};
};

inline bool sdo_direction_params_valid(const SdoDirectionParams& params) {
    return params.timeout_ms >= 1u && params.watch_period_ms >= 1u;
}

// Pack timeout_ms and watch_period_ms into SdoRequestItem.data for SET commands.
inline uint32_t sdo_pack_direction_params(const SdoDirectionParams& params) {
    return (static_cast<uint32_t>(params.timeout_ms) << 16) |
           static_cast<uint32_t>(params.watch_period_ms);
}

// Unpack and validate params from a SET command item data field.
inline int sdo_unpack_direction_params(uint32_t packed, SdoDirectionParams* out) {
    if (!out) return ZMC_ERR_SDO_INVALID_ARG;
    SdoDirectionParams params{};
    params.timeout_ms = static_cast<uint16_t>((packed >> 16) & 0xFFFFu);
    params.watch_period_ms = static_cast<uint16_t>(packed & 0xFFFFu);
    if (!sdo_direction_params_valid(params)) return ZMC_ERR_SDO_INVALID_ARG;
    *out = params;
    return 0;
}

// Registered watch counts per upload/download direction (GET_WATCH_COUNT result).
struct SdoWatchCounts {
    uint16_t upload_count{0};
    uint16_t download_count{0};
};

// Pack upload/download watch counts into SdoResultItem.data for GET_WATCH_COUNT.
inline uint32_t sdo_pack_watch_counts(const SdoWatchCounts& counts) {
    return (static_cast<uint32_t>(counts.upload_count) << 16) |
           static_cast<uint32_t>(counts.download_count);
}

// Unpack watch counts from a GET_WATCH_COUNT result data field.
inline int sdo_unpack_watch_counts(uint32_t packed, SdoWatchCounts* out) {
    if (!out) return ZMC_ERR_SDO_INVALID_ARG;
    out->upload_count = static_cast<uint16_t>((packed >> 16) & 0xFFFFu);
    out->download_count = static_cast<uint16_t>(packed & 0xFFFFu);
    return 0;
}

}  // namespace zmc

#endif  // _ZMC_SDO_CONFIG_HPP_
