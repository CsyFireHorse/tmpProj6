#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Cam Plot Scaling - 凸轮缩放和偏移绘图脚本
展示不同MasterScaling、SlaveScaling、MasterOffset、SlaveOffset对凸轮曲线的影响

原始凸轮主轴范围 [0,360], 从轴范围 [0,180]
"""

import sys
import os
import numpy as np
import matplotlib.pyplot as plt

# 添加项目路径
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

try:
    from pymc import FbReadCAM, FbCamCalc
    print("模块导入成功")
except ImportError as e:
    print(f"Import error: {e}")
    print("Please ensure pymc module is available")
    sys.exit(1)

def plot_cam_scaling(master_scaling=1.0, slave_scaling=1.0, master_offset=0.0, slave_offset=0.0, 
                    master_absolute=True, slave_absolute=True, title_suffix=""):
    """绘制凸轮缩放和偏移效果
    
    Args:
        master_scaling: 主轴缩放
        slave_scaling: 从轴缩放  
        master_offset: 主轴偏移
        slave_offset: 从轴偏移
        master_absolute: 主轴绝对模式
        slave_absolute: 从轴绝对模式
        title_suffix: 标题后缀
    """
    fname = b'abc3.cam'  # 凸轮文件
    
    # 创建功能块
    readCam = FbReadCAM()
    camCalc = FbCamCalc()
    
    # 读取凸轮文件
    readCam.bExecute = True
    readCam.sFileName = fname
    readCam.runCycle()
    
    # 等待读取完成
    max_attempts = 100
    attempts = 0
    while not readCam.bDone and not readCam.bError and attempts < max_attempts:
        readCam.runCycle()
        attempts += 1
    
    if readCam.bError:
        print(f"读取凸轮文件失败: ErrorID = {readCam.iErrorID}")
        return
    
    if attempts >= max_attempts:
        print("读取凸轮文件超时")
        return
    
    print(f"凸轮文件读取成功，尝试次数: {attempts}")
    
    # 设置凸轮表
    camCalc.set_cam_table(readCam.CAM)
    camCalc.Enable = True
    camCalc.set_affine_params(
        master_offset=master_offset,
        slave_offset=slave_offset,
        master_scaling=master_scaling,
        slave_scaling=slave_scaling,
        master_engage_pos=0.0,
        slave_engage_pos=0.0,
        master_absolute=master_absolute,
        slave_absolute=slave_absolute
    )
    
    # 根据主轴缩放确定主轴位置范围
    if master_scaling >= 1.0:
        # 主轴加速或正常速度，主轴范围 [0, 360]
        master_positions = np.linspace(0, 360, 361)
    else:
        # 主轴减速，需要更大的范围来覆盖完整凸轮周期
        # 减速0.5倍需要720度才能覆盖360度凸轮周期
        max_master = 360 / master_scaling
        master_positions = np.linspace(0, max_master, int(max_master) + 1)
    
    slave_positions = []
    
    # 计算每个主轴位置对应的从轴位置
    print(f"计算 {len(master_positions)} 个主轴位置点...")
    for i, master_pos in enumerate(master_positions):
        camCalc.set_master_pva(
            position=master_pos,
            velocity=0.0,
            acceleration=0.0
        )
        camCalc.runCycle()
        
        if not camCalc.is_error():
            slave_pva = camCalc.get_slave_pva()
            slave_positions.append(slave_pva['position'])
        else:
            print(f"计算错误 at master_pos={master_pos}, ErrorID={camCalc.get_error_id()}")
            slave_positions.append(0.0)
        
        # 进度显示
        if i % 100 == 0:
            print(f"进度: {i}/{len(master_positions)}")
    
    print(f"计算完成，从轴位置范围: {min(slave_positions):.2f} ~ {max(slave_positions):.2f}")
    
    # 绘图
    plt.rcParams['font.sans-serif'] = ['SimHei', 'Arial Unicode MS', 'DejaVu Sans']
    plt.rcParams['axes.unicode_minus'] = False
    
    plt.figure(figsize=(10, 6))
    plt.plot(master_positions, slave_positions, 'b-', linewidth=2, label='凸轮曲线')
    plt.xlabel('主轴位置 (度)', fontsize=12)
    plt.ylabel('从轴位置 (度)', fontsize=12)
    plt.title(f'凸轮缩放和偏移效果{title_suffix}\nMasterScaling={master_scaling}, SlaveScaling={slave_scaling}, MasterOffset={master_offset}, SlaveOffset={slave_offset}', 
              fontsize=12, fontweight='bold')
    plt.grid(True, alpha=0.3)
    plt.legend()
    
    # 设置坐标轴范围
    # X轴范围：案例5和6使用[0,720]，其他使用[0,360]
    if master_scaling == 0.5 or (master_offset != 0 or slave_offset != 0):
        x_min, x_max = 0, 720
    else:
        x_min, x_max = 0, 360
    
    plt.xlim(x_min, x_max)
    
    # Y轴范围：所有案例都使用[0,360]
    plt.ylim(0, 360)
    
    # 添加网格线
    x_ticks = np.arange(0, x_max + 1, 60)
    plt.xticks(x_ticks)
    y_ticks = np.arange(0, 361, 30)
    plt.yticks(y_ticks)
    
    plt.tight_layout()
    
    # 保存图片
    filename = f'cam_scaling_MS{master_scaling}_SS{slave_scaling}_MO{master_offset}_SO{slave_offset}.png'
    plt.savefig(filename, dpi=300, bbox_inches='tight')
    print(f"图片已保存: {filename}")
    
    plt.show()
    
    return master_positions, slave_positions

def main():
    """主函数 - 生成所有缩放和偏移组合的图片"""
    
    # 定义测试案例
    test_cases = [
        # 基础案例
        {
            'master_scaling': 1.0, 'slave_scaling': 1.0, 
            'master_offset': 0, 'slave_offset': 0,
            'title': ' - 原始凸轮曲线'
        },
        # 从轴缩放测试
        {
            'master_scaling': 1.0, 'slave_scaling': 2.0, 
            'master_offset': 0, 'slave_offset': 0,
            'title': ' - 从轴拉伸2倍'
        },
        {
            'master_scaling': 1.0, 'slave_scaling': 0.5, 
            'master_offset': 0, 'slave_offset': 0,
            'title': ' - 从轴压缩0.5倍'
        },
        # 主轴缩放测试
        {
            'master_scaling': 2.0, 'slave_scaling': 1.0, 
            'master_offset': 0, 'slave_offset': 0,
            'title': ' - 主轴加速2倍'
        },
        {
            'master_scaling': 0.5, 'slave_scaling': 1.0, 
            'master_offset': 0, 'slave_offset': 0,
            'title': ' - 主轴减速0.5倍'
        },
        # 偏移测试
        {
            'master_scaling': 1.0, 'slave_scaling': 1.0, 
            'master_offset': 20, 'slave_offset': 30,
            'title': ' - 主轴偏移20度，从轴偏移30度'
        }
    ]
    
    print("开始生成凸轮缩放和偏移效果图...")
    
    for i, case in enumerate(test_cases, 1):
        print(f"\n=== 案例 {i}: {case['title']} ===")
        plot_cam_scaling(
            master_scaling=case['master_scaling'],
            slave_scaling=case['slave_scaling'],
            master_offset=case['master_offset'],
            slave_offset=case['slave_offset'],
            title_suffix=case['title']
        )
    
    print("\n=== 所有图片生成完成 ===")
    print("图片已保存到当前目录，可用于完善 fig.md 文档")

if __name__ == "__main__":
    main()
