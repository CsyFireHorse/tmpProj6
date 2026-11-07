#include "fb_cpp_def.h"
#include "fbt_axis_base.h"

#include "fbt_fb_base.hpp"
#include "math.h"
#include "zmc_fb_predef.h"

namespace zmc {
// FbtEnBase
#define POWER_TIMEOUT (ZMC_FB_POWER_TIMEOUT_COUNT)
struct FbtPowerImpl : public FbtEnBase<FB_TYPE_POWER> {
    uint16_t _cnt = 0;
};

FbtPower::FbtPower() {
    all_zero_init<FbtPowerStruct>(*this);
    this->pImpl = new FbtPowerImpl();
}

FbtPower::~FbtPower() { delete static_cast<FbtPowerImpl*>(pImpl); }

inline void FbtPower_update_stat(FbtPower& pow, FbStatEnum2 sm) {
    static_cast<FbtPowerImpl*>(pow.pImpl)->fbStat = (FbStatEnum)sm;
    switch (sm) {
        case FB_STAT_UNABLE:
            pow.Busy = false;
            pow.Error = false;
            pow.ErrorID = ZMC_NO_ERROR;
            break;
        case FB_STAT_BUSYING:
            pow.Busy = true;
            pow.Error = false;
            pow.ErrorID = ZMC_NO_ERROR;
            break;
        case FB_STAT_VALID:
            pow.Busy = true;
            pow.Error = false;
            pow.ErrorID = ZMC_NO_ERROR;
            break;
        case FB_STAT_EN_ERROR:
            pow.Busy = false;
            pow.Error = true;
            // pow.ErrorID = ZMC_NO_ERROR;
            break;
        default:
            break;
    }
}

inline void FbtPower_onError(FbtPower& pow, MC_ERROR_CODE errCode) {
    static_cast<FbtPowerImpl*>(pow.pImpl)->fbStat = (FbStatEnum)FB_STAT_EN_ERROR;
    pow.Busy = false;
    pow.Error = true;
    pow.ErrorID = errCode;
    ;
}

// error : memory error (ZMC_ERR_P_TIMEOUT) & no memory error (ZMC_ERR_fbpower_when_axis_error)
// no memory error will be  cleared auto
// enable is low, cant clear error
// switch bRegulatorOn to close,  clear timeout error
void FbtPower::runCycle() {
    if (!Axis || !get_axis_impl(Axis)->is_axis_valid()) {
        bRegulatorRealState = false;
        bDriveStartRealState = false;
        FbtPower_onError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
        return;
    }
    auto& ax = get_axis_impl_ref(Axis);
    bRegulatorRealState = ax.regulatorRealState;
    bDriveStartRealState = ax.driveStartRealState;
    Status = bRegulatorRealState & bDriveStartRealState;
    if (!Enable) {
        if (!bRegulatorOn && !bDriveStart) {
            // It dont keep same with codesys, but is`s easy to understand.
            if (ax.axisStat != mcDisabled && ax.axisStat != mcErrorStop)
                ax.setAxisState(mcDisabled);
            ax.cmd = AXIS_CMD_POWEROFF;
        }
        return FbtPower_update_stat(*this, FB_STAT_UNABLE);
    }
    ax.bRegulatorOn = bRegulatorOn;
    ax.bDriveStart = bDriveStart;
    auto& _cnt = static_cast<FbtPowerImpl*>(pImpl)->_cnt;
    if (bRegulatorOn && bDriveStart) {
        const MC_ERROR_CODE communication_error = ax.getCommunicationErrorId();
        if (ax.axisStat == mcErrorStop && communication_error != ZMC_NO_ERROR) {
            return FbtPower_onError(*this, communication_error);
        }
        if (ax.axisStat == mcErrorStop && ax.driveError) {
            return FbtPower_onError(*this, ZMC_ERR_P_WHEN_DRIVE_ERROR);
        }
        if (!Status) {
            if (!bRegulatorRealState && (ax.axisStat == mcStandstill || isAxisQuickstopping(ax)))
                ax.setAxisState(mcDisabled);
            else if (bRegulatorRealState && !bDriveStartRealState && ax.axisStat == mcStandstill) {
                ax.setAxisState(mcStopping, AXIS_STATE_DETAIL_QUICK_STOP);
            }

            if (_cnt++ > POWER_TIMEOUT) {
                FbtPower_onError(*this, ZMC_ERR_P_TIMEOUT);
                ax.cmd = AXIS_CMD_POWEROFF;
                return;
            } else {
                FbtPower_update_stat(*this, FB_STAT_BUSYING);
                ax.cmd = AXIS_CMD_POWERON;
            }
        } else {
            if (ax.axisStat == mcDisabled || isAxisQuickstopping(ax)) {
                ax.setAxisState(mcStandstill);
            }
            _cnt = 0;
            ax.cmd = AXIS_CMD_POWERON;
            FbtPower_update_stat(*this, FB_STAT_VALID);
        }
    } else if (bRegulatorOn && !bDriveStart) {
        if (bRegulatorRealState && !bDriveStartRealState) {
            if (ax.axisStat != mcDisabled && ax.axisStat != mcErrorStop &&
                ax.axisStat != mcStopping) {
                ax.setAxisState(mcStopping, AXIS_STATE_DETAIL_QUICK_STOP);
            }
        }
        ax.cmd = AXIS_CMD_QUICKSTOP;
        FbtPower_update_stat(*this, FB_STAT_BUSYING);
        _cnt = 0;
    } else {
        if (!bRegulatorRealState) {
            if (ax.axisStat == mcStandstill || isAxisQuickstopping(ax)) {
                ax.setAxisState(mcDisabled);
            }
        }
        FbtPower_update_stat(*this, FB_STAT_BUSYING);
        if (ax.cmd != AXIS_CMD_RESET) ax.cmd = AXIS_CMD_POWEROFF;
        _cnt = 0;
    }
}

FbtReadActualPosition::FbtReadActualPosition() {
    all_zero_init<FbtReadActualPositionStruct>(*this);
}
FbtReadActualPosition::~FbtReadActualPosition() {}
void FbtReadActualPosition::runCycle() {
    if (Enable) {
        if (Axis) {
            auto& ax = get_axis_impl_ref(Axis);
            Position = ax.getModuloActPos();
            synEnFbStat(*this, FB_STAT_VALID);
        } else {
            synEnFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
        }
    } else {
        synEnFbStat(*this, FB_STAT_UNABLE);
    }
}

FbtReadAxisError::FbtReadAxisError() { all_zero_init<FbtReadAxisErrorStruct>(*this); }
FbtReadAxisError::~FbtReadAxisError() {}
void FbtReadAxisError::runCycle() {
    if (Enable) {
        if (Axis) {
            auto& ax = get_axis_impl_ref(Axis);
            this->AxisError = ax.dwErrorID != 0;
            this->AxisErrorID = static_cast<MC_ERROR_CODE>(ax.dwErrorID);
            this->SWEndSwitchActive = ax.usiSWEndSwitchState;
            synEnFbStat(*this, FB_STAT_VALID);
        } else {
            synEnFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
        }
    } else {
        synEnFbStat(*this, FB_STAT_UNABLE);
    }
}

FbtReadActualVelocity::FbtReadActualVelocity() {
    all_zero_init<FbtReadActualVelocityStruct>(*this);
}
FbtReadActualVelocity::~FbtReadActualVelocity() {}
void FbtReadActualVelocity::runCycle() {
    if (Enable) {
        if (Axis) {
            auto& ax = get_axis_impl_ref(Axis);
            Velocity = ax.fActVelocity;
            synEnFbStat(*this, FB_STAT_VALID);
        } else {
            synEnFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
        }
    } else {
        synEnFbStat(*this, FB_STAT_UNABLE);
    }
}

FbtReadStatus::FbtReadStatus() { all_zero_init<FbtReadStatusStruct>(*this); }
FbtReadStatus::~FbtReadStatus() {}
void FbtReadStatus::runCycle() {
    if (Enable) {
        if (Axis) {
            auto& ax = get_axis_impl_ref(Axis);
            Disabled = ax.axisStat == mcDisabled;
            Standstill = ax.axisStat == mcStandstill;
            DiscreteMotion = ax.axisStat == mcDiscreteMotion;
            ContinuousMotion = ax.axisStat == mcContinuousMotion;
            SynchronizedMotion = ax.axisStat == mcSynchronizedMotion;
            Stopping = ax.axisStat == mcStopping;
            Homing = ax.axisStat == mcHoming;
            ErrorStop = ax.axisStat == mcErrorStop;

            ConstantVelocity = ax.ConstantVelocity;
            Accelerating = ax.Accelerating;
            Decelerating = ax.Decelerating;
            synEnFbStat(*this, FB_STAT_VALID);
        } else {
            synEnFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
        }
    } else {
        synEnFbStat(*this, FB_STAT_UNABLE);
    }
}

FbtReadSetValues::FbtReadSetValues() { all_zero_init<FbtReadSetValuesStruct>(*this); }
FbtReadSetValues::~FbtReadSetValues() {}
void FbtReadSetValues::runCycle() {
    if (Enable) {
        if (Axis) {
            auto& ax = get_axis_impl_ref(Axis);
            Position = ax.fSetPosition;
            Velocity = ax.fSetVelocity;
            Acceleration = ax.fSetAcceleration;
            Jerk = 0;
            synEnFbStat(*this, FB_STAT_VALID);
        } else {
            synEnFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
        }
    } else {
        synEnFbStat(*this, FB_STAT_UNABLE);
    }
}

FbtReadActualValues::FbtReadActualValues() { all_zero_init<FbtReadActualValuesStruct>(*this); }
FbtReadActualValues::~FbtReadActualValues() {}
void FbtReadActualValues::runCycle() {
    if (Enable) {
        if (Axis) {
            auto& ax = get_axis_impl_ref(Axis);
            Position = ax.fActPosition;
            Velocity = ax.fActVelocity;
            Acceleration = ax.fActAcceleration;
            Jerk = 0;
            synEnFbStat(*this, FB_STAT_VALID);
        } else {
            synEnFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
        }
    } else {
        synEnFbStat(*this, FB_STAT_UNABLE);
    }
}
FbtReadActualDriveValues::FbtReadActualDriveValues() {
    all_zero_init<FbtReadActualDriveValuesStruct>(*this);
}
FbtReadActualDriveValues::~FbtReadActualDriveValues() {}
void FbtReadActualDriveValues::runCycle() {
    if (Enable) {
        if (Axis) {
            auto& ax = get_axis_impl_ref(Axis);
            Position = ax.diActPosition;
            Velocity = ax.diActVelocity;
            Acceleration = ax.diActAcceleration;
            Jerk = 0;
            synEnFbStat(*this, FB_STAT_VALID);
        } else {
            synEnFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
        }
    } else {
        synEnFbStat(*this, FB_STAT_UNABLE);
    }
}

FbtReadSetDriveValues::FbtReadSetDriveValues() {
    all_zero_init<FbtReadSetDriveValuesStruct>(*this);
}
FbtReadSetDriveValues::~FbtReadSetDriveValues() {}
void FbtReadSetDriveValues::runCycle() {
    if (Enable) {
        if (Axis) {
            auto& ax = get_axis_impl_ref(Axis);
            Position = ax.diSetPosition;
            Velocity = ax.diSetVelocity;
            Acceleration = ax.diSetAcceleration;
            Jerk = 0;
            synEnFbStat(*this, FB_STAT_VALID);
        } else {
            synEnFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
        }
    } else {
        synEnFbStat(*this, FB_STAT_UNABLE);
    }
}

FbtReadActualTorque::FbtReadActualTorque() { all_zero_init<FbtReadActualTorqueStruct>(*this); }
FbtReadActualTorque::~FbtReadActualTorque() {}
void FbtReadActualTorque::runCycle() {
    if (Enable) {
        if (Axis) {
            auto& ax = get_axis_impl_ref(Axis);
            Torque = ax.fActTorque;
            synEnFbStat(*this, FB_STAT_VALID);
        } else {
            synEnFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
        }
    } else {
        synEnFbStat(*this, FB_STAT_UNABLE);
    }
}

FbtReadSetPosition::FbtReadSetPosition() { all_zero_init<FbtReadSetPositionStruct>(*this); }
FbtReadSetPosition::~FbtReadSetPosition() {}
void FbtReadSetPosition::runCycle() {
    if (Enable) {
        if (Axis) {
            auto& ax = get_axis_impl_ref(Axis);
            Position = ax.calc_modulo(ax.fSetPosition);
            synEnFbStat(*this, FB_STAT_VALID);
        } else {
            synEnFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
        }
    } else {
        synEnFbStat(*this, FB_STAT_UNABLE);
    }
}

// Ring buffer for storing historical position data
template <typename T, int SZ>
class TrackingErrorRingBuffer {
   private:
    T data[SZ];
    uint16_t writeIndex = 0;
    uint16_t count = 0;

   public:
    TrackingErrorRingBuffer() { clear(); }

    // Push new element to ring buffer
    void push(const T& value) {
        data[writeIndex] = value;
        writeIndex = (writeIndex + 1) % SZ;
        if (count < SZ) {
            count++;
        }
    }

    // Access historical elements: [0] = current (latest), [1] = previous cycle, [2] = 2 cycles
    // ago, etc.
    T operator[](int index) const {
        if (index < 0 || index >= count) {
            return T{};  // Return default value for invalid index
        }
        // For index=0: get latest (current), for index=1: get previous, etc.
        int readIndex = (writeIndex - 1 - index + SZ) % SZ;
        return data[readIndex];
    }

    // Get current size (number of elements stored)
    int size() const { return count; }

    // Get buffer capacity
    constexpr int capacity() const { return SZ; }

    // Clear buffer
    void clear() {
        writeIndex = 0;
        count = 0;
        for (int i = 0; i < SZ; i++) {
            data[i] = T{};
        }
    }

    // Check if buffer has enough history for given cycles
    bool hasEnoughHistory(int cycles) const { return cycles <= count; }
};

class FbtImplGetTrackingError : public FbtEnBase<FB_TYPE_GETTRACKINGERROR> {
   public:
    FbtGetTrackingErrorStruct* pParam = nullptr;

    DWORD maxLastTime = 0;
    double maxError = 0;
#define N (4)
    TrackingErrorRingBuffer<LREAL, N> posHistory;

   public:
    void runCycle() {
        auto& param = *pParam;

        if (!param.Enable) {
            reset();
            param.Valid = false;
            param.Busy = false;
            param.Error = false;
            param.ErrorID = ZMC_NO_ERROR;
            param.ActTrackingError = 0;
            param.MaxTrackingError = 0;
            return;
        }

        if (!param.Axis) {
            param.Error = true;
            param.ErrorID = ZMC_AXIS_IS_NO_AXIS_REF;
            param.Valid = false;
            param.Busy = false;
            return;
        }

        auto& ax = get_axis_impl_ref(param.Axis);
        param.Busy = true;
        param.Error = false;
        param.ErrorID = ZMC_NO_ERROR;

        // Push current fSavePosition to ring buffer
        posHistory.push(ax.fSavePosition);

        // Get compensated set position based on DeadTimeCycles
        double compensatedSetPos;

        if (param.DeadTimeCycles < N) {
            // Check if we have enough history
            if (!posHistory.hasEnoughHistory(param.DeadTimeCycles)) {
                param.Valid = false;
                return;
            }
            compensatedSetPos = posHistory[param.DeadTimeCycles];
        } else {
            // Invalid DeadTimeCycles value
            param.Error = true;
            param.ErrorID = ZMC_AXIS_IS_NO_AXIS_REF;  // Use appropriate error code
            param.Valid = false;
            param.Busy = false;
            return;
        }

        // Calculate current tracking error (absolute difference between actual and compensated set
        // position)
        double currentError = fabs(ax.fActPosition - compensatedSetPos);
        param.ActTrackingError = currentError;

        // Update maximum tracking error
        if (currentError > maxError) {
            maxError = currentError;
            param.MaxTrackingError = maxError;
            param.TimeAtMax = param.TimeStamp;
        }

        param.Valid = true;
    }

    void reset() {
        maxError = 0;
        maxLastTime = 0;
        posHistory.clear();
    }
};

FbtGetTrackingError::FbtGetTrackingError() {
    make_impl<FbtImplGetTrackingError, FbtGetTrackingErrorStruct>(*this);
    DeadTimeCycles = 2;
}

FbtGetTrackingError::~FbtGetTrackingError() {
    if (pImpl) {
        delete static_cast<FbtImplGetTrackingError*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtGetTrackingError::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplGetTrackingError*>(pImpl);
        impl->runCycle();
    }
}

FbtGetTravelledDistance::FbtGetTravelledDistance() {}
FbtGetTravelledDistance::~FbtGetTravelledDistance() {}
void FbtGetTravelledDistance::runCycle() {}

class FbtImplGetMaxSetAccDec : public FbtEnBase<FB_TYPE_GETMAXSETACCDEC> {
   public:
    FbtGetMaxSetAccDecStruct* pParam;
    double maxAcc = 0;
    DWORD lastTime = 0;

   public:
    void runCycle() {
        auto& param = *pParam;

        if (!param.Enable) {
            reset();
            param.Valid = false;
            param.Busy = false;
            param.Error = false;
            param.ErrorID = ZMC_NO_ERROR;
            return;
        }

        if (!param.Axis) {
            param.Error = true;
            param.ErrorID = ZMC_AXIS_IS_NO_AXIS_REF;
            param.Valid = false;
            param.Busy = false;
            return;
        }

        auto& ax = get_axis_impl_ref(param.Axis);
        param.Busy = true;
        param.Error = false;
        param.ErrorID = ZMC_NO_ERROR;

        double curAbsAcc = ax.fSetAcceleration > 0 ? ax.fSetAcceleration : -ax.fSetAcceleration;
        if (curAbsAcc > maxAcc) {
            maxAcc = curAbsAcc;
            param.MaxAcceleration = maxAcc;
            param.TimeAtMax = param.TimeStamp;
            param.Valid = true;
        }
    }

    void reset() {
        maxAcc = 0;
        lastTime = 0;
    }
};

FbtGetMaxSetAccDec::FbtGetMaxSetAccDec() {
    make_impl<FbtImplGetMaxSetAccDec, FbtGetMaxSetAccDecStruct>(*this);
}

FbtGetMaxSetAccDec::~FbtGetMaxSetAccDec() {
    if (pImpl) {
        delete static_cast<FbtImplGetMaxSetAccDec*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtGetMaxSetAccDec::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplGetMaxSetAccDec*>(pImpl);
        impl->runCycle();
    }
}

class FbtImplGetMaxSetVelocity : public FbtEnBase<FB_TYPE_GETMAXSETVELOCITY> {
   public:
    FbtGetMaxSetVelocityStruct* pParam = nullptr;
    double maxVel = 0;
    DWORD lastTime = 0;

    void runCycle() {
        auto& param = *pParam;

        if (!param.Enable) {
            maxVel = 0;
            param.Valid = false;
            param.Busy = false;
            param.Error = false;
            param.ErrorID = ZMC_NO_ERROR;
            return;
        }

        if (!param.Axis) {
            param.Error = true;
            param.ErrorID = ZMC_AXIS_IS_NO_AXIS_REF;
            param.Valid = false;
            param.Busy = false;
            return;
        }

        auto& ax = get_axis_impl_ref(param.Axis);
        param.Busy = true;
        param.Error = false;
        param.ErrorID = ZMC_NO_ERROR;
        param.Valid = true;
        double curAbsVel = ax.fSetVelocity > 0 ? ax.fSetVelocity : -ax.fSetVelocity;

        if (curAbsVel > maxVel) {
            maxVel = curAbsVel;
            param.MaxVelocity = maxVel;
            param.TimeAtMax = param.TimeStamp;
        }
    }
};

FbtGetMaxSetVelocity::FbtGetMaxSetVelocity() {
    make_impl<FbtImplGetMaxSetVelocity, FbtGetMaxSetVelocityStruct>(*this);
}

FbtGetMaxSetVelocity::~FbtGetMaxSetVelocity() {
    if (pImpl) {
        delete static_cast<FbtImplGetMaxSetVelocity*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtGetMaxSetVelocity::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplGetMaxSetVelocity*>(pImpl);
        impl->runCycle();
    }
}

// Legacy buffered diff; FbtReadValues is stateless now
class FbtImplReadValues : public FbtEnBase<FB_TYPE_READVALUES> {
   public:
    FbtReadValuesStruct* pParam = nullptr;

    // store last 4 position samples (engineering units)
    double pos[4] = {0, 0, 0, 0};
    int count = 0;

    inline void reset() {
        pos[0] = pos[1] = pos[2] = pos[3] = 0.0;
        count = 0;
    }

    inline void push(double p) {
        // shift right: pos[3] oldest, pos[0] newest
        pos[3] = pos[2];
        pos[2] = pos[1];
        pos[1] = pos[0];
        pos[0] = p;
        if (count < 4) count++;
    }

    void runCycle() {
        auto& fb = *pParam;

        if (!fb.Enable) {
            // disabled state
            fb.Valid = false;
            fb.Busy = false;
            fb.Error = false;
            fb.ErrorID = ZMC_NO_ERROR;
            reset();
            return;
        }

        if (!fb.Axis) {
            fb.Valid = false;
            fb.Busy = false;
            fb.Error = true;
            fb.ErrorID = ZMC_AXIS_IS_NO_AXIS_REF;
            return;
        }

        auto& ax = get_axis_impl_ref(fb.Axis);
        fb.Busy = true;
        fb.Error = false;
        fb.ErrorID = ZMC_NO_ERROR;

        // choose source
        double engPos;
        switch (fb.Source) {
            case ZMC_VALUESELECT_SET:
                engPos = ax.diSetPosition;
                break;
            case ZMC_VALUESELECT_ACTUAL:
                engPos = ax.diActPosition;
                break;
            case 2:
                engPos = ax.fSetPosition;
                break;
            case 3:
                engPos = ax.fActPosition;
                break;
            default:
                engPos = 0;
                break;
        }

        // push and compute
        push(engPos);
        fb.Position = engPos;

        double dt = ax.cycleTime;

        if (count >= 2) {
            // first difference (backward)
            fb.Velocity = (pos[0] - pos[1]) / dt;
        } else {
            fb.Velocity = 0;
        }

        if (count >= 3) {
            // second difference
            fb.Acceleration = (pos[0] - 2.0 * pos[1] + pos[2]) / (dt * dt);
        } else {
            fb.Acceleration = 0;
        }

        if (count >= 4) {
            // third difference
            fb.Jerk = (pos[0] - 3.0 * pos[1] + 3.0 * pos[2] - pos[3]) / (dt * dt * dt);
            fb.Valid = true;
        } else {
            fb.Jerk = 0;
            fb.Valid = false;  // not enough history yet
        }
    }
};

FbtReadValues::FbtReadValues() { all_zero_init<FbtReadValuesStruct>(*this); }

FbtReadValues::~FbtReadValues() {}

void FbtReadValues::runCycle() {
    if (Enable) {
        if (Axis) {
            auto& ax = get_axis_impl_ref(Axis);
            switch (Source) {
                case ZMC_VALUESELECT_SET:
                    Position = ax.fSetPosition;
                    Velocity = ax.fSetVelocity;
                    Acceleration = ax.fSetAcceleration;
                    break;
                case ZMC_VALUESELECT_ACTUAL:
                    Position = ax.fActPosition;
                    Velocity = ax.fActVelocity;
                    Acceleration = ax.fActAcceleration;
                    break;
                case ZMC_VALUESELECT_SAVE:
                    Position = ax.fSavePosition;
                    Velocity = ax.fSaveVelocity;
                    Acceleration = ax.fSaveAcceleration;
                    break;
                default:
                    Position = ax.fSetPosition;
                    Velocity = ax.fSetVelocity;
                    Acceleration = ax.fSetAcceleration;
                    break;
            }
            Jerk = 0;
            synEnFbStat(*this, FB_STAT_VALID);
        } else {
            synEnFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
        }
    } else {
        synEnFbStat(*this, FB_STAT_UNABLE);
    }
}

FbtReadPowerParam::FbtReadPowerParam() { all_zero_init<FbtReadPowerParamStruct>(*this); }
FbtReadPowerParam::~FbtReadPowerParam() {}
void FbtReadPowerParam::runCycle() {
    if (Enable) {
        if (Axis) {
            auto& ax = get_axis_impl_ref(Axis);
            ControlWord = ax.wControlWord;
            StatusWord = ax.wStatusWord;
            SetPosition = ax.diSetPosition;
            ActPosition = ax.diActPosition;
            synEnFbStat(*this, FB_STAT_VALID);
        } else {
            synEnFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
        }
    } else {
        synEnFbStat(*this, FB_STAT_UNABLE);
    }
}

FbtSetOverride::FbtSetOverride() {
    all_zero_init<FbtSetOverrideStruct>(*this);
    VelFactor = 1.0;
    AccFactor = 1.0;
    JerkFactor = 1.0;
}
FbtSetOverride::~FbtSetOverride() {}
void FbtSetOverride::runCycle() {
    if (Enable) {
        if (Axis) {
            auto& ax = get_axis_impl_ref(Axis);
            if (VelFactor > 1 || AccFactor > 1 || JerkFactor > 1 || VelFactor <= 0 ||
                AccFactor <= 0 || JerkFactor <= 0) {
                Enabled = false;
                Busy = false;
                Error = true;
                ErrorID = ZMC_MOVE_INVALID_DYNAMIC_FACTOR;
                return;
            }
            if (ax.fFactorVel != VelFactor || ax.fFactorAcc != AccFactor ||
                ax.fFactorJerk != JerkFactor) {
                ax.fFactorVel = VelFactor;
                ax.fFactorAcc = AccFactor;
                ax.fFactorJerk = JerkFactor;
                ax.incOverridePairity();
            }
            Enabled = true;
            Busy = true;
            Error = false;
            ErrorID = ZMC_NO_ERROR;
        } else {
            Enabled = false;
            Busy = false;
            Error = true;
            ErrorID = ZMC_AXIS_IS_NO_AXIS_REF;
        }
    } else {
        Enabled = false;
        Busy = false;
        Error = false;
        ErrorID = ZMC_NO_ERROR;
    }
}

FbtGetOverride::FbtGetOverride() { all_zero_init<FbtGetOverrideStruct>(*this); }
FbtGetOverride::~FbtGetOverride() {}
void FbtGetOverride::runCycle() {
    if (Enable) {
        if (Axis) {
            auto& ax = get_axis_impl_ref(Axis);
            VelFactor = ax.fFactorVel;
            AccFactor = ax.fFactorAcc;
            JerkFactor = ax.fFactorJerk;
            synEnFbStat(*this, FB_STAT_VALID);
        } else {
            synEnFbError(*this, ZMC_AXIS_IS_NO_AXIS_REF);
        }
    } else {
        synEnFbStat(*this, FB_STAT_UNABLE);
    }
}

FbtCheckLimits::FbtCheckLimits() { all_zero_init<FbtCheckLimitsStruct>(*this); }

FbtCheckLimits::~FbtCheckLimits() {}
void FbtCheckLimits::runCycle() {
    if (bEnable) {
        if (Axis) {
            auto& ax = get_axis_impl_ref(Axis);
            bBusy = true;
            bError = false;
            iErrorID = ZMC_NO_ERROR;
            bLimitsExceeded = false;

            // Check velocity limits if enabled
            if (bCheckVel) {
                double currentVel = fabs(ax.fSetVelocity);
                if (currentVel > ax.fSWMaxVelocity) {
                    bLimitsExceeded = true;
                }
            }
            if (bCheckAccDec) {
                if (ax.fSetVelocity > 1e7) {
                    if (ax.fSetAcceleration > ax.fSWMaxAcceleration) {
                        bLimitsExceeded = true;
                    }
                    if (ax.fSetAcceleration < -ax.fSWMaxDeceleration) {
                        bLimitsExceeded = true;
                    }
                } else if (ax.fSetVelocity < -1e-7) {
                    if (ax.fSetAcceleration < -ax.fSWMaxAcceleration) {
                        bLimitsExceeded = true;
                    }
                    if (ax.fSetAcceleration > ax.fSWMaxDeceleration) {
                        bLimitsExceeded = true;
                    }
                } else {
                    double currentAcc = fabs(ax.fSetAcceleration);
                    bLimitsExceeded =
                        currentAcc > ax.fSWMaxAcceleration || currentAcc > ax.fSWMaxAcceleration;
                }
            }
        } else {
            bBusy = false;
            bError = true;
            iErrorID = ZMC_AXIS_IS_NO_AXIS_REF;
            bLimitsExceeded = false;
        }
    } else {
        bBusy = false;
        bError = false;
        iErrorID = ZMC_NO_ERROR;
        bLimitsExceeded = false;
    }
}

class FbtImplInPosition : public FbtEnBase<FB_TYPE_INPOSITION> {
   public:
    FbtInPositionStruct* pParam;
    uint32_t inPositionStartTime = 0;
    uint32_t enableStartTime = 0;
    bool wasInWindow = false;

   public:
    void runCycle() {
        auto& param = *pParam;
        if (!param.bEnable) {
            reset();
            param.bInPosition = false;
            param.bBusy = false;
            param.bTimeOut = false;
            param.bError = false;
            param.iErrorID = ZMC_NO_ERROR;
            return;
        }

        if (!param.Axis) {
            param.bError = true;
            param.iErrorID = ZMC_AXIS_IS_NO_AXIS_REF;
            param.bInPosition = false;
            param.bBusy = false;
            param.bTimeOut = false;
            return;
        }
#if 0
        auto& ax = get_axis_impl_ref(param.Axis);
        param.bBusy = true;
        param.bError = false;
        param.iErrorID = ZMC_NO_ERROR;
        
        // Record enable start time
        if (enableStartTime == 0) {
            enableStartTime = ax.dwTime; // Assuming dwTime is available in axis implementation
        }
        
        // Calculate position error
        double posError = fabs(ax.fActPosition - ax.fSetPosition);
        bool currentlyInWindow = (posError <= param.fPosWindow);
        
        // Check if we just entered the position window
        if (currentlyInWindow && !wasInWindow) {
            inPositionStartTime = ax.dwTime;
        }
        
        // Check if we've been in the position window long enough
        if (currentlyInWindow && wasInWindow) {
            uint32_t timeInWindow = ax.dwTime - inPositionStartTime;
            if (timeInWindow >= (uint32_t)(param.fPosTime * 1000)) { // Convert seconds to milliseconds
                param.bInPosition = true;
            }
        } else {
            param.bInPosition = false;
        }
        
        // Check for timeout
        if (param.fTimeOut > 0) {
            uint32_t totalTime = ax.dwTime - enableStartTime;
            if (totalTime >= (uint32_t)(param.fTimeOut * 1000) && !param.bInPosition) {
                param.bTimeOut = true;
            }
        }
        
        wasInWindow = currentlyInWindow;
#endif
    }

    void reset() {
        inPositionStartTime = 0;
        enableStartTime = 0;
        wasInWindow = false;
    }
};

FbtInPosition::FbtInPosition() { make_impl<FbtImplInPosition, FbtInPositionStruct>(*this); }

FbtInPosition::~FbtInPosition() {
    if (pImpl) {
        delete static_cast<FbtImplInPosition*>(pImpl);
        pImpl = nullptr;
    }
}
void FbtInPosition::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplInPosition*>(pImpl);
        impl->runCycle();
    }
}

// Empty implementation class for CheckAxisCommunication
class FbtImplCheckAxisCommunication : public FbtEnBase<FB_TYPE_CHECKAXISCOMMUNICATION> {
   public:
    FbtCheckAxisCommunicationStruct* pParam = nullptr;
    void runCycle() {
        auto& param = *pParam;

        if (!param.bEnable) {
            param.bValid = false;
            param.bError = false;
            param.eErrorID = ZMC_NO_ERROR;
            param.bOperational = false;
            param.eComState = MC_COMSTATE_UNKNOWN;
            param.wComState = 0;
            return;
        }

        if (!param.Axis) {
            param.bValid = false;
            param.bError = true;
            param.eErrorID = ZMC_AXIS_IS_NO_AXIS_REF;
            param.bOperational = false;
            param.eComState = MC_COMSTATE_UNKNOWN;
            param.wComState = 0;
            return;
        }

        // Empty implementation - interface only
        param.bValid = true;
        param.bError = false;
        param.eErrorID = ZMC_NO_ERROR;
        param.bOperational = false;             // Default not operational
        param.eComState = MC_COMSTATE_UNKNOWN;  // Default unknown state
        param.wComState = 0;                    // Default raw state
    }
};

FbtCheckAxisCommunication::FbtCheckAxisCommunication() {
    make_impl<FbtImplCheckAxisCommunication, FbtCheckAxisCommunicationStruct>(*this);
}

FbtCheckAxisCommunication::~FbtCheckAxisCommunication() {
    if (pImpl) {
        delete static_cast<FbtImplCheckAxisCommunication*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtCheckAxisCommunication::runCycle() {
    if (pImpl) {
        auto impl = static_cast<FbtImplCheckAxisCommunication*>(pImpl);
        impl->runCycle();
    }
}

// Implementation class for EstimateDeadTime
class FbtImplEstimateDeadTime : public FbtEnBase<FB_TYPE_ESTIMATEDEADTIME> {
   public:
    FbtEstimateDeadTimeStruct* pParam = nullptr;

    void onClear() {
        auto& param = *pParam;
        param.Valid = false;
        // param.DeadTimeCycles = 0.0;
        param.Busy = false;
        param.Error = false;
        param.ErrorID = ZMC_NO_ERROR;
    }
    void onError(MC_ERROR_CODE err) {
        auto& param = *pParam;
        param.Valid = false;
        param.Busy = false;
        param.Error = true;
        param.ErrorID = err;
    }
    void runCycle() {
        auto& param = *pParam;

        if (!param.Enable) {
            onClear();
            return;
        }

        if (!param.Axis) {
            onError(ZMC_AXIS_IS_NO_AXIS_REF);
            return;
        }

        auto& ax = get_axis_impl_ref(param.Axis);

        // Check if axis is moving with constant velocity
        if (ax.axisStat != mcContinuousMotion || ax.fActVelocity == 0.0) {
            param.Valid = false;
            param.DeadTimeCycles = 0.0;
            return;
        }

        // Calculate dead time using the formula: tDelay = (fSavePos - fActPos) / fSetVel
        // lagRatio = tDelay / CycleTime
        double positionDiff = ax.fSavePosition - ax.fActPosition;
        double setVelocity = ax.fSaveVelocity;

        if (setVelocity != 0.0 && fabs(ax.fSaveAcceleration) < 1e-6) {
            // Calculate time delay in seconds
            double tDelay = positionDiff / setVelocity;
            double lagRatio = tDelay / ax.cycleTime;

            // Update dead time estimate
            param.DeadTimeCycles = lagRatio;
            param.Valid = true;
            param.Busy = true;
            param.Error = false;
            param.ErrorID = ZMC_NO_ERROR;
        } else {
            param.Valid = false;
            param.DeadTimeCycles = 0.0;
        }
    }
};

FbtEstimateDeadTime::FbtEstimateDeadTime() {
    make_impl<FbtImplEstimateDeadTime, FbtEstimateDeadTimeStruct>(*this);

    // Initialize default values
    this->Enable = false;
    this->Valid = false;
    this->DeadTimeCycles = 0.0;
}

FbtEstimateDeadTime::~FbtEstimateDeadTime() {
    if (pImpl) {
        delete static_cast<FbtImplEstimateDeadTime*>(pImpl);
        pImpl = nullptr;
    }
}

void FbtEstimateDeadTime::runCycle() {
    if (pImpl) {
        static_cast<FbtImplEstimateDeadTime*>(pImpl)->runCycle();
    }
}

} /* namespace zmc */
