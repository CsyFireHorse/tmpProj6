/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-17
@ArtifactStatusAuthority
 * GPT-5 Codex
*/
#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <thread>

#include "common/zmc/fbt_errorcode_def.h"
#include "common/zmc/zmc_predef.h"
#include "sdo_rt_thread.hpp"
#include "sdo_test_helper.hpp"

using fb_test::sdo::CountingAdapterCtx;
using fb_test::sdo::FakeSdoStore;
using fb_test::sdo::make_counting_adapter;
using fb_test::sdo::make_key;
using fb_test::sdo::make_store_adapter;
using fb_test::sdo::wait_for_byte;
using fb_test::sdo::wait_for_download_count;
using fb_test::sdo::wait_for_u32;

namespace {

int32_t upload_specific_error(void* ctx, const SdoCKey* key, uint32_t* len, void* data,
                              uint32_t timeout_ms) {
  (void)ctx;
  (void)key;
  (void)len;
  (void)data;
  (void)timeout_ms;
  return -1234;
}

class SdoRtThreadFixture : public ::testing::Test {
protected:
  void SetUp() override {
    store_.set_u32(make_key(0x6061), 0x12345678u);
    adapter_ = make_store_adapter(&store_);
  }

  void StartRt() {
    rt_.bind_adapter(&adapter_);
    rt_.set_pump_divider(1);
    rt_.start();
  }

  FakeSdoStore store_{};
  SdoCAdapter adapter_{};
  zmc::SdoRtThread rt_{};
};

} // namespace

TEST_F(SdoRtThreadFixture, OneShotUpload) {
  StartRt();
  const zmc::SdoKey key = make_key(0x6061);
  ASSERT_EQ(0, rt_.sdo_read_once(key, 1));

  uint32_t out = 0;
  EXPECT_TRUE(wait_for_u32(rt_, key, &out));
  EXPECT_EQ(0x78u, out);
  rt_.stop();
}

TEST(SdoRt, PreservesAdapterErrorInCachedResult) {
  SdoCAdapter adapter = {};
  adapter.upload = upload_specific_error;
  adapter.download = fb_test::sdo::store_download;
  adapter.get_slave_state = fb_test::sdo::operational_slave_state;

  zmc::SdoRtThread rt;
  rt.bind_adapter(&adapter);
  rt.set_pump_divider(1);
  rt.start();

  const zmc::SdoKey key = make_key(0x6061);
  ASSERT_EQ(0, rt.sdo_read_once(key, 1));
  for (int i = 0; i < 100 && rt.last_seq() == 0; ++i) {
    rt.pump_results();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  ASSERT_GT(rt.last_seq(), 0u);

  uint32_t value = 0;
  EXPECT_EQ(-1234, rt.get_u32(key, &value));
  rt.stop();
}

TEST_F(SdoRtThreadFixture, OneShotDownload) {
  StartRt();
  const zmc::SdoKey key = make_key(0x6060);
  const uint32_t download_value = 0x11223344u;
  ASSERT_EQ(0, rt_.sdo_write(key, 1, &download_value));

  uint32_t out = 0;
  EXPECT_TRUE(wait_for_u32(rt_, key, &out));
  EXPECT_EQ(0x44u, out);
  rt_.stop();
}

TEST_F(SdoRtThreadFixture, WatchUploadPeriodic) {
  StartRt();
  const zmc::SdoKey key = make_key(0x6061);
  store_.set_u32(key, 0xAABBCCDDu);
  ASSERT_EQ(0, rt_.add_watch_upload(key, 20u, 1u));

  std::this_thread::sleep_for(std::chrono::milliseconds(80));
  rt_.pump_results();

  uint32_t out = 0;
  EXPECT_EQ(0, rt_.get_u32(key, &out));
  EXPECT_EQ(0xDDu, out);
  rt_.stop();
}

TEST_F(SdoRtThreadFixture, WatchDownloadPeriodic) {
  StartRt();
  const zmc::SdoKey key = make_key(0x6060);
  const uint8_t mode = 0x08;
  ASSERT_EQ(0, rt_.add_watch_download(key, 20u, 1u, &mode));

  std::this_thread::sleep_for(std::chrono::milliseconds(1100));

  std::vector<uint8_t> bytes(1);
  uint32_t len = 1;
  EXPECT_TRUE(store_.get_bytes(key, 1, bytes.data(), &len));
  EXPECT_EQ(1u, len);
  EXPECT_EQ(0x08u, bytes[0]);
  rt_.stop();
}

TEST_F(SdoRtThreadFixture, UpdateWatchWriteValueE2E) {
  StartRt();
  const zmc::SdoKey key = make_key(0x6060);
  zmc::SdoDirectionParams params{};
  params.watch_period_ms = 20u;
  ASSERT_EQ(0, rt_.set_direction_params(zmc::SDO_DIR_DOWNLOAD, params));
  ASSERT_EQ(0, rt_.add_watch_download_with_value(key, 1000u, 0x08, 1u));
  ASSERT_TRUE(wait_for_byte(store_, key, 0x08, 250));

  ASSERT_EQ(0, rt_.update_watch_write_value(key, 0x09, 1));

  for (int i = 0; i < 100; ++i) {
    rt_.pump_results();
    if (wait_for_byte(store_, key, 0x09, 1)) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  std::vector<uint8_t> bytes(1);
  uint32_t len = 1;
  EXPECT_TRUE(store_.get_bytes(key, 1, bytes.data(), &len));
  EXPECT_EQ(0x09, bytes[0]);
  rt_.stop();
}

TEST_F(SdoRtThreadFixture, RemoveWatchStopsPeriodic) {
  CountingAdapterCtx counter{};
  SdoCAdapter counting = make_counting_adapter(&counter);
  counter.store.set_u32(make_key(0x6061), 0x11111111u);

  rt_.bind_adapter(&counting);
  rt_.set_pump_divider(1);
  rt_.start();

  const zmc::SdoKey key = make_key(0x6061);
  ASSERT_EQ(0, rt_.add_watch_upload(key, 20u, 1u));
  std::this_thread::sleep_for(std::chrono::milliseconds(120));
  const uint32_t count_before_remove = counter.upload_count;

  ASSERT_EQ(0, rt_.remove_watch(key));
  std::this_thread::sleep_for(std::chrono::milliseconds(120));
  const uint32_t count_after_remove = counter.upload_count;
  EXPECT_LE(count_after_remove - count_before_remove, 2u);
  rt_.stop();
}

TEST(SdoRt, OneShotDownloadAlwaysCallsAdapter) {
  CountingAdapterCtx counter{};
  SdoCAdapter adapter = make_counting_adapter(&counter);

  zmc::SdoRtThread rt;
  rt.bind_adapter(&adapter);
  rt.set_pump_divider(1);
  rt.start();

  const zmc::SdoKey key = make_key(1, 0x607A, 0, 4);
  const uint32_t value = 0x11223344u;
  ASSERT_EQ(0, rt.sdo_write(key, 4, &value));

  uint32_t out = 0;
  ASSERT_TRUE(wait_for_u32(rt, key, &out));
  EXPECT_EQ(value, out);
  const uint32_t count_after_first = counter.download_count;

  ASSERT_EQ(0, rt.sdo_write(key, 4, &value));
  ASSERT_TRUE(wait_for_download_count(counter, count_after_first + 1u));
  EXPECT_EQ(count_after_first + 1u, counter.download_count);
  rt.stop();
}

TEST(SdoRt, NonOperationalSlaveSkipsUploadAndReplacesCachedSuccess) {
  CountingAdapterCtx counter{};
  const zmc::SdoKey key = make_key(1, 0x6061, 0, 1);
  counter.store.set_u32(key, 0x11u);
  counter.non_operational_after_state_call = 2u;
  SdoCAdapter adapter = make_counting_adapter(&counter);

  zmc::SdoRtThread rt;
  rt.bind_adapter(&adapter);
  rt.set_pump_divider(1);
  rt.start();

  ASSERT_EQ(0, rt.sdo_read_once(key, 1));
  uint32_t value = 0;
  ASSERT_TRUE(wait_for_u32(rt, key, &value));
  const uint32_t upload_count = counter.upload_count;
  const uint32_t result_seq = rt.last_seq();

  ASSERT_EQ(0, rt.sdo_read_once(key, 1));
  for (int i = 0; i < 100 && rt.last_seq() == result_seq; ++i) {
    rt.pump_results();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  EXPECT_GT(rt.last_seq(), result_seq);
  EXPECT_EQ(upload_count, counter.upload_count);
  EXPECT_EQ(ZMC_ERR_ECAT_SLAVE_NOT_OP, rt.get_u32(key, &value));
  rt.stop();
}

TEST(SdoRt, SlaveStateReadFailureSkipsDownload) {
  CountingAdapterCtx counter{};
  counter.slave_state_result = -33;
  SdoCAdapter adapter = make_counting_adapter(&counter);

  zmc::SdoRtThread rt;
  rt.bind_adapter(&adapter);
  rt.set_pump_divider(1);
  rt.start();

  const zmc::SdoKey key = make_key(2, 0x6060, 0, 1);
  const uint8_t value = 0x08u;
  ASSERT_EQ(0, rt.sdo_write(key, 1, &value));
  for (int i = 0; i < 100 && rt.last_seq() == 0u; ++i) {
    rt.pump_results();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  EXPECT_GT(rt.last_seq(), 0u);
  EXPECT_GT(counter.slave_state_count, 0u);
  EXPECT_EQ(0u, counter.download_count);
  uint32_t out = 0;
  EXPECT_EQ(ZMC_ERR_ECAT_GET_SLAVE_STATE_FAILED, rt.get_u32(key, &out));
  rt.stop();
}

TEST(SdoRt, DownloadWatchSkipsThenForcesAdapterWrite) {
  CountingAdapterCtx counter{};
  SdoCAdapter adapter = make_counting_adapter(&counter);

  zmc::SdoRtThread rt;
  rt.bind_adapter(&adapter);
  rt.set_pump_divider(1);
  rt.start();

  zmc::SdoDirectionParams params{};
  params.timeout_ms = zmc::SDO_DEFAULT_TIMEOUT_MS;
  params.watch_period_ms = 20u;
  ASSERT_EQ(0, rt.set_direction_params(zmc::SDO_DIR_DOWNLOAD, params));

  const zmc::SdoKey key = make_key(1, 0x6060, 0, 1);
  const uint8_t value = 0x08u;
  ASSERT_EQ(0, rt.add_watch_download(key, params.watch_period_ms, 1u, &value));
  ASSERT_TRUE(wait_for_download_count(counter, 1u));

  // Matching watch cycles synthesize results without immediately touching the adapter.
  std::this_thread::sleep_for(std::chrono::milliseconds(60));
  EXPECT_EQ(1u, counter.download_count);

  // Nine consecutive skips are bounded; the tenth unchanged watch writes again.
  ASSERT_TRUE(wait_for_download_count(counter, 2u));
  EXPECT_EQ(2u, counter.download_count);
  rt.stop();
}

TEST(SdoRt, NonOperationalSlavePreventsCachedDownloadSuccess) {
  CountingAdapterCtx counter{};
  counter.non_operational_after_state_call = 2u;
  SdoCAdapter adapter = make_counting_adapter(&counter);

  zmc::SdoRtThread rt;
  rt.bind_adapter(&adapter);
  rt.set_pump_divider(1);
  rt.start();

  zmc::SdoDirectionParams params{};
  params.timeout_ms = zmc::SDO_DEFAULT_TIMEOUT_MS;
  params.watch_period_ms = 10u;
  ASSERT_EQ(0, rt.set_direction_params(zmc::SDO_DIR_DOWNLOAD, params));

  const zmc::SdoKey key = make_key(1, 0x6060, 0, 1);
  const uint8_t value = 0x08u;
  ASSERT_EQ(0, rt.add_watch_download(key, params.watch_period_ms, 1u, &value));
  ASSERT_TRUE(wait_for_download_count(counter, 1u));
  std::this_thread::sleep_for(std::chrono::milliseconds(40));
  rt.pump_results();

  EXPECT_EQ(1u, counter.download_count);
  EXPECT_GT(counter.slave_state_count, 1u);
  uint32_t out = 0;
  EXPECT_EQ(ZMC_ERR_ECAT_SLAVE_NOT_OP, rt.get_u32(key, &out));
  rt.stop();
}

TEST(SdoRt, FailedForcedDownloadWatchRetriesNextPeriod) {
  CountingAdapterCtx counter{};
  counter.fail_download_call = 2u;
  counter.download_failure_code = -77;
  SdoCAdapter adapter = make_counting_adapter(&counter);

  zmc::SdoRtThread rt;
  rt.bind_adapter(&adapter);
  rt.set_pump_divider(1);
  rt.start();

  zmc::SdoDirectionParams params{};
  params.timeout_ms = zmc::SDO_DEFAULT_TIMEOUT_MS;
  params.watch_period_ms = 10u;
  ASSERT_EQ(0, rt.set_direction_params(zmc::SDO_DIR_DOWNLOAD, params));

  const zmc::SdoKey key = make_key(1, 0x6060, 0, 1);
  const uint8_t value = 0x09u;
  ASSERT_EQ(0, rt.add_watch_download(key, params.watch_period_ms, 1u, &value));

  ASSERT_TRUE(wait_for_download_count(counter, 2u));
  ASSERT_TRUE(wait_for_download_count(counter, 3u));
  EXPECT_EQ(3u, counter.download_count);
  EXPECT_TRUE(wait_for_byte(counter.store, key, value));
  rt.stop();
}

TEST(SdoRt, OpmodeAdapterLengthsAreOneByte) {
  CountingAdapterCtx counter{};
  counter.store.set_u32(make_key(0x6061), 0x12345678u);
  SdoCAdapter adapter = make_counting_adapter(&counter);

  zmc::SdoRtThread rt;
  rt.bind_adapter(&adapter);
  rt.set_pump_divider(1);
  rt.start();

  const zmc::SdoKey key6061 = make_key(0x6061);
  ASSERT_EQ(0, rt.sdo_read_once(key6061, 1));
  uint32_t out = 0;
  ASSERT_TRUE(wait_for_u32(rt, key6061, &out));
  EXPECT_EQ(1u, counter.last_upload_len);

  const zmc::SdoKey key6060 = make_key(0x6060);
  const uint8_t mode_value = 0x08;
  ASSERT_EQ(0, rt.sdo_write(key6060, 1, &mode_value));
  ASSERT_TRUE(wait_for_byte(counter.store, key6060, 0x08));
  EXPECT_EQ(1u, counter.last_download_len);
  rt.stop();
}

TEST(SdoRt, RequestStopExitsWorker) {
  FakeSdoStore store{};
  SdoCAdapter adapter = make_store_adapter(&store);

  zmc::SdoRtThread rt;
  rt.bind_adapter(&adapter);
  rt.set_pump_divider(1);
  rt.start();
  rt.request_stop();
  rt.stop();
  SUCCEED();
}

TEST(SdoRt, StopDropsUnconsumedResultsBeforeRestart) {
  CountingAdapterCtx counter{};
  const zmc::SdoKey key = make_key(1, 0x6061, 0);
  counter.store.set_u32(key, 0x11u);
  SdoCAdapter adapter = make_counting_adapter(&counter);

  zmc::SdoRtThread rt;
  rt.bind_adapter(&adapter);
  rt.set_pump_divider(1);
  rt.start();
  ASSERT_EQ(0, rt.sdo_read_once(key, 1));
  for (int i = 0; i < 100 && counter.upload_count == 0; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  ASSERT_GT(counter.upload_count, 0u);
  rt.stop();

  // The result existed only in the worker-owned channel and must be gone after
  // join.
  rt.pump_results();
  uint32_t value = 0;
  EXPECT_EQ(ZMC_ERR_SDO_NO_RESULT, rt.get_u32(key, &value));

  counter.store.set_u32(key, 0x22u);
  rt.start();
  ASSERT_EQ(0, rt.sdo_read_once(key, 1));
  ASSERT_TRUE(wait_for_u32(rt, key, &value));
  EXPECT_EQ(0x22u, value);
  rt.stop();
}
