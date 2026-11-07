#include "srv/SrvQcmd.hpp"
#include "SrvState.hpp"
#include <cstring>
#include "impl/fbt_impl_enum_def.h"
#include "fbmEnum.h"
// AI modification: GPT-6 (Codex), 2026-10-26. Delegate synchronous requests to qcmd.
// GPT-6 (Codex), 2026-10-27. Route commands and waits through the owning device slot.
// Model signature: GPT-6. Business state stays inside the service implementation.
// Parameter mirror getters added by GPT-6 Codex, 2026-10-12.
namespace zmc {
namespace srv {
namespace {
void copyAxisState(const AxisState& src, RpcAxisState& dst) {
    dst.axis_id = src.axis_id;
    dst.axis_state = src.axis_state;
    dst.error_id = src.error_id;
    dst.tick = src.tick;
    dst.act_pos = src.act_pos;
    dst.set_pos = src.set_pos;
}

void copyAxisState2(const AxisState2& src, RpcAxisState2& dst) {
    dst.axis_id = src.axis_id;
    dst.axis_state = src.axis_state;
    dst.error_id = src.error_id;
    dst.tick = src.tick;
    dst.act_pos = src.act_pos;
    dst.set_pos = src.set_pos;
    dst.set_vel = src.set_vel;
    dst.set_acc = src.set_acc;
    dst.set_position = src.set_position;
    dst.act_position = src.act_position;
}

void copyFlowCtrlConfigToNative(const RpcFlowCtrlConfig& src, FlowCtrlConfig& dst) {
    dst.period = src.period;
    dst.batchSize = src.batch_size;
    dst.verboseFlag = src.verbose_flag;
    dst.reserved[0] = src.reserved[0];
    dst.reserved[1] = src.reserved[1];
    dst.reserved[2] = src.reserved[2];
}

void copyFlowCtrlConfigToRpc(const FlowCtrlConfig& src, RpcFlowCtrlConfig& dst) {
    dst.period = src.period;
    dst.batch_size = src.batchSize;
    dst.verbose_flag = src.verboseFlag;
    dst.reserved[0] = src.reserved[0];
    dst.reserved[1] = src.reserved[1];
    dst.reserved[2] = src.reserved[2];
}

void copyFlowCtrlStatusToRpc(const FlowCtrlStatus& src, RpcFlowCtrlStatus& dst) {
    dst.period = src.period;
    dst.batch_size = src.batchSize;
    dst.verbose_flag = src.verboseFlag;
    // The wire keeps three reserved bytes; the progress only replaces former reserved[1..2].
    dst.reserved[0] = src.reserved;
    std::memcpy(&dst.reserved[1], &src.processed_seq, sizeof(uint16_t));
    dst.latency = src.latency;
    dst.axis_num = src.axis_num;
    dst.cycle_time = src.cycleTime;
}

int32_t toHomeStatus(int status) {
    switch (status) {
        case FB_STAT_UNACTIVATED:
        case FB_STAT_BUSY_ACTIVE:
            return 3;
        case FB_STAT_DONE:
            return 4;
        case FB_STAT_ERROR:
            return 5;
        case FB_STAT_ABORTED:
            return 6;
        case FB_STAT_IDLE:
        default:
            return 0;
    }
}

}  // namespace

SrvQcmd::SrvQcmd() {}
SrvQcmd::~SrvQcmd() {}

int SrvQcmd::ping(RpcSession s, uint32_t timeout_ms) {
    if (timeout_ms == 0U) return ZMC_ERR_MOTCMD_INVALID_ARG;
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->ping(timeout_ms, lock);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::poweron(RpcSession s, int32_t axis) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->powerOn(axis);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::poweroff(RpcSession s, int32_t axis) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->powerOff(axis);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::axisenable(RpcSession s, int32_t axis, bool enable) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->axisEnable(axis, enable);
}

// Read the mirror under the existing command serialization.
int SrvQcmd::getAxisParameter(RpcSession s, int32_t axis, uint16_t type, double& value) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->getAxisParameter(axis, type, value);
}

int SrvQcmd::getunits(RpcSession s, int32_t axis, double& units) {
    return getAxisParameter(s, axis, FBM_SET_NUM_DEN, units);
}

int SrvQcmd::getspeed(RpcSession s, int32_t axis, double& speed) {
    return getAxisParameter(s, axis, FBM_SET_SPEED, speed);
}

int SrvQcmd::getaccel(RpcSession s, int32_t axis, double& accel) {
    return getAxisParameter(s, axis, FBM_SET_ACCEL, accel);
}

int SrvQcmd::getdecel(RpcSession s, int32_t axis, double& decel) {
    return getAxisParameter(s, axis, FBM_SET_DECEL, decel);
}

int SrvQcmd::getjerk(RpcSession s, int32_t axis, double& jerk) {
    return getAxisParameter(s, axis, FBM_SET_JERK, jerk);
}

// Select and submit as one serialized request; do not ignore selection errors.
// The ratio payload carries num/den directly without a double round-trip.
int SrvQcmd::setunits_num_den(RpcSession s, int32_t axis, int32_t num, int32_t den) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->setUnitsNumDen(axis, num, den);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::setspeed(RpcSession s, int32_t axis, double speed) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->setSpeed(axis, speed);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::setaccel(RpcSession s, int32_t axis, double accel) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->setAccel(axis, accel);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::setdecel(RpcSession s, int32_t axis, double decel) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->setDecel(axis, decel);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::setjerk(RpcSession s, int32_t axis, double jerk) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->setJerk(axis, jerk);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::setfastdecel(RpcSession s, int32_t axis, double value) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->setFastDecel(axis, value);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::setspeedratio(RpcSession s, int32_t axis, double ratio) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->setSpeedRatio(axis, ratio);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::setdirection(RpcSession s, int32_t axis, int32_t dir) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->setDirection(axis, dir);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::setbuffermode(RpcSession s, int32_t axis, int32_t mode) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->setBufferMode(axis, mode);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::setmasteraxis(RpcSession s, int32_t axis, int32_t master_axis) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->setMasterAxis(axis, master_axis);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::stop(RpcSession s, int32_t axis) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->stop(axis);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::halt(RpcSession s, int32_t axis) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->halt(axis);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::reset(RpcSession s, int32_t axis) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->reset(axis);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::movesgabs(RpcSession s, int32_t axis, double pos) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->moveAbs(axis, pos);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::movesgrel(RpcSession s, int32_t axis, double pos) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->moveRel(axis, pos);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::moveadd(RpcSession s, int32_t axis, double pos) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->moveAdd(axis, pos);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::home(RpcSession s, int32_t axis, double pos) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->home(axis, pos);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::homing(RpcSession s, int32_t axis) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->homing(axis);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::gearin(RpcSession s, int32_t axis) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->gearIn(axis);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::gearout(RpcSession s, int32_t axis) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->gearOut(axis);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::setposition(RpcSession s, int32_t axis, double pos) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->setPosition(axis, pos);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::datum(RpcSession s, int32_t axis, int32_t mode) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->datum(axis, mode);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::movevelocity(RpcSession s, int32_t axis, int32_t dir) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->moveVelocity(axis, dir);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::forward(RpcSession s, int32_t axis) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->forward(axis);
}

// Select and submit as one serialized request; do not ignore selection errors.
int SrvQcmd::reverse(RpcSession s, int32_t axis) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->reverse(axis);
}

// Start SDO-configured homing on the explicit target axis.
int SrvQcmd::home_start(RpcSession s, int32_t axis, const RpcHomeSdoConfig& cfg) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    HomeSdoConfig native = {};
    native.mode = cfg.mode;
    native.offset = cfg.offset;
    native.velocity_fast = cfg.velocity_fast;
    native.velocity_slow = cfg.velocity_slow;
    native.acceleration = cfg.acceleration;
    native.timeout_ms = cfg.timeout_ms;
    return slot->cmdIf->homeSdo(axis, native);
}

// Return the established RPC home-status numbering.
int SrvQcmd::home_get_status(RpcSession s, int32_t axis, int32_t& status, int32_t& error_id) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    int native_status = FB_STAT_IDLE;
    const int ret = slot->cmdIf->getHomeStatus(axis, native_status, error_id);
    status = toHomeStatus(native_status);
    return ret;
}

// Preserve explicit base selection for existing script semantics.
int SrvQcmd::base(RpcSession s, int32_t axis) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->setBaseAxis(axis);
}

// Select the supplied axes before submitting the group move.
int SrvQcmd::moveabs(RpcSession s, int32_t len, const int32_t* axes, const double* pos) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->multiMoveAbs(len, axes, pos);
}

// Select the supplied axes before submitting the group move.
int SrvQcmd::moverel(RpcSession s, int32_t len, const int32_t* axes, const double* pos) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->multiMoveRel(len, axes, pos);
}

// Select the supplied two axes before submitting the circular move.
int SrvQcmd::movecirc(RpcSession s, int32_t len, const int32_t* axes, double end1, double end2,
                      double centre1, double centre2, int32_t direction) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->moveCirc(len, axes, end1, end2, centre1, centre2, direction);
}

// Select the supplied two axes before submitting the absolute circular move.
int SrvQcmd::movecircabs(RpcSession s, int32_t len, const int32_t* axes, double end1, double end2,
                         double centre1, double centre2, int32_t direction) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->moveCircAbs(len, axes, end1, end2, centre1, centre2, direction);
}

// Select the supplied two axes before submitting the three-point circular move.
int SrvQcmd::movecirc2(RpcSession s, int32_t len, const int32_t* axes, double mid1, double mid2,
                       double end1, double end2) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->moveCirc2(len, axes, mid1, mid2, end1, end2);
}

// Select the supplied two axes before submitting the absolute three-point circular move.
int SrvQcmd::movecirc2abs(RpcSession s, int32_t len, const int32_t* axes, double mid1, double mid2,
                          double end1, double end2) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->moveCirc2Abs(len, axes, mid1, mid2, end1, end2);
}

// Reuse the existing typed command or script path.
int SrvQcmd::groupselect(RpcSession s, int32_t index) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->groupSelect(index);
}

// Reuse the existing typed command or script path.
int SrvQcmd::groupsetaxes(RpcSession s, int32_t len, const int32_t* axes) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->groupSetAxes(len, axes);
}

// Reuse the existing typed command or script path.
int SrvQcmd::groupenable(RpcSession s, int32_t len, const int32_t* axes) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->groupEnable(len, axes);
}

// Reuse the existing typed command or script path.
int SrvQcmd::groupdisable(RpcSession s, int32_t len, const int32_t* axes) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->groupDisable(len, axes);
}

// Reuse the existing typed command or script path.
int SrvQcmd::groupreset(RpcSession s, int32_t len, const int32_t* axes) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->groupReset(len, axes);
}

// Reuse the existing typed command or script path.
int SrvQcmd::waitticks(RpcSession s, uint32_t ticks, int32_t axis, uint32_t timeout_ms,
                       uint32_t poll_ms) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->waitTicks(ticks, axis, timeout_ms, poll_ms);
}

// Reuse the existing typed command or script path.
int SrvQcmd::waitseconds(RpcSession s, double seconds, int32_t axis, uint32_t timeout_ms,
                         uint32_t poll_ms) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->waitSeconds(seconds, axis, timeout_ms, poll_ms);
}

// Block on the shared condition variable until the axis reaches standstill or ErrorStop.
// GLM-5.3 (opencode), 2026-10-23.
int SrvQcmd::waitaxisidle(RpcSession s, int32_t axis, uint32_t timeout_ms) {
    if (timeout_ms == 0U) return ZMC_ERR_MOTCMD_INVALID_ARG;
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->waitAxisIdleSync(axis, timeout_ms, lock);
}

// Reuse the existing typed command or script path.
int SrvQcmd::executecommand(RpcSession s, const std::string& text) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->executeCommand(text.c_str());
}

// Reuse the existing typed command or script path.
int SrvQcmd::flush(RpcSession s) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->flush();
}

// Configure the one per-device timeout used by synchronous numeric and IO reads.
int SrvQcmd::set_sync_read_timeout(RpcSession s, uint32_t timeout_ms) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    slot->cmdIf->setSyncReadTimeout(timeout_ms);
    return 0;
}

// Return the configured timeout for synchronous numeric and IO reads.
int SrvQcmd::get_sync_read_timeout(RpcSession s, uint32_t& timeout_ms) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    timeout_ms = slot->cmdIf->getSyncReadTimeout();
    return 0;
}

// Numeric reads share the existing request and result ownership. GPT-6 Codex.
int SrvQcmd::read_parameter(RpcSession s, int32_t axis, int32_t parameter_number,
                            double& value) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->readParameter(axis, parameter_number, value, lock);
}

// Writes share the read request's ACK, timeout and cleanup ownership. GPT-6 Codex.
int SrvQcmd::write_parameter(RpcSession s, int32_t axis, int32_t parameter_number,
                             double value) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->writeParameter(axis, parameter_number, value, lock);
}

int SrvQcmd::read_di_sync(RpcSession s, uint32_t io, bool& value) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->readDiSync(io, value, lock);
}

int SrvQcmd::read_do_sync(RpcSession s, uint32_t io, bool& value) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->readDoSync(io, value, lock);
}

int SrvQcmd::read_ai_sync(RpcSession s, uint32_t io, uint32_t& value) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->readAiSync(io, value, lock);
}

int SrvQcmd::read_ao_sync(RpcSession s, uint32_t io, uint32_t& value) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->readAoSync(io, value, lock);
}

// Submit the existing asynchronous IO command.
int SrvQcmd::read_di(RpcSession s, uint32_t io) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->requestReadDi(io);
}

// Submit the existing asynchronous IO command.
int SrvQcmd::read_di_multi(RpcSession s, uint32_t first, uint32_t last) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->requestReadDiMulti(first, last);
}

// Submit the existing asynchronous IO command.
int SrvQcmd::read_do(RpcSession s, uint32_t io) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->requestReadDo(io);
}

// Submit the existing asynchronous IO command.
int SrvQcmd::read_do_multi(RpcSession s, uint32_t first, uint32_t last) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->requestReadDoMulti(first, last);
}

// Submit the existing asynchronous IO command.
int SrvQcmd::write_do(RpcSession s, uint32_t io, bool value) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->writeDo(io, value);
}

// Submit the existing asynchronous IO command.
int SrvQcmd::write_do_multi(RpcSession s, uint32_t first, uint32_t last, uint32_t value) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->writeDoMulti(first, last, value);
}

// Submit the existing asynchronous IO command.
int SrvQcmd::read_ai(RpcSession s, uint32_t io) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->requestReadAi(io);
}

// Submit the existing asynchronous IO command.
int SrvQcmd::read_ao(RpcSession s, uint32_t io) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->requestReadAo(io);
}

// Submit the existing asynchronous IO command.
int SrvQcmd::write_ao(RpcSession s, uint32_t io, uint32_t value) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->writeAo(io, value);
}

// Submit the existing asynchronous IO command.
int SrvQcmd::write_virtual_di(RpcSession s, uint32_t io, bool value) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->writeVirtualDi(io, value);
}

// Submit the existing asynchronous IO command.
int SrvQcmd::write_virtual_ai(RpcSession s, uint32_t io, uint32_t value) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    return slot->cmdIf->writeVirtualAi(io, value);
}

// Preserve each transport's existing refresh and error contract.
int SrvQcmd::getaxisstate(RpcSession s, int32_t axis, RpcAxisState& out, bool refresh) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    if (refresh) slot->cmdIf->updateStatusCache();
    AxisState value = {};
    const int ret = slot->cmdIf->getAxisState(axis, value);
    if (ret == 0) copyAxisState(value, out);
    return ret;
}

// Preserve each transport's existing refresh and error contract.
int SrvQcmd::getaxisstate2(RpcSession s, int32_t axis, RpcAxisState2& out, bool refresh) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    if (refresh) slot->cmdIf->updateStatusCache();
    AxisState2 value = {};
    const int ret = slot->cmdIf->getAxisState2(axis, value);
    if (ret == 0) copyAxisState2(value, out);
    return ret;
}

// Copy the existing persistent command result snapshot.
int SrvQcmd::getcmdresults(RpcSession s, std::vector<RpcCmdResultInfo>& out, bool refresh) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    if (refresh) slot->cmdIf->updateStatusCache();
    // The public alias is the shared result type, so no field conversion is needed.
    return slot->cmdIf->getCmdResults(out);
}

// Read flow status without introducing another cache.
int SrvQcmd::getflowstatus(RpcSession s, RpcFlowCtrlStatus& out, bool refresh) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    if (refresh) slot->cmdIf->updateStatusCache();
    FlowCtrlStatus value = {};
    const int ret = slot->cmdIf->getFlowStatus(value);
    if (ret == 0) copyFlowCtrlStatusToRpc(value, out);
    return ret;
}

// Read the existing flow configuration.
int SrvQcmd::getflowconfig(RpcSession s, RpcFlowCtrlConfig& out) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    FlowCtrlConfig value = {};
    const int ret = slot->cmdIf->getFlowConfig(value);
    if (ret == 0) copyFlowCtrlConfigToRpc(value, out);
    return ret;
}

// Set the existing flow configuration.
int SrvQcmd::setflowconfig(RpcSession s, const RpcFlowCtrlConfig& cfg) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    FlowCtrlConfig native = {};
    copyFlowCtrlConfigToNative(cfg, native);
    return slot->cmdIf->setFlowConfig(native);
}

// Read the persistent IO result snapshot.
int SrvQcmd::io_results(RpcSession s, std::vector<CmdResult>& out) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    slot->cmdIf->updateStatusCache();
    return slot->cmdIf->getIoResults(out);
}

// Clear only the existing requested mirror scope.
int SrvQcmd::dashboard_clear(RpcSession s) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    slot->cmdIf->clearDashboardCache();
    return 0;
}

// Clear only the existing requested mirror scope.
int SrvQcmd::cache_reset(RpcSession s) {
    Impl::DeviceSlot* slot;
    std::unique_lock<std::mutex> lock;
    const int auth = impl_->lockSession(s, slot, lock);
    if (auth != 0) return auth;
    slot->cmdIf->resetCache();
    slot->cmdIf->initAxisInfo(slot->motion.axis_count());
    return 0;
}

// Keep status draining, ACK and script gates on the same instance.
int SrvQcmd::runOnce(size_t devIdx) {
    Impl::DeviceSlot& slot = impl_->deviceSlot[devIdx];
    std::unique_lock<std::mutex> lock(slot.mutex, std::try_to_lock);
    if (!lock.owns_lock() || !slot.cmdIf) return 0;
    return slot.cmdIf->runOnce();
}

}  // namespace srv
}  // namespace zmc
