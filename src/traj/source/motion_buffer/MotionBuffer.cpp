/*
 * @ArtifactStatus: garbage
 * @ArtifactStatusDate: 2026-08-25
 * @ArtifactStatusAuthority: user
 */
#include "MotionBuffer.h"

#include "MotionBufferImpl.h"

namespace zmc {

MotionBuffer::MotionBuffer() : impl_(new Impl()) {}

MotionBuffer::~MotionBuffer() { delete impl_; }

int MotionBuffer::init(uint8_t axis_count) {
    if (!impl_) {
        return ZMC_ERR_MOTBUF_PLAN_FAILED;
    }
    return impl_->init(axis_count);
}

void MotionBuffer::set_cycle_time(double cycle_time) {
    if (impl_) {
        impl_->set_cycle_time(cycle_time);
    }
}

int MotionBuffer::set_current_frame(const MotionFrame &current_frame) {
    return impl_ ? impl_->set_current_frame(current_frame) : ZMC_ERR_MOTBUF_NOT_INITIALIZED;
}

int MotionBuffer::set_base(const uint8_t *axes, uint8_t count) {
    return impl_ ? impl_->set_base(axes, count) : ZMC_ERR_MOTBUF_NOT_INITIALIZED;
}

int MotionBuffer::set_junction_mode(MotionJunctionMode mode) {
    return impl_ ? impl_->set_junction_mode(mode) : ZMC_ERR_MOTBUF_NOT_INITIALIZED;
}

int MotionBuffer::set_override(MotionOverrideType type, double factor) {
    return impl_ && impl_->initialized() ? impl_->set_override(type, factor)
                                        : ZMC_ERR_MOTBUF_NOT_INITIALIZED;
}

void MotionBuffer::setInput(MotionAxInput *input, int len) {
    if (impl_) {
        impl_->set_input(input, len);
    }
}

int MotionBuffer::push_line(const double *target, MotionCoordinateMode coordinate_mode,
                            const MotionLimits &limits) {
    return impl_ ? impl_->push_line(target, coordinate_mode, limits)
                 : ZMC_ERR_MOTBUF_NOT_INITIALIZED;
}

int MotionBuffer::push_arc_through(const double *through_point, const double *end_point,
                                   MotionCoordinateMode coordinate_mode,
                                   const MotionLimits &limits) {
    return impl_ ? impl_->push_arc_through(through_point, end_point, coordinate_mode, limits)
                 : ZMC_ERR_MOTBUF_NOT_INITIALIZED;
}

int MotionBuffer::push_arc_center(const double *center, const double *end_point,
                                  MotionArcDirection direction,
                                  MotionCoordinateMode coordinate_mode,
                                  const MotionLimits &limits) {
    return impl_ ? impl_->push_arc_center(center, end_point, direction, coordinate_mode, limits)
                 : ZMC_ERR_MOTBUF_NOT_INITIALIZED;
}

void MotionBuffer::runCycle() {
    if (impl_) {
        impl_->runCycle();
    }
}

void MotionBuffer::getOutput(MotionAxOutput *output, int len) const {
    if (impl_) {
        impl_->get_output(output, len);
    }
}

void MotionBuffer::reset() {
    if (impl_) {
        impl_->reset();
    }
}

MotionAxisGroupState MotionBuffer::state(uint8_t axis_index) const {
    return impl_ ? impl_->state(axis_index) : MOTION_AXIS_GROUP_DISABLE;
}

uint8_t MotionBuffer::axis_count() const { return impl_ ? impl_->axis_count : 0; }

uint8_t MotionBuffer::base_count() const { return impl_ ? impl_->base_count() : 0; }

int32_t MotionBuffer::queued_count() const { return impl_ ? impl_->queued_count() : 0; }

int32_t MotionBuffer::free_count() const { return impl_ ? impl_->free_count() : 0; }

}  // namespace zmc
