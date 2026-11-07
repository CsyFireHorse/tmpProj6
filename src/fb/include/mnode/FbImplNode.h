/*
 * @Author: ChenShengyong
 * @Date: 2024-03-01 17:15:05
 * @Last Modified by: ChenShengyong
 * @Last Modified time: 2024-12-12 16:52:49
 */
#ifndef _ZMC_FB_IMPL_NODE_H_
#define _ZMC_FB_IMPL_NODE_H_

#include "FbImplBase.h"

namespace zmc {

class FbtNode;
class PfbMoveBaseOrig : public FbImplBase {  //  FbExecuteImpl<PfbMoveBase>
   public:
    void setContinuousUpdate(BOOL_T continuous_update) {
        FbImplBase::ctrlParam.ContinuousUpdate_ = continuous_update;
    };
    void setExecute(BOOL_T val) {
        FbImplBase::ctrlParam.execute_ = val;
    };
    // inline BOOL_T isValid() const { return _fbOStat.valid_; };
    inline BOOL_T isCommandAborted() const { return FbImplBase::_fbOStat.aborted_; };
    inline BOOL_T isActive() const { return FbImplBase::_fbOStat.active_; };
    inline BOOL_T isDone() const { return _fbOStat.done_; };
    inline BOOL_T isBusy() const { return _fbOStat.busy_; };
};

class FbImplNode: public PfbMoveBaseOrig{
   public:
    FbtNode* pNode;
};

typedef FbImplNode PfbMoveBas;

} /* namespace zmc */
#endif