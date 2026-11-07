/*
 * @ArtifactStatus: draft
 * @ArtifactStatusDate: 2026-08-26
 * @ArtifactStatusAuthority: GLM-5.2
 */
#ifndef ZMC_MOTION_BUFFER_TYPES_H_
#define ZMC_MOTION_BUFFER_TYPES_H_

#include <stdint.h>

namespace zmc {

enum MotionBufferError {
    // ZMC_ERR_MOTBUF_ will moved to zmc_errorcode_def.h
    MOTION_BUFFER_OK = 0,
    ZMC_ERR_MOTBUF_INVALID_ARGUMENT = 17900,
    ZMC_ERR_MOTBUF_NOT_INITIALIZED,
    ZMC_ERR_MOTBUF_INVALID_STATE,
    ZMC_ERR_MOTBUF_FULL,
    ZMC_ERR_MOTBUF_INITIAL_STATE,
    ZMC_ERR_MOTBUF_PLAN_FAILED,
    ZMC_ERR_MOTBUF_NOT_SUPPORTED,
};

enum MotionJunctionMode { MOTION_JUNCTION_STOP_AT_CORNER, MOTION_JUNCTION_BLEND };

// Select the future override dimension without exposing planner internals.
enum MotionOverrideType : uint8_t {
    MOTION_OVERRIDE_VELOCITY,
    MOTION_OVERRIDE_ACCELERATION,
    MOTION_OVERRIDE_JERK
};

struct MotionFrame {
    double *position;
    double *velocity;
    double *acceleration;
};

enum MotionAxisState {
    MOTION_AXIS_DISABLE,
    MOTION_AXIS_STANDSTILL,
    MOTION_AXIS_MOVING,
    MOTION_AXIS_STOPPING,
    MOTION_AXIS_ERRORSTOP
};

enum MotionAxisGroupState {
    MOTION_AXIS_GROUP_DISABLE,
    MOTION_AXIS_GROUP_STANDSTILL,
    MOTION_AXIS_GROUP_MOVING,
    MOTION_AXIS_GROUP_STOPPING,
    MOTION_AXIS_GROUP_ERRORSTOP
};

struct MotionAxInput {
    uint8_t idx;
    MotionAxisState state;
    uint16_t errorCode;
    double actPos;
};

struct MotionAxOutput {
    uint8_t idx;
    // Bound-group runtime state for this axis; not the external single-axis input state.
    MotionAxisGroupState state;
    // Bound-group latched error code; 0 when the axis has no group or no error.
    uint16_t errorCode;
    double pos;
    double vel;
    double acc;
};

}  // namespace zmc

#endif  // ZMC_MOTION_BUFFER_TYPES_H_
