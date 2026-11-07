/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-17
@ArtifactStatusAuthority
 * GPT-5 Codex
*/
#ifndef _FB_TEST_SDO_TEST_HELPER_HPP_
#define _FB_TEST_SDO_TEST_HELPER_HPP_

#include <chrono>
#include <cstdint>
#include <cstring>
#include <thread>
#include <unordered_map>
#include <vector>

#include "SdoMessage.hpp"
#include "ecat/EcatState.hpp"
#include "sdo_adapter.h"
#include "sdo_rt.hpp"

namespace fb_test {
namespace sdo {

inline uint8_t default_len_for_index(uint16_t index) {
  return (index == 0x6060 || index == 0x6061) ? 1u : 4u;
}

inline zmc::SdoKey make_key(uint16_t slave_id, uint16_t index,
                            uint8_t subindex = 0, uint8_t len = 0) {
  (void)len;
  zmc::SdoKey key{};
  key.slave_id = slave_id;
  key.index = index;
  key.subindex = subindex;
  return key;
}

inline zmc::SdoKey make_key(uint16_t index) { return make_key(1, index, 0); }

struct FakeSdoStore {
  std::unordered_map<zmc::SdoKey, std::vector<uint8_t>, zmc::SdoKeyHash> data;

  void set_u32(const zmc::SdoKey &key, uint32_t value) {
    std::vector<uint8_t> bytes(4);
    std::memcpy(bytes.data(), &value, sizeof(value));
    data[key] = bytes;
  }

  bool get_bytes(const zmc::SdoKey &key, uint32_t max_len, void *out,
                 uint32_t *out_len) const {
    if (!out || !out_len)
      return false;
    auto it = data.find(key);
    if (it == data.end())
      return false;
    uint32_t copy_len = static_cast<uint32_t>(it->second.size());
    if (copy_len > max_len)
      copy_len = max_len;
    if (copy_len > 0)
      std::memcpy(out, it->second.data(), static_cast<size_t>(copy_len));
    *out_len = copy_len;
    return true;
  }

  void set_bytes(const zmc::SdoKey &key, uint32_t len, const void *bytes) {
    if (!bytes || len == 0)
      return;
    const auto *in = static_cast<const uint8_t *>(bytes);
    data[key] = std::vector<uint8_t>(in, in + len);
  }
};

inline int32_t store_upload(void *ctx, const SdoCKey *key, uint32_t *len,
                            void *data, uint32_t timeout_ms) {
  (void)timeout_ms;
  if (!ctx || !key || !len || !data)
    return -1;
  FakeSdoStore *store = static_cast<FakeSdoStore *>(ctx);
  zmc::SdoKey cpp_key{};
  cpp_key.slave_id = key->slave_id;
  cpp_key.index = key->index;
  cpp_key.subindex = key->subindex;
  return store->get_bytes(cpp_key, *len, data, len) ? 0 : -1;
}

inline int32_t store_download(void *ctx, const SdoCKey *key, uint32_t len,
                              const void *data, uint32_t timeout_ms) {
  (void)timeout_ms;
  if (!ctx || !key || !data || len == 0)
    return -1;
  FakeSdoStore *store = static_cast<FakeSdoStore *>(ctx);
  zmc::SdoKey cpp_key{};
  cpp_key.slave_id = key->slave_id;
  cpp_key.index = key->index;
  cpp_key.subindex = key->subindex;
  store->set_bytes(cpp_key, len, data);
  return 0;
}

inline int32_t operational_slave_state(void *ctx, uint16_t slave_id,
                                       uint8_t *slave_state) {
  (void)ctx;
  (void)slave_id;
  if (!slave_state)
    return -1;
  *slave_state = zmc::ECAT_STATE_OPERATIONAL;
  return 0;
}

inline SdoCAdapter make_store_adapter(FakeSdoStore *store) {
  SdoCAdapter adapter = {};
  adapter.ctx = store;
  adapter.upload = store_upload;
  adapter.download = store_download;
  adapter.get_slave_state = operational_slave_state;
  return adapter;
}

inline bool wait_for_byte(FakeSdoStore &store, const zmc::SdoKey &key,
                          uint8_t expected, int max_retries = 100) {
  for (int i = 0; i < max_retries; ++i) {
    std::vector<uint8_t> bytes(1);
    uint32_t len = 1;
    if (store.get_bytes(key, 1, bytes.data(), &len) && len == 1 &&
        bytes[0] == expected) {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  return false;
}

inline bool wait_for_u32(zmc::SdoRt &rt, const zmc::SdoKey &key, uint32_t *out,
                         int max_retries = 100) {
  for (int i = 0; i < max_retries; ++i) {
    rt.pump_results();
    if (rt.get_u32(key, out) == 0)
      return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  return false;
}

struct CountingAdapterCtx {
  FakeSdoStore store;
  uint32_t upload_count{0};
  uint32_t download_count{0};
  uint32_t fail_download_call{0};
  int32_t download_failure_code{-1};
  uint32_t last_upload_len{0};
  uint32_t last_download_len{0};
  uint32_t last_upload_timeout_ms{0};
  uint32_t last_download_timeout_ms{0};
  uint8_t slave_state{zmc::ECAT_STATE_OPERATIONAL};
  int32_t slave_state_result{0};
  uint32_t slave_state_count{0};
  uint32_t non_operational_after_state_call{0};
};

inline int32_t counting_slave_state(void *ctx, uint16_t slave_id,
                                    uint8_t *slave_state) {
  (void)slave_id;
  if (!ctx || !slave_state)
    return -1;
  CountingAdapterCtx *counter = static_cast<CountingAdapterCtx *>(ctx);
  ++counter->slave_state_count;
  if (counter->slave_state_result != 0)
    return counter->slave_state_result;
  *slave_state = counter->non_operational_after_state_call != 0 &&
                         counter->slave_state_count >= counter->non_operational_after_state_call
                     ? 4u
                     : counter->slave_state;
  return 0;
}

inline int32_t counting_upload(void *ctx, const SdoCKey *key, uint32_t *len,
                               void *data, uint32_t timeout_ms) {
  if (!ctx || !key || !len || !data)
    return -1;
  CountingAdapterCtx *counter = static_cast<CountingAdapterCtx *>(ctx);
  ++counter->upload_count;
  counter->last_upload_len = *len;
  counter->last_upload_timeout_ms = timeout_ms;
  return store_upload(&counter->store, key, len, data, timeout_ms);
}

inline int32_t counting_download(void *ctx, const SdoCKey *key, uint32_t len,
                                 const void *data, uint32_t timeout_ms) {
  if (!ctx || !key || !data || len == 0)
    return -1;
  CountingAdapterCtx *counter = static_cast<CountingAdapterCtx *>(ctx);
  ++counter->download_count;
  counter->last_download_len = len;
  counter->last_download_timeout_ms = timeout_ms;
  if (counter->download_count == counter->fail_download_call)
    return counter->download_failure_code;
  return store_download(&counter->store, key, len, data, timeout_ms);
}

inline SdoCAdapter make_counting_adapter(CountingAdapterCtx *ctx) {
  SdoCAdapter adapter = {};
  adapter.ctx = ctx;
  adapter.upload = counting_upload;
  adapter.download = counting_download;
  adapter.get_slave_state = counting_slave_state;
  return adapter;
}

inline bool wait_for_download_count(CountingAdapterCtx &counter,
                                    uint32_t expected,
                                    int max_retries = 200) {
  for (int i = 0; i < max_retries; ++i) {
    if (counter.download_count >= expected)
      return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  return false;
}

} // namespace sdo
} // namespace fb_test

#endif // _FB_TEST_SDO_TEST_HELPER_HPP_
