/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-18
@ArtifactStatusAuthority GPT-5.6 Sol
*/
#include <gtest/gtest.h>

#include <chrono>
#include <cstdint>
#include <cstring>
#include <thread>

#include "sdo_rt.hpp"
#include "sdo_rt_thread.hpp"
#include "sdo_adapter.h"
#include "ecat/EcatState.hpp"

namespace {

struct FakeSdoTransfer {
    uint32_t upload_value;
};

int32_t fake_upload(void* ctx, const SdoCKey* key, uint32_t* len, void* data,
                    uint32_t timeout_ms) {
    (void)timeout_ms;
    if (!ctx || !key || !len || !data) {
        return -1;
    }
    FakeSdoTransfer* fake = static_cast<FakeSdoTransfer*>(ctx);
    uint32_t copy_len = *len;
    if (copy_len > sizeof(fake->upload_value)) {
        copy_len = sizeof(fake->upload_value);
    }
    std::memcpy(data, &fake->upload_value, static_cast<size_t>(copy_len));
    *len = copy_len;
    return 0;
}

int32_t fake_download(void* ctx, const SdoCKey* key, uint32_t len, const void* data,
                      uint32_t timeout_ms) {
    (void)ctx;
    (void)timeout_ms;
    return (key && len > 0 && data) ? 0 : -1;
}

int32_t fake_upload_null_ctx(void* ctx, const SdoCKey* key, uint32_t* len, void* data,
                             uint32_t timeout_ms) {
    (void)ctx;
    (void)key;
    (void)timeout_ms;
    if (!len || !data) {
        return -1;
    }
    uint32_t value = 0x55667788u;
    uint32_t copy_len = *len;
    if (copy_len > sizeof(value)) {
        copy_len = sizeof(value);
    }
    std::memcpy(data, &value, static_cast<size_t>(copy_len));
    *len = copy_len;
    return 0;
}

int32_t fake_get_slave_state(void* ctx, uint16_t slave_id, uint8_t* slave_state) {
    (void)ctx;
    (void)slave_id;
    if (!slave_state) {
        return -1;
    }
    *slave_state = zmc::ECAT_STATE_OPERATIONAL;
    return 0;
}

zmc::SdoKey make_key(uint16_t index) {
    zmc::SdoKey key;
    key.slave_id = 1;
    key.index = index;
    key.subindex = 0;
    return key;
}

bool wait_for_u32(zmc::SdoRt& rt, const zmc::SdoKey& key, uint32_t* out) {
    for (int i = 0; i < 100; ++i) {
        rt.pump_results();
        if (rt.get_u32(key, out) == 0) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return false;
}

}  // namespace

TEST(SdoRt, UsesAdapterForUpload) {
    FakeSdoTransfer fake = {0x12345678u};
    SdoCAdapter adapter = {};
    adapter.ctx = &fake;
    adapter.upload = fake_upload;
    adapter.download = fake_download;
    adapter.get_slave_state = fake_get_slave_state;

    zmc::SdoRtThread rt;
    rt.bind_adapter(&adapter);
    rt.set_pump_divider(1);
    rt.start();

    zmc::SdoKey key = make_key(0x6061);
    ASSERT_EQ(0, rt.sdo_read_once(key, 1));

    uint32_t out = 0;
    EXPECT_TRUE(wait_for_u32(rt, key, &out));
    EXPECT_EQ(0x78u, out);
    rt.stop();
}

TEST(SdoRt, UsesAdapterForDownloadResult) {
    FakeSdoTransfer fake = {0};
    SdoCAdapter adapter = {};
    adapter.ctx = &fake;
    adapter.upload = fake_upload;
    adapter.download = fake_download;
    adapter.get_slave_state = fake_get_slave_state;

    zmc::SdoRtThread rt;
    rt.bind_adapter(&adapter);
    rt.set_pump_divider(1);
    rt.start();

    zmc::SdoKey key = make_key(0x6060);
    uint32_t value = 0x11223344u;
    ASSERT_EQ(0, rt.sdo_write(key, 1, &value));

    uint32_t out = 0;
    EXPECT_TRUE(wait_for_u32(rt, key, &out));
    EXPECT_EQ(0x44u, out);
    rt.stop();
}

TEST(SdoRt, AllowsNullAdapterContext) {
    SdoCAdapter adapter = {};
    adapter.ctx = nullptr;
    adapter.upload = fake_upload_null_ctx;
    adapter.download = fake_download;
    adapter.get_slave_state = fake_get_slave_state;

    zmc::SdoRtThread rt;
    rt.bind_adapter(&adapter);
    rt.set_pump_divider(1);
    rt.start();

    zmc::SdoKey key = make_key(0x6061);
    ASSERT_EQ(0, rt.sdo_read_once(key, 1));

    uint32_t out = 0;
    EXPECT_TRUE(wait_for_u32(rt, key, &out));
    EXPECT_EQ(0x88u, out);
    rt.stop();
}
