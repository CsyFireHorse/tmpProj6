#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Cam Plot StartMode - 凸轮StartMode比较绘图脚本
基于不同的MasterAbsolute和SlaveAbsolute组合进行凸轮运动分析

绘图要素：
- camIn.execute: 凸轮执行状态
- master.pos: 主轴实际位置
- cam.master.pos: 凸轮主轴位置（经过缩放和偏移处理）
- cam.slave.pos: 凸轮从轴位置（通过FbCamCalc计算）
- slave.pos: 从轴实际位置

StartMode = 0 (绝对模式) 的四种组合：
1. rel&abs: MasterAbsolute=False, SlaveAbsolute=True
2. rel&rel: MasterAbsolute=False, SlaveAbsolute=False  
3. abs&rel: MasterAbsolute=True, SlaveAbsolute=False
4. abs&abs: MasterAbsolute=True, SlaveAbsolute=True

使用方法:

1. 单个演示: python cam_plot_startmode.py --start-mode 0 --master-absolute --slave-relative --periodic
2. StartMode=0比较: python cam_plot_startmode.py --title startMode=0
3. StartMode=1比较: python cam_plot_startmode.py --title startMode=1
4. StartMode=2比较: python cam_plot_startmode.py --title startMode=2
5. 周期vs非周期: python cam_plot_startmode.py --title periodic&nonperiodic
6. 所有StartMode: python cam_plot_startmode.py --title startModeAll
7. 自定义参数: python cam_plot_startmode.py --start-mode 2 --master-offset 10 --slave-offset 5 --master-scale 2 --slave-scale 0.5

StartMode比较组:
- StartMode=0: [[rel&abs, rel&rel], [abs&rel, abs&abs]]
- StartMode=1: [[master=rel], [master=abs]]
- StartMode=2: [[rel&rel, rel&abs], [abs&rel, abs&abs]]
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
        FbAxisWrap, AxisCiaConfigWrapper, FbPower, FbMoveVelocity,
        FbReadCAM, FbCamTableSelect, FbCamIn, FbCamCalc,
        FbReadSetValues, FbSetMovementType, FbReadStatus,
        FbSetPosition
    )
    print("所有模块导入成功")
except ImportError as e:
    print(f"Import error: {e}")
    print("Please ensure pymc module is available")
    sys.exit(1)

def create_cam_startmode_demo(start_mode=0, master_absolute=False, slave_absolute=True, slave_init_pos=20, trigger_pos=200, periodic=True, master_offset=0, slave_offset=0, master_scale=1, slave_scale=1):
    """创建凸轮StartMode演示
    
    Args:
        start_mode: 启动模式 (0=absolute, 1=relative, 2=ramp_in, 3=ramp_in_pos, 4=ramp_in_neg)
        master_absolute: 主轴绝对模式
        slave_absolute: 从轴绝对模式
        slave_init_pos: 从轴初始位置
        trigger_pos: 凸轮触发位置
        periodic: 周期模式
        master_offset: 主轴偏移
        slave_offset: 从轴偏移
        master_scale: 主轴缩放
        slave_scale: 从轴缩放
    """
    fname = b'abc3.cam'  # 凸轮文件
    # 仿真参数
    total_cycles = 1800  # 总周期数，覆盖1.5个凸轮周期
    dt = 0.01  # 时间步长 (10ms)

    # 创建主从轴
    axis_master = FbAxisWrap()
    axis_slave = FbAxisWrap()
    
    # 配置主轴 - 模轴360
    cfgM = AxisCiaConfigWrapper()
    cfgM.set_virtual_mode(True)
    cfgM.set_cycle_time(int(dt*1000000))
    cfgM.set_id(0)
    axis_master.set_config(cfgM.get_config_ptr())
    
    # 配置从轴 - 模轴360
    cfgS = AxisCiaConfigWrapper()
    cfgS.set_virtual_mode(True)
    cfgS.set_cycle_time(int(dt*1000000))# us
    cfgS.set_id(1)
    axis_slave.set_config(cfgS.get_config_ptr())
    
    # 功能块
    powerM = FbPower()
    powerS = FbPower()
    moveVel = FbMoveVelocity()
    readCam = FbReadCAM()
    camTbl = FbCamTableSelect()
    camIn = FbCamIn()
    camCalc = FbCamCalc()
    
    # 读取设定值功能块
    readSetM = FbReadSetValues()
    readSetS = FbReadSetValues()
    
    # 读取状态功能块
    readStatusM = FbReadStatus()
    readStatusS = FbReadStatus()
    
    # 设置运动类型功能块
    setMoveTypeM = FbSetMovementType()
    setMoveTypeS = FbSetMovementType()
    
    # 设置位置功能块
    setPosS = FbSetPosition()
    
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
    setPosS.Axis = axis_slave.ref
    
    # 参数设置
    moveVel.set_move_params(velocity=50, acceleration=500, deceleration=500, jerk=0)
    
    # 凸轮表选择参数
    camTbl.Periodic = periodic
    camTbl.MasterAbsolute = master_absolute
    camTbl.SlaveAbsolute = slave_absolute
    
    # 凸轮输入参数
    camIn.MasterOffset = master_offset
    camIn.SlaveOffset = slave_offset
    camIn.MasterScaling = master_scale
    camIn.SlaveScaling = slave_scale
    camIn.VelocityDiff = 50
    camIn.Acceleration = 500
    camIn.Deceleration = 500
    camIn.Jerk = 0
    camIn.TappetHysteresis = 0.0
    camIn.StartMode = start_mode
    
    # 凸轮计算参数 - 初始设置，后续会在凸轮启动时更新
    camCalc.Enable = True
    camCalc.set_affine_params(
        master_offset=master_offset,
        slave_offset=slave_offset,
        master_scaling=master_scale,
        slave_scaling=slave_scale,
        master_engage_pos=0.0,  # 将在凸轮启动时更新
        slave_engage_pos=0.0,   # 将在凸轮启动时更新
        master_absolute=master_absolute,
        slave_absolute=slave_absolute
    )
    
    # 设置运动类型为模轴
    setMoveTypeM.iMovementType = 0  # 主轴模轴
    setMoveTypeS.iMovementType = 1  # 从轴模轴
    setMoveTypeM.fPositionPeriod = 240 
    setMoveTypeS.fPositionPeriod = 360
    
    # 数据记录
    time_data = []
    master_pos_data = []
    slave_pos_data = []
    endofprofile_data = []
    execute_data = []
    cam_master_pos_data = []
    cam_slave_pos_data = []
    

    
    # 状态变量
    master_begin_pos = 0
    slave_begin_pos = 0
    cam_triggered = False
    cam_start_time = 0
    
    # 生成模式描述
    start_mode_names = ['absolute', 'relative', 'ramp_in']
    mode_desc = f"SM{start_mode}({start_mode_names[start_mode]})_{'abs' if master_absolute else 'rel'}&{'abs' if slave_absolute else 'rel'}"
    print(f"开始凸轮StartMode演示 - {mode_desc}模式")
    print(f"从轴初始位置: {slave_init_pos}, 触发位置: {trigger_pos}")
    
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
            camTbl.Execute = readCam.bDone
            camTbl.CamTable = readCam.CAM

        if idx == 15:
            # 启动主轴运动
            moveVel.Execute = powerM.Status
            
        if idx == 0:
            # 设置从轴初始位置
            setPosS.Position = slave_init_pos
            setPosS.Execute = True
            
        # 检查是否到达触发位置
        if not cam_triggered and readSetM.Position >= trigger_pos:
            # 启动凸轮
            master_begin_pos = readSetM.Position  # 记录凸轮触发时的主轴位置
            slave_begin_pos = readSetS.Position   # 记录凸轮触发时的从轴位置
            camIn.Execute = powerS.Status 
            camIn.CamTableID = camTbl.CamTableID
            camCalc.CamTable = readCam.CAM
            cam_triggered = True
            cam_start_time = current_time
            
            # 更新凸轮计算的啮合位置
            camCalc.MasterEngagePos = master_begin_pos
            camCalc.SlaveEngagePos = slave_begin_pos
            
            print(f"凸轮在位置 {readSetM.Position:.2f} 启动，时间 {current_time:.2f}s")
            print(f"启动时从轴位置: {slave_begin_pos:.2f}")

        # 运行功能块
        setPosS.runCycle()
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
        setPosS.runCycle()
        
        # 更新凸轮计算的主轴PVA

        camCalc.set_master_pva(
            position=readSetM.Position,
            velocity=readSetM.Velocity,
            acceleration=readSetM.Acceleration
        )
        camCalc.runCycle()
        
        # 轴运行
        axis_master.run_cycle()
        axis_slave.run_cycle()
        
        # 计算凸轮主轴位置
        if cam_triggered:
            master_pos = readSetM.Position
            if master_absolute:
                # 绝对模式: 从当前主轴位置开始
                cam_master_pos = (camIn.MasterScaling * master_pos + camIn.MasterOffset) % 360
            else:
                # 相对模式: 从凸轮表起始位置开始
                cam_master_pos = (camIn.MasterScaling * (master_pos - master_begin_pos) + camIn.MasterOffset) % 360
            
            # 计算凸轮从轴位置 - 使用FbCamCalc
            if camCalc.Busy:
                slave_pva = camCalc.get_slave_pva()
                cam_slave_pos = slave_pva['position']
            else:
                # 备用计算方式
                cam_slave_pos = (readSetS.Position - camIn.SlaveOffset) / camIn.SlaveScaling
        else:
            cam_master_pos = camIn.MasterOffset
            cam_slave_pos = camIn.SlaveOffset
        
        # 记录数据 - 记录所有时刻的数据
        time_data.append(current_time)
        master_pos_data.append(readSetM.Position)
        slave_pos_data.append(readSetS.Position)
        endofprofile_data.append(1.0 if camIn.EndOfProfile else 0.0)
        execute_data.append(1.0 if camIn.Execute else 0.0)
        cam_master_pos_data.append(cam_master_pos)
        cam_slave_pos_data.append(cam_slave_pos)
        
        # 进度显示
        if idx % 300 == 0:
            print(f"进度: {idx}/{total_cycles} ({idx/total_cycles*100:.1f}%)")
    
    print("仿真完成，开始绘图...")
    
    # 绘图
    plt.rcParams['font.sans-serif'] = ['SimHei', 'Arial Unicode MS', 'DejaVu Sans']
    plt.rcParams['axes.unicode_minus'] = False
    
    fig, axes = plt.subplots(5, 1, figsize=(12, 12))
    fig.suptitle(f'凸轮StartMode演示-{mode_desc}模式', fontsize=14, fontweight='bold')
    
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
    
    # 从轴实际位置
    axes[4].plot(time_data, slave_pos_data, 'g-', linewidth=1.5, label='从轴实际位置')
    axes[4].set_ylabel('从轴位置 (度)', fontsize=10)
    axes[4].set_xlabel('时间 (s)', fontsize=10)
    axes[4].grid(True, alpha=0.3)
    axes[4].legend()
    
    # 调整布局
    plt.tight_layout()
    
    # 保存图片
    filename_suffix = f"_SM{start_mode}_{'abs' if master_absolute else 'rel'}{'abs' if slave_absolute else 'rel'}_S{slave_init_pos}_T{trigger_pos}"
    output_file = f'cam_plot_startmode{filename_suffix}.png'
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
        'endofprofile_data': endofprofile_data,
        'execute_data': execute_data,
        'cam_master_pos_data': cam_master_pos_data,
        'cam_slave_pos_data': cam_slave_pos_data,
        'params': {
            'start_mode': start_mode,
            'master_absolute': master_absolute,
            'slave_absolute': slave_absolute,
            'slave_init_pos': slave_init_pos,
            'trigger_pos': trigger_pos
        }
    }

def create_comparison_plot(result1, result2, title1, title2, filename):
    """创建对比图"""
    plt.rcParams['font.sans-serif'] = ['SimHei', 'Arial Unicode MS', 'DejaVu Sans']
    plt.rcParams['axes.unicode_minus'] = False
    
    fig, axes = plt.subplots(5, 2, figsize=(16, 12))
    fig.suptitle(f'凸轮StartMode对比: {title1} vs {title2}', fontsize=16, fontweight='bold')
    
    # Execute对比
    axes[0, 0].plot(result1['time_data'], result1['execute_data'], 'orange', linewidth=2, label='Execute')
    axes[0, 0].set_ylabel('Execute', fontsize=10)
    axes[0, 0].set_ylim(-0.1, 1.1)
    axes[0, 0].grid(True, alpha=0.3)
    axes[0, 0].legend()
    
    axes[0, 1].plot(result2['time_data'], result2['execute_data'], 'orange', linewidth=2, label='Execute')
    axes[0, 1].set_ylabel('Execute', fontsize=10)
    axes[0, 1].set_ylim(-0.1, 1.1)
    axes[0, 1].grid(True, alpha=0.3)
    axes[0, 1].legend()
    
    # 主轴实际位置对比
    axes[1, 0].plot(result1['time_data'], result1['master_pos_data'], 'b-', linewidth=1.5, label='主轴实际位置')
    axes[1, 0].set_ylabel('主轴位置 (度)', fontsize=10)
    axes[1, 0].grid(True, alpha=0.3)
    axes[1, 0].legend()
    
    axes[1, 1].plot(result2['time_data'], result2['master_pos_data'], 'b-', linewidth=1.5, label='主轴实际位置')
    axes[1, 1].set_ylabel('主轴位置 (度)', fontsize=10)
    axes[1, 1].grid(True, alpha=0.3)
    axes[1, 1].legend()
    
    # 凸轮主轴位置对比
    axes[2, 0].plot(result1['time_data'], result1['cam_master_pos_data'], 'c-', linewidth=1.5, label='凸轮主轴位置')
    axes[2, 0].set_ylabel('凸轮主轴位置 (度)', fontsize=10)
    axes[2, 0].grid(True, alpha=0.3)
    axes[2, 0].legend()
    
    axes[2, 1].plot(result2['time_data'], result2['cam_master_pos_data'], 'c-', linewidth=1.5, label='凸轮主轴位置')
    axes[2, 1].set_ylabel('凸轮主轴位置 (度)', fontsize=10)
    axes[2, 1].grid(True, alpha=0.3)
    axes[2, 1].legend()
    
    # 凸轮从轴位置对比
    axes[3, 0].plot(result1['time_data'], result1['cam_slave_pos_data'], 'm-', linewidth=1.5, label='凸轮从轴位置')
    axes[3, 0].set_ylabel('凸轮从轴位置 (度)', fontsize=10)
    axes[3, 0].grid(True, alpha=0.3)
    axes[3, 0].legend()
    
    axes[3, 1].plot(result2['time_data'], result2['cam_slave_pos_data'], 'm-', linewidth=1.5, label='凸轮从轴位置')
    axes[3, 1].set_ylabel('凸轮从轴位置 (度)', fontsize=10)
    axes[3, 1].grid(True, alpha=0.3)
    axes[3, 1].legend()
    
    # 从轴实际位置对比
    axes[4, 0].plot(result1['time_data'], result1['slave_pos_data'], 'g-', linewidth=1.5, label='从轴实际位置')
    axes[4, 0].set_ylabel('从轴位置 (度)', fontsize=10)
    axes[4, 0].set_xlabel('时间 (s)', fontsize=10)
    axes[4, 0].grid(True, alpha=0.3)
    axes[4, 0].legend()
    
    axes[4, 1].plot(result2['time_data'], result2['slave_pos_data'], 'g-', linewidth=1.5, label='从轴实际位置')
    axes[4, 1].set_ylabel('从轴位置 (度)', fontsize=10)
    axes[4, 1].set_xlabel('时间 (s)', fontsize=10)
    axes[4, 1].grid(True, alpha=0.3)
    axes[4, 1].legend()
    
    plt.tight_layout()
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    print(f"对比图已保存: {filename}")
    plt.show()

def run_comparison_demo(param, paramA, paramB, titleA, titleB, filename):
    """运行比较演示的通用函数
    
    Args:
        param: 基础参数字典
        paramA: 模式A的参数字典
        paramB: 模式B的参数字典
        titleA: 模式A的标题
        titleB: 模式B的标题
        filename: 输出文件名
    """
    print(f"比较 {titleA} vs {titleB} 模式")
    
    # 合并参数
    paramsA = param.copy()
    paramsA.update(paramA)
    paramsB = param.copy()
    paramsB.update(paramB)
    
    # 运行演示
    resultA = create_cam_startmode_demo(**paramsA)
    resultB = create_cam_startmode_demo(**paramsB)
    
    # 创建对比图
    create_comparison_plot(resultA, resultB, titleA, titleB, filename)

def run_title_demos(title, base_param):
    """根据title运行相应的演示"""
    print(f"=== 开始{title}演示 ===")
    
    # 演示配置字典
    demo_configs = {
        'startMode=0': {
            'title': 'StartMode=0 (absolute) 比较组',
            'comparisons': [
                {
                    'name': '1. 比较rel&abs vs rel&rel模式',
                    'paramA': {'start_mode': 0, 'master_absolute': False, 'slave_absolute': True},
                    'paramB': {'start_mode': 0, 'master_absolute': False, 'slave_absolute': False, 'trigger_pos': 280},
                    'titleA': 'SM0_rel&abs', 'titleB': 'SM0_rel&rel',
                    'filename': 'cam_startmode_SM0_relabs_vs_relrel.png'
                },
                {
                    'name': '2. 比较abs&rel vs abs&abs模式',
                    'paramA': {'start_mode': 0, 'master_absolute': True, 'slave_absolute': False},
                    'paramB': {'start_mode': 0, 'master_absolute': True, 'slave_absolute': True},
                    'titleA': 'SM0_abs&rel', 'titleB': 'SM0_abs&abs',
                    'filename': 'cam_startmode_SM0_absrel_vs_absabs.png'
                }
            ]
        },
        'startMode=1': {
            'title': 'StartMode=1 (relative) 比较组',
            'comparisons': [
                {
                    'name': '1. 比较master=rel vs master=abs模式',
                    'paramA': {'start_mode': 1, 'master_absolute': False, 'slave_absolute': True},
                    'paramB': {'start_mode': 1, 'master_absolute': True, 'slave_absolute': True},
                    'titleA': 'SM1_master=rel', 'titleB': 'SM1_master=abs',
                    'filename': 'cam_startmode_SM1_master_rel_vs_abs.png'
                }
            ]
        },
        'startMode=2': {
            'title': 'StartMode=2 (ramp_in) 比较组',
            'comparisons': [
                {
                    'name': '1. 比较rel&rel vs rel&abs模式',
                    'paramA': {'start_mode': 2, 'master_absolute': False, 'slave_absolute': False},
                    'paramB': {'start_mode': 2, 'master_absolute': False, 'slave_absolute': True},
                    'titleA': 'SM2_rel&rel', 'titleB': 'SM2_rel&abs',
                    'filename': 'cam_startmode_SM2_relrel_vs_relabs.png'
                },
                {
                    'name': '2. 比较abs&rel vs abs&abs模式',
                    'paramA': {'start_mode': 2, 'master_absolute': True, 'slave_absolute': False},
                    'paramB': {'start_mode': 2, 'master_absolute': True, 'slave_absolute': True},
                    'titleA': 'SM2_abs&rel', 'titleB': 'SM2_abs&abs',
                    'filename': 'cam_startmode_SM2_absrel_vs_absabs.png'
                }
            ]
        },
        'periodic&nonperiodic': {
            'title': '周期 vs 非周期比较组',
            'comparisons': [
                {
                    'name': '1. 比较rel&abs模式 (周期 vs 非周期)',
                    'paramA': {'start_mode': 0, 'master_absolute': False, 'slave_absolute': True, 'periodic': True},
                    'paramB': {'start_mode': 0, 'master_absolute': False, 'slave_absolute': True, 'periodic': False},
                    'titleA': 'rel&abs_周期', 'titleB': 'rel&abs_非周期',
                    'filename': 'cam_startmode_periodic_vs_nonperiodic_relabs.png'
                },
                {
                    'name': '2. 比较rel&rel模式 (周期 vs 非周期)',
                    'paramA': {'start_mode': 0, 'master_absolute': False, 'slave_absolute': False, 'trigger_pos': 280, 'periodic': True},
                    'paramB': {'start_mode': 0, 'master_absolute': False, 'slave_absolute': False, 'trigger_pos': 280, 'periodic': False},
                    'titleA': 'rel&rel_周期', 'titleB': 'rel&rel_非周期',
                    'filename': 'cam_startmode_periodic_vs_nonperiodic_relrel.png'
                }
            ]
        }
    }
    
    if title == 'startModeAll':
        print("\n=== 所有StartMode比较组 ===")
        # 运行所有StartMode的比较
        for demo_title in ['startMode=0', 'startMode=1', 'startMode=2', 'periodic&nonperiodic']:
            run_title_demos(demo_title, base_param)
    elif title in demo_configs:
        config = demo_configs[title]
        print(f"\n=== {config['title']} ===")
        
        for comparison in config['comparisons']:
            print(comparison['name'])
            run_comparison_demo(
                base_param,
                comparison['paramA'],
                comparison['paramB'],
                comparison['titleA'],
                comparison['titleB'],
                comparison['filename']
            )
    
    print(f"\n=== {title}演示完成 ===")
    print("所有图片已保存，可以对比分析不同StartMode的影响")



def parse_arguments():
    """解析命令行参数"""
    parser = argparse.ArgumentParser(description='凸轮StartMode比较绘图脚本')
    
    # 主要模式选择
    parser.add_argument('--mode', choices=['single'], default='single',
                       help='运行模式: single=单个演示')
    
    # 标题/演示类型选择
    parser.add_argument('--title', choices=['none', 'startMode=0', 'startMode=1', 'startMode=2', 'periodic&nonperiodic', 'startModeAll'], 
                       default='none',
                       help='演示类型: none=单个演示, startMode=0/1/2=特定StartMode比较, periodic&nonperiodic=周期比较, startModeAll=所有StartMode比较')
    
    # StartMode参数
    parser.add_argument('--start-mode', type=int, choices=[0, 1, 2, 3, 4], default=0,
                       help='启动模式: 0=absolute, 1=relative, 2=ramp_in, 3=ramp_in_pos, 4=ramp_in_neg')

    # 主轴和从轴模式
    parser.add_argument('--master-absolute', action='store_true',
                       help='主轴绝对模式 (默认: 相对模式)')
    parser.add_argument('--slave-absolute', action='store_true', default=True,
                       help='从轴绝对模式 (默认: True)')
    parser.add_argument('--slave-relative', action='store_true',
                       help='从轴相对模式 (覆盖slave-absolute)')
    
    # 位置参数
    parser.add_argument('--slave-init-pos', type=float, default=20.0,
                       help='从轴初始位置 (默认: 20)')
    parser.add_argument('--trigger-pos', type=float, default=200.0,
                       help='凸轮触发位置 (默认: 200)')
    
    # 凸轮参数
    parser.add_argument('--periodic', action='store_true', default=True,
                       help='周期模式 (默认: True)')
    parser.add_argument('--non-periodic', action='store_true',
                       help='非周期模式 (覆盖periodic)')
    
    # 缩放和偏移参数
    parser.add_argument('--master-offset', type=float, default=0.0,
                       help='主轴偏移 (默认: 0)')
    parser.add_argument('--slave-offset', type=float, default=0.0,
                       help='从轴偏移 (默认: 0)')
    parser.add_argument('--master-scale', type=float, default=1.0,
                       help='主轴缩放 (默认: 1)')
    parser.add_argument('--slave-scale', type=float, default=1.0,
                       help='从轴缩放 (默认: 1)')
    
    
    return parser.parse_args()

if __name__ == "__main__":
    try:
        args = parse_arguments()
        
        # 处理从轴模式
        slave_absolute = args.slave_absolute
        if args.slave_relative:
            slave_absolute = False
        
        # 处理周期模式
        periodic = args.periodic
        if args.non_periodic:
            periodic = False
        
        # 基础参数
        base_param = {
            'slave_init_pos': args.slave_init_pos,
            'trigger_pos': args.trigger_pos,
            'periodic': periodic,
            'master_offset': args.master_offset,
            'slave_offset': args.slave_offset,
            'master_scale': args.master_scale,
            'slave_scale': args.slave_scale
        }
        
        # 运行模式
        if args.title in ['startMode=0', 'startMode=1', 'startMode=2', 'periodic&nonperiodic', 'startModeAll']:
            # 使用title指定的演示类型
            run_title_demos(args.title, base_param)
        else:
            # 单个演示
            print(f"运行单个演示 - StartMode={args.start_mode}, Master={'abs' if args.master_absolute else 'rel'}, Slave={'abs' if slave_absolute else 'rel'}, Periodic={periodic}")
            params = base_param.copy()
            params.update({
                'start_mode': args.start_mode,
                'master_absolute': args.master_absolute,
                'slave_absolute': slave_absolute
            })
            create_cam_startmode_demo(**params)
        
    except Exception as e:
        print(f"运行错误: {e}")
        import traceback
        traceback.print_exc()
