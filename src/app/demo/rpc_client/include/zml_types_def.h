#ifndef ZML_TYPES_DEF_H
#define ZML_TYPES_DEF_H

#include <stdint.h>

/* Drive-home SDO configuration accepted by the RPC API. */
typedef struct ZML_HomeSdoConfig {
    int32_t mode;
    double offset;
    double velocity_fast;
    double velocity_slow;
    double acceleration;
    int32_t timeout_ms;
} ZML_HomeSdoConfig;

#endif /* ZML_TYPES_DEF_H */
