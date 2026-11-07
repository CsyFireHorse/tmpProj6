#include "motion_node_super.h"
#include "mc_enum_def.h"
#include <vector>
// #include "FbFactoryDef.h"


namespace zmc {

// Forward declarations
// template <typename T, typename P>
// class FbtImplMoveBlendBase;


inline bool isBufferModeBlendable(MC_BUFFER_MODE buffer_mode) {
    return buffer_mode > MC_BUFFER_MODE::mcBuffered;
}

union MotbufEvent{
    uint8_t value;
    struct {
        uint8_t isBlend:1;
        uint8_t isReload:1;
        uint8_t isOverride:1;
        uint8_t isContUpdate:1;
    };
};


struct MotionNodeLookaheadParam{
    MotbufEvent event;
    double current_position;
    double current_velocity;
    double current_acceleration;
};

// Structure to hold lookahead information for testing
struct LookaheadNodeInfo {
    int nodeIndex;
    const char* typeName;
    double posOrDist;
    double velocity; // max velocity
    double vEndLimit;  // velocity limit
    double vEndRelax; // reachable vel
    bool canBlendNext;
    bool canBlendPrev;
    
    LookaheadNodeInfo() : nodeIndex(0), typeName(""), posOrDist(0.0), 
                         velocity(0.0), vEndLimit(0.0), vEndRelax(0.0),
                         canBlendNext(false), canBlendPrev(false) {}
};

// Motion node lookahead list
class MotionNodeLookaheadList : public MotionNodeSuper {
private:        
    // Helper function to check if a node is blendable
    bool isNodeBlendable(const Node<FbtImplParam>* node) const;
    
    // Process begin position and check direction consistency
    int process_begin_position(double next_begin_pos);
    
    // Calculate vEndLimit for each FB based on blend mode
    void calculate_v_end_limit(int count);
    
    // Step 2&3: Combined forward and backward pass
    int forward_backward_pass(int count, double current_vel = 0.0);
    
    // Step 4: Replan the first FB with new end velocity
    void replan_first_fb(Node<FbtImplParam>* first_node);
    
    // Main lookahead algorithm implementation (4 steps)
    bool lookahead(const MotionNodeLookaheadParam& param);

public:
        // Detect number of consecutive blendable nodes
    int detectBlendableNum() const;
    
    void push_fb(void* pFb);
    // Main cycle function - detects new nodes and triggers lookahead
    void runCycle(const MotionNodeLookaheadParam& param);
    void print_results(int count) const;
    
    // Get lookahead information as vector for testing
    std::vector<LookaheadNodeInfo> getLookaheadInfo(int count) const;
};

// Demo function for testing the integrated lookahead algorithm
void demo_integrated_lookahead();

}  // namespace zmc
