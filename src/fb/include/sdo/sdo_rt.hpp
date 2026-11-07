/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-21
@ArtifactStatusAuthority
 * GLM-5.2 (opencode)
 */
#ifndef _ZMC_SDO_RT_HPP_
#define _ZMC_SDO_RT_HPP_

#include <cstdint>

#include "SdoMessage.hpp"
#include "sdo_adapter.h"
#include "sdo_config.hpp"
#include "sdo_key.hpp"

namespace zmc {

class SdoRtThread;
class AxisFbBridgeWithThread;

namespace detail {
class SdoRtImpl;
}

class SdoRt {
   public:
    // Status-returning methods use zero for success and preserve SDO or adapter errors.
    SdoRt();
    ~SdoRt();

    // Bind the low-level SDO adapter used by the worker thread.
    int bind_adapter(const SdoCAdapter *adapter);
    // Clear the adapter binding and reset worker-side caches.
    void clear_adapter();
    // Copy completed Axis CoE results from Manager one-shot staging.
    bool take_oneshot_results(SdoResult *out);
    // Drain worker results into the RT-side cache at the configured divider.
    void pump_results();
    // Set how often pump_results drains the result queue; zero resets to default.
    void set_pump_divider(uint32_t n);
    // Read the last SDO result as a zero-extended u32 or return its error code.
    int get_u32(const SdoKey &key, uint32_t *out, uint32_t *seq = nullptr) const;
    // Queue one upload request. A nonzero request id is reserved for an
    // Axis CoE read and is preserved for direct result routing.
    int sdo_read_once(const SdoKey &key, uint32_t max_len, uint32_t request_id = 0);
    // Queue one download request. A nonzero request id is reserved for an
    // Axis CoE write and is preserved for direct result routing.
    int sdo_write(const SdoKey &key, uint32_t len, const void *data,
                  uint32_t request_id = 0);
    // Report whether one key is owned by a registered download watch.
    bool is_download_watch_key(const SdoKey &key) const;
    // Register a periodic upload watch; period_ms is API-compat only (use
    // set_direction_params).
    int add_watch_upload(const SdoKey &key, uint32_t period_ms, uint32_t max_len);
    // Register a periodic download watch; period_ms is API-compat only (use
    // set_direction_params).
    int add_watch_download(const SdoKey &key, uint32_t period_ms, uint32_t len, const void *data);
    // Update a watched download value and enqueue the new value for the worker.
    int update_watch_write_value(const SdoKey &key, int32_t new_value, uint32_t value_size);
    // Register a download watch from a scalar value with an explicit byte size.
    int add_watch_download_with_value(const SdoKey &key, uint32_t period_ms,
                                      int32_t initial_value, uint32_t value_size);
    // Remove any upload or download watch for this key.
    int remove_watch(const SdoKey &key);
    // Remove all watches from the worker-managed watch list.
    void clear_watches();
    // Enqueue direction params update (SDO_DIR_UPLOAD or SDO_DIR_DOWNLOAD only).
    int set_direction_params(uint8_t data_direction, const SdoDirectionParams &params);
    // Read the unified direction-params snapshot (direct read, no queue);
    // seq stays 0 until pump_results consumed a successful SET or GET result.
    int get_direction_params(uint8_t data_direction, SdoDirectionParams *out,
                             uint32_t *seq = nullptr) const;
    // Enqueue GET_*_PARAMS query; its result refreshes the unified snapshot
    // via pump_results.
    int query_direction_params(uint8_t data_direction);
    // Enqueue GET_WATCH_COUNT query; result returns via ResultQueue and
    // pump_results.
    int query_watch_count();
    // Read watch-count snapshot (upload + download); returns ZMC_ERR_SDO_NO_RESULT until queried.
    int get_queried_watch_count(SdoWatchCounts *out, uint32_t *seq = nullptr) const;
    // Return the latest result sequence observed by pump_results.
    uint32_t last_seq() const;

    // Run the SDO worker loop in the current thread.
    void run_blocking();
    // Ask the worker loop to exit by setting the flag and queuing an Exit
    // request.
    void request_stop();
    // Temporarily stop queue and watch processing in the worker loop.
    void pause();
    // Resume queue and watch processing after pause().
    void resume();
    bool sdo_started() const { return sdo_started_; }

    SdoRt(const SdoRt &) = delete;
    SdoRt &operator=(const SdoRt &) = delete;

   private:
    friend class SdoRtThread;
    friend class AxisFbBridgeWithThread;

    // Clear queues after the worker thread has been joined.
    void clear_runtime_queues_after_join();
    // Hidden implementation that composes the frontend manager and backend
    // worker.
    detail::SdoRtImpl *impl_;
    // Opaque native thread handle managed by SdoRtThread helpers.
    void *thread_;
    // Tracks whether a worker thread is currently active for this instance.
    bool sdo_started_ = false;
};

}  // namespace zmc

#endif  // _ZMC_SDO_RT_HPP_
