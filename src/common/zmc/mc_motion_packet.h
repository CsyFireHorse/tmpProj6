#ifndef __MC_MOTION_PACKET_H
#define __MC_MOTION_PACKET_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MC_CMD_PACKET_PAYLOAD_MAX 508u

/* Shared fixed-size motion packet used by command and status queues. */
typedef struct MCMotionPacket {
    uint16_t seq;
    uint16_t payload_len;
    uint8_t payload[MC_CMD_PACKET_PAYLOAD_MAX];
} MCMotionPacket;

/* Command envelope is an API alias of the shared motion packet. */
typedef MCMotionPacket MCCmdPacket;

/* Status envelope reuses the same wire layout as the command envelope. */
typedef MCMotionPacket MCStatusPacket;

#ifdef __cplusplus
}
#endif

#endif /* __MC_MOTION_PACKET_H */
