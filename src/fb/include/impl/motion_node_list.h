#ifndef _ZMC_MOTION_NODE_LIST_H_
#define _ZMC_MOTION_NODE_LIST_H_

#include "fbt_common.h"
#include "mc_enum_def_spec.h"
#include "fbt_impl_enum_def.h"
#include "fbt_fb_base.hpp"
#include "motion_node.h"

namespace zmc{


template <typename T>
class NodeList {
   public:
    NodeList() {
        head.pPrev = nullptr;
        head.pNext = &tail;
        tail.pPrev = &head;
        tail.pNext = nullptr;
    }
    Node<T> head;
    Node<T> tail;
    Node<T>* pHead = &head;
    Node<T>* pTail = &tail;

   public:
    bool isValid()const{
        return pHead == &head && pTail == &tail;
    }
    void clear() {
        while (pHead->pNext != pTail) {
            pop_front();
        }
    }
    bool isEmpty() const { return pHead->pNext == pTail; }
    // void push_back(T&& pMove) {
    //     auto newNode = new Node<T>();
    //     newNode->pMov = pMove;

    //     newNode->pPrev = pTail->pPrev;
    //     newNode->pNext = pTail;
    //     pTail->pPrev->pNext = newNode;
    //     pTail->pPrev = newNode;
    // }
    void push_back(Node<T>* pNode) {
        pNode->pPrev = pTail->pPrev;
        pNode->pNext = pTail;
        pTail->pPrev->pNext = pNode;
        pTail->pPrev = pNode;
    }

    void pop_front() {
        if (pHead->pNext == pTail) return;

        auto toRemove = pHead->pNext;
        toRemove->pNext->pPrev = pHead;
        pHead->pNext = toRemove->pNext;
        // delete toRemove;
    }
    bool hasNext() const { return pHead->pNext != pTail && pHead->pNext->pNext != pTail; }
    T* current() { return (pHead->pNext != pTail) ? pHead->pNext : pHead; }
    const T* current() const { return (pHead->pNext != pTail) ? pHead->pNext : pHead; }

    inline
    T& currentNode() { return *((pHead->pNext != pTail) ? pHead->pNext : pHead); }
    inline
    const T& currentNode() const { return *((pHead->pNext != pTail) ? pHead->pNext : pHead); }
    inline int size() const {
        int cnt = 0;
        auto p = pHead->pNext;
        while (p != pTail) {
            cnt++;
            p = p->pNext;
        }
        return cnt;
    }
    bool isContainNode(T* pNode) const{
        while (pHead->pNext != pTail){
            if (pHead->pNext == pNode){
                return true;
            }
        }
        return false;
    }
};

class FbtImplParam;

class MotionNodeList : public NodeList<FbtImplParam> {
   public:
    MotionNodeList();
    template<typename P>
    void push_back(P* pNode) {
        pNode->pPrev = (P*) pTail->pPrev;
        pNode->pNext = (P*) pTail;
        pTail->pPrev->pNext = (Node<FbtImplParam>*) pNode;
        pTail->pPrev = (Node<FbtImplParam>*) pNode;
    }
    template<typename P>
    bool isCurrentNode(P* pNode) const{
        return ((P*) pHead->pNext) == pNode;
    }
    void clearFbByAborting();
    void clearFbByAxisStat(MC_AXIS_STATES);  
   private:
    void _clearFbWhenAxisDisabled();
    void _clearFbWhenErrorStop();
};

}
#endif