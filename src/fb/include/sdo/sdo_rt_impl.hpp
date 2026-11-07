/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-21
@ArtifactStatusAuthority GLM-5.2 (opencode)
*/
#ifndef _ZMC_SDO_RT_IMPL_HPP_
#define _ZMC_SDO_RT_IMPL_HPP_

#include "SdoRtManager.hpp"
#include "SdoWorker.hpp"
#include "sdo_rt.hpp"

namespace zmc {
namespace detail {

// Compose the frontend Manager and backend Worker without owning business
// logic.
class SdoRtImpl {
   public:
    SdoRtImpl();
    ~SdoRtImpl() = default;

    // Forward adapter binding to the Worker.
    int bind_adapter(const SdoCAdapter *adapter);
    // Clear Worker adapter state.
    void clear_adapter();
    // Take completed Axis CoE results from Manager one-shot staging.
    bool take_oneshot_results(SdoResult *out);
    // Forward result pumping to Manager.
    void pump_results();
    // Forward pump-divider configuration to Manager.
    void set_pump_divider(uint32_t n);
    // Forward RT cache access to Manager.
    int get_u32(const SdoKey &key, uint32_t *out, uint32_t *seq = nullptr) const;
    // Forward one-shot upload generation to Manager.
    int sdo_read_once(const SdoKey &key, uint32_t max_len, uint32_t request_id = 0);
    // Forward one-shot download generation to Manager.
    int sdo_write(const SdoKey &key, uint32_t len, const void *data,
                  uint32_t request_id = 0);
    // Read frontend download-watch ownership for one key.
    bool is_download_watch_key(const SdoKey &key) const;
    // Forward upload-watch generation to Manager.
    int add_watch_upload(const SdoKey &key, uint32_t period_ms, uint32_t max_len);
    // Forward download-watch generation to Manager.
    int add_watch_download(const SdoKey &key, uint32_t period_ms, uint32_t len, const void *data);
    // Forward watched-download updates to Manager.
    int update_watch_write_value(const SdoKey &key, int32_t new_value, uint32_t value_size);
    // Forward scalar download-watch generation to Manager.
    int add_watch_download_with_value(const SdoKey &key, uint32_t period_ms,
                                      int32_t initial_value, uint32_t value_size);
    // Forward watch removal to Manager.
    int remove_watch(const SdoKey &key);
    // Forward watch clearing to Manager.
    void clear_watches();
    // Forward params generation to Manager.
    int set_direction_params(uint8_t data_direction, const SdoDirectionParams &params);
    // Forward unified params-snapshot access to Manager.
    int get_direction_params(uint8_t data_direction, SdoDirectionParams *out,
                             uint32_t *seq = nullptr) const;
    // Forward active-params queries to Manager.
    int query_direction_params(uint8_t data_direction);
    // Forward watch-count queries to Manager.
    int query_watch_count();
    // Forward queried-watch-count access to Manager.
    int get_queried_watch_count(SdoWatchCounts *out, uint32_t *seq = nullptr) const;
    // Forward last-sequence access to Manager.
    uint32_t last_seq() const;
    // Enter the Worker loop in the current thread.
    void run_blocking();
    // Set the Worker stop flag and enqueue the Manager Exit request.
    void request_stop();
    // Forward pause to Worker.
    void pause();
    // Forward resume to Worker.
    void resume();
    // Reset Worker-owned runtime state after thread join.
    void reset_after_join();

    SdoRtImpl(const SdoRtImpl &) = delete;
    SdoRtImpl &operator=(const SdoRtImpl &) = delete;

   private:
    SdoWorker worker_;
    SdoRtManager manager_;
};

}  // namespace detail
}  // namespace zmc

#endif  // _ZMC_SDO_RT_IMPL_HPP_
