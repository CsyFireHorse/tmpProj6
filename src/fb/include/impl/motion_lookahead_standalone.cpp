#ifndef MOTION_LOOKAHEAD_STANDALONE_HPP
#define MOTION_LOOKAHEAD_STANDALONE_HPP

#include <iostream>

#include <cmath>
#include <memory>

namespace zmc {

// Buffer mode enumeration
enum class MC_BUFFER_MODE : int {
    mcAborting = 0,
    mcBuffered = 1,
    mcBlendingLow = 2,
    mcBlendingHigh = 3,
    mcBlendingPrevious = 4,
    mcBlendingNext = 5
};

// Function block type enumeration
enum class FB_TYPE : int {
    FB_TYPE_NULL = 0,
    FB_TYPE_MOVEABSOLUTE = 1,
    FB_TYPE_MOVERELATIVE = 2,
    FB_TYPE_MOVEADDITIVE,
};

// Utility functions
inline int get_sign(double val, double tolerance = 1e-6) {
    if (std::abs(val) < tolerance) return 0;
    return (val > 0) ? 1 : -1;
}

inline double get_abs_min(double absVal, double val) {
    if (val > 0) {
        return std::min(absVal, val);
    } else {
        return std::max(-absVal, val);
    }
}

// Forward declaration
template<typename T>
class Node;

class FbMoveNode;

inline bool isBufferModeBlendable(MC_BUFFER_MODE buffer_mode) {
    return buffer_mode > MC_BUFFER_MODE::mcBuffered;
}

typedef bool ZMC_BOOL;
typedef double ZMC_LREAL;
typedef void* AXIS_REF;
typedef short MC_DIRECTION;
typedef short MC_ERROR;

typedef struct FbtMoveAbsoluteStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Position;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    MC_DIRECTION Direction;
    MC_BUFFER_MODE BufferMode;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Active;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtMoveAbsoluteStruct;

typedef struct FbtMoveRelativeStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Distance;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    MC_BUFFER_MODE BufferMode;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL Active;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtMoveRelativeStruct;

typedef struct FbtMoveAdditiveStruct{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL Distance;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    // output
    ZMC_BOOL Done;
    ZMC_BOOL Busy;
    ZMC_BOOL CommandAborted;
    ZMC_BOOL Error;
    MC_ERROR ErrorID;
}FbtMoveAdditiveStruct;


// pos/dist, vel, acc,dec, jerk is same in FbtMoveAbsoluteStruct and FbtMoveRelativeStruct
// but only buffer mode is different in moveAbs & moveRel
typedef struct FbtMoveAbsOrRel{
    void* pImpl;

    AXIS_REF Axis;
    // input
    ZMC_BOOL Execute;
    ZMC_LREAL PosOrDist;
    ZMC_LREAL Velocity;
    ZMC_LREAL Acceleration;
    ZMC_LREAL Deceleration;
    ZMC_LREAL Jerk;
    MC_DIRECTION Direction;
    MC_BUFFER_MODE BufferMode;

    double get_pos_or_distance() const {
        return PosOrDist;
    }
    MC_BUFFER_MODE getBufferMode(FB_TYPE fb_type){
        if(fb_type == FB_TYPE::FB_TYPE_MOVEABSOLUTE){
            return BufferMode;
        }else if(fb_type == FB_TYPE::FB_TYPE_MOVERELATIVE){
            return reinterpret_cast<FbtMoveRelativeStruct*>(this)->BufferMode;
        }else{
            return MC_BUFFER_MODE::mcAborting;
        }
    }
};

// Motion function block implementation
class FbMoveImpl {
public:
    // Basic parameters
    FB_TYPE fb_type;
    FbtMoveAbsOrRel* pMoveAbsOrRel = nullptr; 
    
private:
    // Lookahead calculation results
    double v_end_limit = 0.0;        // Based on blend mode
    double v_end_relax = 0.0;        // Final calculated end velocity
    
    // Internal calculation
    double _movement_distance = 0.0;
public:
    // Helper functions to access struct members
    double get_pos_or_distance() const {
        return pMoveAbsOrRel->get_pos_or_distance();
    }

    double get_velocity() const {
        return pMoveAbsOrRel->Velocity;
    }

    double get_acceleration() const {
        return pMoveAbsOrRel->Acceleration;
    }

    double get_deceleration() const {
        return pMoveAbsOrRel->Deceleration;
    }

    MC_BUFFER_MODE get_buffer_mode() const {
        return pMoveAbsOrRel->getBufferMode(fb_type);
    }

public:
    
    bool is_blendable() const {
        return (fb_type == FB_TYPE::FB_TYPE_MOVEABSOLUTE || 
                fb_type == FB_TYPE::FB_TYPE_MOVERELATIVE) && isBufferModeBlendable(get_buffer_mode());
    }
    
    double update_begin_position(double begin_pos) {
        if (fb_type == FB_TYPE::FB_TYPE_MOVEABSOLUTE) {
            _movement_distance = get_pos_or_distance() - begin_pos;
            return get_pos_or_distance();
        } else if (fb_type == FB_TYPE::FB_TYPE_MOVERELATIVE) {
            _movement_distance = get_pos_or_distance();
            return begin_pos + get_pos_or_distance();
        }else{
            return begin_pos;
        }
    }
    
    double get_movement_distance() const {
        return _movement_distance;
    }
    
    int get_movement_direction() const {
        return get_sign(_movement_distance);
    }
    
    double cal_reachable_forward_vel(double current_vel) const {
        if (std::abs(_movement_distance) < 1e-6) {
            return current_vel;
        }
        
        if (_movement_distance > 0) {
            return std::sqrt(current_vel * current_vel + 2 * get_acceleration() * _movement_distance);
        } else {
            double max_vel_squared = current_vel * current_vel + 2 * get_deceleration() * std::abs(_movement_distance);
            return -std::sqrt(std::max(0.0, max_vel_squared));
        }
    }
    
    double cal_reachable_backward_vel(double target_vel) const {
        if (std::abs(_movement_distance) < 1e-6) {
            return target_vel;
        }
        
        if (_movement_distance > 0) {
            double max_vel_squared = target_vel * target_vel + 2 * get_deceleration() * _movement_distance;
            return std::sqrt(std::max(0.0, max_vel_squared));
        } else {
            double max_vel_squared = target_vel * target_vel + 2 * get_acceleration() * std::abs(_movement_distance);
            return -std::sqrt(std::max(0.0, max_vel_squared));
        }
    }
    
    double calculate_v_end_limit(const FbMoveImpl* next_node) const {
        if (!next_node) return 0.0;
        
        double current_vel = get_velocity();
        double next_vel = next_node->get_velocity();
        
        switch (get_buffer_mode()) {
            case MC_BUFFER_MODE::mcBlendingLow:
                return std::min(current_vel, next_vel);
            case MC_BUFFER_MODE::mcBlendingHigh:
                return std::max(current_vel, next_vel);
            case MC_BUFFER_MODE::mcBlendingPrevious:
                return current_vel;
            case MC_BUFFER_MODE::mcBlendingNext:
                return next_vel;
            default:
                return 0.0;
        }
    }

    // Getters and setters for v_end_limit and v_end_relax
    double get_v_end_limit() const { return v_end_limit; }
    void set_v_end_limit(double val) { v_end_limit = val; }
    double get_v_end_relax() const { return v_end_relax; }
    void set_v_end_relax(double val) { v_end_relax = val; }
};

// class FbMoveNode:public FbMoveImpl {
// public:
//     Node<FbMoveImpl>* pPrev = nullptr;
//     Node<FbMoveImpl>* pNext = nullptr;
// };

// Double-linked list node template
template<typename T>
class Node : public T {
public:
    Node* pPrev = nullptr;
    Node* pNext = nullptr;
};

class FbMoveNode: public Node<FbMoveImpl>{};

class FbtMoveAbsolute:public FbtMoveAbsoluteStruct{
public:
    FbtMoveAbsolute(){
        auto pImpl = new FbMoveNode();
        pImpl->fb_type = FB_TYPE::FB_TYPE_MOVEABSOLUTE;
        pImpl->pMoveAbsOrRel = (FbtMoveAbsOrRel*)this;
        this->pImpl = pImpl;
    }
    ~FbtMoveAbsolute() {
        if(pImpl) {
            delete static_cast<FbMoveNode*>(pImpl);
            pImpl = nullptr;
        }
    }
};
class FbtMoveRelative:public FbtMoveRelativeStruct{
public:
    FbtMoveRelative(){
        auto pImpl = new FbMoveNode();
        pImpl->fb_type = FB_TYPE::FB_TYPE_MOVERELATIVE;
        pImpl->pMoveAbsOrRel = (FbtMoveAbsOrRel*)this;
        this->pImpl = pImpl;
    }
    ~FbtMoveRelative() {
        if(pImpl) {
            delete static_cast<FbMoveNode*>(pImpl);
            pImpl = nullptr;
        }
    }
};

class FbtMoveAdditive:public FbtMoveAdditiveStruct{
public:
    FbtMoveAdditive(){
        auto pImpl = new FbMoveNode();
        pImpl->fb_type = FB_TYPE::FB_TYPE_MOVEADDITIVE;
        pImpl->pMoveAbsOrRel = (FbtMoveAbsOrRel*)this;
        this->pImpl = pImpl;
    }
    ~FbtMoveAdditive() {
        if(pImpl) {
            delete static_cast<FbMoveNode*>(pImpl);
            pImpl = nullptr;
        }
    }
};

// Node list template
template<typename T>
class NodeList {
public:
    Node<T> head;
    Node<T> tail;
    Node<T>* pHead;
    Node<T>* pTail;
    
public:
    NodeList() : pHead(&head), pTail(&tail) {
        pHead->pPrev = nullptr;
        pHead->pNext = pTail;
        pTail->pPrev = pHead;
        pTail->pNext = nullptr;
    }
    
    ~NodeList() {
        // Don't delete nodes here as they are owned by FbMoveImpl
        pHead->pNext = pTail;
        pTail->pPrev = pHead;
    }
    
    void clear() {
        // Just reset the links
        pHead->pNext = pTail;
        pTail->pPrev = pHead;
    }
    
    bool isEmpty() const {
        return pHead->pNext == pTail;
    }
    
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
        // Don't delete the node
    }
    
    bool hasNext() const {
        return pHead->pNext != pTail && pHead->pNext->pNext != pTail;
    }
    
    Node<T>* current() {
        return (pHead->pNext != pTail) ? pHead->pNext : nullptr;
    }
    
    const Node<T>* current() const {
        return (pHead->pNext != pTail) ? pHead->pNext : nullptr;
    }
};

// Motion node list
class MotionNodeList : public NodeList<FbMoveImpl> {
public:
    template<typename P>
    void push_back(P* pNode) {
        pNode->pPrev = (P*)pTail->pPrev;
        pNode->pNext = (P*)pTail;
        pTail->pPrev->pNext = (Node<FbMoveImpl>*)pNode;
        pTail->pPrev = (Node<FbMoveImpl>*)pNode;
    }
};

// Motion node lookahead list
class MotionNodeLookaheadList : public MotionNodeList {
private:
    bool isReload;
    bool isOverride;
    double current_position;
    double current_velocity;
    
public:    
    void set_current_state(double position, double velocity) {
        current_position = position;
        current_velocity = velocity;
    }
    
    // Helper function to check if a node is blendable
    bool isNodeBlendable(const Node<FbMoveImpl>* node) const {
        return node && node->is_blendable();
        // if (!node) {
        //     std::cout << "Node is null" << std::endl;
        //     return false;
        // }
        // bool blendable = node->is_blendable();
        // std::cout << "Node blendable check: fb_type=" << (int)node->fb_type 
        //           << ", buffer_mode=" << (int)node->get_buffer_mode() 
        //           << ", is_blendable=" << blendable << std::endl;
        // return blendable;
    }
    
    // Detect number of consecutive blendable nodes
    int detectBlendableNum() const {
        int cnt = 0;
        auto p = pHead->pNext;
        while (p != pTail) {
            if (isNodeBlendable(p)) {
                cnt++;
                p = p->pNext;
            } else {
                break; // Stop at first non-blendable node
            }
        }
        return cnt;
    }
    void push_fb(void* pFb){
        auto node = static_cast<FbMoveNode*>(((FbtMoveAbsOrRel*)pFb)->pImpl);
        push_back(node);
    }

    // Process begin position and check direction consistency
    int process_begin_position(double next_begin_pos) {
        int prev_direction = 0;
        int actual_count = 0;
        
        auto p = pHead->pNext;
        int i = 0;
        while (p != pTail) {
            if (!isNodeBlendable(p)) {
                break;
            }
            
            next_begin_pos = p->update_begin_position(next_begin_pos);
            int current_direction = get_sign(p->get_movement_distance());
            
            // Break lookahead if direction changes (ignore zero direction)
            if (i > 0 && current_direction != prev_direction && 
                prev_direction != 0 && current_direction != 0) {
                // std::cout << "Direction change detected at node " << i 
                        //  << ": " << prev_direction << " -> " << current_direction << std::endl;
                break;
            }
            
            actual_count++;
            prev_direction = current_direction;
            p = p->pNext;
            i++;
        }
        return actual_count;
    }
    
    // Calculate vEndLimit for each FB based on blend mode
    void calculate_v_end_limit(int count) {
        auto p = pHead->pNext;
        for (int i = 0; i < count - 1 && p != pTail && p->pNext != pTail; i++) {
            auto next_node = p->pNext;
            if (!p || !next_node) {
                p = p->pNext;
                continue;
            }
            double v_end_limit = p->calculate_v_end_limit(next_node);
            p->set_v_end_limit(v_end_limit);
            p->set_v_end_relax(v_end_limit); // Initialize
            // std::cout << "Node " << i << ": vEndLimit = " << p->get_v_end_limit() << std::endl;
            
            p = p->pNext;
        }
        
        // Last node
        // if (count > 0 && p) {
        //     std::cout << "Node " << (count-1) << ": vEndLimit = " << p->get_v_end_limit() << std::endl;
        // }
    }
    
    // Step 2&3: Combined forward and backward pass
    void forward_backward_pass(int count, double current_vel = 0.0) {
        // Forward pass - calculate ascending transition velocities
        double vc = current_vel;
        auto p = pHead->pNext;
        
        for (int i = 0; i < count && p != pTail; i++) {
            if (!p) {
                p = p->pNext;
                continue;
            }
            
            // Calculate maximum reachable velocity for forward transition
            vc = p->cal_reachable_forward_vel(vc);
            
            // Take minimum with vEndLimit and save to vEndRelax
            vc = get_abs_min(p->get_v_end_limit(), vc);
            p->set_v_end_relax(vc);
            
            p = p->pNext;
        }
        
        // Backward pass - calculate descending transition velocities
        vc = 0.0;
        
        // Start from the last processed node (p is now at the end)
        p = p->pPrev;
        
        // Backward iteration using double-linked pointers
        while (p != pHead->pNext && p->pPrev != pHead) {
            auto prev_node = p->pPrev;
            
            // Calculate maximum reachable velocity for backward transition
            vc = p->cal_reachable_backward_vel(vc);
            
            // Take minimum with current vEndRelax and update previous node
            vc = get_abs_min(std::abs(prev_node->get_v_end_relax()), vc);
            prev_node->set_v_end_relax(vc);
            
            p = p->pPrev;
        }
    }
    
    // Step 4: Replan the first FB with new end velocity
    void replan_first_fb(Node<FbMoveImpl>* first_node) {
        if (!first_node) return;
        
        // std::cout << "Replanning first FB with vEndRelax = " 
        //  << first_node->get_v_end_relax() << std::endl;
        // In real implementation, this would trigger trajectory replanning
    }
    
    // Main lookahead algorithm implementation (4 steps)
    bool lookahead() {
        // std::cout << "\n=== Starting Lookahead ===" << std::endl;
        
        // Step 1: Traverse FBs, process only those with bBlendNext==true
        // Calculate movement direction, break lookahead if direction changes
        int actual_count = process_begin_position(current_position);
        if (actual_count < 2) {
            // std::cout << "Not enough blendable nodes for lookahead" << std::endl;
            return false;
        }
        
        // std::cout << "Processing " << actual_count << " blendable nodes" << std::endl;
        
        // Step 2: Calculate vEndLimit for each FB based on blend mode
        calculate_v_end_limit(actual_count);
        
        // Step 3&4: Combined forward and backward pass
        // std::cout << "\n--- Forward & Backward Pass ---" << std::endl;
        forward_backward_pass(actual_count, current_velocity);
        
        // Step 5: Select first FB and recalculate velocity planning using vEndRelax
        replan_first_fb(pHead->pNext);
        
        return true;
    }
    
    void print_results(int count) const{
        std::cout << "\n--- Final Results ---" << std::endl;
        auto p = pHead->pNext;
        for (int i = 0; i < count && p != pTail; i++) {
            if (!p) {
                p = p->pNext;
                continue;
            }
            const char* typeNameArr[] = {"NOP", "MOVEABS", "MOVEREL", "MOVEADD"};
            const char* type_name = typeNameArr[static_cast<int>(p->fb_type)];
            
            std::cout << "Node " << i << ": Type=" << type_name 
                     << ", Pos/Dist=" << p->get_pos_or_distance()
                     << ", Vel=" << p->get_velocity()
                     << ", vEndLimit=" << p->get_v_end_limit()
                     << ", vEndRelax=" << p->get_v_end_relax() << std::endl;
            
            p = p->pNext;
        }
    }
    
    // Main cycle function - detects new nodes and triggers lookahead
    void runCycle() {
        lookahead();
    }
};

// Demo function
// some cases:
// blend end by MoveAdd
// blend end by reverse direction
// blend end by bufferMode
void demo_lookahead() {
    std::cout << "Motion Lookahead Algorithm Demo" << std::endl;
    std::cout << "运动前瞻算法演示\n" << std::endl;
    
    // Create lookahead list
    MotionNodeLookaheadList lookahead_list;
    lookahead_list.set_current_state(0.0, 10.0);
    
    // Create motion commands using stack allocation
    FbtMoveAbsolute move1;
    move1.Position = -30.0;
    move1.Velocity = 50.0;
    move1.Acceleration = 80.0;
    move1.Deceleration = 60.0;
    move1.BufferMode = MC_BUFFER_MODE::mcBlendingLow;
    
    FbtMoveRelative move2;
    move2.Distance = -25.0;
    move2.Velocity = 40.0;
    move2.Acceleration = 70.0;
    move2.Deceleration = 50.0;
    move2.BufferMode = MC_BUFFER_MODE::mcBlendingHigh;
    
    FbtMoveRelative move3;
    move3.Distance = -15.0;
    move3.Velocity = 60.0;
    move3.Acceleration = 90.0;
    move3.Deceleration = 70.0;
    move3.BufferMode = MC_BUFFER_MODE::mcBlendingNext;
    
    FbtMoveAbsolute move4;
    move4.Position = 200.0;
    move4.Velocity = 45.0;
    move4.Acceleration = 100.0;
    move4.Deceleration = 80.0;
    move4.BufferMode = MC_BUFFER_MODE::mcBlendingNext;
    
    // Add nodes to list
    lookahead_list.push_fb(&move1);
    lookahead_list.push_fb(&move2);
    lookahead_list.push_fb(&move3);
    lookahead_list.push_fb(&move4);
    
    int actual_count = lookahead_list.detectBlendableNum();
    printf("detectBlendableNum = %d\n", actual_count);
    // Run the lookahead algorithm
    lookahead_list.runCycle();
    // Print final results
    lookahead_list.print_results(actual_count);
}

} // namespace zmc

#endif // MOTION_LOOKAHEAD_STANDALONE_HPP

// Main function for standalone execution
// int main() {
//     zmc::demo_lookahead();
//     return 0;
// }