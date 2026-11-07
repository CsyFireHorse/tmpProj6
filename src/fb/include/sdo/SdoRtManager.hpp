/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-21
@ArtifactStatusAuthority GLM-5.2 (opencode)
*/
#ifndef _ZMC_SDO_RT_MANAGER_HPP_
#define _ZMC_SDO_RT_MANAGER_HPP_

#include <cstdint>

#include "SdoChannel.hpp"
#include "common/zmc/zmc_predef.h"

namespace zmc {

// Generate frontend requests and consume worker results into RT-side caches.
class SdoRtManager {
   public:
    // Status-returning methods use zero for success and preserve SDO or adapter errors.
    // Bind Manager to the restricted worker-owned channel frontend.
    explicit SdoRtManager(SdoChannel::Frontend frontend);
    ~SdoRtManager() = default;

    // Register a scalar download watch.
    int add_watch_download_with_value(const SdoKey &key, uint32_t period_ms,
                                      int32_t initial_value, uint32_t value_size);
    // Queue a new value for an existing download watch.
    int update_watch_write_value(const SdoKey &key, int32_t new_value, uint32_t value_size);
    // Copy completed Axis CoE results from the one-shot cache.
    bool take_oneshot_results(SdoResult *out);
    // Consume worker results according to the pump divider.
    void pump_results();
    // Configure the existing result-pump divider.
    void set_pump_divider(uint32_t n);
    // Read one RT-cached SDO value or return its cached transfer error.
    int get_u32(const SdoKey &key, uint32_t *out, uint32_t *seq = nullptr) const;
    // Queue one upload request. A nonzero request id is preserved for direct
    // Axis CoE result routing.
    int sdo_read_once(const SdoKey &key, uint32_t max_len, uint32_t request_id = 0);
    // Queue one download request; zero assigns a normal Manager request id,
    // while an Axis CoE request id is preserved for direct result routing.
    int sdo_write(const SdoKey &key, uint32_t len, const void *data,
                  uint32_t request_id = 0);
    // Report whether one key is owned by a registered download watch.
    bool is_download_watch_key(const SdoKey &key) const;
    // Queue one upload-watch command.
    int add_watch_upload(const SdoKey &key, uint32_t period_ms, uint32_t max_len);
    // Queue one download-watch command.
    int add_watch_download(const SdoKey &key, uint32_t period_ms, uint32_t len, const void *data);
    // Queue a direction-params update.
    int set_direction_params(uint8_t data_direction, const SdoDirectionParams &params);
    // Read the unified result-confirmed params snapshot; seq stays 0 until a
    // successful SET or GET result has been pumped.
    int get_direction_params(uint8_t data_direction, SdoDirectionParams *out,
                             uint32_t *seq = nullptr) const;
    // Queue a query for the Worker active params.
    int query_direction_params(uint8_t data_direction);
    // Queue a worker watch-count query.
    int query_watch_count();
    // Read a returned worker watch-count result.
    int get_queried_watch_count(SdoWatchCounts *out, uint32_t *seq = nullptr) const;
    // Queue removal of one upload or download watch.
    int remove_watch(const SdoKey &key);
    // Queue removal of all watches and clear frontend watch bookkeeping.
    void clear_watches();
    // Queue the lifecycle Exit request.
    int request_exit();
    // Return the last consumed worker result sequence.
    uint32_t last_seq() const noexcept { return last_seq_; }

    SdoRtManager(const SdoRtManager &) = delete;
    SdoRtManager &operator=(const SdoRtManager &) = delete;

   private:
    // Store one fixed-capacity RT cache entry.
    struct DictEntry {
        bool used{false};
        SdoKey key{};
        SdoValue value{};
    };

    // Track frontend ownership of one download-watch key.
    struct KeyEntry {
        bool used{false};
        SdoKey key{};
    };

    // Assign or preserve a request id and push one nonempty batch.
    int enqueue_request(SdoRequest &request, uint32_t request_id = 0);
    // Build and enqueue one single-item command.
    int enqueue_command(uint8_t direction, const SdoKey &key, uint8_t len, uint32_t data = 0,
                        uint32_t request_id = 0);
    // Flush the existing pending download batch.
    int flush_pending_write();
    // Append one value to the pending download batch.
    int append_pending_write(const SdoKey &key, uint8_t len, uint32_t data);
    // Route one transfer result into the RT dictionary.
    void update_dict(const SdoResultItem &item);
    // Commit one successful SET-echo or GET-params result into the unified
    // Manager params cache.
    void update_direction_params(const SdoResultItem &item);
    // Route one watch-count result into its query snapshot.
    void update_queried_watch_count(const SdoResultItem &item);
    // Find one RT dictionary value.
    const SdoValue *find_dict(const SdoKey &key) const;
    // Record one frontend download-watch key.
    int add_download_watch_key(const SdoKey &key);
    // Test frontend ownership of one download-watch key.
    bool has_download_watch_key(const SdoKey &key) const;
    // Remove one frontend download-watch key.
    void remove_download_watch_key(const SdoKey &key);
    // Clear all frontend download-watch keys.
    void clear_download_watch_keys();

    SdoChannel::Frontend frontend_;
    DictEntry dict_[SDO_FIXED_CACHE_CAPACITY]{};
    KeyEntry download_watch_keys_[SDO_FIXED_CACHE_CAPACITY]{};
    SdoRequest pending_req_{};
    // Completed Axis CoE results waiting for bridge dispatch.
    SdoResult oneshot_staging_{};
    uint32_t request_id_{0};
    uint32_t last_seq_{0};
    uint32_t pump_divider_{SDO_DEFAULT_PUMP_DIVIDER};
    uint32_t pump_tick_{0};
    // Unified worker-active params snapshot per data direction; seq stays 0
    // until the first successful SET or GET result is pumped.
    SdoDirectionParams direction_params_[2];
    uint32_t direction_params_seq_[2]{0, 0};
    SdoWatchCounts queried_watch_count_{};
    uint32_t queried_watch_count_seq_{0};
};

}  // namespace zmc

#endif  // _ZMC_SDO_RT_MANAGER_HPP_
