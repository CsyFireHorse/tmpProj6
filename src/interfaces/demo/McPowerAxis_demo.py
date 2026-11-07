import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), '../build/lib.win-amd64-cpython-310'))
sys.path.append(os.path.join(os.path.dirname(__file__), '../build/lib.win-amd64-cpython-312'))
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(os.path.join(os.path.dirname(__file__), '../../build/lib.win-amd64-cpython-312'))
from typing import List
import zfbWrap


def power_demo():
    pow = zfbWrap.McPower()    
    print("pow", pow.Status)
    zfbWrap.MC_Power_create(pow)
    # ax = zfbWrap.AxisCia402InpWrap()
    axw = zfbWrap.axis_virtual_new()
    pow.bDriveStart = True
    pow.bRegulatorOn = True
    pow.Enable = True
    ax = zfbWrap.axis_virtual_get_ref(axw)
    pow.Axis = ax
    print("idx,powerOn,status")
    zfbWrap.axis_virtual_runcycle(axw)
    for i in range(5):
        zfbWrap.MC_Power_exec(pow)
        print(i, int(pow.bDriveStartRealState), int(pow.Status))
        zfbWrap.axis_virtual_runcycle(axw)
    zfbWrap.MC_Power_destroy(pow)
    

def power_move_demo():
    # 创建 Power 功能块和 Move 功能块
    pow = zfbWrap.McPower()
    move = zfbWrap.McMoveAbsolute()
    zfbWrap.MC_Power_create(pow)
    zfbWrap.MC_MoveAbsolute_create(move)
    cfg = zfbWrap.AxisCiaConfig()
    zfbWrap.AxisCiaConfig_default_init(cfg)
    cfg.cycle_time = 10000
    
    # zfbWrap.axis_cia402_get_config
    axw = zfbWrap.axis_virtual_new()
    zfbWrap.axis_virtual_set_config(axw, cfg)
    ax = zfbWrap.axis_virtual_get_ref(axw)

    # 配置 Power
    pow.bDriveStart = True
    pow.bRegulatorOn = True
    pow.Enable = True
    pow.Axis = ax

    # 配置 Move
    move.Axis = ax
    move.Position = 80.0
    move.Velocity = 200.0
    move.Acceleration = 1000.0
    move.Deceleration = 1000.0
    
    # move.BufferMode = 0  # mcAborting or mcBuffered, 具体值按实际定义

    print("idx,axId,time,axSt,pos,vel,acc")
    zfbWrap.axis_virtual_runcycle(axw)
    for i in range(80):
        if i == 10:
            move.Execute = True 
        zfbWrap.MC_Power_exec(pow)
        zfbWrap.MC_MoveAbsolute_exec(move)
        # print(i, int(pow.bDriveStartRealState), int(pow.Status), int(move.Execute), int(move.Busy))
        
        zfbWrap.axis_virtual_runcycle(axw)
        zfbWrap.axis_virtual_print(axw, 1)
        # time.sleep(0.02)  # 20ms周期
    zfbWrap.MC_Power_destroy(pow)
    zfbWrap.MC_MoveAbsolute_destroy(move)

if __name__ == "__main__":
    # power_demo()
    power_move_demo()
    
    
