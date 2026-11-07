#include "FbtGroupAdmin.h"
#include <cstring>
#include "AxisGrpZmot.h"
#include "../axgrp/AxisGrpZmotImpl.h"
#include "AxisManager.h"
#include "impl/fbt_axis_base.h"
#include "impl/fbt_fb_base.hpp"
#include "fbt_axis_opaque.h"

namespace zmc {

template <typename TStruct>
inline void fbGroupResetState(TStruct* s) {
    if (!s) return;
    std::memset(s, 0, sizeof(*s));
}

template <typename TStruct>
class FbtImplParamGroupAdminTmpl : public FbtImplParamTmpl<TStruct> {};


template <typename T, typename P>
class FbExeGroupAdminTmpl : public FbtExeTmpl<T, FbtImplParamGroupAdminTmpl<P>> {
   public:
    void onCapture() { this->execute = this->pParam->Execute; }
    void onStarted() {
        if (this->fbStat != FB_STAT_IDLE) return;
        auto& param = *this->pParam;
        if (!this->pParam || !param.AxisGroup) return this->onError(static_cast<MC_ERROR_CODE>(ZMC_ERR_NO_AXISGRP_REF));
        this->fbStat = FB_STAT_BUSY_ACTIVE;
    }

    void onUpdate() {
        this->pParam->Done = this->isDone();
        this->pParam->Busy = this->isBusy();
        this->pParam->Error = this->isError();
        this->pParam->ErrorID = static_cast<MC_ERROR>(this->getErrorID());
    }
};


class FbtAddAxisToGroupImpl : public FbExeGroupAdminTmpl<FbtAddAxisToGroupImpl, FbtAddAxisToGroupStruct> {
   public:
    void onExecuting() {
        auto& param = *this->pParam;
        if (!param.Axis || !static_cast<AxisOpaque*>(param.Axis)->is_valid()) return this->onError(ZMC_AXIS_IS_NO_AXIS_REF);

        AxisGrpZmot& group = *param.AxisGroup;
        auto& d = get_grp_inner(group);
        int ret = d.addAxis(param.Axis);
        if (ret){
            return this->onError(static_cast<MC_ERROR_CODE>(ret));
        }
        this->fbStat = FB_STAT_DONE;
    }
};

class FbtGroupEnableImpl : public FbExeGroupAdminTmpl<FbtGroupEnableImpl, FbtGroupEnableStruct> {
   public:
    void onExecuting() {
        auto& param = *this->pParam;
        auto& d = get_grp_inner(*param.AxisGroup);
        if (d.axNum <= 0){
            return this->onError(static_cast<MC_ERROR_CODE>(ZMC_ERR_AXISGRP_EMPTY));
        }
        if (d.groupState != Disabled) {
            return this->onError(static_cast<MC_ERROR_CODE>(ZMC_ERR_AXISGRP_INVALID_STATE));
        }
        d.groupState = Standby;
        d.errorId = 0;
        this->fbStat = FB_STAT_DONE;
    }
};

class FbtGroupDisableImpl : public FbExeGroupAdminTmpl<FbtGroupDisableImpl, FbtGroupDisableStruct> {
   public:
    void onExecuting() {
        auto& param = *this->pParam;
        auto& d = get_grp_inner(*param.AxisGroup);
        if (d.groupState != Standby) {
            return this->onError(static_cast<MC_ERROR_CODE>(ZMC_ERR_AXISGRP_INVALID_STATE));
        }
        d.groupState = Disabled;
        this->fbStat = FB_STAT_DONE;
    }
};

class FbtGroupResetImpl : public FbExeGroupAdminTmpl<FbtGroupResetImpl, FbtGroupResetStruct> {
   public:
    void onExecuting() {
        auto& param = *this->pParam;
        auto& d = get_grp_inner(*param.AxisGroup);
        if (d.groupState != ErrorStop) {
            return this->onError(static_cast<MC_ERROR_CODE>(ZMC_ERR_AXISGRP_NO_ERROR_TO_RESET));
        }
        d.errorId = 0;
        d.groupState = Standby;
        this->fbStat = FB_STAT_DONE;
    }
};

class FbtUngroupAllAxesImpl : public FbExeGroupAdminTmpl<FbtUngroupAllAxesImpl, FbtUngroupAllAxesStruct> {
   public:
    void onExecuting() {
        auto& param = *this->pParam;
        auto& d = get_grp_inner(*param.AxisGroup);
        int ret = d.ungroupAll();
        if (ret){
            return this->onError(static_cast<MC_ERROR_CODE>(ret));
        }
        this->fbStat = FB_STAT_DONE;
    }
};

class FbtRemoveAxisFromGroupImpl : public FbExeGroupAdminTmpl<FbtRemoveAxisFromGroupImpl, FbtRemoveAxisFromGroupStruct> {
   public:
    void onExecuting() {
        auto& param = *this->pParam;
        AxisGrpZmot& group = *param.AxisGroup;
        auto* d = &get_grp_inner(group);
        int ret = d->removeAxis(param.Axis);
        if (ret){
            return this->onError(static_cast<MC_ERROR_CODE>(ret));
        }
        this->fbStat = FB_STAT_DONE;
    }
};

class FbtGroupSetUnitsImpl : public FbExeGroupAdminTmpl<FbtGroupSetUnitsImpl, FbtGroupSetUnitsStruct> {
   public:
    void onExecuting() {
        auto& param = *this->pParam;
        
        // TODO: store units in group when AxisGrpZmot supports it
        (void)param.Units;
        this->fbStat = FB_STAT_DONE;
    }
};

class FbtGroupSetPositionImpl : public FbExeGroupAdminTmpl<FbtGroupSetPositionImpl, FbtGroupSetPositionStruct> {
   public:
    void onExecuting() {
        auto& param = *this->pParam;
        AxisGrpZmot& group = *param.AxisGroup;
        auto* d = &get_grp_inner(group);
        if (d->groupState != Standby) {
            return this->onError(static_cast<MC_ERROR_CODE>(ZMC_ERR_AXISGRP_INVALID_STATE));
        }
        for (int i = 0; i < d->axNum; ++i) {
            // TODO
            const double pos = param.Position.v[i];
            if (param.Relative) {
                d->fSetPos[i] += pos;
            } else {
                d->fSetPos[i] = pos;
            }
        }
        this->fbStat = FB_STAT_DONE;
    }
};

FbtAddAxisToGroup::FbtAddAxisToGroup() {
    make_impl<FbtAddAxisToGroupImpl, FbtAddAxisToGroupStruct>(*this);
}

FbtAddAxisToGroup::~FbtAddAxisToGroup() {
    delete static_cast<FbtAddAxisToGroupImpl*>(pImpl);
}

void FbtAddAxisToGroup::runCycle() {
    auto impl = static_cast<FbtAddAxisToGroupImpl*>(pImpl);
    impl->runCycle();
}

FbtGroupEnable::FbtGroupEnable() {
    make_impl<FbtGroupEnableImpl, FbtGroupEnableStruct>(*this);
}

FbtGroupEnable::~FbtGroupEnable() {
    delete static_cast<FbtGroupEnableImpl*>(pImpl);
}

void FbtGroupEnable::runCycle() {
    auto impl = static_cast<FbtGroupEnableImpl*>(pImpl);
    impl->runCycle();
}

FbtGroupDisable::FbtGroupDisable() {
    make_impl<FbtGroupDisableImpl, FbtGroupDisableStruct>(*this);
}

FbtGroupDisable::~FbtGroupDisable() {
    delete static_cast<FbtGroupDisableImpl*>(pImpl);
}

void FbtGroupDisable::runCycle() {
    auto impl = static_cast<FbtGroupDisableImpl*>(pImpl);
    impl->runCycle();
}

FbtGroupReset::FbtGroupReset() {
    make_impl<FbtGroupResetImpl, FbtGroupResetStruct>(*this);
}

FbtGroupReset::~FbtGroupReset() {
    delete static_cast<FbtGroupResetImpl*>(pImpl);
}

void FbtGroupReset::runCycle() {
    auto impl = static_cast<FbtGroupResetImpl*>(pImpl);
    impl->runCycle();
}

FbtUngroupAllAxes::FbtUngroupAllAxes() {
    make_impl<FbtUngroupAllAxesImpl, FbtUngroupAllAxesStruct>(*this);
}

FbtUngroupAllAxes::~FbtUngroupAllAxes() {
    delete static_cast<FbtUngroupAllAxesImpl*>(pImpl);
}

void FbtUngroupAllAxes::runCycle() {
    auto impl = static_cast<FbtUngroupAllAxesImpl*>(pImpl);
    impl->runCycle();
}

class FbGroupPowerImpl : public FbtEnableTmpl<FbGroupPowerImpl> {
   public:
    FbtGroupPowerStruct* pParam = nullptr;

    void init(FbtGroupPowerStruct& param) {
        pParam = &param;
        pParam->pImpl = this;
    }

    void onCapture() { this->execute = (pParam && pParam->Enable); }

    void onEnable() {
        if (!pParam || !pParam->AxisGroup) {
            this->onError(static_cast<MC_ERROR_CODE>(ZMC_ERR_NO_AXISGRP_REF));
            return;
        }
        // TODO: call MC_Power for each axis in the group
        this->fbStat = FB_STAT_DONE;
        this->error_code = ZMC_NO_ERROR;
    }

    void onUpdate() {
        if (!pParam) return;
        pParam->Status = this->isDone();
        pParam->Busy = this->isBusy();
        pParam->Error = this->isError();
        pParam->ErrorID = static_cast<MC_ERROR>(this->getErrorID());
    }
};

FbtGroupPower::FbtGroupPower() {
    fbGroupResetState<FbtGroupPowerStruct>(this);
    auto* impl = new FbGroupPowerImpl();
    impl->init(static_cast<FbtGroupPowerStruct&>(*this));
    pImpl = impl;
}

FbtGroupPower::~FbtGroupPower() {
    auto* impl = static_cast<FbGroupPowerImpl*>(pImpl);
    delete impl;
    pImpl = nullptr;
}

void FbtGroupPower::runCycle() {
    if (!pImpl) return;
    static_cast<FbGroupPowerImpl*>(pImpl)->runCycle();
}

FbtRemoveAxisFromGroup::FbtRemoveAxisFromGroup() {
    make_impl<FbtRemoveAxisFromGroupImpl, FbtRemoveAxisFromGroupStruct>(*this);
}

FbtRemoveAxisFromGroup::~FbtRemoveAxisFromGroup() {
    delete static_cast<FbtRemoveAxisFromGroupImpl*>(pImpl);
}

void FbtRemoveAxisFromGroup::runCycle() {
    auto impl = static_cast<FbtRemoveAxisFromGroupImpl*>(pImpl);
    impl->runCycle();
}

FbtGroupSetUnits::FbtGroupSetUnits() {
    make_impl<FbtGroupSetUnitsImpl, FbtGroupSetUnitsStruct>(*this);
}

FbtGroupSetUnits::~FbtGroupSetUnits() {
    delete static_cast<FbtGroupSetUnitsImpl*>(pImpl);
}

void FbtGroupSetUnits::runCycle() {
    auto impl = static_cast<FbtGroupSetUnitsImpl*>(pImpl);
    impl->runCycle();
}

FbtGroupSetPosition::FbtGroupSetPosition() {
    make_impl<FbtGroupSetPositionImpl, FbtGroupSetPositionStruct>(*this);
}

FbtGroupSetPosition::~FbtGroupSetPosition() {
    delete static_cast<FbtGroupSetPositionImpl*>(pImpl);
}

void FbtGroupSetPosition::runCycle() {
    auto impl = static_cast<FbtGroupSetPositionImpl*>(pImpl);
    impl->runCycle();
}

}  // namespace zmc
