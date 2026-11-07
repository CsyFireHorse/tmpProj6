#ifndef _ZMC_POOL_OBJ_PTR_H_  
#define _ZMC_POOL_OBJ_PTR_H_  
   
#include "stdint.h"
#include <memory>
#include "zmc_ringbuffer_m.hpp"

namespace zmc{  

// you can only manage allocate SZ - 1 element
template<typename T, unsigned int SZ=256>
class PoolObjPtr{
public:
    PoolObjPtr(int maxSize=SZ):_maxSize(maxSize){
        _ptr = new T[maxSize];
        for (int i=0; i<maxSize; i++){
            ptrArr.push(&_ptr[i]);
        }        
    }
    ~PoolObjPtr(){ delete [] _ptr; _ptr = nullptr;}
    T* allocate(){
        T* ptr = nullptr;
        ptrArr.pop(ptr);
        return ptr;
    }
    void deallocate(T* val){
        ptrArr.push(val);
    }
    int size()const { return ptrArr.size();}
private:
    int _maxSize;
    T* _ptr;
    zmc::ZmcRingbufM<T*, SZ> ptrArr;
};

}
#endif