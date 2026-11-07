/*
@ArtifactStatus garbage
@ArtifactStatusDate 2026-09-21
@ArtifactStatusAuthority GPT-5 Codex
*/
#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <thread>

#include "sdo_rt_thread.hpp"
#include "sdo_test_helper.hpp"

using fb_test::sdo::FakeSdoStore;
using fb_test::sdo::make_key;
using fb_test::sdo::make_store_adapter;

namespace {

class SdoOneshotDispatchFixture : public ::testing::Test {
  protected:
    void SetUp() override {
        adapter_ = make_store_adapter(&store_);
        rt_.bind_adapter(&adapter_);
        rt_.set_pump_divider(1);
        rt_.start();
    }

    void TearDown() override { rt_.stop(); }

    // Pump until the requested Axis CoE result leaves one-shot staging.
    bool wait_for_axis_result(uint32_t request_id, zmc::SdoResultItem* out,
                              int max_retries = 200) {
        for (int i = 0; i < max_retries; ++i) {
            rt_.pump_results();
            zmc::SdoResult batch{};
            if (rt_.take_oneshot_results(&batch)) {
                for (uint8_t j = 0; j < batch.item_count; ++j) {
                    if (batch.items[j].request_id != request_id) continue;
                    *out = batch.items[j];
                    return true;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        return false;
    }

    FakeSdoStore store_{};
    SdoCAdapter adapter_{};
    zmc::SdoRtThread rt_{};
};

TEST(SdoRequestId, EncodesAxisAndTag) {
    const uint32_t first = zmc::sdo_axis_coe_write_request_id(0u, 1u);
    EXPECT_TRUE(zmc::sdo_request_id_is_axis_coe_write(first));
    EXPECT_EQ(0u, zmc::sdo_request_id_axis_index(first));
    EXPECT_EQ(1u, zmc::sdo_request_id_axis_tag(first));

    const uint32_t last = zmc::sdo_axis_coe_write_request_id(63u, 0x00FFFFFFu);
    EXPECT_TRUE(zmc::sdo_request_id_is_axis_coe_write(last));
    EXPECT_EQ(63u, zmc::sdo_request_id_axis_index(last));
    EXPECT_EQ(0x00FFFFFFu, zmc::sdo_request_id_axis_tag(last));
    EXPECT_FALSE(zmc::sdo_request_id_is_axis_coe_write(1u));
}

TEST_F(SdoOneshotDispatchFixture, AxisWriteUsesOneshotCacheOnly) {
    const zmc::SdoKey key = make_key(1, 0x2000);
    const uint32_t value = 0x11223344u;
    const uint32_t request_id = zmc::sdo_axis_coe_write_request_id(3u, 7u);
    ASSERT_EQ(0, rt_.sdo_write(key, 4, &value, request_id));

    zmc::SdoResultItem item{};
    ASSERT_TRUE(wait_for_axis_result(request_id, &item));
    EXPECT_EQ(request_id, item.request_id);
    EXPECT_EQ(key, item.key);
    EXPECT_EQ(0, item.result_code);
    EXPECT_EQ(value, item.data);
    uint32_t cached = 0;
    EXPECT_EQ(ZMC_ERR_SDO_NO_RESULT, rt_.get_u32(key, &cached));
}

TEST_F(SdoOneshotDispatchFixture, LegacyWriteKeepsDictionaryFlow) {
    const zmc::SdoKey key = make_key(1, 0x2003);
    const uint32_t value = 0x55667788u;
    ASSERT_EQ(0, rt_.sdo_write(key, 4, &value));

    uint32_t cached = 0;
    for (int i = 0; i < 200; ++i) {
        rt_.pump_results();
        if (rt_.get_u32(key, &cached) == 0) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    EXPECT_EQ(0, rt_.get_u32(key, &cached));
    EXPECT_EQ(value, cached);
}

TEST_F(SdoOneshotDispatchFixture, FullStagingLeavesRemainingResultsQueued) {
    const uint32_t value = 7u;
    for (uint32_t axis = 0; axis < 17u; ++axis) {
        const zmc::SdoKey key = make_key(1, static_cast<uint16_t>(0x2100u + axis));
        ASSERT_EQ(0, rt_.sdo_write(key, 4, &value,
                                   zmc::sdo_axis_coe_write_request_id(axis, 1u)));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    zmc::SdoResult batch{};
    rt_.pump_results();
    ASSERT_TRUE(rt_.take_oneshot_results(&batch));
    EXPECT_EQ(SDO_RESULT_MAX_ITEMS, static_cast<uint32_t>(batch.item_count));

    rt_.pump_results();
    ASSERT_TRUE(rt_.take_oneshot_results(&batch));
    EXPECT_EQ(SDO_RESULT_MAX_ITEMS, static_cast<uint32_t>(batch.item_count));

    rt_.pump_results();
    ASSERT_TRUE(rt_.take_oneshot_results(&batch));
    ASSERT_EQ(1u, batch.item_count);
    EXPECT_EQ(zmc::sdo_axis_coe_write_request_id(16u, 1u), batch.items[0].request_id);
}

}  // namespace
