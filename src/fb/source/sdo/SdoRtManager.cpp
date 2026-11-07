/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-21
@ArtifactStatusAuthority GLM-5.2 (opencode)
*/
#include "SdoRtManager.hpp"

#include <cstring>

namespace zmc {

namespace {

constexpr uint32_t kLegacyRequestIdMask = 0x3FFFFFFFu;

int sdo_length_result(uint32_t len) {
    return len >= 1u && len <= 4u ? 0 : ZMC_ERR_SDO_INVALID_LENGTH;
}

uint32_t sdo_load_data(const void *data, uint8_t len) {
    uint32_t value = 0;
    if (data && sdo_len_valid(len)) {
        std::memcpy(&value, data, len);
    }
    return sdo_mask_value(value, len);
}

}  // namespace

SdoRtManager::SdoRtManager(SdoChannel::Frontend frontend)
    : frontend_(frontend),
      direction_params_{sdo_direction_params_default(SDO_DIR_UPLOAD),
                        sdo_direction_params_default(SDO_DIR_DOWNLOAD)} {}

int SdoRtManager::enqueue_request(SdoRequest &request, uint32_t request_id) {
    if (request.empty()) return ZMC_ERR_SDO_INVALID_ARG;
    if (request_id == 0) {
        request_id_ = (request_id_ + 1u) & kLegacyRequestIdMask;
        if (request_id_ == 0) request_id_ = 1u;
        request_id = request_id_;
    }
    request.request_id = request_id;
    return frontend_.push_request(request);
}

int SdoRtManager::enqueue_command(uint8_t direction, const SdoKey &key, uint8_t len,
                                  uint32_t data, uint32_t request_id) {
    SdoRequest request{};
    const int append_result = request.append_item(direction, key, len, data);
    if (append_result != 0) return append_result;
    return enqueue_request(request, request_id);
}

int SdoRtManager::flush_pending_write() {
    if (pending_req_.empty()) return 0;
    const int enqueue_result = enqueue_request(pending_req_);
    if (enqueue_result != 0) return enqueue_result;
    pending_req_.reset();
    return 0;
}

int SdoRtManager::append_pending_write(const SdoKey &key, uint8_t len, uint32_t data) {
    if (!sdo_len_valid(len)) return ZMC_ERR_SDO_INVALID_LENGTH;
    int append_result = pending_req_.append_item(SDO_DIR_DOWNLOAD_WATCH, key, len,
                                                  sdo_mask_value(data, len));
    if (append_result == 0) return 0;
    if (append_result != ZMC_ERR_SDO_CAPACITY) return append_result;
    const int flush_result = flush_pending_write();
    if (flush_result != 0) return flush_result;
    return pending_req_.append_item(SDO_DIR_DOWNLOAD_WATCH, key, len,
                                    sdo_mask_value(data, len));
}

void SdoRtManager::update_dict(const SdoResultItem &item) {
    SdoValue value{};
    value.result_code = item.result_code;
    value.data_len = item.data_len;
    value.data = sdo_mask_value(item.data, item.data_len);
    value.seq = item.seq;

    for (auto &entry : dict_) {
        if (entry.used && entry.key == item.key) {
            entry.value = value;
            return;
        }
    }
    for (auto &entry : dict_) {
        if (!entry.used) {
            entry.used = true;
            entry.key = item.key;
            entry.value = value;
            return;
        }
    }
    dict_[0].used = true;
    dict_[0].key = item.key;
    dict_[0].value = value;
}

void SdoRtManager::update_direction_params(const SdoResultItem &item) {
    // A SET echo and a GET query result both carry the worker-active params
    // for one data direction; either one refreshes the unified snapshot.
    const bool is_upload_params = item.direction == SDO_DIR_SET_UPLOAD_PARAMS ||
                                  item.direction == SDO_DIR_GET_UPLOAD_PARAMS;
    const bool is_download_params = item.direction == SDO_DIR_SET_DOWNLOAD_PARAMS ||
                                    item.direction == SDO_DIR_GET_DOWNLOAD_PARAMS;
    if (!is_upload_params && !is_download_params) return;
    const uint8_t data_direction = is_upload_params ? SDO_DIR_UPLOAD : SDO_DIR_DOWNLOAD;
    if (item.result_code != 0 || item.data_len != 4) return;

    SdoDirectionParams params{};
    if (sdo_unpack_direction_params(item.data, &params) != 0) return;
    const uint8_t direction_index = data_direction == SDO_DIR_DOWNLOAD ? 1u : 0u;
    direction_params_[direction_index] = params;
    direction_params_seq_[direction_index] = item.seq;
}

void SdoRtManager::update_queried_watch_count(const SdoResultItem &item) {
    if (item.result_code != 0 || item.data_len != 4) return;

    SdoWatchCounts counts{};
    if (sdo_unpack_watch_counts(item.data, &counts) != 0) return;
    queried_watch_count_ = counts;
    queried_watch_count_seq_ = item.seq;
}

const SdoValue *SdoRtManager::find_dict(const SdoKey &key) const {
    for (const auto &entry : dict_) {
        if (entry.used && entry.key == key) return &entry.value;
    }
    return nullptr;
}

int SdoRtManager::add_download_watch_key(const SdoKey &key) {
    for (auto &entry : download_watch_keys_) {
        if (entry.used && entry.key == key) return 0;
    }
    for (auto &entry : download_watch_keys_) {
        if (!entry.used) {
            entry.used = true;
            entry.key = key;
            return 0;
        }
    }
    return ZMC_ERR_SDO_CAPACITY;
}

bool SdoRtManager::has_download_watch_key(const SdoKey &key) const {
    for (const auto &entry : download_watch_keys_) {
        if (entry.used && entry.key == key) return true;
    }
    return false;
}

void SdoRtManager::remove_download_watch_key(const SdoKey &key) {
    for (auto &entry : download_watch_keys_) {
        if (entry.used && entry.key == key) {
            entry.used = false;
            return;
        }
    }
}

void SdoRtManager::clear_download_watch_keys() {
    for (auto &entry : download_watch_keys_) {
        entry.used = false;
    }
}

bool SdoRtManager::take_oneshot_results(SdoResult *out) {
    if (!out || oneshot_staging_.empty()) return false;
    *out = oneshot_staging_;
    oneshot_staging_.reset();
    return true;
}

void SdoRtManager::pump_results() {
    ++pump_tick_;
    if (pump_tick_ < pump_divider_) return;
    pump_tick_ = 0;

    SdoResult result{};
    while (!oneshot_staging_.full() && frontend_.pop_result(result)) {
        for (uint8_t i = 0; i < result.item_count; ++i) {
            const SdoResultItem &item = result.items[i];
            if (sdo_direction_is_get_watch_count(item.direction)) {
                update_queried_watch_count(item);
            } else if (sdo_data_direction_valid(item.direction)) {
                if (sdo_request_id_is_axis_coe(item.request_id)) {
                    (void)oneshot_staging_.append_item(item);
                } else {
                    update_dict(item);
                }
            } else {
                update_direction_params(item);
            }
            last_seq_ = item.seq;
        }
    }

    (void)flush_pending_write();
}

void SdoRtManager::set_pump_divider(uint32_t n) {
    pump_divider_ = n == 0 ? 1 : n;
    pump_tick_ = 0;
}

int SdoRtManager::get_u32(const SdoKey &key, uint32_t *out, uint32_t *seq) const {
    if (!out) return ZMC_ERR_SDO_INVALID_ARG;
    const SdoValue *value = find_dict(key);
    if (!value) return ZMC_ERR_SDO_NO_RESULT;
    if (value->result_code != 0) return value->result_code;
    if (!sdo_len_valid(value->data_len)) return ZMC_ERR_SDO_INVALID_LENGTH;

    *out = sdo_mask_value(value->data, value->data_len);
    if (seq) *seq = value->seq;
    return 0;
}

int SdoRtManager::sdo_read_once(const SdoKey &key, uint32_t max_len, uint32_t request_id) {
    const int length_result = sdo_length_result(max_len);
    if (length_result != 0) return length_result;
    return enqueue_command(SDO_DIR_UPLOAD, key, static_cast<uint8_t>(max_len), 0, request_id);
}

int SdoRtManager::sdo_write(const SdoKey &key, uint32_t len, const void *data,
                            uint32_t request_id) {
    if (!data) return ZMC_ERR_SDO_INVALID_ARG;
    const int length_result = sdo_length_result(len);
    if (length_result != 0) return length_result;
    const uint8_t value_len = static_cast<uint8_t>(len);
    SdoRequest request{};
    const int append_result = request.append_download(key, value_len, sdo_load_data(data, value_len));
    if (append_result != 0) return append_result;
    return enqueue_request(request, request_id);
}

bool SdoRtManager::is_download_watch_key(const SdoKey &key) const {
    return has_download_watch_key(key);
}

int SdoRtManager::add_watch_upload(const SdoKey &key, uint32_t period_ms, uint32_t max_len) {
    // Direction params continue to own the effective watch period.
    (void)period_ms;
    const int length_result = sdo_length_result(max_len);
    if (length_result != 0) return length_result;
    return enqueue_command(SDO_DIR_ADD_WATCH_UPLOAD, key, static_cast<uint8_t>(max_len));
}

int SdoRtManager::add_watch_download(const SdoKey &key, uint32_t period_ms, uint32_t len,
                                     const void *data) {
    // Direction params continue to own the effective watch period.
    (void)period_ms;
    if (!data) return ZMC_ERR_SDO_INVALID_ARG;
    const int length_result = sdo_length_result(len);
    if (length_result != 0) return length_result;
    const uint8_t value_len = static_cast<uint8_t>(len);
    const bool already_registered = has_download_watch_key(key);
    if (!already_registered) {
        const int add_result = add_download_watch_key(key);
        if (add_result != 0) return add_result;
    }
    const int enqueue_result = enqueue_command(SDO_DIR_ADD_WATCH_DOWNLOAD, key, value_len,
                                               sdo_load_data(data, value_len));
    if (enqueue_result != 0 && !already_registered) remove_download_watch_key(key);
    return enqueue_result;
}

int SdoRtManager::set_direction_params(uint8_t data_direction,
                                       const SdoDirectionParams &params) {
    const uint8_t set_direction = sdo_set_params_direction_for(data_direction);
    if (set_direction == SDO_DIR_NOP || !sdo_direction_params_valid(params)) {
        return ZMC_ERR_SDO_INVALID_ARG;
    }
    SdoKey key{};
    return enqueue_command(set_direction, key, 4, sdo_pack_direction_params(params));
}

int SdoRtManager::get_direction_params(uint8_t data_direction, SdoDirectionParams *out,
                                       uint32_t *seq) const {
    if (!out || !sdo_data_direction_valid(data_direction)) return ZMC_ERR_SDO_INVALID_ARG;
    const uint8_t direction_index = data_direction == SDO_DIR_DOWNLOAD ? 1u : 0u;
    *out = direction_params_[direction_index];
    if (seq) *seq = direction_params_seq_[direction_index];
    return 0;
}

int SdoRtManager::query_direction_params(uint8_t data_direction) {
    const uint8_t get_direction = sdo_get_params_direction_for(data_direction);
    if (get_direction == SDO_DIR_NOP) return ZMC_ERR_SDO_INVALID_ARG;
    SdoKey key{};
    return enqueue_command(get_direction, key, 4, 0);
}

int SdoRtManager::query_watch_count() {
    SdoKey key{};
    return enqueue_command(SDO_DIR_GET_WATCH_COUNT, key, 4, 0);
}

int SdoRtManager::get_queried_watch_count(SdoWatchCounts *out, uint32_t *seq) const {
    if (!out) return ZMC_ERR_SDO_INVALID_ARG;
    if (queried_watch_count_seq_ == 0) return ZMC_ERR_SDO_NO_RESULT;
    *out = queried_watch_count_;
    if (seq) *seq = queried_watch_count_seq_;
    return 0;
}

int SdoRtManager::remove_watch(const SdoKey &key) {
    const int enqueue_result = enqueue_command(SDO_DIR_REMOVE_WATCH, key, 4);
    if (enqueue_result != 0) return enqueue_result;
    remove_download_watch_key(key);
    return 0;
}

void SdoRtManager::clear_watches() {
    SdoRequest request{};
    SdoKey key{};
    (void)request.append_item(SDO_DIR_CLEAR_WATCHES, key, 4);
    (void)enqueue_request(request);
    clear_download_watch_keys();
    pending_req_.reset();
}

int SdoRtManager::request_exit() {
    SdoRequest request{};
    SdoKey key{};
    const int append_result = request.append_item(SDO_DIR_EXIT, key, 4);
    if (append_result != 0) return append_result;
    return enqueue_request(request);
}

int SdoRtManager::add_watch_download_with_value(const SdoKey &key, uint32_t period_ms,
                                                int32_t initial_value, uint32_t value_size) {
    const int length_result = sdo_length_result(value_size);
    if (length_result != 0) return length_result;
    return add_watch_download(key, period_ms, value_size, &initial_value);
}

int SdoRtManager::update_watch_write_value(const SdoKey &key, int32_t new_value,
                                           uint32_t value_size) {
    const int length_result = sdo_length_result(value_size);
    if (length_result != 0) return length_result;
    if (!has_download_watch_key(key)) return ZMC_ERR_SDO_WATCH_NOT_FOUND;
    return append_pending_write(key, static_cast<uint8_t>(value_size),
                                sdo_load_data(&new_value, static_cast<uint8_t>(value_size)));
}

}  // namespace zmc
