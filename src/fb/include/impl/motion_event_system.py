#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Motion Event Handling System
运动事件处理系统

This system implements motion event handling with focus on event triggers,
corresponding actions and flow control. Algorithm calculations are simplified.

该系统实现运动事件处理，主要侧重于事件触发、对应动作和流程控制。算法计算部分简化处理。
"""

from typing import List, Optional, Any
from enum import Enum, IntEnum
import copy

class BufferMode(IntEnum):
    """Buffer mode enumeration / 缓冲模式枚举"""
    mcAborting = 0
    mcBuffered = 1
    mcBlendingLow = 2
    mcBlendingHigh = 3
    mcBlendingPrevious = 4
    mcBlendingNext = 5

class FbStat(Enum):
    """Function block status / 功能块状态"""
    IDLE = "idle"
    BUSY_UNACTIVATE = "busy_unactivate"
    BUSY_ACTIVATE = "busy_activate"
    DONE = "done"
    ABORTED = "aborted"
    ERROR = "error"

class AxisState(Enum):
    """Axis state enumeration / 轴状态枚举"""
    STANDSTILL = "standstill"
    DISCRETE_MOTION = "discrete_motion"
    CONTINUOUS_MOTION = "continuous_motion"
    SYNC_MOTION = "sync_motion"
    ERROR = "error"

class Direction(IntEnum):
    """Motion direction / 运动方向"""
    POSITIVE = 1
    NEGATIVE = -1
    SHORTEST = 0

class Traj:
    """Trajectory class (simplified) / 轨迹类（简化）"""
    def __init__(self):
        self.is_built = False
        self.start_pos = 0.0
        self.end_pos = 0.0
        self.duration_ticks = 0  # Duration in ticks
        self.start_tick = 0
    
    def build(self, start_pos: float, end_pos: float, vel: float, acc: float, dec: float, start_tick: int = 0, cycle_time: float = 0.1):
        """Build trajectory / 构建轨迹"""
        self.start_pos = start_pos
        self.end_pos = end_pos
        self.start_tick = start_tick
        # Simplified calculation
        distance = abs(end_pos - start_pos)
        duration_seconds = distance / vel if vel > 0 else 0.0
        self.duration_ticks = int(duration_seconds / cycle_time)
        self.is_built = True
        print(f"    Traj built: {start_pos:.1f} -> {end_pos:.1f}, duration={self.duration_ticks} ticks, start_tick={start_tick}")
    
    def get_position_at_tick(self, current_tick: int) -> float:
        """Get position at given tick / 获取给定tick的位置"""
        if not self.is_built or current_tick < self.start_tick:
            return self.start_pos
        
        elapsed_ticks = current_tick - self.start_tick
        if elapsed_ticks >= self.duration_ticks:
            return self.end_pos
        
        # Simplified linear interpolation
        progress = elapsed_ticks / self.duration_ticks if self.duration_ticks > 0 else 1.0
        return self.start_pos + (self.end_pos - self.start_pos) * progress
    
    def is_completed(self, current_tick: int) -> bool:
        """Check if trajectory is completed / 检查轨迹是否完成"""
        return self.is_built and (current_tick - self.start_tick) >= self.duration_ticks
    
    def get_remaining_ticks(self, current_tick: int) -> int:
        """Get remaining ticks for completion / 获取完成所需的剩余tick数"""
        if not self.is_built or current_tick < self.start_tick:
            return self.duration_ticks
        
        elapsed_ticks = current_tick - self.start_tick
        return max(0, self.duration_ticks - elapsed_ticks)

class Vel:
    """Velocity profile class (simplified) / 速度规划类（简化）"""
    def __init__(self):
        self.is_built = False
        self.end_vel = 0.0
        self.velocity_ratio = 1.0
        self.velocity = 0.0
        self.acceleration = 100.0
        self.deceleration = 100.0
        self.jerk = 1000.0
        self.prev_velocity = 0.0
        
    def build(self, end_vel: float = 0.0):
        """Build velocity profile / 构建速度规划"""
        self.end_vel = end_vel
        self.is_built = True
        print(f"    Vel built: velocity={self.velocity:.1f}, end_vel={self.end_vel:.1f}")
    
    def apply_override(self, ratio: float):
        """Apply velocity override / 应用速度覆盖"""
        self.velocity_ratio = ratio
        effective_vel = self.velocity * ratio
        print(f"    Vel override applied: ratio={ratio:.2f}, effective_vel={effective_vel:.1f}")

    def  velocity_updated(self, velocity: float):
        self.prev_velocity = self.velocity
        self.velocity = velocity
        return abs(self.velocity - self.prev_velocity) > 1e-6

class MoveImpl:
    """Motion implementation class / 运动实现类"""
    
    def __init__(self):
        # Status and flags
        self.fb_stat = FbStat.IDLE
        self.is_traj_built = False
        self.is_vel_built = False
        
        # Motion parameters
        self.position = 0.0
        self.direction = Direction.SHORTEST
        self.buffer_mode = BufferMode.mcBuffered
        
        # Previous values for edge detection
        self._prev_execute = False
        # Blend control
        self.f_end_vel = 0.0
        
        # Components
        self.traj = Traj()
        self.vel = Vel()
        
        # Reference to motion buffer
        self.mot_buf: Optional['MotBuf'] = None
        
        # Heartbeat
        self.last_heartbeat_tick_parity = False
        
        # Motion type
        self.motion_type = AxisState.DISCRETE_MOTION
        self.fb = None
        
    def is_blendable(self) -> bool:
        return self.buffer_mode >= BufferMode.mcBlendingLow
    
    def heartbeat_matches_tick(self, is_odd: bool) -> bool:
        return self.last_heartbeat_tick_parity == is_odd
    
    def update_heartbeat(self, is_odd: bool):
        self.last_heartbeat_tick_parity = is_odd
    
    def update_parameters(self):
        self.position = self.fb.get_target_position()
        self.vel.velocity = self.fb.velocity
        self.vel.acceleration = self.fb.acceleration
        self.vel.deceleration = self.fb.deceleration
        self.vel.jerk = self.fb.jerk
        if isinstance(self.fb, MovAbs):
            self.direction = self.fb.direction
        self.buffer_mode = self.fb.buffer_mode
        self.mot_buf = self.fb.axis
                
    def update_velocity(self, velocity: float):
        self.vel.velocity = velocity
    
    def on_plan(self):
        print(f"  onPlan: Planning new motion to pos={self.position:.1f}")
        # Add to motion buffer and auto-activate if first
        self.mot_buf.move_list.append(self)
        if len(self.mot_buf.move_list) == 1:
            self.fb_stat = FbStat.BUSY_ACTIVATE
            self.mot_buf.axis_state = self.motion_type
        else:
            self.fb_stat = FbStat.BUSY_UNACTIVATE
            
        current_pos = self.mot_buf.get_current_position()
        current_tick = self.mot_buf.current_tick
        self.traj.build(current_pos, self.position, self.vel.velocity, self.vel.acceleration, 
                      self.vel.deceleration, current_tick, self.mot_buf.cycle_time)
        self.is_traj_built = True
        
        self.vel.build(self.f_end_vel)
        self.is_vel_built = True
    
    def on_reload(self):
        print(f"  onReload: Reloading motion with new parameters")
            
        current_pos = self.mot_buf.get_current_position()
        current_tick = self.mot_buf.current_tick
        self.traj.build(current_pos, self.position, self.vel.velocity, self.vel.acceleration, 
                      self.vel.deceleration, current_tick, self.mot_buf.cycle_time)
        self.vel.build(self.f_end_vel)
    
    def on_continuous_update(self):
        print(f"  onContinuousUpdate: Updating velocity to {self.vel.velocity:.1f}")
        self.vel.build(self.f_end_vel)
    
    def on_override(self, ratio: float):
        print(f"  onOverride: Applying velocity ratio {ratio:.2f}")
        self.vel.apply_override(ratio)
    
    def on_blend(self):
        print(f"  onBlend: Blending motion with end_vel={self.f_end_vel:.1f}")
        self.vel.build(self.f_end_vel)
    
    def on_forward(self):   
        if self.fb_stat == FbStat.BUSY_ACTIVATE and self.traj.is_built:
            new_position = self.traj.get_position_at_tick(self.mot_buf.current_tick)
            self.mot_buf.update_position(new_position)
            print(f"  onForward: Position calculated: {new_position:.2f} at tick {self.mot_buf.current_tick}")
            
            # Check for completion
            if self.traj.is_completed(self.mot_buf.current_tick):
                self.fb_stat = FbStat.DONE
                print(f"  FB: State transition to DONE")
                
                # Remove self from buffer
                if self.mot_buf.move_list and self.mot_buf.move_list[0] == self:
                    self.mot_buf.move_list.pop(0)
     
    @property
    def execute_rising_edge(self):
        return self.execute and not self._prev_execute
    
    def runCycle(self):
        self.mot_buf = self.fb.axis
        # Update heartbeat
        self.update_heartbeat(self.mot_buf.current_tick % 2 == 1)
        # Detect execute rising edge
        # Detect velocity change for continuous update
        # Event handling based on current state
        if self.fb_stat == FbStat.IDLE:
            if self.execute_rising_edge:
                print(f"{self.__class__.__name__}: Execute rising edge detected in IDLE state")
                self.update_parameters()
                self.on_plan()
                self.on_forward()
        elif self.fb_stat in [FbStat.BUSY_UNACTIVATE, FbStat.BUSY_ACTIVATE]:
            if self.fb_stat == FbStat.BUSY_UNACTIVATE :
                if self.mot_buf.move_list and self.mot_buf.move_list[0] == self:
                    self.mot_buf.axis_state = AxisState.DISCRETE_MOTION #
                    self.fb_stat = FbStat.BUSY_ACTIVATE

            if self.execute_rising_edge:
                print(f"{self.__class__.__name__}: Execute rising edge detected in BUSY state")
                self.update_parameters()
                self.on_reload()
            elif self.fb.continuous_update and self.vel.velocity_updated(self.fb.velocity) :
                print(f"{self.__class__.__name__}: Velocity change detected: {self.vel.prev_velocity:.1f} -> {self.fb.velocity:.1f}")
                self.on_continuous_update()
            # Forward calculation
            self.on_forward()
        else:
            if not self.execute:
                self.on_clear()
                return 

        # Save previous values
        self._prev_execute = self.execute
    
    def on_clear(self):
        print(f"{self.__class__.__name__}: Clear motion")
        self.fb_stat = FbStat.IDLE
        self.is_traj_built = False
        self._prev_execute = False
        self.is_vel_built = False


class MovAbsOrRel:
    """Move Absolute or Relative function block / 绝对或相对运动功能块"""
    
    def __init__(self):
        self.p_impl = MoveImpl()
        self.p_impl.fb = self
        
        # Input parameters
        self.execute = False
        self.continuous_update = False
        self.velocity = 50.0
        self.acceleration = 100.0
        self.deceleration = 100.0
        self.jerk = 1000.0
        self.buffer_mode = BufferMode.mcBuffered
        
        # Output parameters
        self.done = False
        self.busy = False
        self.active = False
        self.command_aborted = False
        self.error = False
        self.error_id = 0
        
        # Axis reference
        self.axis: Optional['MotBuf'] = None
    
    def get_target_position(self) -> float:
        raise NotImplementedError("Subclass must implement get_target_position")
    
    def run_cycle(self):
        if not self.axis:
            return
            
        self.p_impl.execute = self.execute
        self.p_impl.runCycle()    
        # Update output status
        self.busy = self.p_impl.fb_stat in [FbStat.BUSY_UNACTIVATE, FbStat.BUSY_ACTIVATE]
        self.active = self.p_impl.fb_stat == FbStat.BUSY_ACTIVATE
        self.done = self.p_impl.fb_stat == FbStat.DONE
        self.command_aborted = self.p_impl.fb_stat == FbStat.ABORTED
        self.error = self.p_impl.fb_stat == FbStat.ERROR
        


class MovAbs(MovAbsOrRel):
    """Move Absolute function block / 绝对运动功能块"""
    
    def __init__(self):
        super().__init__()
        self.position = 0.0
        self.direction = Direction.SHORTEST
    
    def get_target_position(self) -> float:
        return self.position

class MovRel(MovAbsOrRel):
    """Move Relative function block / 相对运动功能块"""
    
    def __init__(self):
        super().__init__()
        self.distance = 0.0
    
    def get_target_position(self) -> float:
        return self.axis.get_current_position() + self.distance

class MotBuf:
    """Motion Buffer class / 运动缓冲区类"""
    
    def __init__(self, cycle_time: float = 0.1):
        # Event flags
        self.b_override_ev = False
        self.b_blend_ev = False
        
        # Parameters
        self.velocity_ratio = 1.0
        self.cycle_time = cycle_time
        
        # Motion list
        self.move_list: List[MoveImpl] = []
        
        # Current state
        self.current_position = 0.0
        self.current_velocity = 0.0
        self.current_tick = 0
        self.output_position = 0.0
        
        # Axis state
        self.axis_state = AxisState.STANDSTILL
        self.axis_error = False
        
        # Blend info
        self.blend_info = {
            'active': False,
            'remaining_ticks': 0,
            'start_pos': 0.0,
            'end_pos': 0.0
        }
    
    def get_current_position(self) -> float:
        return self.current_position
    
    def get_current_time(self) -> float:
        return self.current_tick * self.cycle_time
    
    def update_position(self, new_position: float):
        self.output_position = new_position
        self.current_position = new_position
    
    def trigger_override_event(self, ratio: float):
        print(f"MotBuf: Override event triggered with ratio={ratio:.2f}")
        self.velocity_ratio = ratio
        self.b_override_ev = True
    
    def check_active_fb_heartbeat(self):
        if self.axis_state == AxisState.STANDSTILL:
            return
            
        is_odd = self.current_tick % 2 == 1
        if not self.move_list:
            self.axis_error = True
            self.axis_state = AxisState.ERROR
            return
            
        active_fb = self.move_list[0]
        if active_fb.fb_stat != FbStat.BUSY_ACTIVATE:
            self.axis_error = True
            self.axis_state = AxisState.ERROR
            return
            
        if not active_fb.heartbeat_matches_tick(is_odd):
            active_fb.fb_stat = FbStat.ERROR
            self.axis_error = True
            self.axis_state = AxisState.ERROR
    
    def on_blend(self):
        if len(self.move_list) < 2:
            return
            
        # Only process blendable moves
        blendable_moves = []
        for move in self.move_list:
            if move.is_blendable():
                blendable_moves.append(move)
            else: 
                break
        
        if len(blendable_moves) < 2:
            return
            
        # Calculate end velocities based on buffer mode
        for i in range(len(blendable_moves) - 1):
            current_move = blendable_moves[i]
            next_move = blendable_moves[i + 1]
            
            if current_move.buffer_mode == BufferMode.mcBlendingLow:
                current_move.f_end_vel = min(current_move.vel.velocity, next_move.vel.velocity)
            elif current_move.buffer_mode == BufferMode.mcBlendingHigh:
                current_move.f_end_vel = max(current_move.vel.velocity, next_move.vel.velocity)
            elif current_move.buffer_mode == BufferMode.mcBlendingPrevious:
                current_move.f_end_vel = current_move.vel.velocity
            elif current_move.buffer_mode == BufferMode.mcBlendingNext:
                current_move.f_end_vel = next_move.vel.velocity
            
        # Apply blend to current move
        self.move_list[0].on_blend()
    
    def run_cycle(self):
        # Handle events
        if self.b_override_ev:
            for move in self.move_list:
                move.on_override(self.velocity_ratio)
            self.b_override_ev = False
        
        if self.b_blend_ev:
            self.on_blend()
            self.b_blend_ev = False
        
        if self.axis_state == AxisState.STANDSTILL:
            pass
        elif self.axis_state == AxisState.ERROR:
            pass
        else:
            if not self.move_list:
                self.axis_state = AxisState.ERROR
                return
            else:
                first_fb = self.move_list[0]
                if first_fb.fb_stat == FbStat.BUSY_ACTIVATE:
                    self.check_active_fb_heartbeat()
                elif first_fb.fb_stat == FbStat.ERROR:
                    self.axis_state = AxisState.ERROR
                    
        self.current_tick += 1    

def demo():
    """Demonstration of the motion event system / 运动事件系统演示"""
    print("Motion Event System Demo")
    print("运动事件系统演示\n")
    
    # Create motion buffer with 0.1s cycle time / 创建运动缓冲区，周期时间0.1秒
    mbuf = MotBuf(cycle_time=0.1)
    mbuf.current_position = 0.0
    
    # Create motion function blocks / 创建运动功能块
    mov1 = MovAbs()
    mov1.axis = mbuf
    mov1.position = 100.0
    mov1.velocity = 50.0
    mov1.buffer_mode = BufferMode.mcBlendingLow
    
    mov2 = MovRel()
    mov2.axis = mbuf
    mov2.distance = 50.0
    mov2.velocity = 40.0
    mov2.buffer_mode = BufferMode.mcBlendingHigh
    
    mov3 = MovAbs()
    mov3.axis = mbuf
    mov3.position = 200.0
    mov3.velocity = 30.0
    mov3.buffer_mode = BufferMode.mcBuffered  # Not blendable
    
    # Simulation loop / 仿真循环
    for i in range(40):
        print(f"\n=== Cycle {i} ===")
        
        # Trigger events at specific cycles / 在特定周期触发事件
        if i == 0:
            print("Triggering mov1.execute")
            mov1.execute = True
            
        if i == 1:
            mov1.execute = False  # Reset execute
            
        if i == 3:
            print("Triggering mov2.execute (should trigger blend)")
            mov2.execute = True
            
        if i == 4:
            mov2.execute = False  # Reset execute
            
        if i == 6:
            print("Triggering mov3.execute (should NOT trigger blend - not blendable)")
            mov3.execute = True
            
        if i == 7:
            mov3.execute = False  # Reset execute
            
        if i == 10:
            print("Enabling continuous update for mov1")
            mov1.continuous_update = True
            mov1.velocity = 60.0  # Change velocity
            
        if i == 15:
            print("Triggering override event")
            mbuf.trigger_override_event(0.8)
            
        if i == 20:
            print("Triggering mov1 reload")
            mov1.execute = True
            mov1.position = 150.0
            
        if i == 21:
            mov1.execute = False
        
        # Simulate missing heartbeat (skip mov1.run_cycle) / 模拟丢失心跳（跳过mov1.run_cycle）
        if i == 25:
            print("Simulating missing heartbeat for mov1 (skipping run_cycle)")
            mov2.run_cycle()
            mov3.run_cycle()
        else:
            # Run cycles / 运行周期
            mov1.run_cycle()
            mov2.run_cycle()
            mov3.run_cycle()
        
        mbuf.run_cycle()
        
        # Print status / 打印状态
        if mov1.busy or mov2.busy or mov3.busy or mbuf.move_list:
            print(f"  mov1: busy={mov1.busy}, active={mov1.active}, done={mov1.done}, error={mov1.error}")
            print(f"  mov2: busy={mov2.busy}, active={mov2.active}, done={mov2.done}, error={mov2.error}")
            print(f"  mov3: busy={mov3.busy}, active={mov3.active}, done={mov3.done}, error={mov3.error}")
            print(f"  mbuf: moves={len(mbuf.move_list)}, pos={mbuf.current_position:.2f}, tick={mbuf.current_tick}, axis_state={mbuf.axis_state.value}")

if __name__ == "__main__":
    demo() 