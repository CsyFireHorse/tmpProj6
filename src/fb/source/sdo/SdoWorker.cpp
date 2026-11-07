/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-17
@ArtifactStatusAuthority GPT-5 Codex
*/
#include "SdoWorker.hpp"

#include "ecat/EcatState.hpp"
#include "common/zmc/fbt_errorcode_def.h"
#include "common/zmc/pci_errno.h"
#include "common/zmc/pcie_compat.h"
#include "common/zmc/zmc_predef.h"

namespace zmc {

namespace {

// Preserve the existing idle cadence.
constexpr uint32_t SDO_POLL_SLEEP_US = 2000;
// Force every tenth unchanged watch download through the adapter.
constexpr uint8_t SDO_WRITE_CACHE_MAX_CONSECUTIVE_SKIPS = 9;

bool sdo_adapter_valid(const SdoCAdapter &adapter) {
    return adapter.upload != nullptr && adapter.download != nullptr;
}

SdoCKey to_sdo_c_key(const SdoKey &key) {
    SdoCKey out{};
    out.slave_id = key.slave_id;
    out.index = key.index;
    out.subindex = key.subindex;
    return out;
}

bool same_sdo_object(const SdoRequestItem &item, const SdoKey &key) {
    return item.slave_id == key.slave_id && item.index == key.index &&
           item.subindex == key.subindex;
}

}  // namespace

SdoWorker::SdoWorker()
    : active_params_{sdo_direction_params_default(SDO_DIR_UPLOAD),
                     sdo_direction_params_default(SDO_DIR_DOWNLOAD)} {}

SdoChannel::Frontend SdoWorker::frontend() { return channel_.frontend(); }

int SdoWorker::bind_adapter(const SdoCAdapter *adapter) {
    if (!adapter || !sdo_adapter_valid(*adapter)) return ZMC_ERR_SDO_ADAPTER_INVALID;
    adapter_ = *adapter;
    return 0;
}

void SdoWorker::clear_adapter() {
    adapter_ = SdoCAdapter();
    clear_write_cache();
}

void SdoWorker::run_blocking() {
    stop_flag_ = false;
    run();
}

void SdoWorker::request_stop() { stop_flag_ = true; }

void SdoWorker::pause() { paused_ = true; }

void SdoWorker::resume() { paused_ = false; }

void SdoWorker::reset_after_join() {
    channel_.reset();
    next_watch_index_ = 0;
    pending_user_sdo_.clear();
}

void SdoWorker::clear_write_cache() {
    for (auto &entry : write_cache_) {
        entry.used = false;
        entry.consecutive_skip_count = 0;
    }
}

bool SdoWorker::try_skip_cached_download(const SdoKey &key, uint8_t len, uint32_t data) {
    const uint32_t masked = sdo_mask_value(data, len);
    for (auto &entry : write_cache_) {
        if (!entry.used || !(entry.key == key)) continue;
        if (entry.len != len) return false;
        if (entry.data != masked ||
            entry.consecutive_skip_count >= SDO_WRITE_CACHE_MAX_CONSECUTIVE_SKIPS) {
            return false;
        }
        ++entry.consecutive_skip_count;
        return true;
    }
    return false;
}

void SdoWorker::invalidate_write_cache(const SdoKey &key, uint8_t len) {
    for (auto &entry : write_cache_) {
        if (!entry.used || !(entry.key == key) || entry.len != len) continue;
        entry.used = false;
        entry.consecutive_skip_count = 0;
        return;
    }
}

void SdoWorker::update_write_cache(const SdoKey &key, uint8_t len, uint32_t data) {
    const uint32_t masked = sdo_mask_value(data, len);
    for (auto &entry : write_cache_) {
        if (entry.used && entry.key == key && entry.len == len) {
            entry.data = masked;
            entry.consecutive_skip_count = 0;
            return;
        }
    }
    for (auto &entry : write_cache_) {
        if (!entry.used) {
            entry.used = true;
            entry.key = key;
            entry.len = len;
            entry.consecutive_skip_count = 0;
            entry.data = masked;
            return;
        }
    }
    write_cache_[0].used = true;
    write_cache_[0].key = key;
    write_cache_[0].len = len;
    write_cache_[0].consecutive_skip_count = 0;
    write_cache_[0].data = masked;
}

int SdoWorker::append_cached_download_result(const SdoKey &key, uint8_t len, uint32_t data,
                                             uint32_t id, uint32_t &result_seq,
                                             SdoResult &results) {
    SdoResultItem item{};
    item.key = key;
    item.direction = SDO_DIR_DOWNLOAD;
    item.request_id = id;
    item.result_code = 0;
    item.data_len = len;
    item.data = sdo_mask_value(data, len);
    item.seq = ++result_seq;
    return results.append_item(item);
}

int32_t SdoWorker::check_slave_state(uint16_t slave_id) const {
    uint8_t slave_state = 0u;
    if (!adapter_.get_slave_state) return ZMC_ERR_ECAT_GET_SLAVE_STATE_FAILED;

    const int32_t result = adapter_.get_slave_state(adapter_.ctx, slave_id, &slave_state);
    if (result == ECAT_E_INVALID_SLAVE_INDEX) return ZMC_ERR_ECAT_INVALID_SLAVE_INDEX;
    if (result != 0) return ZMC_ERR_ECAT_GET_SLAVE_STATE_FAILED;
    if (slave_state == 0u) return ZMC_ERR_ECAT_SLAVE_NOT_PRESENT;
    if ((slave_state & ECAT_STATE_MASK) != ECAT_STATE_OPERATIONAL) {
        return ZMC_ERR_ECAT_SLAVE_NOT_OP;
    }
    return 0;
}

int SdoWorker::append_upload_result(const SdoKey &key, uint8_t expected_len, uint32_t id,
                                    uint32_t &result_seq, SdoResult &results) {
    SdoResultItem item{};
    item.key = key;
    item.direction = SDO_DIR_UPLOAD;
    item.request_id = id;
    item.result_code = ZMC_ERR_SDO_ADAPTER_INVALID;
    item.data_len = 0;
    item.data = 0;
    item.seq = ++result_seq;

    if (sdo_adapter_valid(adapter_)) {
        item.result_code = check_slave_state(key.slave_id);
    }
    if (item.result_code == 0) {
        uint32_t len = expected_len;
        uint32_t data = 0;
        const SdoCKey c_key = to_sdo_c_key(key);
        item.result_code = adapter_.upload(adapter_.ctx, &c_key, &len, &data,
                                           direction_params(SDO_DIR_UPLOAD).timeout_ms);
        item.data_len = sdo_len_valid(static_cast<uint8_t>(len)) ? static_cast<uint8_t>(len) : 0;
        item.data = sdo_mask_value(data, item.data_len);
    }

    return results.append_item(item);
}

int SdoWorker::append_download_result(const SdoKey &key, uint8_t len, uint32_t data,
                                      uint32_t id, uint32_t &result_seq,
                                      SdoResult &results, bool allow_cache_skip) {
    const uint32_t masked = sdo_mask_value(data, len);

    int32_t state_result = ZMC_ERR_SDO_ADAPTER_INVALID;
    if (sdo_adapter_valid(adapter_)) state_result = check_slave_state(key.slave_id);
    if (state_result != 0) {
        // A state error must invalidate any value that could synthesize success.
        invalidate_write_cache(key, len);
        SdoResultItem item{};
        item.key = key;
        item.direction = SDO_DIR_DOWNLOAD;
        item.request_id = id;
        item.result_code = state_result;
        item.data_len = 0;
        item.data = masked;
        item.seq = ++result_seq;
        return results.append_item(item);
    }
    if (allow_cache_skip && try_skip_cached_download(key, len, masked)) {
        return append_cached_download_result(key, len, masked, id, result_seq, results);
    }

    // A failed real write must not leave a cached value that can synthesize success.
    invalidate_write_cache(key, len);

    SdoResultItem item{};
    item.key = key;
    item.direction = SDO_DIR_DOWNLOAD;
    item.request_id = id;
    item.result_code = ZMC_ERR_SDO_ADAPTER_INVALID;
    item.data_len = 0;
    item.data = masked;
    item.seq = ++result_seq;

    if (sdo_adapter_valid(adapter_)) {
        const SdoCKey c_key = to_sdo_c_key(key);
        item.result_code = adapter_.download(adapter_.ctx, &c_key, len, &masked,
                                             direction_params(SDO_DIR_DOWNLOAD).timeout_ms);
        item.data_len = len;
    }
    if (item.result_code == 0) update_write_cache(key, len, masked);
    return results.append_item(item);
}

int SdoWorker::append_command_result(const SdoRequestItem &request, uint32_t id,
                                     int32_t result_code, uint32_t &result_seq,
                                     SdoResult &results, uint8_t data_len, uint32_t data) {
    SdoResultItem item{};
    item.key = sdo_request_item_key(request);
    item.result_code = result_code;
    item.data_len = data_len;
    item.direction = request.direction;
    item.data = data;
    item.seq = ++result_seq;
    item.request_id = id;
    return results.append_item(item);
}

bool SdoWorker::dispatch_request(const SdoRequest &request, uint32_t &result_seq) {
    SdoResult results{};
    bool exit_requested = false;
    for (uint8_t i = 0; i < request.item_count; ++i) {
        const SdoRequestItem &item = request.items[i];
        if (!sdo_direction_valid(item.direction) || !sdo_len_valid(item.len)) {
            const int error = sdo_direction_valid(item.direction) ? ZMC_ERR_SDO_INVALID_LENGTH
                                                                   : ZMC_ERR_SDO_INVALID_ARG;
            (void)append_command_result(item, request.request_id, error, result_seq, results);
            continue;
        }

        if (sdo_data_direction_valid(item.direction)) {
            SdoRequest sdo_request{};
            sdo_request.request_id = request.request_id;
            sdo_request.item_count = 1;
            sdo_request.items[0] = item;
            pending_user_sdo_.push_back(sdo_request);
            continue;
        }

        const SdoKey key = sdo_request_item_key(item);
        switch (item.direction) {
            case SDO_DIR_DOWNLOAD_WATCH:
                update_download_watch_data(key, item.len, item.data);
                break;
            case SDO_DIR_ADD_WATCH_UPLOAD:
            case SDO_DIR_ADD_WATCH_DOWNLOAD:
            case SDO_DIR_REMOVE_WATCH:
            case SDO_DIR_CLEAR_WATCHES: {
                const int apply_result = apply_watch_command(item);
                if (apply_result == 0 && item.direction == SDO_DIR_CLEAR_WATCHES) {
                    clear_write_cache();
                }
                (void)append_command_result(item, request.request_id, apply_result, result_seq,
                                            results);
                break;
            }
            // SET/GET enum pairs keep upload directly before download.
            case SDO_DIR_SET_UPLOAD_PARAMS:
            case SDO_DIR_SET_DOWNLOAD_PARAMS: {
                SdoDirectionParams params{};
                int apply_result = sdo_unpack_direction_params(item.data, &params);
                if (apply_result == 0) {
                    const uint8_t data_direction = static_cast<uint8_t>(
                        item.direction - SDO_DIR_SET_UPLOAD_PARAMS + SDO_DIR_UPLOAD);
                    apply_result = apply_direction_params(data_direction, params);
                }
                (void)append_command_result(item, request.request_id, apply_result, result_seq,
                                            results, 4, item.data);
                break;
            }
            case SDO_DIR_GET_UPLOAD_PARAMS:
            case SDO_DIR_GET_DOWNLOAD_PARAMS: {
                const uint8_t data_direction = static_cast<uint8_t>(
                    item.direction - SDO_DIR_GET_UPLOAD_PARAMS + SDO_DIR_UPLOAD);
                const SdoDirectionParams params = direction_params(data_direction);
                (void)append_command_result(item, request.request_id, 0, result_seq, results, 4,
                                            sdo_pack_direction_params(params));
                break;
            }
            case SDO_DIR_GET_WATCH_COUNT: {
                SdoWatchCounts counts{};
                counts.upload_count = static_cast<uint16_t>(watch_count_for(SDO_DIR_UPLOAD));
                counts.download_count = static_cast<uint16_t>(watch_count_for(SDO_DIR_DOWNLOAD));
                (void)append_command_result(item, request.request_id, 0, result_seq, results, 4,
                                            sdo_pack_watch_counts(counts));
                break;
            }
            case SDO_DIR_EXIT:
                exit_requested = true;
                break;
            default:
                break;
        }
        if (exit_requested) break;
    }

    if (!results.empty()) (void)channel_.push_result(results);
    return exit_requested;
}

void SdoWorker::handle_request(const SdoRequest &request, uint32_t &result_seq) {
    SdoResult results{};
    for (uint8_t i = 0; i < request.item_count; ++i) {
        const SdoRequestItem &item = request.items[i];
        const SdoKey key = sdo_request_item_key(item);
        if (item.direction == SDO_DIR_UPLOAD) {
            (void)append_upload_result(key, item.len, request.request_id, result_seq, results);
        } else {
            update_download_watch_data(key, item.len, item.data);
            (void)append_download_result(key, item.len, item.data, request.request_id,
                                         result_seq, results, false);
        }
    }

    if (!results.empty()) (void)channel_.push_result(results);
}

bool SdoWorker::scan_requests(uint32_t &result_seq, bool &did_work) {
    SdoRequest request{};
    while (channel_.pop_request(request)) {
        did_work = true;
        if (dispatch_request(request, result_seq)) return true;
        if (stop_flag_) break;
    }
    return false;
}

int SdoWorker::apply_watch_command(const SdoRequestItem &request) {
    if (request.direction == SDO_DIR_CLEAR_WATCHES) {
        watch_list_.clear();
        next_watch_index_ = 0;
        return 0;
    }
    if (request.direction == SDO_DIR_REMOVE_WATCH) {
        const SdoKey key = sdo_request_item_key(request);
        for (auto it = watch_list_.begin(); it != watch_list_.end(); ++it) {
            if (same_sdo_object(it->item, key)) {
                const std::size_t removed_index =
                    static_cast<std::size_t>(it - watch_list_.begin());
                if (removed_index < next_watch_index_) --next_watch_index_;
                watch_list_.erase(it);
                if (watch_list_.empty() || next_watch_index_ >= watch_list_.size()) {
                    next_watch_index_ = 0;
                }
                break;
            }
        }
        return 0;
    }

    if (request.direction == SDO_DIR_ADD_WATCH_UPLOAD ||
        request.direction == SDO_DIR_ADD_WATCH_DOWNLOAD) {
        return add_or_update_watch(request);
    }
    return ZMC_ERR_SDO_INVALID_ARG;
}

int SdoWorker::add_or_update_watch(const SdoRequestItem &request) {
    SdoRequestItem watch_item = request;
    // ADD_WATCH enum values keep upload directly before download.
    watch_item.direction = static_cast<uint8_t>(
        request.direction - SDO_DIR_ADD_WATCH_UPLOAD + SDO_DIR_UPLOAD);
    watch_item.data = sdo_mask_value(request.data, request.len);
    const SdoKey key = sdo_request_item_key(watch_item);

    for (auto &watch : watch_list_) {
        if (same_sdo_object(watch.item, key)) {
            watch.item = watch_item;
            return 0;
        }
    }

    SdoWatchItem watch{};
    watch.item = watch_item;
    watch.last_tick_ms = 0;
    watch_list_.push_back(watch);
    return 0;
}

int SdoWorker::apply_direction_params(uint8_t data_direction, const SdoDirectionParams &params) {
    const uint8_t direction_index = data_direction == SDO_DIR_DOWNLOAD ? 1u : 0u;
    active_params_[direction_index] = params;
    return 0;
}

const SdoDirectionParams &SdoWorker::direction_params(uint8_t data_direction) const {
    const uint8_t direction_index = data_direction == SDO_DIR_DOWNLOAD ? 1u : 0u;
    return active_params_[direction_index];
}

uint32_t SdoWorker::watch_count_for(uint8_t data_direction) const {
    if (!sdo_data_direction_valid(data_direction)) return 0;
    uint32_t count = 0;
    for (const auto &watch : watch_list_) {
        if (watch.item.direction == data_direction) ++count;
    }
    return count;
}

void SdoWorker::update_download_watch_data(const SdoKey &key, uint8_t len, uint32_t data) {
    for (auto &watch : watch_list_) {
        if (watch.item.direction != SDO_DIR_DOWNLOAD) continue;
        if (!(sdo_request_item_key(watch.item) == key)) continue;
        watch.item.len = len;
        watch.item.data = sdo_mask_value(data, len);
        return;
    }
}

bool SdoWorker::handle_watch(SdoWatchItem &watch, uint32_t now_ms, uint32_t &result_seq,
                             SdoResult &results) {
    const uint32_t period_ms = direction_params(watch.item.direction).watch_period_ms;
    if (now_ms - watch.last_tick_ms < period_ms) return false;

    const SdoKey key = sdo_request_item_key(watch.item);
    if (watch.item.direction == SDO_DIR_UPLOAD) {
        (void)append_upload_result(key, watch.item.len, 0, result_seq, results);
    } else {
        (void)append_download_result(key, watch.item.len, watch.item.data, 0, result_seq, results,
                                     true);
    }
    watch.last_tick_ms = now_ms;
    return true;
}

bool SdoWorker::handle_one_due_watch(uint32_t now_ms, uint32_t &result_seq) {
    SdoResult results{};
    const std::size_t watch_count = watch_list_.size();
    if (watch_count == 0) {
        next_watch_index_ = 0;
        return false;
    }

    next_watch_index_ %= watch_count;
    for (std::size_t offset = 0; offset < watch_count; ++offset) {
        const std::size_t index = (next_watch_index_ + offset) % watch_count;
        if (!handle_watch(watch_list_[index], now_ms, result_seq, results)) continue;
        if (!results.empty()) {
            (void)channel_.push_result(results);
        }
        next_watch_index_ = (index + 1) % watch_count;
        return true;
    }
    return false;
}

void SdoWorker::run() {
    clear_write_cache();
    uint32_t result_seq = 0;
    bool prefer_user_sdo = true;

    while (!stop_flag_) {
        if (paused_) {
            usleep(SDO_POLL_SLEEP_US);
            continue;
        }

        bool did_work = false;
        if (scan_requests(result_seq, did_work)) return;
        if (stop_flag_) break;

        const uint32_t now_ms = get_tick_ms();
        const bool user_sdo_ready = !pending_user_sdo_.empty();
        if ((!prefer_user_sdo || !user_sdo_ready) &&
            handle_one_due_watch(now_ms, result_seq)) {
            prefer_user_sdo = true;
            did_work = true;
        } else if (user_sdo_ready) {
            const SdoRequest request = pending_user_sdo_.front();
            pending_user_sdo_.pop_front();
            handle_request(request, result_seq);
            prefer_user_sdo = false;
            did_work = true;
        }
        if (stop_flag_) break;

        if (!did_work) usleep(SDO_POLL_SLEEP_US);
    }
}

}  // namespace zmc
