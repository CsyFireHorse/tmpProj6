#ifndef _ZMC_FBT_AXIS_BASE_H_
#define _ZMC_FBT_AXIS_BASE_H_

#include <cstring>

#include "fbt_common.h"
#include "mc_enum_def_spec.h"
#include "fbt_impl_enum_def.h"
#include "motion_node_super.h"
#include "fbt_axis_transform.h"
#include "fbt_axis_limit.h"
#include "base/include/axis_utils.h"
#include "zmc_fb_predef.h"

namespace zmc {

enum {
    E_OP_MODE_TO_CTRL_MODE = 11,
};
enum {
    E_AXIS_USE_ACT_PVA,
    E_AXIS_USE_CONST_POS,
    E_AXIS_USE_SET_PVA,
};

struct AxisImplCore {
    void* native;
    bool fbError = false;
    bool bError = false;                    //
    bool bStartReference = false;           //  start homing
    bool bStartReferenceRealState = false;  //
    // LREAL fReference = 0;  // home position ?
    // bool bVirtual;
    bool bLogical;
    bool bSimulation = false;
    bool bDeaf = false;  // ignore diActPosition
    // add by ChenSy
    bool bMotorRotaryType = false;  //  false for linear type, true for rotary type
    // int8_t iMovementType = 0; // use bModuloType instead
    bool iMovementType = true;  // false for modulo axis, true for  finite axis, see Axis_Type

    bool ConstantVelocity = false;
    bool Accelerating = false;
    bool Decelerating = false;
    bool bSetValuesModifiedByMoveSuperimposed = false;
    int8_t nDirection = mcPositiveDirection;  // see MC_DIRECTION
    uint8_t eAxisMtype = 0;                   // see AXIS_CTYPE
    uint8_t eRampType;                        //  Velocity_Ramp_Type
    uint8_t byControllerMode = 3;             // 3 = CSP
    uint8_t byRealControllerMode = 3;
    uint8_t byMotbufValue = 0;    // use to record parity fbOverride
    uint8_t byMotbufEvent = 0;    // see MotbufEvent
    uint16_t byResTimeNumer = 0;  // den is 32768
    uint8_t usiEncoderBitWidth = 0;
    // nAxisState
    MC_AXIS_STATES axisStat = mcDisabled;
    MC_AXIS_STATES axisStatBak = mcDisabled;
    AxisStateDetail axisStateDetail = AXIS_STATE_DETAIL_DEFAULT;
    uint8_t byMovingHistory =
        0;  // record history status whether the axis is moving by user command
    union {
        uint8_t powerExtra = 0;
        struct {
            // power
            uint8_t bRegulatorOn : 1;
            uint8_t bDriveStart : 1;
            uint8_t regulatorRealState : 1;
            uint8_t driveStartRealState : 1;
            uint8_t bRegulatorRealStateBak : 1;
            uint8_t bDriveStartRealStateBak : 1;
        };
    };
    union {
        uint8_t byCmdExtra = 0;  // auto clear
        struct {
            uint8_t bControlAxis : 1;   // from controlAxisByPos
            uint8_t bClearAxError : 1;  // from fbReset
        };
    };
    union {
        uint8_t byPosCmdVal = 0;
        struct {
            // see E_AXIS_USE_ACT_PVA
            uint8_t byPosCmdCur : 2;
            uint8_t byPosCmdBak : 2;
        };
    };
    union {
        uint8_t axisErrExtra = 0;
        struct {
            uint8_t driveError : 1;
            uint8_t axisSwError : 1;
            uint8_t communicationError : 1;
            uint8_t notOperationalError : 1;  // Slave query succeeded, but it is not healthy OP.
            uint8_t bDriveErrorBak : 1;
            uint8_t bAxisSwErrorBak : 1;
            uint8_t bCommunicationErrorBak : 1;
        };
    };
    inline bool isPrevPowerOn() const { return bRegulatorRealStateBak && bDriveStartRealStateBak; }
    inline bool isPowerOn() const { return regulatorRealState && driveStartRealState; }

    // Changes the main state and its producer-owned detail together.
    void setAxisState(MC_AXIS_STATES state, AxisStateDetail detail = AXIS_STATE_DETAIL_DEFAULT) {
        axisStat = state;
        axisStateDetail = detail;
    }

    // Refreshes ErrorStop power detail and clears details incompatible with the main state.
    void updateAxisStateDetail() {
        if (axisStat == mcErrorStop) {
            axisStateDetail =
                isPowerOn() ? AXIS_STATE_DETAIL_DEFAULT : AXIS_STATE_DETAIL_ERROR_POWER_OFF;
            return;
        }
    }

    // Encodes only the explicit main-state/detail pair; motion nodes are not inspected.
    MC_AXIS_STATES_EX getAxisStateEx() const {
        switch (axisStateDetail) {
            case AXIS_STATE_DETAIL_HOME:
                if (axisStat == mcHoming) return mcHomeEx;
                break;
            case AXIS_STATE_DETAIL_GEAR_OUT:
                if (axisStat == mcContinuousMotion) return mcGearOutEx;
                break;
            case AXIS_STATE_DETAIL_FOLLOW:
                if (axisStat == mcSynchronizedMotion) return mcFollowEx;
                break;
            case AXIS_STATE_DETAIL_QUICK_STOP:
                if (axisStat == mcStopping) return mcQuickStoppingEx;
                break;
            case AXIS_STATE_DETAIL_ERROR_POWER_OFF:
                if (axisStat == mcErrorStop) return mcErrorStopPowerOffEx;
                break;
            case AXIS_STATE_DETAIL_DEFAULT:
            default:
                break;
        }
        return static_cast<MC_AXIS_STATES_EX>(axisStat);
    }

    // Prefer the current fieldbus fault; retain its latched reaction until explicit reset.
    MC_ERROR_CODE getCommunicationErrorId() const {
        if (communicationError) return ZMC_DI_GENERAL_COMMUNICATION_ERROR;
        if (notOperationalError) return ZMC_AXIS_IS_NOT_OPERATIONAL;
        if (dwErrorID == ZMC_DI_GENERAL_COMMUNICATION_ERROR ||
            dwErrorID == ZMC_AXIS_IS_NOT_OPERATIONAL)
            return static_cast<MC_ERROR_CODE>(dwErrorID);
        return ZMC_NO_ERROR;
    }

    void requestSwError(uint16_t error_id) {
        if (dwErrorID) return;
        axisSwError = true;
        dwErrorID = error_id;
        setAxisState(mcErrorStop);
    }

    // use to check axis valid, constant magic number
    uint16_t wMagicNumber = AXIS_MAGIC_NUMBER;
    uint16_t wAxisStructID = 0;          //
    uint16_t uiDriveInterfaceError = 0;  // CiA402 object 0x603F:0
    uint16_t dwErrorID = ZMC_NO_ERROR;   //
    uint16_t fbeFBError = 0;

    LREAL cycleTime = 0.02;  // task cycle time

    LREAL fSetActTimeLagCycles = 0;

    // bool bSetValuesModifiedByMoveSuperimposed = false;
    // bool bAvoidReversalOnHaltStop = false;

    uint8_t swSmEx = 0;  // status word sm ex
    AxisCmdEnum cmd = AXIS_CMD_NOP;
    AxisCmdEnum cmdBak = AXIS_CMD_NOP;
    uint8_t byCount = 0;
    uint16_t wStatusWord = 0;
    uint16_t wControlWord = 0;

    uint32_t m_cycleCounter = 0;
    int32_t* pDwExtraData = nullptr;

    void reset() {
        void* savedNative = native;
        int32_t* savedExtraData = pDwExtraData;
        std::memset(this, 0, sizeof(*this));
        native = savedNative;
        pDwExtraData = savedExtraData;
        iMovementType = true;
        nDirection = mcPositiveDirection;
        byControllerMode = 3;
        byRealControllerMode = 3;
        wMagicNumber = AXIS_MAGIC_NUMBER;
        cycleTime = 0.02;
    }

    inline bool is_modulo_type() const { return !iMovementType; }
    inline double get_current_time() const { return m_cycleCounter * cycleTime; }
    inline double get_offset_time(int ofs_tick) const {
        if (ofs_tick > m_cycleCounter) return 0;
        return (m_cycleCounter - ofs_tick) * cycleTime;
    }
    inline bool is_prev_moving() const {
        return axisStatBak != mcStandstill && axisStatBak != mcDisabled;
    }
    inline bool is_axis_stat_moving(MC_AXIS_STATES axStat) const {
        return axStat != mcStandstill && axStat != mcDisabled && axStat != mcErrorStop;
    }
    inline bool isOverrideMatch(bool v) const { return (v == (byMotbufValue & 1)); }
    inline void incOverridePairity() { byMotbufValue = (byMotbufValue + 1) & 1; }
    inline void set_res_time(double res_time) { byResTimeNumer = res_time / cycleTime * 32768; }
    inline double get_res_time() const { return byResTimeNumer * cycleTime / 32768; }
    inline bool is_axis_valid() const { return wMagicNumber == AXIS_MAGIC_NUMBER; }
    void bind_extra_data(int32_t* pDwExtraData) { this->pDwExtraData = pDwExtraData; }
};

template <typename Base>
class AxPvatLayer : public Base {
   public:
    LREAL fSetPosition = 0;
    LREAL fSetVelocity = 0;
    LREAL fSetAcceleration = 0;
    LREAL fActPosition = 0;
    LREAL fActVelocity = 0;
    LREAL fActAcceleration = 0;
    LREAL fSavePosition = 0;
    LREAL fSaveVelocity = 0;
    LREAL fSaveAcceleration = 0;

    LREAL fAimPosition = 0;
    LREAL fMarkPosition = 0;
    LREAL fMarkVelocity = 0;
    LREAL fMarkAcceleration = 0;
    LREAL fSetTorque;
    LREAL fActTorque;

    LREAL fScalefactor = 1;
    LREAL fFactorVel = 1;
    LREAL fFactorAcc = 1;
    LREAL fFactorTor = 1;
    LREAL fFactorJerk = 1;

    bool isSetPvaUnmoved() const {
        if (this->fSetVelocity < 1e-6 && this->fSetVelocity > -1e-6 &&
            this->fSetAcceleration < 1e-6 && this->fSetAcceleration > -1e-6) {
            return true;
        } else
            return false;
    }
};

template <typename Base, typename T = MotionNodeSuper>
class AxMotbufLayer : public Base {
   public:
    FbtBase* fbBak = nullptr;
    T pNode;
    inline bool isMotionNodevalid() const { return pNode.isValid(); }
};

template <typename Base>
class AxEncoderLayer : public Base {
   public:
    int32_t diEncoderPosition = 0;
    int32_t diEnLastPosition = 0;
    int32_t diEncoderTurn = 0;

    int32_t diEnPos2diEnPos(int32_t pos, int32_t& cnt) const {
        if (this->usiEncoderBitWidth <= 0 || this->usiEncoderBitWidth >= 32) {
            return pos;
        }
        return updateEncoder(this->usiEncoderBitWidth, cnt, pos);
    }
};

template <typename Base>
class AxCia402Layer : public Base {};

struct AxisImplBase
    : public AxCia402Layer<AxMotbufLayer<
          AxLimitLayer<AxTransformLayer<AxPvatLayer<AxEncoderLayer<AxisImplCore>>>>>> {
   public:
    inline double getModuloSetPos() const { return this->calc_modulo(this->fSetPosition); }
    inline double getModuloActPos() const { return this->calc_modulo(this->fActPosition); }
    void setPower(bool en) {
        this->bDriveStart = en;
        this->bRegulatorOn = en;
        if (en && this->axisStat == mcDisabled) this->setAxisState(mcStandstill);
        if (!en && this->axisStat == mcStandstill) this->setAxisState(mcDisabled);
    }
};

inline bool isAxisQuickstopping(const AxisImplBase& ax) {
    return ax.axisStat == mcStopping && ax.pNode.isEmpty();
}

void axisInfoShift(AxisImplBase& axis, LREAL offset);

inline AxisImplBase* get_axis_impl(AXIS_REF axis) { return (AxisImplBase*)axis; }
inline AxisImplBase& get_axis_impl_ref(AXIS_REF axis) { return (AxisImplBase&)*axis; }

}  // namespace zmc
#endif
