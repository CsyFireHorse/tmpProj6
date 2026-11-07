/*
@ArtifactStatus garbage
@ArtifactStatusDate 2026-09-20
@ArtifactStatusAuthority GPT-5 Codex
*/
#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <thread>

#include <gtest/gtest.h>

#include "bridge/AxisFbBridgeWithThread.h"
#include "axis/axis_coe_channel.h"
#include "common/ecat/ecat_pdo_key.h"
#include "common/zmc/fbt_errorcode_def.h"
#include "ecat/EcatState.hpp"
#include "fb/fb_cpp_def.h"
#include "fb/mc_parameter_enum_def.h"
#include "impl/fbt_axis_base.h"
#include "pou/AxPou.hpp"
#include "sdo/sdo_rt.hpp"

namespace {

const char kOnePhysicalAxisIni[] = R"(
[base]
axis_num = 1
[axis_0]
is_axis_virtual = false
)";

constexpr int kStatusOffset = 0;
constexpr int kModeInputOffset = 2;
constexpr int kErrorOffset = 4;
constexpr int kPositionInputOffset = 8;
constexpr int kControlOffset = 0;
constexpr int kModeOutputOffset = 2;
constexpr int kPositionOutputOffset = 4;

struct BridgeCycleBuffers {
    alignas(4) std::array<uint8_t, 16> input{};
    alignas(4) std::array<uint8_t, 16> output{};
};

struct FixedSdoReply {
    uint16_t expected_slave_id{0};
    uint16_t expected_index{0};
    uint16_t value{0};
    int32_t upload_result{0};
};

int32_t fixed_sdo_upload(void* ctx, const SdoCKey* key, uint32_t* len, void* data,
                         uint32_t timeout_ms) {
    (void)timeout_ms;
    if (!ctx || !key || !len || !data) {
        return -1;
    }
    FixedSdoReply* reply = static_cast<FixedSdoReply*>(ctx);
    if (key->slave_id != reply->expected_slave_id || key->index != reply->expected_index ||
        key->subindex != 0u || *len != sizeof(reply->value)) {
        return -2;
    }
    if (reply->upload_result != 0) {
        return reply->upload_result;
    }
    std::memcpy(data, &reply->value, sizeof(reply->value));
    *len = sizeof(reply->value);
    return 0;
}

int32_t fixed_sdo_download(void* ctx, const SdoCKey* key, uint32_t len, const void* data,
                           uint32_t timeout_ms) {
    (void)ctx;
    (void)key;
    (void)len;
    (void)data;
    (void)timeout_ms;
    return 0;
}

int32_t operational_slave_state(void* ctx, uint16_t slave_id, uint8_t* slave_state) {
    (void)ctx;
    (void)slave_id;
    if (!slave_state) {
        return -1;
    }
    *slave_state = zmc::ECAT_STATE_OPERATIONAL;
    return 0;
}

SdoCAdapter make_sdo_adapter(FixedSdoReply* reply) {
    SdoCAdapter adapter{};
    adapter.ctx = reply;
    adapter.upload = fixed_sdo_upload;
    adapter.download = fixed_sdo_download;
    adapter.get_slave_state = operational_slave_state;
    return adapter;
}

ZmcPioOffsetLayout make_one_axis_layout(bool map_error, uint8_t slave_channel_idx) {
    ZmcPioOffsetLayout layout{};
    layout.slot_count = 1;
    layout.descriptor_pod.tx_cap = 16;
    layout.descriptor_pod.rx_cap = 16;
    layout.descriptor_pod.slave_count = 1;
    layout.slots[0].slave_idx = 2;
    layout.slots[0].slave_channel_idx = slave_channel_idx;
    layout.slots[0].po_ctrl_word_offset = kControlOffset;
    layout.slots[0].po_mode_offset = kModeOutputOffset;
    layout.slots[0].po_p_offset = kPositionOutputOffset;
    layout.slots[0].po_v_offset = -1;
    layout.slots[0].po_t_offset = -1;
    layout.slots[0].pi_stat_offset = kStatusOffset;
    layout.slots[0].pi_mode_offset = kModeInputOffset;
    layout.slots[0].pi_error_offset = map_error ? kErrorOffset : -1;
    layout.slots[0].pi_p_offset = kPositionInputOffset;
    layout.slots[0].pi_v_offset = -1;
    layout.slots[0].pi_t_offset = -1;
    return layout;
}

int configure_bridge(zmc::AxisFbBridge& bridge, bool map_error, uint8_t slave_channel_idx) {
    int ret = bridge.configure(nullptr, kOnePhysicalAxisIni, 1u);
    if (ret != 0) {
        return ret;
    }
    ret = bridge.auto_bind(0, 1);
    if (ret != 0) {
        return ret;
    }
    const ZmcPioOffsetLayout layout = make_one_axis_layout(map_error, slave_channel_idx);
    return bridge.apply_pio_layout(&layout);
}

bool wait_for_watch_counts(zmc::SdoRt* sdo, uint16_t upload_count,
                           uint16_t download_count) {
    if (!sdo || sdo->query_watch_count() != 0) {
        return false;
    }
    for (int i = 0; i < 200; ++i) {
        sdo->pump_results();
        zmc::SdoWatchCounts counts{};
        if (sdo->get_queried_watch_count(&counts) == 0 && counts.upload_count == upload_count &&
            counts.download_count == download_count) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return false;
}

bool wait_for_axis_error(zmc::AxisFbBridge& bridge, BridgeCycleBuffers& buffers,
                         uint16_t expected) {
    for (int i = 0; i < 200; ++i) {
        if (bridge.run_cycle(buffers.input.data(), buffers.output.data()) != 0) {
            return false;
        }
        AXIS_REF axis = zmc::AxPouTaskBinderAuto::getAxisByIndex(0);
        if (axis && zmc::get_axis_impl_ref(axis).uiDriveInterfaceError == expected) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return false;
}

}  // namespace

TEST(AxisFbBridgeDriveError, PdoValueUpdatesAxisAndSkipsSdoWatch) {
    zmc::AxisFbBridgeWithThread runtime;
    zmc::AxisFbBridge& bridge = runtime.bridge();
    ASSERT_EQ(0, configure_bridge(bridge, true, 0u));

    FixedSdoReply reply{};
    SdoCAdapter adapter = make_sdo_adapter(&reply);
    ASSERT_EQ(0, bridge.bind_sdo_adapter(&adapter));
    ASSERT_EQ(0, runtime.start_sdo());
    bridge.native_sdo()->resume();
    ASSERT_EQ(0, bridge.register_opmode_watches(8u));
    ASSERT_TRUE(wait_for_watch_counts(bridge.native_sdo(), 0u, 0u));

    BridgeCycleBuffers buffers;
    const uint16_t pdo_error = 0x4321u;
    std::memcpy(buffers.input.data() + kErrorOffset, &pdo_error, sizeof(pdo_error));
    ASSERT_EQ(0, bridge.run_cycle(buffers.input.data(), buffers.output.data()));

    AXIS_REF axis = zmc::AxPouTaskBinderAuto::getAxisByIndex(0);
    ASSERT_NE(nullptr, axis);
    EXPECT_EQ(pdo_error, zmc::get_axis_impl_ref(axis).uiDriveInterfaceError);

    zmc::FbtReadParameter read_parameter;
    read_parameter.Axis = axis;
    read_parameter.Enable = true;
    read_parameter.ParameterNumber = PARA_uiDriveInterfaceError;
    read_parameter.runCycle();
    EXPECT_TRUE(read_parameter.Valid);
    EXPECT_FALSE(read_parameter.Error);
    EXPECT_DOUBLE_EQ(static_cast<double>(pdo_error), read_parameter.Value);
}

TEST(AxisFbBridgeDriveError, SdoFallbackUsesChannelOffsetAndTwoByteValue) {
    constexpr uint8_t kSlaveChannel = 1u;
    constexpr uint16_t kExpectedError = 0x1357u;

    zmc::AxisFbBridgeWithThread runtime;
    zmc::AxisFbBridge& bridge = runtime.bridge();
    ASSERT_EQ(0, configure_bridge(bridge, false, kSlaveChannel));

    FixedSdoReply reply{};
    reply.expected_slave_id = 2u;
    reply.expected_index =
        static_cast<uint16_t>(PDO_PI_ERROR + kSlaveChannel * PDO_MULT_SERVO_OFFSET);
    reply.value = kExpectedError;
    SdoCAdapter adapter = make_sdo_adapter(&reply);
    ASSERT_EQ(0, bridge.bind_sdo_adapter(&adapter));
    ASSERT_EQ(0, runtime.start_sdo());
    bridge.native_sdo()->resume();
    ASSERT_EQ(0, bridge.register_opmode_watches(8u));
    ASSERT_TRUE(wait_for_watch_counts(bridge.native_sdo(), 1u, 0u));

    BridgeCycleBuffers buffers;
    EXPECT_TRUE(wait_for_axis_error(bridge, buffers, kExpectedError));
}

TEST(AxisFbBridgeDriveError, FailedSdoValueDoesNotOverwriteAxisError) {
    constexpr uint16_t kPreviousError = 0x3579u;

    zmc::AxisFbBridgeWithThread runtime;
    zmc::AxisFbBridge& bridge = runtime.bridge();
    ASSERT_EQ(0, configure_bridge(bridge, false, 0u));

    AXIS_REF axis = zmc::AxPouTaskBinderAuto::getAxisByIndex(0);
    ASSERT_NE(nullptr, axis);
    zmc::get_axis_impl_ref(axis).uiDriveInterfaceError = kPreviousError;

    FixedSdoReply reply{};
    reply.expected_slave_id = 2u;
    reply.expected_index = PDO_PI_ERROR;
    reply.upload_result = -42;
    SdoCAdapter adapter = make_sdo_adapter(&reply);
    ASSERT_EQ(0, bridge.bind_sdo_adapter(&adapter));
    ASSERT_EQ(0, runtime.start_sdo());
    bridge.native_sdo()->resume();
    ASSERT_EQ(0, bridge.register_opmode_watches(8u));
    ASSERT_TRUE(wait_for_watch_counts(bridge.native_sdo(), 1u, 0u));

    BridgeCycleBuffers buffers;
    const uint32_t result_seq = bridge.native_sdo()->last_seq();

    bool saw_failed_result = false;
    for (int i = 0; i < 200; ++i) {
        ASSERT_EQ(0, bridge.run_cycle(buffers.input.data(), buffers.output.data()));
        if (bridge.native_sdo()->last_seq() > result_seq) {
            saw_failed_result = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    ASSERT_TRUE(saw_failed_result);
    EXPECT_EQ(kPreviousError, zmc::get_axis_impl_ref(axis).uiDriveInterfaceError);
}

TEST(AxisFbBridgeDriveError, SuccessfulZeroSdoValueClearsAxisError) {
    zmc::AxisFbBridgeWithThread runtime;
    zmc::AxisFbBridge& bridge = runtime.bridge();
    ASSERT_EQ(0, configure_bridge(bridge, false, 0u));

    AXIS_REF axis = zmc::AxPouTaskBinderAuto::getAxisByIndex(0);
    ASSERT_NE(nullptr, axis);
    zmc::get_axis_impl_ref(axis).uiDriveInterfaceError = 0x2468u;

    FixedSdoReply reply{};
    reply.expected_slave_id = 2u;
    reply.expected_index = PDO_PI_ERROR;
    reply.value = 0u;
    SdoCAdapter adapter = make_sdo_adapter(&reply);
    ASSERT_EQ(0, bridge.bind_sdo_adapter(&adapter));
    ASSERT_EQ(0, runtime.start_sdo());
    bridge.native_sdo()->resume();
    ASSERT_EQ(0, bridge.register_opmode_watches(8u));

    BridgeCycleBuffers buffers;
    EXPECT_TRUE(wait_for_axis_error(bridge, buffers, 0u));
}

TEST(AxisFbBridgeDriveError, CoeSubmitPreservesSdoQueueError) {
    zmc::AxisFbBridgeWithThread runtime;
    zmc::AxisFbBridge& bridge = runtime.bridge();
    ASSERT_EQ(0, configure_bridge(bridge, true, 0u));

    zmc::SdoRt* sdo = bridge.native_sdo();
    ASSERT_NE(nullptr, sdo);
    const zmc::SdoKey key{};
    int submit_result = 0;
    for (uint32_t i = 0; i < 128u && submit_result == 0; ++i) {
        submit_result = sdo->sdo_read_once(key, 1u);
    }
    EXPECT_EQ(ZMC_ERR_SDO_QUEUE_FULL, submit_result);

    AXIS_REF axis = zmc::AxPouTaskBinderAuto::getAxisByIndex(0);
    ASSERT_NE(nullptr, axis);
    uint32_t tag = 0;
    ASSERT_EQ(0, zmc::axis_coe_req_enqueue_read(axis, 0x2000u, 0u, 2u, &tag));

    BridgeCycleBuffers buffers;
    ASSERT_EQ(0, bridge.run_cycle(buffers.input.data(), buffers.output.data()));
    int32_t result = 0;
    EXPECT_EQ(zmc::AXIS_COE_RESULT_FOUND, zmc::axis_coe_result_query(axis, tag, &result));
    EXPECT_EQ(ZMC_ERR_SDO_QUEUE_FULL, result);
}
