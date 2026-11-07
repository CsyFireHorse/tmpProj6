#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Cam Plot Base - 凸轮基础绘图脚本
基于凸轮运动控制的核心要素进行绘图分析

绘图要素：
- execute: 凸轮执行状态
- master.pos: 主轴实际位置
- slave.pos: 从轴实际位置  
- cam.master.pos: 凸轮主轴位置（经过缩放和偏移处理）
- cam.slave.pos: 凸轮从轴位置（通过GetCamSlaveSetPosition计算）
- endOfProfile: 凸轮周期结束标志

主轴位置计算：
- 绝对模式: cam.master.pos = mod(MasterScaling*MasterPosition + MasterOffset, modulo)
- 相对模式: cam.master.pos = mod(MasterScaling*(MasterPosition-masterBeginPos) + MasterOffset, modulo)

从轴位置计算：
- cam.slave.pos = SlaveScaling*CAM(MasterScaling*MasterPosition + MasterOffset) + SlaveOffset
- 通过GetCamSlaveSetPosition函数计算

使用方法:
1. 基础演示: python cam_plot_base.py
2. 非周期模式: python cam_plot_base.py nonperiodic
3. 周期模式: python cam_plot_base.py periodic
4. 比较演示: python cam_plot_base.py compare
"""

import sys
import os
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.font_manager import FontProperties

# 添加项目路径
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

try:
    from pymc import (
        FbAxisWrap, AxisCiaConfigWrapper, FbPower, FbMoveVelocity,
        FbReadCAM, FbCamTableSelect, FbCamIn,
        FbReadSetValues, FbSetMovementType, FbReadStatus,
        FbGetCamSlaveSetPosition
    )
except ImportError as e:
    print(f"Import error: {e}")
    print("Please ensure pymc module is available")
    sys.exit(1)

def create_cam_base_demo(periodic=True, start_pos=100, trigger_pos=120, master_offset=0, slave_offset=0):
    """创建凸轮基础演示
    
    Args:
        periodic: 是否周期模式
        start_pos: 主轴起点位置
        trigger_pos: 凸轮启动位置
        master_offset: 主轴偏移
        slave_offset: 从轴偏移
    """
    fname = b'abc3.cam'  # 凸轮文件
    
    # 创建主从轴
    axis_master = FbAxisWrap()
    axis_slave = FbAxisWrap()
    
    # 配置主轴 - 模轴360
    cfgM = AxisCiaConfigWrapper()
    cfgM.set_virtual_mode(True)
    cfgM.set_cycle_time(10000)  # 10ms
    cfgM.set_id(0)
    axis_master.set_config(cfgM.get_config_ptr())
    
    # 配置从轴 - 模轴360
    cfgS = AxisCiaConfigWrapper()
    cfgS.set_virtual_mode(True)
    cfgS.set_cycle_time(10000)  # 10ms
    cfgS.set_id(1)
    axis_slave.set_config(cfgS.get_config_ptr())
    
    # 功能块
    powerM = FbPower()
    powerS = FbPower()
    moveVel = FbMoveVelocity()
    readCam = FbReadCAM()
    camTbl = FbCamTableSelect()
    camIn = FbCamIn()
    
    # 读取设定值功能块
    readSetM = FbReadSetValues()
    readSetS = FbReadSetValues()
    
    # 读取状态功能块
    readStatusM = FbReadStatus()
    readStatusS = FbReadStatus()
    
    # 设置运动类型功能块
    setMoveTypeM = FbSetMovementType()
    setMoveTypeS = FbSetMovementType()
    
    # 获取凸轮从轴设定位置功能块
    getCamSlavePos = FbGetCamSlaveSetPosition()
    
    # 轴绑定
    powerM.Axis = axis_master.ref
    powerS.Axis = axis_slave.ref
    moveVel.Axis = axis_master.ref
    camTbl.Master = axis_master.ref
    camTbl.Slave = axis_slave.ref
    camIn.Master = axis_master.ref
    camIn.Slave = axis_slave.ref
    readSetM.Axis = axis_master.ref
    readSetS.Axis = axis_slave.ref
    readStatusM.Axis = axis_master.ref
    readStatusS.Axis = axis_slave.ref
    setMoveTypeM.Axis = axis_master.ref
    setMoveTypeS.Axis = axis_slave.ref
    getCamSlavePos.Master = axis_master.ref
    getCamSlavePos.Slave = axis_slave.ref
    
    # 参数设置
    moveVel.set_move_params(velocity=50, acceleration=500, deceleration=500, jerk=0)
    
    # 凸轮表选择参数 - 相对模式
    camTbl.Periodic = periodic
    camTbl.MasterAbsolute = False  # 主轴相对模式
    camTbl.SlaveAbsolute = True    # 从轴绝对模式
    
    # 凸轮输入参数
    camIn.MasterOffset = master_offset
    camIn.SlaveOffset = slave_offset
    camIn.MasterScaling = 1
    camIn.SlaveScaling = 1
    camIn.Acceleration = 1000
    camIn.Deceleration = 1000
    camIn.Jerk = 0
    camIn.TappetHysteresis = 0.0
    camIn.StartMode = 0  # relative mode
    
    # GetCamSlaveSetPosition参数配置
    getCamSlavePos.Enable = True
    getCamSlavePos.MasterOffset = master_offset
    getCamSlavePos.SlaveOffset = slave_offset
    getCamSlavePos.MasterScaling = 1
    getCamSlavePos.SlaveScaling = 1
    
    # 设置运动类型为模轴
    setMoveTypeM.iMovementType = 360  # 主轴模轴360
    setMoveTypeS.iMovementType = 360  # 从轴模轴360
    
    # 数据记录
    time_data = []
    master_pos_data = []
    slave_pos_data = []
    slave_vel_data = []
    endofprofile_data = []
    execute_data = []
    cam_master_pos_data = []
    cam_slave_pos_data = []
    
    # 仿真参数
    total_cycles = 1200  # 总周期数
    dt = 0.01  # 时间步长 (10ms)
    
    # 状态变量
    master_begin_pos = 0
    cam_triggered = False
    cam_start_time = 0
    
    print(f"开始凸轮基础演示 - 周期模式: {periodic}")
    print(f"主轴起点: {start_pos}, 触发位置: {trigger_pos}")
    
    for idx in range(total_cycles):
        current_time = idx * dt

        if idx == 1:
            # 设置运动类型
            setMoveTypeM.bExecute = True
            setMoveTypeS.bExecute = True
            
        # 控制逻辑
        if idx == 1:
            # 上电
            powerM.Enable = True
            powerM.bRegulatorOn = True
            powerM.bDriveStart = True
            powerS.Enable = True
            powerS.bRegulatorOn = True
            powerS.bDriveStart = True
            
        if idx == 2:
            # 启用读取功能块
            readSetM.Enable = True
            readSetS.Enable = True
            readStatusM.Enable = True
            readStatusS.Enable = True
            
        if idx == 5:
            # 读取凸轮文件
            readCam.bExecute = True
            readCam.sFileName = fname
            
        if idx == 8:
            # 选择凸轮表
            camTbl.Execute = True
            camTbl.CamTable = readCam.CAM
            
        if idx == 15:
            # 启动主轴运动
            moveVel.Execute = True
            
        # 检查是否到达触发位置
        if not cam_triggered and readSetM.Position >= trigger_pos:
            # 启动凸轮
            master_begin_pos = readSetM.Position  # 记录凸轮触发时的主轴位置
            camIn.Execute = True
            camIn.CamTableID = camTbl.CamTableID
            getCamSlavePos.CamTableID = camTbl.CamTableID
            cam_triggered = True
            cam_start_time = current_time
            print(f"凸轮在位置 {readSetM.Position:.2f} 启动，时间 {current_time:.2f}s")
        
        # 运行功能块
        powerM.runCycle()
        powerS.runCycle()
        readCam.runCycle()
        camTbl.runCycle()
        moveVel.runCycle()
        camIn.runCycle()
        readSetM.runCycle()
        readSetS.runCycle()
        readStatusM.runCycle()
        readStatusS.runCycle()
        setMoveTypeM.runCycle()
        setMoveTypeS.runCycle()
        getCamSlavePos.runCycle()
        
        # 轴运行
        axis_master.run_cycle()
        axis_slave.run_cycle()
        
        # 计算凸轮主轴位置（相对模式）
        if cam_triggered:
            # 相对模式: mod(MasterScaling*(MasterPosition-masterBeginPos) + MasterOffset, modulo)
            master_pos = readSetM.Position
            cam_master_pos = (camIn.MasterScaling * (master_pos - master_begin_pos) + camIn.MasterOffset) % 360
            
            # 凸轮从轴位置通过GetCamSlaveSetPosition计算
            # cam_slave_pos = getCamSlavePos.fStartPosition
            cam_slave_pos = (readSetS.Position - camIn.SlaveOffset)/camIn.SlaveScaling
        else:
            cam_master_pos = camIn.MasterOffset
            cam_slave_pos = camIn.SlaveOffset
        
        # 记录数据 - 记录所有时刻的数据
        time_data.append(current_time)
        master_pos_data.append(readSetM.Position)
        slave_pos_data.append(readSetS.Position)
        slave_vel_data.append(readSetS.Velocity)
        endofprofile_data.append(1.0 if camIn.EndOfProfile else 0.0)
        execute_data.append(1.0 if camIn.Execute else 0.0)
        cam_master_pos_data.append(cam_master_pos)
        cam_slave_pos_data.append(cam_slave_pos)
        
        # 进度显示
        if idx % 300 == 0:
            print(f"进度: {idx}/{total_cycles} ({idx/total_cycles*100:.1f}%)")
    
    print("仿真完成，开始绘图...")
    
    # 生成参数描述
    mode_desc = "周期" if periodic else "非周期"
    param_desc = f"{mode_desc}模式"
    
    # 绘图
    plt.rcParams['font.sans-serif'] = ['SimHei', 'Arial Unicode MS', 'DejaVu Sans']
    plt.rcParams['axes.unicode_minus'] = False
    
    fig, axes = plt.subplots(6, 1, figsize=(12, 14))
    fig.suptitle(f'凸轮演示-{param_desc}', fontsize=14, fontweight='bold')
    
    # Execute 状态
    axes[0].plot(time_data, execute_data, 'orange', linewidth=2, label='Execute')
    axes[0].set_ylabel('Execute', fontsize=10)
    axes[0].set_ylim(-0.1, 1.1)
    axes[0].grid(True, alpha=0.3)
    axes[0].legend()
    
    # 主轴实际位置
    axes[1].plot(time_data, master_pos_data, 'b-', linewidth=1.5, label='主轴实际位置')
    axes[1].set_ylabel('主轴位置 (度)', fontsize=10)
    axes[1].grid(True, alpha=0.3)
    axes[1].legend()
    
    # 凸轮主轴位置
    axes[2].plot(time_data, cam_master_pos_data, 'c-', linewidth=1.5, label='凸轮主轴位置')
    axes[2].set_ylabel('凸轮主轴位置 (度)', fontsize=10)
    axes[2].grid(True, alpha=0.3)
    axes[2].legend()
    
    # 凸轮从轴位置
    axes[3].plot(time_data, cam_slave_pos_data, 'm-', linewidth=1.5, label='凸轮从轴位置')
    axes[3].set_ylabel('凸轮从轴位置 (度)', fontsize=10)
    axes[3].grid(True, alpha=0.3)
    axes[3].legend()
    
    # 从轴速度
    axes[4].plot(time_data, slave_vel_data, 'g-', linewidth=1.5, label='从轴速度')
    axes[4].set_ylabel('从轴速度 (度/s)', fontsize=10)
    axes[4].grid(True, alpha=0.3)
    axes[4].legend()
    
    # EndOfProfile
    axes[5].plot(time_data, endofprofile_data, 'r-', linewidth=2, label='EndOfProfile')
    axes[5].set_ylabel('EndOfProfile', fontsize=10)
    axes[5].set_ylim(-0.1, 1.1)
    axes[5].set_xlabel('时间 (s)', fontsize=10)
    axes[5].grid(True, alpha=0.3)
    axes[5].legend()
    
    # 调整布局
    plt.tight_layout()
    
    # 保存图片
    filename_suffix = f"_P{periodic}_S{start_pos}_T{trigger_pos}"
    output_file = f'cam_plot_base{filename_suffix}.png'
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"图片已保存: {output_file}")
    
    # 显示图片
    plt.show()
    
    # 打印统计信息
    print("\n=== 仿真统计信息 ===")
    print(f"仿真时间: {time_data[-1]:.2f} 秒")
    print(f"主轴位置范围: {min(master_pos_data):.2f} ~ {max(master_pos_data):.2f} 度")
    print(f"从轴位置范围: {min(slave_pos_data):.2f} ~ {max(slave_pos_data):.2f} 度")
    print(f"凸轮主轴位置范围: {min(cam_master_pos_data):.2f} ~ {max(cam_master_pos_data):.2f} 度")
    print(f"凸轮从轴位置范围: {min(cam_slave_pos_data):.2f} ~ {max(cam_slave_pos_data):.2f} 度")
    print(f"EndOfProfile 触发次数: {sum(endofprofile_data)}")
    print(f"Execute 触发次数: {sum(execute_data)}")
    
    return {
        'time_data': time_data,
        'master_pos_data': master_pos_data,
        'slave_pos_data': slave_pos_data,
        'slave_vel_data': slave_vel_data,
        'endofprofile_data': endofprofile_data,
        'execute_data': execute_data,
        'cam_master_pos_data': cam_master_pos_data,
        'cam_slave_pos_data': cam_slave_pos_data,
        'params': {
            'periodic': periodic,
            'start_pos': start_pos,
            'trigger_pos': trigger_pos,
            'master_offset': master_offset,
            'slave_offset': slave_offset
        }
    }

def create_comparison_plot(result1, result2, title1, title2, filename):
    """创建对比图"""
    plt.rcParams['font.sans-serif'] = ['SimHei', 'Arial Unicode MS', 'DejaVu Sans']
    plt.rcParams['axes.unicode_minus'] = False
    
    fig, axes = plt.subplots(6, 2, figsize=(16, 14))
    fig.suptitle(f'凸轮基础对比: {title1} vs {title2}', fontsize=16, fontweight='bold')
    
    # Execute对比
    axes[0, 0].plot(result1['time_data'], result1['execute_data'], 'orange', linewidth=2, label='Execute')
    axes[0, 0].set_ylabel('Execute', fontsize=10)
    axes[0, 0].set_ylim(-0.1, 1.1)
    axes[0, 0].set_title(f'{title1} - Execute', fontsize=12)
    axes[0, 0].grid(True, alpha=0.3)
    axes[0, 0].legend()
    
    axes[0, 1].plot(result2['time_data'], result2['execute_data'], 'orange', linewidth=2, label='Execute')
    axes[0, 1].set_ylabel('Execute', fontsize=10)
    axes[0, 1].set_ylim(-0.1, 1.1)
    axes[0, 1].set_title(f'{title2} - Execute', fontsize=12)
    axes[0, 1].grid(True, alpha=0.3)
    axes[0, 1].legend()
    
    # 主轴实际位置对比
    axes[1, 0].plot(result1['time_data'], result1['master_pos_data'], 'b-', linewidth=1.5, label='主轴实际位置')
    axes[1, 0].set_ylabel('主轴位置 (度)', fontsize=10)
    axes[1, 0].set_title(f'{title1} - 主轴实际位置', fontsize=12)
    axes[1, 0].grid(True, alpha=0.3)
    axes[1, 0].legend()
    
    axes[1, 1].plot(result2['time_data'], result2['master_pos_data'], 'b-', linewidth=1.5, label='主轴实际位置')
    axes[1, 1].set_ylabel('主轴位置 (度)', fontsize=10)
    axes[1, 1].set_title(f'{title2} - 主轴实际位置', fontsize=12)
    axes[1, 1].grid(True, alpha=0.3)
    axes[1, 1].legend()
    
    # 从轴实际位置对比
    axes[2, 0].plot(result1['time_data'], result1['slave_pos_data'], 'g-', linewidth=1.5, label='从轴实际位置')
    axes[2, 0].set_ylabel('从轴位置 (度)', fontsize=10)
    axes[2, 0].set_title(f'{title1} - 从轴实际位置', fontsize=12)
    axes[2, 0].grid(True, alpha=0.3)
    axes[2, 0].legend()
    
    axes[2, 1].plot(result2['time_data'], result2['slave_pos_data'], 'g-', linewidth=1.5, label='从轴实际位置')
    axes[2, 1].set_ylabel('从轴位置 (度)', fontsize=10)
    axes[2, 1].set_title(f'{title2} - 从轴实际位置', fontsize=12)
    axes[2, 1].grid(True, alpha=0.3)
    axes[2, 1].legend()
    
    # 凸轮主轴位置对比
    axes[3, 0].plot(result1['time_data'], result1['cam_master_pos_data'], 'c-', linewidth=1.5, label='凸轮主轴位置')
    axes[3, 0].set_ylabel('凸轮主轴位置 (度)', fontsize=10)
    axes[3, 0].set_title(f'{title1} - 凸轮主轴位置', fontsize=12)
    axes[3, 0].grid(True, alpha=0.3)
    axes[3, 0].legend()
    
    axes[3, 1].plot(result2['time_data'], result2['cam_master_pos_data'], 'c-', linewidth=1.5, label='凸轮主轴位置')
    axes[3, 1].set_ylabel('凸轮主轴位置 (度)', fontsize=10)
    axes[3, 1].set_title(f'{title2} - 凸轮主轴位置', fontsize=12)
    axes[3, 1].grid(True, alpha=0.3)
    axes[3, 1].legend()
    
    # 凸轮从轴位置对比
    axes[4, 0].plot(result1['time_data'], result1['cam_slave_pos_data'], 'm-', linewidth=1.5, label='凸轮从轴位置')
    axes[4, 0].set_ylabel('凸轮从轴位置 (度)', fontsize=10)
    axes[4, 0].set_title(f'{title1} - 凸轮从轴位置', fontsize=12)
    axes[4, 0].grid(True, alpha=0.3)
    axes[4, 0].legend()
    
    axes[4, 1].plot(result2['time_data'], result2['cam_slave_pos_data'], 'm-', linewidth=1.5, label='凸轮从轴位置')
    axes[4, 1].set_ylabel('凸轮从轴位置 (度)', fontsize=10)
    axes[4, 1].set_title(f'{title2} - 凸轮从轴位置', fontsize=12)
    axes[4, 1].grid(True, alpha=0.3)
    axes[4, 1].legend()
    
    # EndOfProfile对比
    axes[5, 0].plot(result1['time_data'], result1['endofprofile_data'], 'r-', linewidth=2, label='EndOfProfile')
    axes[5, 0].set_ylabel('EndOfProfile', fontsize=10)
    axes[5, 0].set_ylim(-0.1, 1.1)
    axes[5, 0].set_xlabel('时间 (s)', fontsize=10)
    axes[5, 0].set_title(f'{title1} - EndOfProfile', fontsize=12)
    axes[5, 0].grid(True, alpha=0.3)
    axes[5, 0].legend()
    
    axes[5, 1].plot(result2['time_data'], result2['endofprofile_data'], 'r-', linewidth=2, label='EndOfProfile')
    axes[5, 1].set_ylabel('EndOfProfile', fontsize=10)
    axes[5, 1].set_ylim(-0.1, 1.1)
    axes[5, 1].set_xlabel('时间 (s)', fontsize=10)
    axes[5, 1].set_title(f'{title2} - EndOfProfile', fontsize=12)
    axes[5, 1].grid(True, alpha=0.3)
    axes[5, 1].legend()
    
    plt.tight_layout()
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    print(f"对比图已保存: {filename}")
    plt.show()

def run_comparison_demos():
    """运行比较演示"""
    print("=== 开始凸轮基础比较演示 ===")
    
    # 比较1: 非周期 vs 周期模式
    print("\n1. 比较非周期模式 vs 周期模式")
    print("运行非周期模式...")
    result1 = create_cam_base_demo(periodic=False, start_pos=100, trigger_pos=120, master_offset=0, slave_offset=0)
    
    print("运行周期模式...")
    result2 = create_cam_base_demo(periodic=True, start_pos=100, trigger_pos=120, master_offset=0, slave_offset=0)
    
    # 创建非周期vs周期对比图
    create_comparison_plot(result1, result2, "非周期模式", "周期模式", "cam_base_comparison_periodic.png")
    
    print("\n=== 比较演示完成 ===")
    print("所有图片已保存，可以对比分析不同参数的影响")

if __name__ == "__main__":
    try:
        import sys
        
        if len(sys.argv) > 1:
            if sys.argv[1] == "compare":
                # 运行比较演示
                print("运行凸轮基础比较演示...")
                run_comparison_demos()
            elif sys.argv[1] == "nonperiodic":
                # 运行非周期模式
                print("运行非周期模式演示...")
                create_cam_base_demo(periodic=False, start_pos=100, trigger_pos=120, master_offset=0, slave_offset=0)
            elif sys.argv[1] == "periodic":
                # 运行周期模式
                print("运行周期模式演示...")
                create_cam_base_demo(periodic=True, start_pos=100, trigger_pos=120, master_offset=0, slave_offset=0)
            else:
                print("未知参数，使用默认演示")
                create_cam_base_demo()
        else:
            # 运行默认演示
            print("运行默认凸轮基础演示...")
            print("提示:")
            print("  使用 'python cam_plot_base.py nonperiodic' 运行非周期模式")
            print("  使用 'python cam_plot_base.py periodic' 运行周期模式")
            print("  使用 'python cam_plot_base.py compare' 运行比较演示")
            create_cam_base_demo()
        
    except Exception as e:
        print(f"运行错误: {e}")
        import traceback
        traceback.print_exc()
