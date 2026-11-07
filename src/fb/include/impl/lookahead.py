#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Motion Lookahead Algorithm Implementation
运动前瞻算法实现

This script implements the four-step motion lookahead algorithm for blending
moveAbsolute and moveRelative motion commands.

四步运动前瞻算法，用于混合 moveAbsolute 和 moveRelative 运动指令。
"""

from typing import List, Optional
import math
from enum import Enum, IntEnum

class BufferMode(IntEnum):
    """Buffer mode enumeration / 缓冲模式枚举"""
    mcAborting = 0
    mcBuffered = 1
    mcBlendingLow = 2
    mcBlendingHigh = 3
    mcBlendingPrevious = 4
    mcBlendingNext = 5

class FbType(Enum):
    """Function block type enumeration / 功能块类型枚举"""
    FB_TYPE_MOVEABSOLUTE = 1
    FB_TYPE_MOVERELATIVE = 2

def get_sign(val: float) -> int:
    """Get sign of a value / 获取值的符号"""
    if abs(val)<1e-6:
        return 0
    elif val > 0:
        return 1
    else:
        return -1

class FbMove:
    """Motion function block / 运动功能块"""
    
    def __init__(self, fb_type: FbType, pos_or_distance: float, velocity: float, 
                 acceleration: float = 100.0, deceleration: float = 100.0, 
                 buffer_mode: BufferMode = BufferMode.mcBuffered):
        # Basic parameters / 基本参数
        self.fb_type = fb_type
        self.pos_or_distance = pos_or_distance  # Position for abs, Distance for rel
        self.velocity = velocity
        self.acceleration = acceleration
        self.deceleration = deceleration
        self.buffer_mode = buffer_mode
        
        # Blend control flags / 混合控制标志
        self.b_blend_next = buffer_mode > BufferMode.mcBuffered
        
        # Lookahead calculation results / 前瞻计算结果
        self.v_end_limit = 0.0    # Based on blend mode / 基于混合模式
        self.v_end_relax = 0.0    # Final calculated end velocity / 最终计算的结束速度
        
        # Internal calculation / 内部计算
        self._movement_distance = 0.0
        self._begin_pos = 0.0
        
    def is_blendable(self) -> bool:
        """Check if this FB is blendable / 检查此FB是否可混合"""
        return (self.fb_type in [FbType.FB_TYPE_MOVEABSOLUTE, FbType.FB_TYPE_MOVERELATIVE] 
                and self.b_blend_next)
    
    def set_begin_position(self, begin_pos: float):
        """Set beginning position and calculate movement distance and direction
        设置起始位置并计算运动距离和方向"""
        if self.fb_type == FbType.FB_TYPE_MOVEABSOLUTE:
            self._movement_distance = self.pos_or_distance - begin_pos
        else:  # FB_TYPE_MOVERELATIVE
            self._movement_distance = self.pos_or_distance
        self._begin_pos = begin_pos
    
    def update_begin_position(self, begin_pos: float):
        if self.fb_type == FbType.FB_TYPE_MOVEABSOLUTE:
            self._movement_distance = self.pos_or_distance - begin_pos
        else:  # FB_TYPE_MOVERELATIVE
            self._movement_distance = self.pos_or_distance
        self._begin_pos = begin_pos
        return self._movement_distance + self._begin_pos

    def get_end_pos(self) -> float:
        """Get end position / 获取结束位置"""
        if self.fb_type == FbType.FB_TYPE_MOVEABSOLUTE:
            return self.pos_or_distance
        else:  # FB_TYPE_MOVERELATIVE
            return self.pos_or_distance + self._begin_pos
    
    def get_movement_distance(self) -> float:
        """Get movement distance / 获取运动距离"""
        return self._movement_distance
    
    def cal_reachable_forward_vel(self, current_vel: float) -> float:
        """Calculate maximum reachable velocity in forward direction
        计算前向可达的最大速度"""
        if abs(self._movement_distance) < 1e-6:
            return current_vel
        
        if self._movement_distance > 0:
            # Accelerating case / 加速情况
            return math.sqrt(current_vel**2 + 2 * self.acceleration * self._movement_distance)
        else:
            # Decelerating case / 减速情况
            max_vel_squared = current_vel**2 + 2 * self.deceleration * abs(self._movement_distance)
            return -math.sqrt(max(0, max_vel_squared))
    
    def cal_reachable_backward_vel(self, target_vel: float) -> float:
        """Calculate maximum reachable velocity in backward direction
        计算反向可达的最大速度"""
        if abs(self._movement_distance) < 1e-6:
            return target_vel
        
        if self._movement_distance > 0:
            # Need to decelerate to reach target / 需要减速到达目标
            max_vel_squared = target_vel**2 + 2 * self.deceleration * self._movement_distance
            return math.sqrt(max(0, max_vel_squared))
        else:
            # Need to accelerate from higher velocity / 需要从更高速度加速
            max_vel_squared = target_vel**2 + 2 * self.acceleration * abs(self._movement_distance)
            return -math.sqrt(max(0, max_vel_squared))

    def calculate_v_end_limit(self, next_node) -> float:
        """Calculate vEndLimit based on blend mode
        基于混合模式计算vEndLimit"""
        current_vel = self.velocity
        next_vel = next_node.velocity
        
        if self.buffer_mode == BufferMode.mcBlendingLow:
            return min(current_vel, next_vel)
        elif self.buffer_mode == BufferMode.mcBlendingHigh:
            return max(current_vel, next_vel)
        elif self.buffer_mode == BufferMode.mcBlendingPrevious:
            return current_vel
        elif self.buffer_mode == BufferMode.mcBlendingNext:
            return next_vel
        else:
            return 0.0

#   [-absVal, absVal]
def get_abs_min(absVal, val):
    if val > 0:
        return min(absVal, val)
    else:
        return max(-absVal, val)
            
            
class MotionNodeLookaheadList:
    """Motion node lookahead list implementing 4-step algorithm
    运动节点前瞻列表，实现四步算法"""
    
    def __init__(self):
        self.nodes: List[FbMove] = []
        self.current_position = 0.0
        self.current_vel = 0.0
    def add_node(self, node: FbMove):
        """Add a motion node / 添加运动节点"""
        self.nodes.append(node)
       
    def detect_blendable_num(self) -> int:
        """Detect number of consecutive blendable nodes
        检测连续可混合节点的数量"""
        count = 0
        for node in self.nodes:
            if node.is_blendable():
                count += 1
            else:
                break
        return count
        

    
    def forward_pass(self, nodes: List[FbMove], count: int, current_vel:float=0):
        """Step 2: Forward pass - calculate ascending transition velocities
        步骤2：前向遍历 - 计算上升过渡速度"""
        vc = current_vel
        for current in nodes:       
            # Calculate maximum reachable velocity for forward transition
            # 计算前向过渡的最大可达速度
            vc = current.cal_reachable_forward_vel(vc)
            
            # Take minimum with vEndLimit and save to vEndRelax
            # 与vEndLimit取最小值并保存到vEndRelax
            vc = get_abs_min(current.v_end_limit, vc)
            current.v_end_relax = vc
    
    def backward_pass(self, nodes: List[FbMove], count: int):
        """Step 3: Backward pass - calculate descending transition velocities
        步骤3：反向遍历 - 计算下降过渡速度"""
        vc = 0
        for i in range(count - 1, 0, -1):
            current = nodes[i]
            prevNode = nodes[i - 1]
            # next_node = nodes[i + 1]
            
            # Calculate maximum reachable velocity for backward transition
            # 计算反向过渡的最大可达速度
            vc = current.cal_reachable_backward_vel(vc)  # Conservative
            
            # Take minimum with current vEndRelax
            # 与当前vEndRelax取最小值
            vc = get_abs_min(abs(prevNode.v_end_relax), vc)
            prevNode.v_end_relax = vc
    
    def replan_first_fb(self, first_node: FbMove):
        """Step 4: Replan the first FB with new end velocity
        步骤4：使用新的结束速度重新规划第一个FB"""
        print(f"Replanning first FB with vEndRelax = {first_node.v_end_relax:.2f}")
        # In real implementation, this would trigger trajectory replanning
        # 在实际实现中，这将触发轨迹重新规划
    
    def calculate_v_end_limit(self, nodes: List[FbMove], count: int):
        for i in range(count - 1):
            current = nodes[i]
            next_node = nodes[i + 1]
            current.v_end_limit = current.calculate_v_end_limit(next_node)
            current.v_end_relax = current.v_end_limit  # Initialize
            print(f"Node {i}: vEndLimit = {current.v_end_limit:.2f}")
        print(f"Node {i+1}: vEndLimit = {nodes[i+1].v_end_limit:.2f}")
            
    def process_begin_position(self, next_begin_pos):
        prev_direction = 0
        actual_count = 0
        
        for i,node in enumerate(self.nodes):
            if not node.is_blendable():
                break
            
            next_begin_pos = node.update_begin_position(next_begin_pos)
            current_direction = get_sign(node.get_movement_distance())
            
            # Break lookahead if direction changes (ignore zero direction)
            # 方向改变时中断前瞻（忽略零方向）
            if (i > 0 and current_direction != prev_direction and 
                prev_direction != 0 and current_direction != 0):
                print(f"Direction change detected at node {i}: {prev_direction} -> {current_direction}")
                break
            
            actual_count += 1
            prev_direction = current_direction
        return actual_count

    def lookahead(self) -> bool:
        """Main lookahead algorithm implementation (4 steps)
        主要前瞻算法实现（四步）"""
        # if count < 2:
        #     return False
        
        print(f"\n=== Starting Lookahead for {len(self.nodes)} nodes ===")
        
        # Step 1: Traverse FBs, process only those with bBlendNext==true
        # Calculate movement direction, break lookahead if direction changes
        # 步骤1：遍历FB，只处理bBlendNext==true的，计算运动方向，方向改变时中断前瞻
        actual_count = self.process_begin_position(self.current_position)
        if actual_count < 2:
            print("Not enough blendable nodes for lookahead")
            return False
        
        # print(f"Processing {actual_count} nodes with consistent direction: {prev_direction}")
        
        # Step 2: Calculate vEndLimit for each FB based on blend mode
        # 步骤2：基于混合模式为每个FB计算vEndLimit
        self.calculate_v_end_limit(self.nodes, actual_count)
        
        # Step 3: Forward pass - calculate ascending transition velocities
        # 步骤3：前向遍历 - 计算上升过渡速度
        print("\n--- Forward Pass ---")
        self.forward_pass(self.nodes, actual_count, self.current_vel)
        
        # Step 4: Backward pass - calculate descending transition velocities
        # 步骤4：反向遍历 - 计算下降过渡速度
        print("--- Backward Pass ---")
        self.backward_pass(self.nodes, actual_count)
        
        # Print results / 打印结果
        print("\n--- Final Results ---")
        for i in range(actual_count):
            node = self.nodes[i]
            print(f"Node {i}: vEndLimit={node.v_end_limit:.2f}, vEndRelax={node.v_end_relax:.2f}")
        
        # Step 5: Replan the first FB with calculated end velocity
        # 步骤5：使用计算的结束速度重新规划第一个FB
        self.replan_first_fb(self.nodes[0])
        
        return True
    
    def run_cycle(self):
        """Main cycle function - detects new nodes and triggers lookahead
        主循环函数 - 检测新节点并触发前瞻"""
  
        self.lookahead()

def demo_lookahead():
    """Demonstration of the lookahead algorithm
    前瞻算法演示"""
    print("Motion Lookahead Algorithm Demo")
    print("运动前瞻算法演示\n")
    
    # Create lookahead list / 创建前瞻列表
    lookahead_list = MotionNodeLookaheadList()
    lookahead_list.current_position = 0.0
    
    # Add motion commands / 添加运动指令
    # Move absolute to position 100 with blending
    lookahead_list.add_node(FbMove(
        FbType.FB_TYPE_MOVEABSOLUTE, -30.0, 50.0, 
        buffer_mode=BufferMode.mcBlendingLow
    ))
    
    # Move relative +50 with blending
    lookahead_list.add_node(FbMove(
        FbType.FB_TYPE_MOVERELATIVE, -25.0, 40.0,
        buffer_mode=BufferMode.mcBlendingHigh
    ))
    
    # Move relative +30 with blending
    lookahead_list.add_node(FbMove(
        FbType.FB_TYPE_MOVERELATIVE, -15.0, 60.0,
        buffer_mode=BufferMode.mcBlendingNext
    ))
    
    # Move absolute to position 200 (no blending)
    lookahead_list.add_node(FbMove(
        FbType.FB_TYPE_MOVEABSOLUTE, -200.0, 45.0,
        buffer_mode=BufferMode.mcBuffered
    ))
    
    # Run the lookahead algorithm / 运行前瞻算法
    lookahead_list.run_cycle()

if __name__ == "__main__":
    demo_lookahead()

