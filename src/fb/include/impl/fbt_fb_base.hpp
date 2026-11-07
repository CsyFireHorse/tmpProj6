#ifndef _ZMC_FBT_FB_BASE_HPP_
#define _ZMC_FBT_FB_BASE_HPP_

#include "fbt_common.h"
#include "fbt_impl_enum_def.h"
// #include "fbt_move.h"
#include "FbFactoryDef.h"
#include "misc_helper.hpp"

// #include "motion_node_list.h"
#include <cmath>

#include "motion_node.h"

namespace zmc {

template <int FB = 0>
class FbtEnBase {
   public:
    uint8_t fbType = FB;  // see also FB_TYPE
    FbStatEnum fbStat = FB_STAT_IDLE;
    MC_ERROR error_code = ZMC_NO_ERROR;
};

class FbtBase {
   public:
    uint8_t fbType = 0;  // see also FB_TYPE
    FbStatEnum fbStat = FB_STAT_IDLE;
    MC_ERROR error_code = ZMC_NO_ERROR;
    inline bool isIdle() const { return fbStat == FB_STAT_IDLE; }
    inline bool isBusy() const {
        return fbStat == FB_STAT_BUSY_ACTIVE || fbStat == FB_STAT_UNACTIVATED;
    }
    inline bool isDone() { return fbStat == FB_STAT_DONE; }
    inline bool isActive() const { return fbStat == FB_STAT_BUSY_ACTIVE; }
    inline bool isError() const { return fbStat == FB_STAT_ERROR; }
    inline bool isErrorNext() const { return fbStatCmd == FB_STAT_ERROR; }
    inline bool isAborted() const { return fbStat == FB_STAT_ABORTED; }
    inline bool isInXxx() const { return inVelocity; }
    inline bool isReached() const { return inReached_; }
    inline MC_ERROR_CODE getErrorID() const { return static_cast<MC_ERROR_CODE>(error_code); }
    inline void onAborted() { fbStat = FB_STAT_ABORTED; }
    inline void onError(MC_ERROR_CODE err) {
        fbStat = FB_STAT_ERROR;
        error_code = err;
    }
    void sendAborted() {
        if (!isBusy()) return;
        fbStatCmd = FB_STAT_ABORTED;
    }
    void sendError(MC_ERROR_CODE err) {
        if (!isBusy()) return;
        fbStatCmd = FB_STAT_ERROR;
        error_code = err;
    }
    inline bool isDoneErrorAborted() const {
        return fbStat == FB_STAT_DONE || fbStat == FB_STAT_ERROR || fbStat == FB_STAT_ABORTED;
    }
    bool checkReceiveAbortError() {
        if ((this->fbStatCmd == FB_STAT_ABORTED || this->fbStatCmd == FB_STAT_ERROR)) {
            this->fbStat = this->fbStatCmd;
            this->fbStatCmd = FB_STAT_IDLE;
            return false;
        }
        return true;
    }
    union {
        uint8_t bCtrlParam_ = 0;
        struct {
            uint8_t execute : 1;
            uint8_t bContinuousUpdate_ : 1;
            uint8_t byBufferMode_ : 3;
            uint8_t bFlagSpec : 1;
        };
    };
    FbStatEnum fbStatCmd = FB_STAT_IDLE;
    union {
        uint16_t bNodeStat_ = 0;
        struct {
            uint8_t inVelocity : 1;
            uint8_t inReached_ : 1;
            uint8_t _executeBak : 1;
            uint8_t bTrajBuilt : 1;
            uint8_t hasVelPlan : 1;
            uint8_t hasEmitFirst : 1;
            // uint8_t hasLookahead:1;
            uint8_t bBlendNext : 1;
            uint8_t bBlendPrev : 1;
            // uint8_t bCanBlend:1; // can blend next
            // uint8_t bCanBlending:1; // can blend prev
            uint8_t bTickParity : 1;
            uint8_t bOverrideParity : 1;
            uint8_t bNegDirection : 1;
            uint8_t bPosDirection : 1;
        };
    };
    inline void onClear() {
        this->error_code = ZMC_NO_ERROR;
        this->fbStat = FB_STAT_IDLE;
        this->bNodeStat_ = 0;
    }
};

class FbtExeBase : public FbtBase {
   public:
    // uint32_t lastUpdateCycle = 0;
    int32_t startTick = 0;
    inline bool isInCycle(uint32_t cycle) const { return bTickParity == (cycle & 1); }
    inline bool isRunableByUpdateCycle(uint32_t cycle) {
        this->bTickParity = (cycle & 1);
        return true;
    }
    inline void updateCycleTick(uint32_t cycle) { this->bTickParity = (cycle & 1); }
    inline void incOverrideParity() { this->bOverrideParity = !this->bOverrideParity; }
    inline bool isContinuousUpdate() const { return bContinuousUpdate_; }
    inline bool isRisingEdge() const { return execute && !_executeBak; }
    inline bool canBufferNext() const {
        switch (fbType) {
            case FB_TYPE_MOVEVELOCITY:
            case FB_TYPE_MOVEABSOLUTE:
            case FB_TYPE_MOVERELATIVE:
            case FB_TYPE_MOVEDISCRETE:
            case FB_TYPE_MOVEADDITIVE:
            case FB_TYPE_MOVECONTINUOUSABSOLUTE:
            case FB_TYPE_MOVECONTINUOUSRELATIVE:
            case FB_TYPE_POSITIONPROFILE:
            case FB_TYPE_ACCELERATIONPROFILE:
            case FB_TYPE_VELOCITYPROFILE:
            case FB_TYPE_GEARIN:
            case FB_TYPE_GEARINPOS:
            case FB_TYPE_GEAROUT:
            case FB_TYPE_CAMOUT:
                return true;
            default:
                return false;
        }
    }
    inline bool canBufferPrev() { return canBlend() && this->byBufferMode_ == mcBuffered; }
    inline bool canBlend() const {
        return this->fbType == FB_TYPE_MOVEABSOLUTE || this->fbType == FB_TYPE_MOVERELATIVE ||
               this->fbType == FB_TYPE_MOVEDISCRETE || this->fbType == FB_TYPE_MOVEVELOCITY ||
               this->fbType == FB_TYPE_GEARIN;
    }
    inline bool canBlendNext() const {
        return this->fbType == FB_TYPE_MOVEABSOLUTE || this->fbType == FB_TYPE_MOVERELATIVE ||
               this->fbType == FB_TYPE_MOVEDISCRETE;
    }
    inline bool canBlendPrev() const { return canBlend() && this->byBufferMode_ > mcBuffered; }
    inline bool canSuperimposedNext() const {
        switch (fbType) {
            case FB_TYPE_MOVEVELOCITY:
            case FB_TYPE_MOVEABSOLUTE:
            case FB_TYPE_MOVERELATIVE:
            case FB_TYPE_MOVEDISCRETE:
            case FB_TYPE_MOVEADDITIVE:
            case FB_TYPE_MOVECONTINUOUSABSOLUTE:
            case FB_TYPE_MOVECONTINUOUSRELATIVE:
            case FB_TYPE_HALT:
                // case FB_TYPE_GEARIN:
                // case FB_TYPE_GEARINPOS:
                // case FB_TYPE_GEAROUT:
                // case FB_TYPE_CAMOUT:
                return true;
            default:
                return false;
        }
    }
};

// FbtEnableTmpl use for impl, while FbtEnTmpl use for no impl
template <typename T, typename B = FbtExeBase>
class FbtEnTmpl : public B {
   public:
    void runCycle() {
        if (this->Enable) {
            static_cast<T*>(this)->onExecuting();
        } else {
            // static_cast<T*>(this)->onClear();
        }
        // static_cast<T*>(this)->onUpdate();
    }
};

template <typename T, typename B = FbtExeBase>
class FbtEnableTmpl : public B {
   public:
    inline void onUpdate() {}
    inline void onCapture() {}
    void _runCycle() {
        if (this->execute) {
            static_cast<T*>(this)->onEnable();
        } else {
            static_cast<T*>(this)->onClear();
        }
    }
    void runCycle() {
        static_cast<T*>(this)->onCapture();
        _runCycle();
        static_cast<T*>(this)->onUpdate();
    }
};

template <typename T, typename B = FbtExeBase>
class FbtExeTmpl : public B {
   public:
    void onStarted() {}  // this->fbStat = FB_STAT_BUSY_ACTIVE;
    void onExecuting() {}
    inline void onUpdate() {}
    inline void onCapture() {}
    void runCycle() {
        static_cast<T*>(this)->onCapture();
        if (this->isRisingEdge()) {
            static_cast<T*>(this)->onStarted();
        }
        if (this->isBusy()) {
            static_cast<T*>(this)->onExecuting();
        } else if (this->isDoneErrorAborted() && !this->execute) {
            static_cast<T*>(this)->onClear();
        }
        this->_executeBak = this->execute;
        static_cast<T*>(this)->onUpdate();
    }
};

template <typename T, typename B = FbtExeBase>
class FbtExeLevelTmpl : public B {
   public:
    void onStarted() {}  // this->fbStat = FB_STAT_BUSY_ACTIVE;
    void onExecuting() {}
    inline void onUpdate() {}
    void runCycle() {
        static_cast<T*>(this)->onCapture();
        // trigger when execute is high level
        if (this->execute) {
            static_cast<T*>(this)->onStarted();
        }
        if (this->isBusy()) {
            static_cast<T*>(this)->onExecuting();
        } else if (this->isDoneErrorAborted() && !this->execute) {
            static_cast<T*>(this)->onClear();
        }
        this->_executeBak = this->execute;
        static_cast<T*>(this)->onUpdate();
    }
};

class FbtImplParam : public FbtExeBase {
   public:
    void* pParam = nullptr;
};

template <typename T>
class FbtImplParamTmpl : public FbtExeBase {
   public:
    T* pParam = nullptr;
    void init(T& param) {
        all_zero_init<T>(param);
        this->pParam = &param;
        param.pImpl = this;
    }
};

// will be deleted
using FbtImplParamTmplStruct = FbtImplParamTmpl<int>;

template <typename T>
inline void synEnFbStat(T& obj, FbStatEnum2 stat) {
    obj.Valid = (stat == FB_STAT_VALID);
    obj.Busy = (stat == FB_STAT_BUSYING || stat == FB_STAT_VALID);
    obj.Error = (stat == FB_STAT_EN_ERROR);
    obj.ErrorID = ZMC_NO_ERROR;
}

template <typename T>
inline void synEnFbError(T& obj, MC_ERROR_CODE err) {
    obj.Valid = false;
    obj.Busy = false;
    obj.Error = (err != ZMC_NO_ERROR);
    obj.ErrorID = err;
}

template <typename T>
inline void synExeFbStat(T& obj, FbStatEnum stat) {
    obj.Done = (stat == FB_STAT_DONE);
    obj.Busy = (stat == FB_STAT_BUSYING || stat == FB_STAT_VALID);
    obj.Error = (stat == FB_STAT_EN_ERROR);
    if (stat != FB_STAT_ERROR) obj.ErrorID = ZMC_NO_ERROR;
}
template <typename T>
inline void synExeFbError(T& obj, MC_ERROR_CODE err) {
    obj.Done = false;
    obj.Busy = false;
    obj.Error = (err != ZMC_NO_ERROR);
    obj.ErrorID = err;
}

struct FbImplReconstruct {
    void* pImpl;
};

template <typename T, typename B = FbImplReconstruct>
inline T* get_fb_impl(const B& obj) {
    return static_cast<T*>(((const FbImplReconstruct&)obj).pImpl);
}

template <typename T>
inline FbStatEnum get_fb_impl_stat(const T& obj) {
    return get_fb_impl<FbtBase>(obj)->fbStat;
}

template <typename T, typename S, typename R>
void make_impl(R& p) {
    all_zero_init<S>(p);
    p.pImpl = new T();
    (static_cast<T*>(p.pImpl))->pParam = (S*)&p;
}

inline double calcTargetPosByDirection0(MC_DIRECTION direct, double bgPos, double tgPos,
                                        double mModulo) {
    double tgPosMod = std::fmod(tgPos, mModulo);
    if (tgPosMod < 0) tgPosMod += mModulo;
    double bgPosMod = std::fmod(bgPos, mModulo);
    if (bgPosMod < 0) bgPosMod += mModulo;
    double dif = tgPosMod - bgPosMod;
    double endPos = bgPos + dif;
    switch (direct) {
        case mcPositiveDirection:
            if (dif >= 0)
                return endPos;
            else
                return endPos + mModulo;
        case mcNegativeDirection:
            if (dif <= 0)
                return endPos;
            else
                return endPos - mModulo;
        case mcFiniteDirection:
            return tgPos;
        // TODO: seem difficult to implement
        case mcFastestWay:
        case mcShortestWay: {
            if (dif >= 0) {
                if (dif * 2 <= mModulo)
                    return endPos;
                else
                    return endPos - mModulo;
            } else {
                if (-dif * 2 < mModulo)
                    return endPos + mModulo;
                else
                    return endPos;
            }
        }
        default:
            return endPos;
    }
}

// only use for modulo axis to calculate position.
inline double calcTargetPosByDirection(MC_DIRECTION direct, double bgPos, double tgPos,
                                       double mModulo, bool is_neg) {
    return calcTargetPosByDirection0(direct != mcCurrentDirection
                                         ? direct
                                         : (is_neg ? mcNegativeDirection : mcPositiveDirection),
                                     bgPos, tgPos, mModulo);
}

}  // namespace zmc
#endif
