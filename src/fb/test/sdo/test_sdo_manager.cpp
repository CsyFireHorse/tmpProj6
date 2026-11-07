/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-17
@ArtifactStatusAuthority GPT-5 Codex
*/
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "common/zmc/fbt_errorcode_def.h"
#include "SdoRtManager.hpp"
#include "SdoWorker.hpp"
#include "sdo_test_helper.hpp"

using fb_test::sdo::CountingAdapterCtx;
using fb_test::sdo::FakeSdoStore;
using fb_test::sdo::make_counting_adapter;
using fb_test::sdo::make_key;
using fb_test::sdo::make_store_adapter;
using fb_test::sdo::wait_for_byte;

namespace {

// Run the real Worker/Manager boundary without exposing channel backend
// operations.
class SdoManagerRuntime {
public:
  SdoManagerRuntime() : manager_(worker_.frontend()) {
    manager_.set_pump_divider(1);
  }

  ~SdoManagerRuntime() { stop(); }

  void start(const SdoCAdapter &adapter) {
    worker_.bind_adapter(&adapter);
    thread_ = std::thread([this]() { worker_.run_blocking(); });
  }

  void stop() {
    if (!thread_.joinable())
      return;
    worker_.request_stop();
    (void)manager_.request_exit();
    thread_.join();
    worker_.reset_after_join();
  }

  bool wait_for_u32(const zmc::SdoKey &key, uint32_t *out,
                    int max_retries = 100) {
    for (int i = 0; i < max_retries; ++i) {
      manager_.pump_results();
      if (manager_.get_u32(key, out) == 0)
        return true;
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return false;
  }

  bool wait_for_watch_count(uint16_t uploads, uint16_t downloads,
                            int max_retries = 100) {
    for (int i = 0; i < max_retries; ++i) {
      manager_.pump_results();
      zmc::SdoWatchCounts counts{};
      if (manager_.get_queried_watch_count(&counts) == 0 &&
          counts.upload_count == uploads &&
          counts.download_count == downloads) {
        return true;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return false;
  }

  zmc::SdoWorker worker_{};
  zmc::SdoRtManager manager_;
  std::thread thread_;
};

} // namespace

TEST(SdoRtManager, FrontendValidationDoesNotNeedWorker) {
  zmc::SdoWorker worker;
  zmc::SdoRtManager manager(worker.frontend());
  const zmc::SdoKey key = make_key(1, 0x6060, 0);

  EXPECT_EQ(ZMC_ERR_SDO_INVALID_LENGTH, manager.sdo_read_once(key, 0));
  EXPECT_EQ(ZMC_ERR_SDO_INVALID_LENGTH, manager.sdo_read_once(key, 5));
  EXPECT_EQ(ZMC_ERR_SDO_INVALID_ARG, manager.sdo_write(key, 1, nullptr));
  EXPECT_EQ(ZMC_ERR_SDO_WATCH_NOT_FOUND, manager.update_watch_write_value(key, 8, 1));
  EXPECT_EQ(ZMC_ERR_SDO_INVALID_ARG, manager.get_u32(key, nullptr));
  uint32_t value = 0;
  EXPECT_EQ(ZMC_ERR_SDO_NO_RESULT, manager.get_u32(key, &value));
  zmc::SdoWatchCounts counts{};
  EXPECT_EQ(ZMC_ERR_SDO_NO_RESULT, manager.get_queried_watch_count(&counts));

  zmc::SdoDirectionParams params{};
  ASSERT_EQ(0, manager.get_direction_params(zmc::SDO_DIR_UPLOAD, &params));
  EXPECT_EQ(zmc::SDO_DEFAULT_TIMEOUT_MS, params.timeout_ms);
  EXPECT_EQ(zmc::SDO_DEFAULT_UPLOAD_WATCH_PERIOD_MS, params.watch_period_ms);
}

TEST(SdoRtManager, ReportsRequestQueueFull) {
  zmc::SdoWorker worker;
  zmc::SdoRtManager manager(worker.frontend());
  const zmc::SdoKey key = make_key(1, 0x6060, 0);

  int result = 0;
  for (uint32_t i = 0; i < 128u && result == 0; ++i) {
    result = manager.sdo_read_once(key, 1);
  }
  EXPECT_EQ(ZMC_ERR_SDO_QUEUE_FULL, result);
}

TEST(SdoRtManager, ReportsDownloadWatchCapacity) {
  FakeSdoStore store;
  const SdoCAdapter adapter = make_store_adapter(&store);
  SdoManagerRuntime runtime;
  runtime.start(adapter);

  const uint8_t value = 0x08u;
  uint32_t last_seq = runtime.manager_.last_seq();
  for (uint32_t i = 0; i < zmc::SDO_FIXED_CACHE_CAPACITY; ++i) {
    const zmc::SdoKey key = make_key(1, static_cast<uint16_t>(0x2000u + i), 0);
    ASSERT_EQ(0, runtime.manager_.add_watch_download(key, 0, 1, &value));
    for (int retry = 0; retry < 100 && runtime.manager_.last_seq() == last_seq; ++retry) {
      runtime.manager_.pump_results();
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    ASSERT_GT(runtime.manager_.last_seq(), last_seq);
    last_seq = runtime.manager_.last_seq();
  }

  const zmc::SdoKey extra_key = make_key(1, 0x3000u, 0);
  EXPECT_EQ(ZMC_ERR_SDO_CAPACITY,
            runtime.manager_.add_watch_download(extra_key, 0, 1, &value));
}

TEST(SdoRtManager, WorkerRoundTripUpdatesRtDictionary) {
  FakeSdoStore store;
  const zmc::SdoKey key = make_key(1, 0x607A, 0);
  store.set_u32(key, 0x12345678u);
  const SdoCAdapter adapter = make_store_adapter(&store);

  SdoManagerRuntime runtime;
  runtime.start(adapter);
  ASSERT_EQ(0, runtime.manager_.sdo_read_once(key, 4));

  uint32_t value = 0;
  ASSERT_TRUE(runtime.wait_for_u32(key, &value));
  EXPECT_EQ(0x12345678u, value);
  EXPECT_GT(runtime.manager_.last_seq(), 0u);
}

TEST(SdoRtManager, PumpDividerDelaysResultConsumption) {
  FakeSdoStore store;
  const zmc::SdoKey key = make_key(1, 0x607A, 0);
  store.set_u32(key, 0xAABBCCDDu);
  const SdoCAdapter adapter = make_store_adapter(&store);

  SdoManagerRuntime runtime;
  runtime.manager_.set_pump_divider(3);
  runtime.start(adapter);
  ASSERT_EQ(0, runtime.manager_.sdo_read_once(key, 4));
  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  uint32_t value = 0;
  runtime.manager_.pump_results();
  runtime.manager_.pump_results();
  EXPECT_EQ(ZMC_ERR_SDO_NO_RESULT, runtime.manager_.get_u32(key, &value));
  runtime.manager_.pump_results();
  ASSERT_EQ(0, runtime.manager_.get_u32(key, &value));
  EXPECT_EQ(0xAABBCCDDu, value);
}

TEST(SdoRtManager, ParamsCacheCommitsOnlyAfterSuccessfulResultPump) {
  CountingAdapterCtx counter{};
  const zmc::SdoKey key = make_key(1, 0x6061, 0);
  counter.store.set_u32(key, 0x11u);
  const SdoCAdapter adapter = make_counting_adapter(&counter);

  SdoManagerRuntime runtime;
  runtime.start(adapter);

  zmc::SdoDirectionParams configured{};
  configured.timeout_ms = 222u;
  configured.watch_period_ms = 33u;
  ASSERT_EQ(0,
      runtime.manager_.set_direction_params(zmc::SDO_DIR_UPLOAD, configured));
  ASSERT_EQ(0, runtime.manager_.sdo_read_once(key, 1));

  for (int i = 0; i < 100 && counter.upload_count == 0; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  ASSERT_GT(counter.upload_count, 0u);
  EXPECT_EQ(222u, counter.last_upload_timeout_ms);

  zmc::SdoDirectionParams cached{};
  ASSERT_EQ(0,
      runtime.manager_.get_direction_params(zmc::SDO_DIR_UPLOAD, &cached));
  EXPECT_EQ(zmc::SDO_DEFAULT_TIMEOUT_MS, cached.timeout_ms);
  EXPECT_EQ(zmc::SDO_DEFAULT_UPLOAD_WATCH_PERIOD_MS, cached.watch_period_ms);

  runtime.manager_.pump_results();
  ASSERT_EQ(0,
      runtime.manager_.get_direction_params(zmc::SDO_DIR_UPLOAD, &cached));
  EXPECT_EQ(configured.timeout_ms, cached.timeout_ms);
  EXPECT_EQ(configured.watch_period_ms, cached.watch_period_ms);
}

TEST(SdoRtManager, QueryWatchCountObservesWorkerOwnedWatches) {
  FakeSdoStore store;
  const SdoCAdapter adapter = make_store_adapter(&store);
  SdoManagerRuntime runtime;
  runtime.start(adapter);

  const zmc::SdoKey upload_key = make_key(1, 0x6061, 0);
  const zmc::SdoKey download_key = make_key(1, 0x6060, 0);
  const uint8_t mode = 0x08u;
  ASSERT_EQ(0, runtime.manager_.add_watch_upload(upload_key, 0, 1));
  ASSERT_EQ(0, runtime.manager_.add_watch_download(download_key, 0, 1, &mode));
  ASSERT_EQ(0, runtime.manager_.query_watch_count());

  EXPECT_TRUE(runtime.wait_for_watch_count(1, 1));
}

TEST(SdoRtManager, PendingDownloadUpdateFlowsThroughWorker) {
  FakeSdoStore store;
  const SdoCAdapter adapter = make_store_adapter(&store);
  SdoManagerRuntime runtime;
  runtime.start(adapter);

  const zmc::SdoKey key = make_key(1, 0x6060, 0);
  const uint8_t initial_mode = 0x08u;
  zmc::SdoDirectionParams params{};
  params.watch_period_ms = 20u;
  ASSERT_EQ(0, runtime.manager_.set_direction_params(zmc::SDO_DIR_DOWNLOAD, params));
  ASSERT_EQ(0, runtime.manager_.add_watch_download(key, 0, 1, &initial_mode));
  ASSERT_TRUE(wait_for_byte(store, key, initial_mode));

  ASSERT_EQ(0, runtime.manager_.update_watch_write_value(key, 0x123409, 1));
  runtime.manager_.pump_results();
  EXPECT_TRUE(wait_for_byte(store, key, 0x09u));
}

TEST(SdoRtManager, ClearWatchesClearsFrontendDownloadOwnership) {
  zmc::SdoWorker worker;
  zmc::SdoRtManager manager(worker.frontend());
  const zmc::SdoKey key = make_key(1, 0x6060, 0);
  const uint8_t initial_mode = 0x08u;

  ASSERT_EQ(0, manager.add_watch_download(key, 0, 1, &initial_mode));
  manager.clear_watches();
  EXPECT_EQ(ZMC_ERR_SDO_WATCH_NOT_FOUND, manager.update_watch_write_value(key, 0x09, 1));
}
