#ifndef _FBT_NODE_H_
#define _FBT_NODE_H_

// #include "FbBase.h"
// #include "FbImplBase.h"

namespace zmc {

// Forward declarations for plcProj2 FbtImpl types
class FbtExeBase;


class FbtNode {
public:
    FbtExeBase* fbtImpl = nullptr;
    FbtNode* pNext = nullptr;
    FbtNode* pPrev = nullptr;
    
    FbtNode() = default;
    virtual ~FbtNode() = default;
    virtual void onStarted(){};
    virtual void onExecuting(){};
    bool isDone() const;
    bool isError() const;
    bool isAborted() const;
    
    void syncFbStat();
};


template<typename T>
class FbtNodeTmpl : public FbtNode {
public:
    FbtNodeTmpl() {
        fbtImpl = new T();
    }
    
    virtual ~FbtNodeTmpl() {
        if (fbtImpl) {
            delete static_cast<T*>(fbtImpl);
            fbtImpl = nullptr;
        }
    }
    T* getImplPtr(){
        return static_cast<T*>(fbtImpl);
    }
protected:
    void onStarted() override { 
        if (fbtImpl) static_cast<T*>(fbtImpl)->prepareCycle(); 
    }
    
    void onExecuting() override { 
        if (fbtImpl) static_cast<T*>(fbtImpl)->executeCycle(); 
    }
};
} // namespace zmc

#endif 