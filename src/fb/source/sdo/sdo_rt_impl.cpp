/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-21
@ArtifactStatusAuthority GLM-5.2 (opencode)
*/
#include "sdo_rt_impl.hpp"

namespace zmc {
namespace detail {

SdoRtImpl::SdoRtImpl() : worker_(), manager_(worker_.frontend()) {}

int SdoRtImpl::bind_adapter(const SdoCAdapter *adapter) { return worker_.bind_adapter(adapter); }

void SdoRtImpl::clear_adapter() { worker_.clear_adapter(); }

bool SdoRtImpl::take_oneshot_results(SdoResult *out) {
    return manager_.take_oneshot_results(out);
}

void SdoRtImpl::pump_results() { manager_.pump_results(); }

void SdoRtImpl::set_pump_divider(uint32_t n) { manager_.set_pump_divider(n); }

int SdoRtImpl::get_u32(const SdoKey &key, uint32_t *out, uint32_t *seq) const {
    return manager_.get_u32(key, out, seq);
}

int SdoRtImpl::sdo_read_once(const SdoKey &key, uint32_t max_len, uint32_t request_id) {
    return manager_.sdo_read_once(key, max_len, request_id);
}

int SdoRtImpl::sdo_write(const SdoKey &key, uint32_t len, const void *data,
                         uint32_t request_id) {
    return manager_.sdo_write(key, len, data, request_id);
}

bool SdoRtImpl::is_download_watch_key(const SdoKey &key) const {
    return manager_.is_download_watch_key(key);
}

int SdoRtImpl::add_watch_upload(const SdoKey &key, uint32_t period_ms, uint32_t max_len) {
    return manager_.add_watch_upload(key, period_ms, max_len);
}

int SdoRtImpl::add_watch_download(const SdoKey &key, uint32_t period_ms, uint32_t len,
                                  const void *data) {
    return manager_.add_watch_download(key, period_ms, len, data);
}

int SdoRtImpl::update_watch_write_value(const SdoKey &key, int32_t new_value,
                                        uint32_t value_size) {
    return manager_.update_watch_write_value(key, new_value, value_size);
}

int SdoRtImpl::add_watch_download_with_value(const SdoKey &key, uint32_t period_ms,
                                             int32_t initial_value, uint32_t value_size) {
    return manager_.add_watch_download_with_value(key, period_ms, initial_value, value_size);
}

int SdoRtImpl::remove_watch(const SdoKey &key) { return manager_.remove_watch(key); }

void SdoRtImpl::clear_watches() { manager_.clear_watches(); }

int SdoRtImpl::set_direction_params(uint8_t data_direction, const SdoDirectionParams &params) {
    return manager_.set_direction_params(data_direction, params);
}

int SdoRtImpl::get_direction_params(uint8_t data_direction, SdoDirectionParams *out,
                                    uint32_t *seq) const {
    return manager_.get_direction_params(data_direction, out, seq);
}

int SdoRtImpl::query_direction_params(uint8_t data_direction) {
    return manager_.query_direction_params(data_direction);
}

int SdoRtImpl::query_watch_count() { return manager_.query_watch_count(); }

int SdoRtImpl::get_queried_watch_count(SdoWatchCounts *out, uint32_t *seq) const {
    return manager_.get_queried_watch_count(out, seq);
}

uint32_t SdoRtImpl::last_seq() const { return manager_.last_seq(); }

void SdoRtImpl::run_blocking() { worker_.run_blocking(); }

void SdoRtImpl::request_stop() {
    worker_.request_stop();
    (void)manager_.request_exit();
}

void SdoRtImpl::pause() { worker_.pause(); }

void SdoRtImpl::resume() { worker_.resume(); }

void SdoRtImpl::reset_after_join() {
    worker_.reset_after_join();
}

}  // namespace detail
}  // namespace zmc
