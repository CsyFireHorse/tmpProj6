#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
凸轮时序图绘制脚本
绘制凸轮在不同模式下的布尔信号时序图
支持 camIn 和 camOut 两种模式
"""

import matplotlib.pyplot as plt
import numpy as np
import argparse
from matplotlib.patches import Rectangle
import matplotlib.patches as mpatches

# 设置中文字体
plt.rcParams['font.sans-serif'] = ['SimHei', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False

def generate_camin_timing_data(mode='periodic'):
    """
    生成 camIn 时序图数据
    
    Args:
        mode: 'periodic' 或 'non_periodic'
    
    Returns:
        tuple: (time_points, signal_data, signal_labels)
    """
    # 时间轴设置
    time_points = np.arange(0, 101)  # 0-100，总共101个点
    
    # 信号定义（按从上到下的顺序）
    signals = ['CommandAborted', 'EndOfProfile', 'inSync', 'busy', 'execute']
    
    # 根据模式设置状态序列（按百分比分配，总时间100）
    if mode == 'periodic':
        # 周期模式：{idle, busy, EndOfProfile, Busy, EndOfProfile, Busy, CommandAborted}
        # idle: 5%, busy: 30%, EndOfProfile: 2%, busy: 30%, EndOfProfile: 2%, busy: 25%, CommandAborted: 3%, idle: 3%
        states = [
            'idle',      # 0-5 (5%)
            'busy',      # 5-35 (30%) - 很长
            'EndOfProfile', # 35-37 (2%) - 很短，瞬间
            'busy',      # 37-67 (30%) - 很长
            'EndOfProfile', # 67-69 (2%) - 很短，瞬间
            'busy',      # 69-94 (25%) - 很长
            'CommandAborted', # 94-97 (3%) - 很短
            'idle'       # 97-100 (3%) - 回到idle状态
        ]
        state_times = [0, 5, 35, 37, 67, 69, 94, 97, 101]  # 101确保覆盖到100
    else:
        # 非周期模式：{idle, busy, EndOfProfile&CommandAborted}
        # idle: 5%, busy: 30%, EndOfProfile: 60%, CommandAborted: 3%, idle: 2%
        states = [
            'idle',      # 0-5 (5%)
            'busy',      # 5-35 (30%) - 很长
            'EndOfProfile', # 35-95 (60%) - 最长
            'CommandAborted', # 95-98 (3%) - 很短
            'idle'       # 98-100 (2%) - 回到idle状态
        ]
        state_times = [0, 5, 35, 95, 98, 101]  # 101确保覆盖到100
    
    # 生成每个信号的数据
    signal_data = []
    for signal in signals:
        signal_values = []
        for t in time_points:
            # 确定当前时间点处于哪个状态
            current_state = 'idle'
            for j in range(len(state_times) - 1):
                if state_times[j] <= t < state_times[j + 1]:
                    current_state = states[j]
                    break
            if t >= state_times[-1]:
                current_state = states[-1]
            
            # 根据状态确定信号值
            if signal == 'execute':
                value = 1 if current_state != 'idle' else 0
            elif signal == 'busy':
                value = 1 if current_state in ['busy', 'EndOfProfile', 'CommandAborted'] else 0
            elif signal == 'inSync':
                value = 1 if current_state in ['busy', 'EndOfProfile'] else 0
            elif signal == 'EndOfProfile':
                value = 1 if current_state == 'EndOfProfile' else 0
            elif signal == 'CommandAborted':
                value = 1 if current_state == 'CommandAborted' else 0
            
            signal_values.append(value)
        signal_data.append(signal_values)
    
    return time_points, signal_data, signals

def generate_camout_timing_data():
    """
    生成 camOut 时序图数据
    
    Returns:
        tuple: (time_points, signal_data, signal_labels)
    """
    # 时间轴设置
    time_points = np.arange(0, 101)  # 0-100，总共101个点
    
    # 信号定义（按从上到下的顺序）
    signals = ['Stop.Done', 'Stop.Execute', 'Done', 'Busy', 'Execute']
    
    # camOut 状态序列：{idle, busy, Done, StopBusy, StopBusy&ExecuteLow, StopDone, idle}
    # idle: 5%, busy: 3%, Done: 70%, StopBusy: 10%, StopBusy&ExecuteLow: 5%, StopDone: 5%, idle: 2%
    states = [
        'idle',      # 0-5 (5%)
        'busy',      # 5-8 (3%) - 很短
        'Done',      # 8-78 (70%) - 很长
        'StopBusy',  # 78-88 (10%) - 中等
        'StopBusy&ExecuteLow', # 88-93 (5%) - 短
        'StopDone',  # 93-98 (5%) - 短
        'idle'       # 98-100 (2%) - 回到idle状态
    ]
    state_times = [0, 5, 8, 78, 88, 93, 98, 101]  # 101确保覆盖到100
    
    # 生成每个信号的数据
    signal_data = []
    for signal in signals:
        signal_values = []
        for t in time_points:
            # 确定当前时间点处于哪个状态
            current_state = 'idle'
            for j in range(len(state_times) - 1):
                if state_times[j] <= t < state_times[j + 1]:
                    current_state = states[j]
                    break
            if t >= state_times[-1]:
                current_state = states[-1]
            
            # 根据状态确定信号值
            if signal == 'Execute':
                value = 1 if current_state in ['busy', 'Done', 'StopBusy'] else 0
            elif signal == 'Busy':
                value = 1 if current_state in ['busy'] else 0
            elif signal == 'Done':
                value = 1 if current_state in ['Done', 'StopBusy'] else 0
            elif signal == 'Stop.Execute':
                value = 1 if current_state in ['StopBusy', 'StopBusy&ExecuteLow', 'StopDone'] else 0
            elif signal == 'Stop.Done':
                value = 1 if current_state == 'StopDone' else 0
            
            signal_values.append(value)
        signal_data.append(signal_values)
    
    return time_points, signal_data, signals

def plot_timing_diagram(time_points, signal_data, signal_labels, title, save_path=None):
    """
    通用的时序图绘图函数
    
    Args:
        time_points: 时间点数组
        signal_data: 信号数据列表，每个元素是一个信号的数值列表
        signal_labels: 信号标签列表
        title: 图表标题
        save_path: 保存路径，如果为None则显示图片
    """
    # 创建图形
    fig, ax = plt.subplots(figsize=(12, 8))
    
    # 绘制每个信号
    for i, (signal_values, signal_label) in enumerate(zip(signal_data, signal_labels)):
        y_pos = i * 2  # 增加行间隔，每行间隔2个单位
        
        # 绘制阶梯图（去除颜色，使用黑色）
        ax.step(time_points, [v + y_pos for v in signal_values], 
                where='post', linewidth=2, color='black', label=signal_label)
    
    # 设置图形属性
    ax.set_xlim(0, max(time_points))
    ax.set_ylim(-0.5, len(signal_labels) * 2 + 2)
    ax.set_xlabel('时间', fontsize=12)
    ax.set_ylabel('信号', fontsize=12)
    ax.set_title(title, fontsize=14, fontweight='bold')
    
    # 设置y轴标签（按从上到下的顺序）
    ax.set_yticks([i * 2 + 0.5 for i in range(len(signal_labels))])
    ax.set_yticklabels(signal_labels)
    
    # 去除网格
    ax.grid(False)
    
    # 添加图例
    ax.legend(loc='upper right', bbox_to_anchor=(1, 1))
    
    plt.tight_layout()
    
    if save_path:
        plt.savefig(save_path, dpi=300, bbox_inches='tight')
        print(f"时序图已保存到: {save_path}")
    else:
        plt.show()
    
    plt.close()

def create_camin_timing_diagram(mode='periodic', save_path=None):
    """
    创建 camIn 时序图
    
    Args:
        mode: 'periodic' 或 'non_periodic'
        save_path: 保存路径，如果为None则显示图片
    """
    # 生成数据
    time_points, signal_data, signal_labels = generate_camin_timing_data(mode)
    
    # 绘制图表
    title = f'camIn 时序图 - {"周期模式" if mode == "periodic" else "非周期模式"}'
    plot_timing_diagram(time_points, signal_data, signal_labels, title, save_path)

def create_camout_timing_diagram(save_path=None):
    """
    创建 camOut 时序图
    
    Args:
        save_path: 保存路径，如果为None则显示图片
    """
    # 生成数据
    time_points, signal_data, signal_labels = generate_camout_timing_data()
    
    # 绘制图表
    title = 'camOut 时序图'
    plot_timing_diagram(time_points, signal_data, signal_labels, title, save_path)

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description='生成凸轮时序图')
    parser.add_argument('--type', choices=['camin', 'camout'], default='camin',
                       help='时序图类型：camin 或 camout (默认: camin)')
    parser.add_argument('--mode', choices=['periodic', 'non_periodic'], default='periodic',
                       help='camIn 模式：periodic 或 non_periodic (默认: periodic)')
    parser.add_argument('--output', type=str, help='输出文件名')
    
    args = parser.parse_args()
    
    if args.type == 'camin':
        print(f"生成 camIn 时序图 - {args.mode} 模式...")
        if args.output:
            save_path = args.output
        else:
            save_path = f'cam_timing_camin_{args.mode}.png'
        create_camin_timing_diagram(args.mode, save_path)
        print(f"camIn 时序图生成完成: {save_path}")
    elif args.type == 'camout':
        print("生成 camOut 时序图...")
        if args.output:
            save_path = args.output
        else:
            save_path = 'cam_timing_camout.png'
        create_camout_timing_diagram(save_path)
        print(f"camOut 时序图生成完成: {save_path}")

if __name__ == "__main__":
    main()
