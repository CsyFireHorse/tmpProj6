#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Blending Demo - 运动混合演示脚本
演示两个MoveAbsolute功能块的混合运动

绘图要素：
- axis.pos: 轴实际位置
- axis.vel: 轴实际速度
- moveAbs.active: 第一个MoveAbsolute激活状态
- moveAbs2.active: 第二个MoveAbsolute激活状态

Blending Demo 1:
- moveAbs: Pos=100, Vel=100, acc=1000, dec=1000, jerk=0
- moveAbs2: Pos=200, Vel=150, acc=1000, dec=1000, jerk=0, bufferMode=BlendingHigh

Blending Demo 2:
- moveAbs: Pos=100, Vel=100, acc=1000, dec=1000, jerk=0
- moveAbs2: Pos=200, Vel=500, acc=1000, dec=1000, jerk=0, bufferMode=BlendingHigh

使用方法:
1. 运行Demo 1: python blending_demo.py --demo 1
2. 运行Demo 2: python blending_demo.py --demo 2
3. 运行两个Demo: python blending_demo.py --demo all
"""

import sys
import os
import argparse
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.font_manager import FontProperties

# 添加项目路径
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

try:
    from pymc import (
        FbAxisWrap, AxisCiaConfigWrapper, FbPower, FbMoveAbsolute,
        FbReadSetValues, FbReadStatus, Direction, BufferMode
    )
    print("所有模块导入成功")
except ImportError as e:
    print(f"Import error: {e}")
    print("Please ensure pymc module is available")
    sys.exit(1)

def create_blending_demo(demo_type=1):
    """创建blending演示
    
    Args:
        demo_type: 演示类型 (1或2)
    """
    # 仿真参数
    total_cycles = 350  # 总周期数
    dt = 0.01  # 时间步长 (10ms)
    
    # 创建轴
    axis = FbAxisWrap()
    
    # 配置轴
    cfg = AxisCiaConfigWrapper()
    cfg.set_virtual_mode(True)
    cfg.set_cycle_time(int(dt*1000000))  # us
    cfg.set_id(0)
    axis.set_config(cfg.get_config_ptr())
    
    # 功能块
    power = FbPower()
    moveAbs = FbMoveAbsolute()
    moveAbs2 = FbMoveAbsolute()
    readSet = FbReadSetValues()
    readStatus = FbReadStatus()
    
    # 轴绑定
    power.Axis = axis.ref
    moveAbs.Axis = axis.ref
    moveAbs2.Axis = axis.ref
    readSet.Axis = axis.ref
    readStatus.Axis = axis.ref
    
    # 根据演示类型设置参数
    if demo_type == 1:
        # Demo 1: 低速混合
        moveAbs_params = {
            'position': 100.0,
            'velocity': 100.0,
            'acceleration': 1000.0,
            'deceleration': 1000.0,
            'jerk': 0.0,
            'direction': Direction.mcPositiveDirection,
            'buffer_mode': BufferMode.mcBlendingHigh
        }
        moveAbs2_params = {
            'position': 200.0,
            'velocity': 150.0,
            'acceleration': 1000.0,
            'deceleration': 1000.0,
            'jerk': 0.0,
            'direction': Direction.mcPositiveDirection,
            'buffer_mode': BufferMode.mcBlendingHigh
        }
        demo_name = "Blending Demo 1 (低速混合)"
    else:
        # Demo 2: 高速混合
        moveAbs_params = {
            'position': 100.0,
            'velocity': 100.0,
            'acceleration': 1000.0,
            'deceleration': 1000.0,
            'jerk': 0.0,
            'direction': Direction.mcPositiveDirection,
            'buffer_mode': BufferMode.mcBlendingHigh
        }
        moveAbs2_params = {
            'position': 200.0,
            'velocity': 500.0,
            'acceleration': 1000.0,
            'deceleration': 1000.0,
            'jerk': 0.0,
            'direction': Direction.mcPositiveDirection,
            'buffer_mode': BufferMode.mcBlendingHigh
        }
        demo_name = "Blending Demo 2 (高速混合)"
    
    # 数据记录
    time_data = []
    axis_pos_data = []
    axis_vel_data = []
    moveAbs_activate_data = []
    moveAbs2_activate_data = []
    
    print(f"开始{demo_name}")
    print(f"moveAbs: Pos={moveAbs_params['position']}, Vel={moveAbs_params['velocity']}")
    print(f"moveAbs2: Pos={moveAbs2_params['position']}, Vel={moveAbs2_params['velocity']}, BufferMode=BlendingHigh")
    
    for idx in range(total_cycles):
        current_time = idx * dt
        
        # 控制逻辑
        if idx == 1:
            # 上电
            power.Enable = True
            power.bRegulatorOn = True
            power.bDriveStart = True
            
        if idx == 2:
            # 启用读取功能块
            readSet.Enable = True
            readStatus.Enable = True
            
        if idx == 10:
            # 启动第一个MoveAbsolute
            moveAbs.set_move_params(**moveAbs_params)
            moveAbs.Execute = True
            print(f"启动第一个MoveAbsolute到位置 {moveAbs_params['position']}")
            
        if idx == 21:
            # 启动第二个MoveAbsolute (在第一个运动进行中)
            moveAbs2.set_move_params(**moveAbs2_params)
            moveAbs2.Execute = True
            print(f"启动第二个MoveAbsolute到位置 {moveAbs2_params['position']} (混合模式)")

        # 运行功能块
        power.runCycle()
        moveAbs.runCycle()
        moveAbs2.runCycle()

        
        # 轴运行
        axis.run_cycle()
        readSet.runCycle()
        readStatus.runCycle()
        # 记录数据
        time_data.append(current_time)
        axis_pos_data.append(readSet.Position)
        axis_vel_data.append(readSet.Velocity)
        moveAbs_activate_data.append(1.0 if moveAbs.Active else 0.0)
        moveAbs2_activate_data.append(1.0 if moveAbs2.Active else 0.0)
        
        # 进度显示
        if idx % 200 == 0:
            print(f"进度: {idx}/{total_cycles} ({idx/total_cycles*100:.1f}%)")
    
    print("仿真完成，开始绘图...")
    
    # 绘图
    plt.rcParams['font.sans-serif'] = ['SimHei', 'Arial Unicode MS', 'DejaVu Sans']
    plt.rcParams['axes.unicode_minus'] = False
    
    fig, axes = plt.subplots(4, 1, figsize=(12, 10))
    fig.suptitle(f'{demo_name} - 运动混合分析', fontsize=14, fontweight='bold')
    
    # 轴位置
    axes[0].plot(time_data, axis_pos_data, 'b-', linewidth=2, label='轴位置')
    axes[0].axhline(y=moveAbs_params['position'], color='g', linestyle='--', alpha=0.7, label=f'目标1: {moveAbs_params["position"]}')
    axes[0].axhline(y=moveAbs2_params['position'], color='r', linestyle='--', alpha=0.7, label=f'目标2: {moveAbs2_params["position"]}')
    axes[0].set_ylabel('位置 (度)', fontsize=10)
    axes[0].grid(True, alpha=0.3)
    axes[0].legend()
    
    # 轴速度
    axes[1].plot(time_data, axis_vel_data, 'g-', linewidth=2, label='轴速度')
    axes[1].axhline(y=moveAbs_params['velocity'], color='g', linestyle='--', alpha=0.7, label=f'速度1: {moveAbs_params["velocity"]}')
    axes[1].axhline(y=moveAbs2_params['velocity'], color='r', linestyle='--', alpha=0.7, label=f'速度2: {moveAbs2_params["velocity"]}')
    axes[1].set_ylabel('速度 (度/s)', fontsize=10)
    axes[1].grid(True, alpha=0.3)
    axes[1].legend()
    
    # MoveAbs激活状态
    axes[2].plot(time_data, moveAbs_activate_data, 'orange', linewidth=2, label='MoveAbs1 Active')
    axes[2].set_ylabel('MoveAbs1 状态', fontsize=10)
    axes[2].set_ylim(-0.1, 1.1)
    axes[2].grid(True, alpha=0.3)
    axes[2].legend()
    
    # MoveAbs2激活状态
    axes[3].plot(time_data, moveAbs2_activate_data, 'purple', linewidth=2, label='MoveAbs2 Active')
    axes[3].set_ylabel('MoveAbs2 状态', fontsize=10)
    axes[3].set_xlabel('时间 (s)', fontsize=10)
    axes[3].set_ylim(-0.1, 1.1)
    axes[3].grid(True, alpha=0.3)
    axes[3].legend()
    
    # 调整布局
    plt.tight_layout()
    
    # 保存图片
    output_file = f'blending_demo_{demo_type}.png'
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"图片已保存: {output_file}")
    
    # 显示图片
    plt.show()
    
    # 打印统计信息
    print("\n=== 仿真统计信息 ===")
    print(f"仿真时间: {time_data[-1]:.2f} 秒")
    print(f"轴位置范围: {min(axis_pos_data):.2f} ~ {max(axis_pos_data):.2f} 度")
    print(f"轴速度范围: {min(axis_vel_data):.2f} ~ {max(axis_vel_data):.2f} 度/s")
    print(f"MoveAbs1 激活时间: {sum(moveAbs_activate_data) * dt:.2f} 秒")
    print(f"MoveAbs2 激活时间: {sum(moveAbs2_activate_data) * dt:.2f} 秒")
    
    return {
        'time_data': time_data,
        'axis_pos_data': axis_pos_data,
        'axis_vel_data': axis_vel_data,
        'moveAbs_activate_data': moveAbs_activate_data,
        'moveAbs2_activate_data': moveAbs2_activate_data,
        'demo_type': demo_type
    }

def create_comparison_plot(result1, result2):
    """创建两个演示的对比图"""
    plt.rcParams['font.sans-serif'] = ['SimHei', 'Arial Unicode MS', 'DejaVu Sans']
    plt.rcParams['axes.unicode_minus'] = False
    
    fig, axes = plt.subplots(4, 2, figsize=(16, 12))
    fig.suptitle('Blending Demo 对比分析', fontsize=16, fontweight='bold')
    
    # 轴位置对比
    axes[0, 0].plot(result1['time_data'], result1['axis_pos_data'], 'b-', linewidth=2, label='轴位置')
    axes[0, 0].set_ylabel('位置 (度)', fontsize=10)
    axes[0, 0].set_title('Demo 1: 低速混合', fontsize=12)
    axes[0, 0].grid(True, alpha=0.3)
    axes[0, 0].legend()
    
    axes[0, 1].plot(result2['time_data'], result2['axis_pos_data'], 'b-', linewidth=2, label='轴位置')
    axes[0, 1].set_ylabel('位置 (度)', fontsize=10)
    axes[0, 1].set_title('Demo 2: 高速混合', fontsize=12)
    axes[0, 1].grid(True, alpha=0.3)
    axes[0, 1].legend()
    
    # 轴速度对比
    axes[1, 0].plot(result1['time_data'], result1['axis_vel_data'], 'g-', linewidth=2, label='轴速度')
    axes[1, 0].set_ylabel('速度 (度/s)', fontsize=10)
    axes[1, 0].grid(True, alpha=0.3)
    axes[1, 0].legend()
    
    axes[1, 1].plot(result2['time_data'], result2['axis_vel_data'], 'g-', linewidth=2, label='轴速度')
    axes[1, 1].set_ylabel('速度 (度/s)', fontsize=10)
    axes[1, 1].grid(True, alpha=0.3)
    axes[1, 1].legend()
    
    # MoveAbs1激活状态对比
    axes[2, 0].plot(result1['time_data'], result1['moveAbs_activate_data'], 'orange', linewidth=2, label='MoveAbs1 Active')
    axes[2, 0].set_ylabel('MoveAbs1 状态', fontsize=10)
    axes[2, 0].set_ylim(-0.1, 1.1)
    axes[2, 0].grid(True, alpha=0.3)
    axes[2, 0].legend()
    
    axes[2, 1].plot(result2['time_data'], result2['moveAbs_activate_data'], 'orange', linewidth=2, label='MoveAbs1 Active')
    axes[2, 1].set_ylabel('MoveAbs1 状态', fontsize=10)
    axes[2, 1].set_ylim(-0.1, 1.1)
    axes[2, 1].grid(True, alpha=0.3)
    axes[2, 1].legend()
    
    # MoveAbs2激活状态对比
    axes[3, 0].plot(result1['time_data'], result1['moveAbs2_activate_data'], 'purple', linewidth=2, label='MoveAbs2 Active')
    axes[3, 0].set_ylabel('MoveAbs2 状态', fontsize=10)
    axes[3, 0].set_xlabel('时间 (s)', fontsize=10)
    axes[3, 0].set_ylim(-0.1, 1.1)
    axes[3, 0].grid(True, alpha=0.3)
    axes[3, 0].legend()
    
    axes[3, 1].plot(result2['time_data'], result2['moveAbs2_activate_data'], 'purple', linewidth=2, label='MoveAbs2 Active')
    axes[3, 1].set_ylabel('MoveAbs2 状态', fontsize=10)
    axes[3, 1].set_xlabel('时间 (s)', fontsize=10)
    axes[3, 1].set_ylim(-0.1, 1.1)
    axes[3, 1].grid(True, alpha=0.3)
    axes[3, 1].legend()
    
    plt.tight_layout()
    plt.savefig('blending_demo_comparison.png', dpi=300, bbox_inches='tight')
    print("对比图已保存: blending_demo_comparison.png")
    plt.show()

def parse_arguments():
    """解析命令行参数"""
    parser = argparse.ArgumentParser(description='Blending Demo - 运动混合演示脚本')
    
    parser.add_argument('--demo', choices=['1', '2', 'all'], default='1',
                       help='演示类型: 1=低速混合, 2=高速混合, all=两个演示对比')
    
    return parser.parse_args()

if __name__ == "__main__":
    try:
        args = parse_arguments()
        
        if args.demo == 'all':
            print("=== 运行两个Blending演示进行对比 ===")
            result1 = create_blending_demo(1)
            result2 = create_blending_demo(2)
            create_comparison_plot(result1, result2)
        else:
            demo_type = int(args.demo)
            create_blending_demo(demo_type)
        
    except Exception as e:
        print(f"运行错误: {e}")
        import traceback
        traceback.print_exc()
