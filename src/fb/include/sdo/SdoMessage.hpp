/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-17
@ArtifactStatusAuthority GPT-5 Codex
*/
#ifndef _ZMC_SDO_MESSAGE_HPP_
#define _ZMC_SDO_MESSAGE_HPP_

#include <cstddef>
#include <cstdint>

#include "sdo_config.hpp"
#include "sdo_key.hpp"

namespace zmc {

// Hash one SDO object key without changing key identity semantics.
struct SdoKeyHash {
    std::size_t operator()(const SdoKey &key) const noexcept;
};

// Store one cached SDO access result.
struct SdoValue {
    int32_t result_code{0};
    uint8_t data_len{0};
    uint32_t data{0};
    uint32_t seq{0};
};

// Preserve every existing request direction value.
enum SdoDirection : uint8_t {
    SDO_DIR_NOP,
    SDO_DIR_UPLOAD,
    SDO_DIR_DOWNLOAD,
    SDO_DIR_DOWNLOAD_WATCH,
    SDO_DIR_ADD_WATCH_UPLOAD,
    SDO_DIR_ADD_WATCH_DOWNLOAD,
    SDO_DIR_REMOVE_WATCH,
    SDO_DIR_SET_UPLOAD_PARAMS,
    SDO_DIR_SET_DOWNLOAD_PARAMS,
    SDO_DIR_GET_UPLOAD_PARAMS,
    SDO_DIR_GET_DOWNLOAD_PARAMS,
    SDO_DIR_GET_WATCH_COUNT,
    SDO_DIR_CLEAR_WATCHES,
    SDO_DIR_EXIT,
};

constexpr uint32_t SDO_FIXED_CACHE_CAPACITY = 128;
constexpr uint32_t SDO_DEFAULT_PUMP_DIVIDER = 10;
constexpr uint32_t SDO_AXIS_COE_REQUEST_TYPE_MASK = 0xC0000000u;
constexpr uint32_t SDO_AXIS_COE_WRITE_REQUEST_TYPE = 0x40000000u;
constexpr uint32_t SDO_AXIS_COE_READ_REQUEST_TYPE = 0x80000000u;

// Build one Axis CoE download request id from a zero-based axis and axis-local tag.
inline uint32_t sdo_axis_coe_write_request_id(uint32_t axis, uint32_t tag) {
    return SDO_AXIS_COE_WRITE_REQUEST_TYPE | ((axis & 0x3Fu) << 24) | (tag & 0x00FFFFFFu);
}
// Build one Axis CoE upload request id from a zero-based axis and axis-local tag.
inline uint32_t sdo_axis_coe_read_request_id(uint32_t axis, uint32_t tag) {
    return SDO_AXIS_COE_READ_REQUEST_TYPE | ((axis & 0x3Fu) << 24) | (tag & 0x00FFFFFFu);
}
inline bool sdo_request_id_is_axis_coe_write(uint32_t id) {
    return (id & SDO_AXIS_COE_REQUEST_TYPE_MASK) == SDO_AXIS_COE_WRITE_REQUEST_TYPE;
}
// Identify either Axis CoE request direction for one-shot result routing.
inline bool sdo_request_id_is_axis_coe(uint32_t id) {
    const uint32_t type = id & SDO_AXIS_COE_REQUEST_TYPE_MASK;
    return type == SDO_AXIS_COE_WRITE_REQUEST_TYPE || type == SDO_AXIS_COE_READ_REQUEST_TYPE;
}
inline uint32_t sdo_request_id_axis_index(uint32_t id) { return (id >> 24) & 0x3Fu; }
inline uint32_t sdo_request_id_axis_tag(uint32_t id) { return id & 0x00FFFFFFu; }

// Validate an expedited transfer length.
bool sdo_len_valid(uint8_t len);
// Validate an upload or download data direction.
bool sdo_data_direction_valid(uint8_t direction);
// Map a data direction to its SET command.
uint8_t sdo_set_params_direction_for(uint8_t data_direction);
// Map a data direction to its GET command.
uint8_t sdo_get_params_direction_for(uint8_t data_direction);
// Identify a watch-count query.
bool sdo_direction_is_get_watch_count(uint8_t direction);
// Build the existing defaults for one data direction.
SdoDirectionParams sdo_direction_params_default(uint8_t data_direction);
// Validate any request direction without changing accepted values.
bool sdo_direction_valid(uint8_t direction);
// Identify commands whose item data field carries a payload.
bool sdo_direction_has_data(uint8_t direction);
// Return the existing byte mask for a valid expedited length.
uint32_t sdo_mask_for_len(uint8_t len);
// Mask a value to its low transfer-length bytes.
uint32_t sdo_mask_value(uint32_t data, uint8_t len);

// Preserve the exact flat request-item layout.
struct SdoRequestItem {
    uint16_t slave_id;
    uint16_t index;
    uint8_t subindex;
    uint8_t len;
    uint8_t direction;
    uint8_t _padding;
    uint32_t data;
};

// Rebuild an SdoKey from a request item.
SdoKey sdo_request_item_key(const SdoRequestItem &item);

// Preserve the exact request batch layout and append semantics.
struct SdoRequest {
    uint32_t request_id;
    uint8_t item_count;
    SdoRequestItem items[SDO_REQUEST_MAX_ITEMS];

    // Reset only the logical payload.
    void reset();
    // Return whether the batch has no items.
    bool empty() const;
    // Return whether the batch reached its fixed capacity.
    bool full() const;
    // Append one validated item in execution order.
    int append_item(uint8_t direction, const SdoKey &key, uint8_t len, uint32_t data = 0);
    // Append one download item.
    int append_download(const SdoKey &key, uint8_t len, uint32_t data);
    // Append one upload item.
    int append_upload(const SdoKey &key, uint8_t len);
};

// Preserve the exact flat result-item layout.
struct SdoResultItem {
    SdoKey key;
    int32_t result_code;
    uint8_t data_len;
    uint8_t direction;
    uint32_t data;
    uint32_t seq;
    uint32_t request_id;
};

// Store one batch of SDO results.
struct SdoResult {
    uint8_t item_count;
    SdoResultItem items[SDO_RESULT_MAX_ITEMS];

    // Reset the logical payload.
    void reset();
    // Return whether the item payload is empty.
    bool empty() const;
    // Return whether no more result items can be appended.
    bool full() const;
    // Append one result in execution order.
    int append_item(const SdoResultItem &item);
};

static_assert(sizeof(SdoKey) == 6u, "SdoKey layout must remain compact");
static_assert(sizeof(SdoRequestItem) == 12u, "SdoRequestItem layout must remain compact");
}  // namespace zmc

#endif  // _ZMC_SDO_MESSAGE_HPP_
