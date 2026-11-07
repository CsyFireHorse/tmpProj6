#ifndef _ZMC_MOTION_NODE_H_
#define _ZMC_MOTION_NODE_H_



namespace zmc{

template <typename T>
class Node:public T {
   public:
    // T pMov; 
    Node* pPrev = nullptr;
    Node* pNext = nullptr;
};


}
#endif