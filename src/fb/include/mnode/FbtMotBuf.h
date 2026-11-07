#ifndef _FBT_MOT_BUF_H_
#define _FBT_MOT_BUF_H_

#include "FbtNode.h"
// #include "MotBufBase.h"
#include "FbImplNode.h"
#include <string>



namespace zmc {

// Forward declarations
// class MotNode;
struct AxisImplBase;
class FbImplBase;
class FbBase;
class AxisStatInfo;

enum class ClearReason {
    CLEAR_DISABLE = 0,
    CLEAR_STOPPING = 6,
    CLEAR_ERROR = 7
};

class FbtMotBuf {
public:
    AxisImplBase* axisImpl_;
    AxisStatInfo* axisInfo_;
    
public:
    FbtMotBuf();
    ~FbtMotBuf();
    
    void runCycle();
    
    int addFBToQueue(FbBase* fb);
    int addPfbToQueue(FbImplBase* fb);
    bool isEmpty() const;
    void clearAllFB(ClearReason reason);
    void get_commands(double& pos, double& vel, double& acc) const;
    void updateAxisInfo();
    
    void dumpString(std::string& str) const {
        str = "FbtMotBuf with plcProj2 backend";
    }
    
    AxisStatInfo* pAxisInfo;
    void addNodeToQueue(FbtNode* node);

private:
    FbtNode* currentNode();
    bool isAxisContainImpl(FbtNode* node);
    void pop(FbtNode* node);
    
    void syncToPlcProj2();
    void syncFromPlcProj2();
    void initializeAxis(double cycleTime = 0.02);

private:
    FbtNode* headNode_ = nullptr;
    FbtNode* currentNode_ = nullptr;
};

} // namespace zmc

#endif 