/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-21
@ArtifactStatusAuthority
 * GLM-5.2 (opencode)
*/
#include "sdo_rt.hpp"

#include "sdo_rt_impl.hpp"

namespace zmc {

SdoRt::SdoRt() : impl_(new detail::SdoRtImpl()), thread_(nullptr) {}

SdoRt::~SdoRt() { delete impl_; }

int SdoRt::bind_adapter(const SdoCAdapter *adapter) { return impl_->bind_adapter(adapter); }

void SdoRt::clear_adapter() { impl_->clear_adapter(); }

bool SdoRt::take_oneshot_results(SdoResult *out) { return impl_->take_oneshot_results(out); }

void SdoRt::pump_results() { impl_->pump_results(); }

void SdoRt::set_pump_divider(uint32_t n) { impl_->set_pump_divider(n); }

int SdoRt::get_u32(const SdoKey &key, uint32_t *out, uint32_t *seq) const {
    return impl_->get_u32(key, out, seq);
}

int SdoRt::sdo_read_once(const SdoKey &key, uint32_t max_len, uint32_t request_id) {
    return impl_->sdo_read_once(key, max_len, request_id);
}

int SdoRt::sdo_write(const SdoKey &key, uint32_t len, const void *data, uint32_t request_id) {
    return impl_->sdo_write(key, len, data, request_id);
}

bool SdoRt::is_download_watch_key(const SdoKey &key) const {
    return impl_->is_download_watch_key(key);
}

int SdoRt::add_watch_upload(const SdoKey &key, uint32_t period_ms, uint32_t max_len) {
    return impl_->add_watch_upload(key, period_ms, max_len);
}

int SdoRt::add_watch_download(const SdoKey &key, uint32_t period_ms, uint32_t len,
                              const void *data) {
    return impl_->add_watch_download(key, period_ms, len, data);
}

int SdoRt::update_watch_write_value(const SdoKey &key, int32_t new_value, uint32_t value_size) {
    return impl_->update_watch_write_value(key, new_value, value_size);
}

int SdoRt::add_watch_download_with_value(const SdoKey &key, uint32_t period_ms,
                                         int32_t initial_value, uint32_t value_size) {
    return impl_->add_watch_download_with_value(key, period_ms, initial_value, value_size);
}

int SdoRt::remove_watch(const SdoKey &key) { return impl_->remove_watch(key); }

void SdoRt::clear_watches() { impl_->clear_watches(); }

int SdoRt::set_direction_params(uint8_t data_direction, const SdoDirectionParams &params) {
    return impl_->set_direction_params(data_direction, params);
}

int SdoRt::get_direction_params(uint8_t data_direction, SdoDirectionParams *out,
                                uint32_t *seq) const {
    return impl_->get_direction_params(data_direction, out, seq);
}

int SdoRt::query_direction_params(uint8_t data_direction) {
    return impl_->query_direction_params(data_direction);
}

int SdoRt::query_watch_count() { return impl_->query_watch_count(); }

int SdoRt::get_queried_watch_count(SdoWatchCounts *out, uint32_t *seq) const {
    return impl_->get_queried_watch_count(out, seq);
}

uint32_t SdoRt::last_seq() const { return impl_->last_seq(); }

void SdoRt::run_blocking() { impl_->run_blocking(); }

void SdoRt::request_stop() { impl_->request_stop(); }

void SdoRt::pause() { impl_->pause(); }

void SdoRt::resume() { impl_->resume(); }

void SdoRt::clear_runtime_queues_after_join() { impl_->reset_after_join(); }

}  // namespace zmc
