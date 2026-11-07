/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-17
@ArtifactStatusAuthority GLM-5.2 (opencode)
*/
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "SdoMessage.hpp"
#include "sdo_config.hpp"
#include "sdo_rt_thread.hpp"
#include "sdo_test_helper.hpp"

using fb_test::sdo::CountingAdapterCtx;
using fb_test::sdo::FakeSdoStore;
using fb_test::sdo::make_counting_adapter;
using fb_test::sdo::make_key;
using fb_test::sdo::make_store_adapter;

TEST(SdoConfig, DirectionEnumSetParamsValues) {
    EXPECT_EQ(7u, static_cast<uint32_t>(zmc::SDO_DIR_SET_UPLOAD_PARAMS));
    EXPECT_EQ(8u, static_cast<uint32_t>(zmc::SDO_DIR_SET_DOWNLOAD_PARAMS));
    EXPECT_TRUE(zmc::sdo_direction_valid(zmc::SDO_DIR_SET_UPLOAD_PARAMS));
    EXPECT_TRUE(zmc::sdo_direction_valid(zmc::SDO_DIR_SET_DOWNLOAD_PARAMS));
    EXPECT_FALSE(zmc::sdo_direction_valid(99u));
    EXPECT_EQ(zmc::SDO_DIR_SET_DOWNLOAD_PARAMS,
              zmc::sdo_set_params_direction_for(zmc::SDO_DIR_DOWNLOAD));
}

TEST(SdoConfig, DirectionEnumGetParamsValues) {
    EXPECT_EQ(9u, static_cast<uint32_t>(zmc::SDO_DIR_GET_UPLOAD_PARAMS));
    EXPECT_EQ(10u, static_cast<uint32_t>(zmc::SDO_DIR_GET_DOWNLOAD_PARAMS));
    EXPECT_TRUE(zmc::sdo_direction_valid(zmc::SDO_DIR_GET_UPLOAD_PARAMS));
    EXPECT_TRUE(zmc::sdo_direction_valid(zmc::SDO_DIR_GET_DOWNLOAD_PARAMS));
    EXPECT_FALSE(zmc::sdo_direction_valid(14u));
    EXPECT_EQ(zmc::SDO_DIR_GET_DOWNLOAD_PARAMS,
              zmc::sdo_get_params_direction_for(zmc::SDO_DIR_DOWNLOAD));
}

TEST(SdoConfig, DirectionEnumGetWatchCountValues) {
    EXPECT_TRUE(zmc::sdo_direction_valid(zmc::SDO_DIR_GET_WATCH_COUNT));
    EXPECT_TRUE(zmc::sdo_direction_valid(zmc::SDO_DIR_CLEAR_WATCHES));
    EXPECT_FALSE(zmc::sdo_direction_valid(14u));
    EXPECT_TRUE(zmc::sdo_direction_is_get_watch_count(zmc::SDO_DIR_GET_WATCH_COUNT));
}

TEST(SdoConfig, PackUnpackWatchCountsRoundTrip) {
    zmc::SdoWatchCounts input{};
    input.upload_count = 2u;
    input.download_count = 1u;
    const uint32_t packed = zmc::sdo_pack_watch_counts(input);
    zmc::SdoWatchCounts output{};
    ASSERT_EQ(0, zmc::sdo_unpack_watch_counts(packed, &output));
    EXPECT_EQ(input.upload_count, output.upload_count);
    EXPECT_EQ(input.download_count, output.download_count);
}

TEST(SdoConfig, PackUnpackRoundTrip) {
    zmc::SdoDirectionParams input{};
    input.timeout_ms = 200u;
    input.watch_period_ms = 20u;
    const uint32_t packed = zmc::sdo_pack_direction_params(input);
    zmc::SdoDirectionParams output{};
    ASSERT_EQ(0, zmc::sdo_unpack_direction_params(packed, &output));
    EXPECT_EQ(input.timeout_ms, output.timeout_ms);
    EXPECT_EQ(input.watch_period_ms, output.watch_period_ms);
}

TEST(SdoConfig, RejectsZeroPackedValues) {
    zmc::SdoDirectionParams output{};
    EXPECT_EQ(ZMC_ERR_SDO_INVALID_ARG, zmc::sdo_unpack_direction_params(0u, &output));
}

TEST(SdoConfig, DirectionParamsDefaultUpload) {
    const zmc::SdoDirectionParams params = zmc::sdo_direction_params_default(zmc::SDO_DIR_UPLOAD);
    EXPECT_EQ(zmc::SDO_DEFAULT_TIMEOUT_MS, params.timeout_ms);
    EXPECT_EQ(zmc::SDO_DEFAULT_UPLOAD_WATCH_PERIOD_MS, params.watch_period_ms);
}

TEST(SdoConfig, DirectionParamsDefaultDownload) {
    const zmc::SdoDirectionParams params =
        zmc::sdo_direction_params_default(zmc::SDO_DIR_DOWNLOAD);
    EXPECT_EQ(zmc::SDO_DEFAULT_TIMEOUT_MS, params.timeout_ms);
    EXPECT_EQ(zmc::SDO_DEFAULT_DOWNLOAD_WATCH_PERIOD_MS, params.watch_period_ms);
}

namespace {

class SdoRtConfigFixture : public ::testing::Test {
   protected:
    void SetUp() override {
        store_.set_u32(make_key(0x6061), 0xAABBCCDDu);
        adapter_ = make_store_adapter(&store_);
    }

    void StartRt() {
        rt_.bind_adapter(&adapter_);
        rt_.set_pump_divider(1);
        rt_.start();
    }

    void WaitForDirectionParams(uint8_t data_direction, uint32_t timeout_ms,
                                uint32_t watch_period_ms) {
        for (int i = 0; i < 100; ++i) {
            // The Manager cache commits only after consuming a successful SET result.
            rt_.pump_results();
            zmc::SdoDirectionParams output{};
            if (rt_.get_direction_params(data_direction, &output) == 0 &&
                output.timeout_ms == timeout_ms && output.watch_period_ms == watch_period_ms) {
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        FAIL() << "direction params not applied";
    }

    void WaitForQueriedDirectionParams(uint8_t data_direction, uint32_t timeout_ms,
                                       uint32_t watch_period_ms) {
        // The SET echo already confirms the unified snapshot, so capture its seq
        // as the baseline; only the GET query result can advance seq further.
        zmc::SdoDirectionParams baseline{};
        uint32_t baseline_seq = 0;
        ASSERT_EQ(0, rt_.get_direction_params(data_direction, &baseline, &baseline_seq));
        for (int i = 0; i < 100; ++i) {
            rt_.pump_results();
            zmc::SdoDirectionParams output{};
            uint32_t seq = 0;
            if (rt_.get_direction_params(data_direction, &output, &seq) == 0 &&
                output.timeout_ms == timeout_ms && output.watch_period_ms == watch_period_ms &&
                seq > baseline_seq) {
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        FAIL() << "queried direction params not received";
    }

    FakeSdoStore store_{};
    SdoCAdapter adapter_{};
    zmc::SdoRtThread rt_{};
};

}  // namespace

TEST_F(SdoRtConfigFixture, UploadTimeoutApplied) {
    CountingAdapterCtx counter{};
    SdoCAdapter counting = make_counting_adapter(&counter);
    counter.store.set_u32(make_key(0x6061), 0x11u);

    rt_.bind_adapter(&counting);
    rt_.set_pump_divider(1);
    rt_.start();

    zmc::SdoDirectionParams config{};
    config.timeout_ms = 222u;
    config.watch_period_ms = 50u;
    ASSERT_EQ(0, rt_.set_direction_params(zmc::SDO_DIR_UPLOAD, config));
    WaitForDirectionParams(zmc::SDO_DIR_UPLOAD, 222u, 50u);

    const zmc::SdoKey key = make_key(0x6061);
    ASSERT_EQ(0, rt_.sdo_read_once(key, 1u));
    for (int i = 0; i < 100; ++i) {
        rt_.pump_results();
        if (counter.upload_count > 0) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    EXPECT_EQ(222u, counter.last_upload_timeout_ms);
    rt_.stop();
}

TEST_F(SdoRtConfigFixture, WatchUsesDirectionPeriod) {
    StartRt();
    zmc::SdoDirectionParams config{};
    config.timeout_ms = 100u;
    config.watch_period_ms = 20u;
    ASSERT_EQ(0, rt_.set_direction_params(zmc::SDO_DIR_UPLOAD, config));
    WaitForDirectionParams(zmc::SDO_DIR_UPLOAD, 100u, 20u);

    const zmc::SdoKey key = make_key(0x6061);
    ASSERT_EQ(0, rt_.add_watch_upload(key, 999u, 1u));
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    rt_.pump_results();

    uint32_t output = 0;
    EXPECT_EQ(0, rt_.get_u32(key, &output));
    EXPECT_EQ(0xDDu, output);
    rt_.stop();
}

TEST_F(SdoRtConfigFixture, SetParamsViaWorkerThread) {
    StartRt();
    zmc::SdoDirectionParams config{};
    config.timeout_ms = 180u;
    config.watch_period_ms = 35u;
    ASSERT_EQ(0, rt_.set_direction_params(zmc::SDO_DIR_DOWNLOAD, config));
    WaitForDirectionParams(zmc::SDO_DIR_DOWNLOAD, 180u, 35u);
    EXPECT_GT(rt_.last_seq(), 0u);
    rt_.stop();
}

TEST_F(SdoRtConfigFixture, QueryParamsViaWorkerThread) {
    StartRt();
    zmc::SdoDirectionParams config{};
    config.timeout_ms = 180u;
    config.watch_period_ms = 35u;
    ASSERT_EQ(0, rt_.set_direction_params(zmc::SDO_DIR_DOWNLOAD, config));
    WaitForDirectionParams(zmc::SDO_DIR_DOWNLOAD, 180u, 35u);

    ASSERT_EQ(0, rt_.query_direction_params(zmc::SDO_DIR_DOWNLOAD));
    WaitForQueriedDirectionParams(zmc::SDO_DIR_DOWNLOAD, 180u, 35u);

    zmc::SdoDirectionParams output{};
    uint32_t seq = 0;
    ASSERT_EQ(0, rt_.get_direction_params(zmc::SDO_DIR_DOWNLOAD, &output, &seq));
    EXPECT_EQ(180u, output.timeout_ms);
    EXPECT_EQ(35u, output.watch_period_ms);
    EXPECT_GT(seq, 0u);
    rt_.stop();
}

TEST_F(SdoRtConfigFixture, QueryWatchCountViaWorkerThread) {
    StartRt();
    const zmc::SdoKey upload_key1 = make_key(1, 0x6061);
    const zmc::SdoKey upload_key2 = make_key(1, 0x6062);
    const zmc::SdoKey download_key = make_key(1, 0x6060);
    ASSERT_EQ(0, rt_.add_watch_upload(upload_key1, 0u, 1u));
    ASSERT_EQ(0, rt_.add_watch_upload(upload_key2, 0u, 1u));
    uint8_t mode = 0x08u;
    ASSERT_EQ(0, rt_.add_watch_download(download_key, 0u, 1u, &mode));

    ASSERT_EQ(0, rt_.query_watch_count());
    for (int i = 0; i < 100; ++i) {
        rt_.pump_results();
        zmc::SdoWatchCounts counts{};
        if (rt_.get_queried_watch_count(&counts) == 0 && counts.upload_count == 2u &&
            counts.download_count == 1u) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    zmc::SdoWatchCounts output{};
    uint32_t seq = 0;
    ASSERT_EQ(0, rt_.get_queried_watch_count(&output, &seq));
    EXPECT_EQ(2u, output.upload_count);
    EXPECT_EQ(1u, output.download_count);
    EXPECT_GT(seq, 0u);
    rt_.stop();
}
