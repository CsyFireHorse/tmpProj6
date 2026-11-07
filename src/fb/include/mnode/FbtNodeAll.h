#ifndef _FBT_NODE_ALL_H_
#define _FBT_NODE_ALL_H_

#include "FbImplNode.h"
#include "mc_enum_def.h"

namespace zmc {
class FbtMotBuf;


class PfbMoveDiscrete : public FbImplNode {
   public:
    FbtMotBuf* pMotbuf;
    PfbMoveDiscrete();
    ~PfbMoveDiscrete();
    void setBuf(FbtMotBuf*);
    void setExecute(bool);
    void setMotionType(int val);
    void setVelConfig(double vel, double acc, double dec, double jerk);
    void setPosition(double pos);
    void setBufferMode(MC_BUFFER_MODE);

    void runCycle();
    bool isBusy() const;
    bool isDone() const;
    bool isActive() const;
    bool isCommandAborted() const;
};

}  // namespace zmc
#endif