/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-17
@ArtifactStatusAuthority GPT-5 Codex
*/
#include "SdoMessage.hpp"

#include "common/zmc/fbt_errorcode_def.h"

namespace zmc {

std::size_t SdoKeyHash::operator()(const SdoKey &key) const noexcept {
  // Mix the compact slave, index, and subindex identity into one 32-bit value.
  uint32_t value = (static_cast<uint32_t>(key.slave_id) << 16) ^
                   static_cast<uint32_t>(key.index) ^
                   static_cast<uint32_t>(key.subindex);
  value ^= value >> 16;
  value *= 0x7feb352dU;
  value ^= value >> 15;
  value *= 0x846ca68bU;
  value ^= value >> 16;
  return static_cast<std::size_t>(value);
}

bool sdo_len_valid(uint8_t len) { return len >= 1u && len <= 4u; }

bool sdo_data_direction_valid(uint8_t direction) {
  return direction == SDO_DIR_UPLOAD || direction == SDO_DIR_DOWNLOAD;
}

uint8_t sdo_set_params_direction_for(uint8_t data_direction) {
  if (data_direction == SDO_DIR_UPLOAD)
    return SDO_DIR_SET_UPLOAD_PARAMS;
  if (data_direction == SDO_DIR_DOWNLOAD)
    return SDO_DIR_SET_DOWNLOAD_PARAMS;
  return SDO_DIR_NOP;
}

uint8_t sdo_get_params_direction_for(uint8_t data_direction) {
  if (data_direction == SDO_DIR_UPLOAD)
    return SDO_DIR_GET_UPLOAD_PARAMS;
  if (data_direction == SDO_DIR_DOWNLOAD)
    return SDO_DIR_GET_DOWNLOAD_PARAMS;
  return SDO_DIR_NOP;
}

bool sdo_direction_is_get_watch_count(uint8_t direction) {
  return direction == SDO_DIR_GET_WATCH_COUNT;
}

SdoDirectionParams sdo_direction_params_default(uint8_t data_direction) {
  SdoDirectionParams params{};
  params.timeout_ms = SDO_DEFAULT_TIMEOUT_MS;
  params.watch_period_ms = data_direction == SDO_DIR_DOWNLOAD
                               ? SDO_DEFAULT_DOWNLOAD_WATCH_PERIOD_MS
                               : SDO_DEFAULT_UPLOAD_WATCH_PERIOD_MS;
  return params;
}

bool sdo_direction_valid(uint8_t direction) {
  return direction <= SDO_DIR_EXIT;
}

bool sdo_direction_has_data(uint8_t direction) {
  return direction == SDO_DIR_DOWNLOAD ||
         direction == SDO_DIR_DOWNLOAD_WATCH ||
         direction == SDO_DIR_ADD_WATCH_DOWNLOAD ||
         direction == SDO_DIR_SET_UPLOAD_PARAMS ||
         direction == SDO_DIR_SET_DOWNLOAD_PARAMS;
}

uint32_t sdo_mask_for_len(uint8_t len) {
  switch (len) {
  case 1:
    return 0xFFu;
  case 2:
    return 0xFFFFu;
  case 3:
    return 0xFFFFFFu;
  case 4:
    return 0xFFFFFFFFu;
  default:
    return 0u;
  }
}

uint32_t sdo_mask_value(uint32_t data, uint8_t len) {
  return data & sdo_mask_for_len(len);
}

SdoKey sdo_request_item_key(const SdoRequestItem &item) {
  SdoKey key{};
  key.slave_id = item.slave_id;
  key.index = item.index;
  key.subindex = item.subindex;
  return key;
}

void SdoRequest::reset() {
  request_id = 0;
  item_count = 0;
}

bool SdoRequest::empty() const { return item_count == 0; }

bool SdoRequest::full() const { return item_count >= SDO_REQUEST_MAX_ITEMS; }

int SdoRequest::append_item(uint8_t direction, const SdoKey &key, uint8_t len,
                            uint32_t data) {
  if (!sdo_direction_valid(direction)) return ZMC_ERR_SDO_INVALID_ARG;
  if (!sdo_len_valid(len)) return ZMC_ERR_SDO_INVALID_LENGTH;
  if (full()) return ZMC_ERR_SDO_CAPACITY;

  SdoRequestItem &item = items[item_count++];
  item.slave_id = key.slave_id;
  item.index = key.index;
  item.subindex = key.subindex;
  item.len = len;
  item.direction = direction;
  item._padding = 0;
  item.data = sdo_direction_has_data(direction) ? data : 0u;
  return 0;
}

int SdoRequest::append_download(const SdoKey &key, uint8_t len,
                                uint32_t data) {
  return append_item(SDO_DIR_DOWNLOAD, key, len, data);
}

int SdoRequest::append_upload(const SdoKey &key, uint8_t len) {
  return append_item(SDO_DIR_UPLOAD, key, len, 0);
}

void SdoResult::reset() {
  item_count = 0;
}

bool SdoResult::empty() const { return item_count == 0; }

bool SdoResult::full() const { return item_count >= SDO_RESULT_MAX_ITEMS; }

int SdoResult::append_item(const SdoResultItem &item) {
  if (full()) return ZMC_ERR_SDO_CAPACITY;
  items[item_count++] = item;
  return 0;
}

} // namespace zmc
