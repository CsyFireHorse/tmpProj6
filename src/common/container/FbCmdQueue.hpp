#ifndef _ZMC_FB_CMD_QUEUE_HPP_
#define _ZMC_FB_CMD_QUEUE_HPP_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

#include "common/zmc/mc_motion_packet.h"
#include "common/zmc/pcie_compat.h"

// #include "fbmEnum.h"

namespace zmc {

// Maximum payload size for command/status packets (queue capacity must be power of 2).
static constexpr size_t MAX_PAYLOAD_SIZE = MC_CMD_PACKET_PAYLOAD_MAX;

// Single C++ wrapper over the shared C wire layout; no extra fields.
struct MotionPacket : MCMotionPacket {
    // Append one item: {type(u16), param_len(u16), param[param_len]}
    bool appendItem(uint16_t type, const void* param, uint16_t param_len) {
        const size_t item_size = sizeof(uint16_t) + sizeof(uint16_t) + param_len;
        if (payload_len + item_size > MAX_PAYLOAD_SIZE) {
            return false;
        }
        std::memcpy(payload + payload_len, &type, sizeof(uint16_t));
        payload_len += sizeof(uint16_t);
        std::memcpy(payload + payload_len, &param_len, sizeof(uint16_t));
        payload_len += sizeof(uint16_t);
        if (param_len > 0 && param != nullptr) {
            std::memcpy(payload + payload_len, param, param_len);
            payload_len += param_len;
        }
        return true;
    }

    void reset() {
        seq = 0;
        payload_len = 0;
        std::memset(payload, 0, sizeof(payload));
    }

    // First item type = leading u16 of payload (caller ensures payload has an item).
    uint16_t firstType() const { return *reinterpret_cast<const uint16_t*>(payload); }
};

using CmdPacket = MotionPacket;
using StatusPacket = MotionPacket;

static_assert(sizeof(MotionPacket) == sizeof(MCMotionPacket),
              "MotionPacket must not add fields beyond shared motion packet layout");
static_assert(std::is_standard_layout<MotionPacket>::value,
              "MotionPacket must stay standard-layout");
static_assert(std::is_trivially_copyable<MotionPacket>::value,
              "MotionPacket must stay trivially copyable");

// Command queue wrapper over the selected SPSC queue branch.
template <size_t QueueCapacity = 16>
class MotionPacketQueue {
    static_assert((QueueCapacity & (QueueCapacity - 1)) == 0, "QueueCapacity must be power of 2");

   private:
    SPSCQueue<MotionPacket, QueueCapacity> queue_;
    uint16_t seq_counter_ = 0;

   public:
    bool push(const MotionPacket& packet) {
        MotionPacket pkt = packet;
        // Reserve zero for unprocessed progress; commit the counter only after enqueue succeeds.
        pkt.seq = static_cast<uint16_t>(seq_counter_ + 1u);
        if (pkt.seq == 0) pkt.seq = 1;
        if (!queue_.try_push(pkt)) return false;
        seq_counter_ = pkt.seq;
        return true;
    }

    bool pop(MotionPacket& packet) { return queue_.try_pop(packet); }

    // Read only by the serialized producer after a successful push.
    uint16_t lastSequence() const { return seq_counter_; }

    size_t size() const { return queue_.size(); }
    size_t capacity() const { return queue_.capacity(); }
    // Both SPSC branches reserve one ring slot; only the serialized producer uses this query.
    size_t available() const { return QueueCapacity - 1 - queue_.size(); }
    bool empty() const { return queue_.empty(); }
    bool full() const { return queue_.full(); }

    // Discard queued packets and restart the local sequence counter.
    void clear() {
        MotionPacket packet;
        while (queue_.try_pop(packet)) {
        }
        seq_counter_ = 0;
    }
};

template <size_t QueueCapacity = 16>
using CmdQueue = MotionPacketQueue<QueueCapacity>;

template <size_t QueueCapacity = 16>
using StatusQueue = MotionPacketQueue<QueueCapacity>;

// Command queue wrapper with serialized producers.
template <size_t QueueCapacity = 16>
class SerializedCmdQueue : public MotionPacketQueue<QueueCapacity> {
   public:
    bool push(const CmdPacket& packet) {
        return MotionPacketQueue<QueueCapacity>::push(packet);
    }
};

// Default queue type aliases
using CmdQueueType = SerializedCmdQueue<16>;
using StatusQueueType = StatusQueue<256>;

// Command item iterator for parsing payload
class CmdItemIterator {
   private:
    const uint8_t* payload_;
    size_t payload_len_;
    size_t offset_;

   public:
    CmdItemIterator(const uint8_t* payload, size_t payload_len)
        : payload_(payload), payload_len_(payload_len), offset_(0) {}

    // Get next item: returns true if item found, fills type/param_len/param_ptr
    bool next(uint16_t& type, uint16_t& param_len, const uint8_t*& param_ptr) {
        if (offset_ + sizeof(uint16_t) + sizeof(uint16_t) > payload_len_) {
            return false;  // No more items
        }

        // Read type (u16)
        std::memcpy(&type, payload_ + offset_, sizeof(uint16_t));
        offset_ += sizeof(uint16_t);

        // Read param_len (u16)
        std::memcpy(&param_len, payload_ + offset_, sizeof(uint16_t));
        offset_ += sizeof(uint16_t);

        // Check if param_len is valid
        if (offset_ + param_len > payload_len_) {
            return false;  // Invalid param_len
        }

        param_ptr = payload_ + offset_;
        offset_ += param_len;

        return true;
    }

    void reset() { offset_ = 0; }
    size_t getOffset() const { return offset_; }
};

// Status item iterator (same as command iterator)
using StatusItemIterator = CmdItemIterator;

}  // namespace zmc

#endif  // _ZMC_FB_CMD_QUEUE_HPP_

