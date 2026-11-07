/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-17
@ArtifactStatusAuthority GPT-5 Codex
*/
#ifndef _ZMC_SDO_WORKER_HPP_
#define _ZMC_SDO_WORKER_HPP_

#include <cstddef>
#include <cstdint>
#include <deque>
#include <vector>

#include "SdoChannel.hpp"
#include "sdo_adapter.h"

namespace zmc {

// Execute all non-RT SDO requests and periodic watches.
class SdoWorker {
   public:
    SdoWorker();
    ~SdoWorker() = default;

    // Return the restricted frontend capability for Manager construction.
    SdoChannel::Frontend frontend();
    // Copy the adapter callbacks into worker-owned storage.
    int bind_adapter(const SdoCAdapter *adapter);
    // Clear adapter callbacks and worker caches.
    void clear_adapter();
    // Run the worker loop in the current thread.
    void run_blocking();
    // Set the worker stop flag without accessing Manager state.
    void request_stop();
    // Suspend request and watch processing.
    void pause();
    // Resume request and watch processing.
    void resume();
    // Reset worker-owned pending requests and channel lifecycle state after join.
    void reset_after_join();

    SdoWorker(const SdoWorker &) = delete;
    SdoWorker &operator=(const SdoWorker &) = delete;

   private:
    // Store the last successful worker-side download.
    struct WorkerWriteCacheEntry {
        bool used{false};
        SdoKey key{};
        uint8_t len{0};
        uint8_t consecutive_skip_count{0};
        uint32_t data{0};
    };

    // Store one worker-owned periodic SDO operation.
    struct SdoWatchItem {
        SdoRequestItem item{};
        uint32_t last_tick_ms{0};
    };

    // Drain incoming commands and alternate user SDOs with due watch transfers.
    void run();
    // Dispatch controls immediately, queue user SDOs, and report EXIT.
    bool dispatch_request(const SdoRequest &request, uint32_t &result_seq);
    // Execute one queued user SDO request.
    void handle_request(const SdoRequest &request, uint32_t &result_seq);
    // Drain available channel requests through dispatch_request.
    bool scan_requests(uint32_t &result_seq, bool &did_work);
    // Apply one watch-list command to worker-owned state.
    int apply_watch_command(const SdoRequestItem &request);
    // Add or replace one worker-owned SDO watch.
    int add_or_update_watch(const SdoRequestItem &request);
    // Apply validated params to the worker active snapshot.
    int apply_direction_params(uint8_t data_direction, const SdoDirectionParams &params);
    // Return one worker active direction snapshot.
    const SdoDirectionParams &direction_params(uint8_t data_direction) const;
    // Count worker-owned watches for one data direction.
    uint32_t watch_count_for(uint8_t data_direction) const;
    // Update the payload of a matching worker-owned download watch.
    void update_download_watch_data(const SdoKey &key, uint8_t len, uint32_t data);
    // Execute one currently due SDO watch.
    bool handle_one_due_watch(uint32_t now_ms, uint32_t &result_seq);
    // Execute one due SDO watch.
    bool handle_watch(SdoWatchItem &watch, uint32_t now_ms, uint32_t &result_seq,
                      SdoResult &results);
    // Clear the adapter-dependent worker write cache.
    void clear_write_cache();
    // Skip one matching watch download while the bounded allowance remains.
    bool try_skip_cached_download(const SdoKey &key, uint8_t len, uint32_t data);
    // Invalidate one cached value before a real adapter download.
    void invalidate_write_cache(const SdoKey &key, uint8_t len);
    // Update one worker-side download cache entry.
    void update_write_cache(const SdoKey &key, uint8_t len, uint32_t data);
    // Append a synthetic success for a cached download.
    int append_cached_download_result(const SdoKey &key, uint8_t len, uint32_t data,
                                      uint32_t id, uint32_t &result_seq, SdoResult &results);
    // Check that the SDO target slave is present and operational.
    int32_t check_slave_state(uint16_t slave_id) const;
    // Execute one adapter upload and append its result.
    int append_upload_result(const SdoKey &key, uint8_t len, uint32_t id,
                             uint32_t &result_seq, SdoResult &results);
    // Execute or skip one download according to the caller's cache policy.
    int append_download_result(const SdoKey &key, uint8_t len, uint32_t data,
                               uint32_t id, uint32_t &result_seq, SdoResult &results,
                               bool allow_cache_skip);
    // Append one non-transfer command result.
    int append_command_result(const SdoRequestItem &request, uint32_t id,
                              int32_t result_code, uint32_t &result_seq, SdoResult &results,
                              uint8_t data_len = 0, uint32_t data = 0);

    SdoChannel channel_{};
    WorkerWriteCacheEntry write_cache_[SDO_FIXED_CACHE_CAPACITY]{};
    std::vector<SdoWatchItem> watch_list_;
    // Start each watch scan after the last selected item.
    std::size_t next_watch_index_{0};
    std::deque<SdoRequest> pending_user_sdo_;
    SdoDirectionParams active_params_[2];
    volatile bool stop_flag_{false};
    volatile bool paused_{false};
    SdoCAdapter adapter_{};
};

}  // namespace zmc

#endif  // _ZMC_SDO_WORKER_HPP_
