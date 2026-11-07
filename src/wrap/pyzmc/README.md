# PyZMC - Python Motion Control Wrapper

## 功能模块介绍

### 1. 轴控制模块
- **FbAxisWrap**: 虚拟轴创建和管理
- **FbPower**: 轴电源和使能控制
- **AxisCiaConfigWrapper**: 轴配置管理

### 2. 运动控制模块
- **FbMoveAbsolute**: 绝对位置运动
- **FbMoveRelative**: 相对位置运动
- **FbMoveVelocity**: 速度运动
- **FbMoveAdditive**: 叠加运动
- **FbStop**: 停止运动
- **FbHalt**: 暂停运动

### 3. 凸轮表模块
- **PyFbCamTableSelect**: 凸轮表选择
- **PyFbReadCAM**: 凸轮表文件读取
- **PyFbWriteCAM**: 凸轮表文件写入
- **PyFbCamIn**: 凸轮同步运动

### 4. 挺杆控制模块
- **PyFbGetTappet**: 挺杆状态监控
- **setTappetFromCamIn**: 从CamIn设置挺杆数据

### 5. 新增功能块模块

#### 5.1 基础控制功能块
- **FbReset**: 轴复位功能块
- **FbSetPosition**: 位置设定功能块
- **FbHome**: 回零功能块
- **FbHoming**: 回零运动功能块

#### 5.2 状态读取功能块
- **FbReadStatus**: 轴状态读取功能块
- **FbReadActualPosition**: 实际位置读取功能块
- **FbReadActualVelocity**: 实际速度读取功能块
- **FbReadAxisError**: 轴错误读取功能块
- **FbReadSetValues**: 设定值读取功能块
- **FbReadActualValues**: 实际值读取功能块

#### 5.3 参数操作功能块
- **FbReadParameter**: 参数读取功能块
- **FbWriteParameter**: 参数写入功能块

#### 5.4 点动和跟随功能块
- **FbJog**: 点动功能块
- **FbFollowPosition**: 位置跟随功能块
- **FbFollowVelocity**: 速度跟随功能块
- **FbTrackAxis**: 轴跟踪功能块

#### 5.5 同步运动功能块
- **FbGearIn**: 齿轮同步进入功能块
- **FbGearOut**: 齿轮同步退出功能块
- **FbPhasing**: 相位同步功能块

#### 5.6 特殊控制功能块
- **FbSetTorque**: 转矩控制功能块
- **FbInPosition**: 位置到达检测功能块
- **FbCheckLimits**: 限制检查功能块

## 安装与配置

### 环境要求
- Python 3.6+
- Windows 10/11
- Visual Studio 2019+ (用于编译C++库)

### 文件结构
```
src/wrap/pyzmc/
├── README.md          # 本文档
├── pymc.py           # 主要Python包装器
├── mcapi.dll         # 编译后的动态链接库
└── build/            # 构建目录
```

### 编译DLL
```bash
# 在项目根目录执行
cd build
cmake ..
cmake --build . --config Debug
```

### 复制DLL
```bash
# 将编译好的DLL复制到pyzmc目录
copy ..\..\..\build\src\mcapi\Debug\mcapi.dll .
```

## 使用方法

### 1. 基本轴控制

#### 创建虚拟轴
```python
from pymc import FbAxisWrap, AxisCiaConfigWrapper

# 创建轴实例
axis = FbAxisWrap()

# 配置轴参数
config = AxisCiaConfigWrapper()
config.set_virtual_mode(True)
config.set_cycle_time(10000)  # 10ms周期
config.set_axis_type(0)       # 有限轴
config.set_dynamic_limits(velocity=1000, acceleration=5000, deceleration=5000, jerk=0)

# 应用配置
axis.set_config(config.get_config_ptr())

# 运行轴周期
axis.run_cycle()
```

#### 轴使能控制
```python
from pymc import FbPower

# 创建Power功能块
power = FbPower()
power.Axis = axis.ref
power.Enable = True
power.bRegulatorOn = True
power.bDriveStart = True

# 执行使能
power.runCycle()
print(f"轴状态: {power.Status}, 忙状态: {power.Busy}")
```

### 2. 运动控制

#### 绝对位置运动
```python
from pymc import FbMoveAbsolute, Direction, BufferMode

# 创建绝对运动功能块
move_abs = FbMoveAbsolute()
move_abs.Axis = axis.ref

# 设置运动参数
move_abs.set_move_params(
    position=100.0,        # 目标位置
    velocity=50.0,         # 速度
    acceleration=100.0,    # 加速度
    deceleration=100.0,    # 减速度
    jerk=0,               # 加加速度
    direction=Direction.mcPositiveDirection,
    buffer_mode=BufferMode.mcAborting
)

# 执行运动
move_abs.Execute = True
for i in range(50):
    move_abs.runCycle()
    axis.run_cycle()
    if move_abs.Done:
        print("运动完成")
        break
```

#### 速度运动
```python
from pymc import FbMoveVelocity

# 创建速度运动功能块
move_vel = FbMoveVelocity()
move_vel.Axis = axis.ref

# 设置运动参数
move_vel.set_move_params(
    velocity=50.0,         # 目标速度
    acceleration=500.0,    # 加速度
    deceleration=500.0,    # 减速度
    jerk=0,               # 加加速度
    direction=Direction.mcPositiveDirection,
    buffer_mode=BufferMode.mcAborting
)

# 执行运动
move_vel.Execute = True
for i in range(50):
    move_vel.runCycle()
    axis.run_cycle()
    if move_vel.InVelocity:
        print("达到目标速度")
        break
```

### 3. 凸轮表操作

#### 读取凸轮表文件
```python
from pymc import PyFbReadCAM

# 创建读取功能块
read_cam = PyFbReadCAM()

# 设置文件名
read_cam.sFileName = b'abc.cam'

# 执行读取
read_cam.bExecute = True
read_cam.runCycle()

if read_cam.Done:
    print("凸轮表读取成功")
    cam_table = read_cam.CAM
```

#### 凸轮表选择
```python
from pymc import PyFbCamTableSelect

# 创建选择功能块
cam_select = PyFbCamTableSelect()
cam_select.Master = master_axis.ref
cam_select.Slave = slave_axis.ref
cam_select.CamTable = cam_table
cam_select.Periodic = False
cam_select.MasterAbsolute = False
cam_select.SlaveAbsolute = False

# 执行选择
cam_select.Execute = True
cam_select.runCycle()
```

#### 凸轮同步运动
```python
from pymc import PyFbCamIn

# 创建凸轮输入功能块
cam_in = PyFbCamIn()
cam_in.Master = master_axis.ref
cam_in.Slave = slave_axis.ref
cam_in.CamTableID = cam_select.CamTableID

# 设置参数
cam_in.MasterOffset = 0
cam_in.SlaveOffset = 0
cam_in.MasterScaling = 1.0
cam_in.SlaveScaling = 1.0
cam_in.Acceleration = 1000
cam_in.Deceleration = 1000

# 执行凸轮运动
cam_in.Execute = True
for i in range(100):
    cam_in.runCycle()
    master_axis.run_cycle()
    slave_axis.run_cycle()
    
    if cam_in.EndOfProfile:
        print("凸轮运动完成")
        break
```

### 4. 挺杆控制

#### 挺杆状态获取
```python
from pymc import PyFbGetTappet

# 创建挺杆功能块
tappet = PyFbGetTappet()
tappet.iID = 1
tappet.bInitValue = False

# 从凸轮输入设置挺杆数据
tappet.set_tappet_from_cam_in(cam_in)

# 运行周期获取状态
tappet.runCycle()
print(f"挺杆状态: {tappet.bTappet}")
```

### 5. 新增功能块使用示例

#### 5.1 基础控制功能块

```python
from pymc import FbReset, FbSetPosition, FbHome

# Reset功能块
reset = FbReset()
reset.Axis = axis.ref
reset.Execute = True
reset.runCycle()
print(f"Reset完成: {reset.Done}")

# SetPosition功能块
set_pos = FbSetPosition()
set_pos.Axis = axis.ref
set_pos.set_position_params(position=100.0, mode=False)
set_pos.Execute = True
set_pos.runCycle()
print(f"位置设定完成: {set_pos.Done}")

# Home功能块
home = FbHome()
home.Axis = axis.ref
home.set_home_params(position=0.0)
home.Execute = True
home.runCycle()
print(f"回零完成: {home.Done}")
```

#### 5.2 状态读取功能块

```python
from pymc import FbReadStatus, FbReadActualPosition, FbReadActualVelocity

# 读取轴状态
read_status = FbReadStatus()
read_status.Axis = axis.ref
read_status.runCycle()
status_summary = read_status.get_status_summary()
print("轴状态:", status_summary)

# 读取实际位置
read_pos = FbReadActualPosition()
read_pos.Axis = axis.ref
read_pos.runCycle()
print(f"实际位置: {read_pos.Position}")

# 读取实际速度
read_vel = FbReadActualVelocity()
read_vel.Axis = axis.ref
read_vel.runCycle()
print(f"实际速度: {read_vel.Velocity}")
```

#### 5.3 点动功能块

```python
from pymc import FbJog

# 创建点动对象
jog = FbJog()
jog.Axis = axis.ref
jog.set_jog_params(velocity=10.0, acceleration=100.0, deceleration=100.0, jerk=0)

# 正向点动
jog.jog_forward()
for i in range(10):
    power.runCycle()
    jog.runCycle()
    axis.run_cycle()
    print(f"位置: {read_pos.Position}")

# 停止点动
jog.stop_jog()
```

#### 5.4 齿轮同步功能块

```python
from pymc import FbGearIn, FbGearOut

# 进入齿轮同步
gear_in = FbGearIn()
gear_in.Axis = axis.ref
gear_in.Master = master_axis.ref
gear_in.set_gear_params(
    ratio_numerator=2, 
    ratio_denominator=1, 
    acceleration=100.0, 
    deceleration=100.0, 
    jerk=0
)
gear_in.Execute = True
gear_in.runCycle()

# 运行同步运动
for i in range(20):
    power.runCycle()
    gear_in.runCycle()
    axis.run_cycle()

# 退出齿轮同步
gear_out = FbGearOut()
gear_out.Axis = axis.ref
gear_out.Execute = True
gear_out.runCycle()
```

#### 5.5 转矩控制功能块

```python
from pymc import FbSetTorque

# 创建转矩控制对象
set_torque = FbSetTorque()
set_torque.Axis = axis.ref
set_torque.set_torque_params(enable=True, torque=10.0)
set_torque.Execute = True
set_torque.runCycle()

# 运行转矩控制
for i in range(15):
    power.runCycle()
    set_torque.runCycle()
    axis.run_cycle()

# 禁用转矩控制
set_torque.set_torque_params(enable=False, torque=0.0)
set_torque.runCycle()
```

#### 5.6 参数读写功能块

```python
from pymc import FbReadParameter, FbWriteParameter

# 读取参数
read_param = FbReadParameter()
read_param.Axis = axis.ref
read_param.set_parameter_number(1001)
read_param.Execute = True
read_param.runCycle()
print(f"参数值: {read_param.Value}")

# 写入参数
write_param = FbWriteParameter()
write_param.Axis = axis.ref
write_param.set_write_params(param_number=1001, value=123.45)
write_param.Execute = True
write_param.runCycle()
print(f"写入状态: {write_param.Done}")
```

#### 5.7 位置到达检测功能块

```python
from pymc import FbInPosition

# 创建位置到达检测对象
in_pos = FbInPosition()
in_pos.Axis = axis.ref
in_pos.set_inposition_params(pos_window=0.1, pos_time=0.01, timeout=1.0)
in_pos.runCycle()

# 检查位置到达状态
if in_pos.InPosition:
    print("轴已到达目标位置")
else:
    print("轴未到达目标位置")
```

#### 5.8 限制检查功能块

```python
from pymc import FbCheckLimits

# 创建限制检查对象
check_limits = FbCheckLimits()
check_limits.Axis = axis.ref
check_limits.set_check_params(check_vel=True, check_acc_dec=True)
check_limits.runCycle()

# 检查限制状态
print(f"速度限制: {check_limits.VelocityLimit}")
print(f"加减速限制: {check_limits.AccDecLimit}")
```

### 5.9 高级运动控制功能块

#### 连续绝对运动
```python
from pymc import FbMoveContinuousAbsolute, Direction

# 创建连续绝对运动对象
move_cont_abs = FbMoveContinuousAbsolute()
move_cont_abs.Axis = axis.ref
move_cont_abs.set_move_params(
    position=100.0,
    velocity=50.0,
    end_velocity=10.0,
    end_velocity_direction=Direction.mcPositiveDirection,
    acceleration=100.0,
    deceleration=100.0,
    jerk=0.0,
    direction=Direction.mcPositiveDirection,
    adapt_end_vel=True
)
move_cont_abs.Execute = True
move_cont_abs.runCycle()
```

#### 连续相对运动
```python
from pymc import FbMoveContinuousRelative

# 创建连续相对运动对象
move_cont_rel = FbMoveContinuousRelative()
move_cont_rel.Axis = axis.ref
move_cont_rel.set_move_params(
    distance=50.0,
    velocity=30.0,
    end_velocity=5.0,
    end_velocity_direction=Direction.mcNegativeDirection,
    acceleration=80.0,
    deceleration=80.0,
    jerk=0.0,
    adapt_end_vel=False
)
move_cont_rel.Execute = True
move_cont_rel.runCycle()
```

#### 叠加运动
```python
from pymc import FbMoveSuperImposed

# 创建叠加运动对象
move_super = FbMoveSuperImposed()
move_super.Axis = axis.ref
move_super.set_move_params(
    distance=20.0,
    velocity_diff=15.0,
    acceleration=60.0,
    deceleration=60.0,
    jerk=0.0
)
move_super.Execute = True
move_super.runCycle()
```

#### 位置轮廓运动
```python
from pymc import FbPositionProfile

# 创建位置轮廓对象
pos_profile = FbPositionProfile()
pos_profile.Axis = axis.ref
pos_profile.set_profile_params(
    array_size=100,
    position_scale=1.0,
    offset=0.0
)
pos_profile.Execute = True
pos_profile.runCycle()
```

### 5.10 高级齿轮同步功能块

#### 位置齿轮同步
```python
from pymc import FbGearInPos, BufferMode

# 创建位置齿轮同步对象
gear_in_pos = FbGearInPos()
gear_in_pos.Master = master_axis.ref
gear_in_pos.Slave = slave_axis.ref
gear_in_pos.set_gear_params(
    ratio_numerator=2,
    ratio_denominator=1,
    master_sync_pos=0.0,
    slave_sync_pos=0.0,
    master_start_distance=10.0,
    buffer_mode=BufferMode.mcAborting,
    avoid_reversal=True
)
gear_in_pos.Execute = True
gear_in_pos.runCycle()
```

### 5.11 高级跟随功能块

#### 位置速度跟随
```python
from pymc import FbFollowPositionVelocity

# 创建位置速度跟随对象
follow_pos_vel = FbFollowPositionVelocity()
follow_pos_vel.Axis = axis.ref
follow_pos_vel.set_follow_params(
    position=75.0,
    velocity=25.0
)
follow_pos_vel.bExecute = True
follow_pos_vel.runCycle()
```

#### 设定值跟随
```python
from pymc import FbFollowSetValues

# 创建设定值跟随对象
follow_set_values = FbFollowSetValues()
follow_set_values.Axis = axis.ref
follow_set_values.set_follow_params(
    position=100.0,
    velocity=50.0,
    acceleration=200.0,
    jerk=0.0,
    torque=10.0,
    current=5.0
)
follow_set_values.bExecute = True
follow_set_values.runCycle()
```

### 5.12 寸动功能块

```python
from pymc import FbInch

# 创建寸动对象
inch = FbInch()
inch.Axis = axis.ref
inch.set_inch_params(
    distance=5.0,
    velocity=10.0,
    acceleration=50.0,
    deceleration=50.0,
    jerk=0.0
)

# 正向寸动
inch.inch_forward()
for i in range(10):
    inch.runCycle()
    axis.run_cycle()

# 停止寸动
inch.stop_inch()
```

### 5.13 高级凸轮控制功能块

#### CAM边界计算
```python
from pymc import FbCAMBounds

# 创建CAM边界计算对象
cam_bounds = FbCAMBounds()
cam_bounds.set_bounds_params(
    master_vel_max=100.0,
    master_acc_max=500.0,
    master_scaling=1.0,
    slave_scaling=1.0
)
cam_bounds.bExecute = True
cam_bounds.runCycle()
```

#### 凸轮注册
```python
from pymc import FbCamRegister

# 创建凸轮注册对象
cam_register = FbCamRegister()
cam_register.Master = master_axis.ref
cam_register.set_register_params(
    master_offset=0.0,
    master_scaling=1.0,
    tappet_hysteresis=0.1,
    dead_time_compensation=0.0
)
cam_register.Enable = True
cam_register.runCycle()
```

### 5.14 监控和诊断功能块

#### 实际转矩读取
```python
from pymc import FbReadActualTorque

# 创建实际转矩读取对象
read_torque = FbReadActualTorque()
read_torque.Axis = axis.ref
read_torque.Enable = True
read_torque.runCycle()
print(f"实际转矩: {read_torque.Torque}")
```

#### 命令位置读取
```python
from pymc import FbReadCommandPosition

# 创建命令位置读取对象
read_cmd_pos = FbReadCommandPosition()
read_cmd_pos.Axis = axis.ref
read_cmd_pos.Enable = True
read_cmd_pos.runCycle()
print(f"命令位置: {read_cmd_pos.Position}")
```

#### 布尔参数读写
```python
from pymc import FbReadBoolParameter, FbWriteBoolParameter

# 读取布尔参数
read_bool_param = FbReadBoolParameter()
read_bool_param.Axis = axis.ref
read_bool_param.set_parameter_number(1001)
read_bool_param.Enable = True
read_bool_param.runCycle()
print(f"布尔参数值: {read_bool_param.Value}")

# 写入布尔参数
write_bool_param = FbWriteBoolParameter()
write_bool_param.Axis = axis.ref
write_bool_param.set_write_params(1001, True)
write_bool_param.Execute = True
write_bool_param.runCycle()
```

#### 驱动器值读取
```python
from pymc import FbReadSetDriveValues, FbReadActualDriveValues

# 读取驱动器设定值
read_set_drive = FbReadSetDriveValues()
read_set_drive.Axis = axis.ref
read_set_drive.Enable = True
read_set_drive.runCycle()
drive_set_values = read_set_drive.get_drive_values()
print("驱动器设定值:", drive_set_values)

# 读取驱动器实际值
read_actual_drive = FbReadActualDriveValues()
read_actual_drive.Axis = axis.ref
read_actual_drive.Enable = True
read_actual_drive.runCycle()
drive_actual_values = read_actual_drive.get_drive_values()
print("驱动器实际值:", drive_actual_values)
```

#### 跟踪误差获取
```python
from pymc import FbGetTrackingError

# 创建跟踪误差获取对象
get_tracking_error = FbGetTrackingError()
get_tracking_error.Axis = axis.ref
get_tracking_error.set_tracking_params(
    timestamp=0,
    dead_time_cycles=5
)
get_tracking_error.Enable = True
get_tracking_error.runCycle()
print(f"实际跟踪误差: {get_tracking_error.ActTrackingError}")
print(f"最大跟踪误差: {get_tracking_error.MaxTrackingError}")
```

#### 行程距离获取
```python
from pymc import FbGetTravelledDistance

# 创建行程距离获取对象
get_distance = FbGetTravelledDistance()
get_distance.Axis = axis.ref
get_distance.Enable = True
get_distance.runCycle()
print(f"行程距离: {get_distance.Distance}")
```

#### 最大设定值获取
```python
from pymc import FbGetMaxSetAccDec, FbGetMaxSetVelocity

# 获取最大设定加速度
get_max_acc = FbGetMaxSetAccDec()
get_max_acc.Axis = axis.ref
get_max_acc.set_timestamp(0)
get_max_acc.Enable = True
get_max_acc.runCycle()
print(f"最大设定加速度: {get_max_acc.MaxAcceleration}")

# 获取最大设定速度
get_max_vel = FbGetMaxSetVelocity()
get_max_vel.Axis = axis.ref
get_max_vel.set_timestamp(0)
get_max_vel.Enable = True
get_max_vel.runCycle()
print(f"最大设定速度: {get_max_vel.MaxVelocity}")
```

#### 覆盖因子控制
```python
from pymc import FbGetOverride, FbSetOverride

# 获取覆盖因子
get_override = FbGetOverride()
get_override.Axis = axis.ref
get_override.Enable = True
get_override.runCycle()
print(f"速度因子: {get_override.VelFactor}")
print(f"加速度因子: {get_override.AccFactor}")
print(f"加加速度因子: {get_override.JerkFactor}")

# 设置覆盖因子
set_override = FbSetOverride()
set_override.Axis = axis.ref
set_override.set_override_params(
    vel_factor=0.8,
    acc_factor=0.9,
    jerk_factor=1.0
)
set_override.Enable = True
set_override.runCycle()
```

### 5.15 控制模式设置功能块

#### 运动类型设置
```python
from pymc import FbSetMovementType

# 创建运动类型设置对象
set_movement_type = FbSetMovementType()
set_movement_type.Axis = axis.ref
set_movement_type.set_movement_params(
    movement_type=0,
    position_period=1000.0
)
set_movement_type.bExecute = True
set_movement_type.runCycle()
```

#### 斜坡类型设置
```python
from pymc import FbSetRampType

# 创建斜坡类型设置对象
set_ramp_type = FbSetRampType()
set_ramp_type.Axis = axis.ref
set_ramp_type.set_ramp_params(0)
set_ramp_type.bExecute = True
set_ramp_type.runCycle()
```

#### 动态限制修改
```python
from pymc import FbChangeDynamicLimits

# 创建动态限制修改对象
change_limits = FbChangeDynamicLimits()
change_limits.Axis = axis.ref
change_limits.set_dynamic_limits(
    max_velocity=200.0,
    max_acceleration=1000.0,
    max_deceleration=1000.0,
    max_jerk=0.0,
    max_torque=50.0
)
change_limits.bExecute = True
change_limits.runCycle()
```

#### 齿轮比修改
```python
from pymc import FbChangeGearingRatio

# 创建齿轮比修改对象
change_gearing = FbChangeGearingRatio()
change_gearing.Axis = axis.ref
change_gearing.set_gearing_params(
    ratio_denom=1000,
    ratio_num=1,
    position_period=1000.0,
    movement_type=0
)
change_gearing.bExecute = True
change_gearing.runCycle()
```

#### 软件限位设置
```python
from pymc import FbSetSoftwareLimits

# 创建软件限位设置对象
set_sw_limits = FbSetSoftwareLimits()
set_sw_limits.Axis = axis.ref
set_sw_limits.set_software_limits(
    is_activated=True,
    positive_limit=1000.0,
    negative_limit=-1000.0,
    error_decelerate=True,
    error_deceleration=500.0,
    error_max_distance=10.0
)
set_sw_limits.bExecute = True
set_sw_limits.runCycle()
```

#### 控制器模式设置
```python
from pymc import FbSetControllerMode

# 创建控制器模式设置对象
set_controller_mode = FbSetControllerMode()
set_controller_mode.Axis = axis.ref
set_controller_mode.set_controller_mode(0)
set_controller_mode.bExecute = True
set_controller_mode.runCycle()
```

## 完整示例

### 凸轮同步运动示例
```python
def cam_sync_demo():
    """完整的凸轮同步运动示例"""
    
    # 创建主从轴
    master = FbAxisWrap()
    slave = FbAxisWrap()
    
    # 配置轴
    cfg = AxisCiaConfigWrapper()
    cfg.set_virtual_mode(True)
    cfg.set_cycle_time(10000)
    master.set_config(cfg.get_config_ptr())
    slave.set_config(cfg.get_config_ptr())
    
    # 创建功能块
    power_m = FbPower()
    power_s = FbPower()
    move_vel = FbMoveVelocity()
    read_cam = PyFbReadCAM()
    cam_select = PyFbCamTableSelect()
    cam_in = PyFbCamIn()
    tappet = PyFbGetTappet()
    
    # 绑定轴
    power_m.Axis = master.ref
    power_s.Axis = slave.ref
    move_vel.Axis = master.ref
    cam_select.Master = master.ref
    cam_select.Slave = slave.ref
    cam_in.Master = master.ref
    cam_in.Slave = slave.ref
    
    # 设置参数
    move_vel.set_move_params(velocity=50, acceleration=500, deceleration=500, jerk=0)
    tappet.iID = 1
    tappet.bInitValue = False
    
    # 主循环
    for i in range(100):
        # 轴使能
        if i == 1:
            power_m.Enable = power_s.Enable = True
            power_m.bRegulatorOn = power_s.bRegulatorOn = True
            power_m.bDriveStart = power_s.bDriveStart = True
        
        # 读取凸轮表
        if i == 5:
            read_cam.bExecute = True
            read_cam.sFileName = b'abc.cam'
        
        # 选择凸轮表
        if i == 8:
            cam_select.Execute = True
            cam_select.CamTable = read_cam.CAM
        
        # 启动主轴运动
        if i == 10:
            move_vel.Execute = True
        
        # 启动凸轮同步
        if i == 15:
            cam_in.Execute = True
            cam_in.CamTableID = cam_select.CamTableID
        
        # 运行所有功能块
        power_m.runCycle()
        power_s.runCycle()
        read_cam.runCycle()
        cam_select.runCycle()
        move_vel.runCycle()
        cam_in.runCycle()
        tappet.set_tappet_from_cam_in(cam_in)
        tappet.runCycle()
        
        # 运行轴周期
        master.run_cycle()
        slave.run_cycle()
        
        # 打印状态
        print(f"周期 {i}: 主轴位置={master.ref.contents.fSavePosition:.3f}, "
              f"从轴位置={slave.ref.contents.fSavePosition:.3f}, "
              f"挺杆状态={tappet.bTappet}")

if __name__ == "__main__":
    cam_sync_demo()
```

## 运行方法

### 基本运行
```bash
python pymc.py
```

### 选择不同的演示函数
在 `pymc.py` 文件的 `if __name__ == "__main__":` 部分，可以注释/取消注释不同的演示函数：

```python
if __name__ == "__main__":
    # powerDemo2()           # 电源控制演示
    # moveAbsoluteDemo()     # 绝对位置运动演示
    # moveVelDemo()          # 速度运动演示
    # axisConfigDemo()       # 轴配置演示
    py_cam_in_demo()         # 凸轮同步运动演示
    # tappetDemo()           # 挺杆控制演示
    newFunctionBlocksDemo()  # 新增功能块演示
    parameterDemo()          # 参数读写演示
    torqueDemo()             # 转矩控制演示
```

### 演示函数说明

#### 基础演示函数
- **powerDemo2()**: 演示轴电源控制和使能
- **moveAbsoluteDemo()**: 演示绝对位置运动控制
- **moveVelDemo()**: 演示速度运动控制
- **axisConfigDemo()**: 演示轴配置管理
- **py_cam_in_demo()**: 演示凸轮同步运动
- **tappetDemo()**: 演示挺杆控制

#### 新增演示函数
- **newFunctionBlocksDemo()**: 综合演示所有新增功能块
  - Reset、SetPosition、Home等基础控制功能
  - ReadStatus、ReadActualPosition等状态读取功能
  - Jog点动功能
  - GearIn/GearOut齿轮同步功能
  - InPosition位置到达检测
  - CheckLimits限制检查

- **advancedMotionDemo()**: 高级运动控制演示
  - 位置、加速度、速度轮廓功能
  - 动态限制修改
  - 软件限位设置

- **camAdvancedDemo()**: 高级凸轮控制演示
  - CAM边界计算
  - 凸轮注册
  - 数字凸轮开关

- **monitoringDemo()**: 监控和诊断演示
  - 实际转矩、命令位置读取
  - 布尔参数读写
  - 驱动器值读取
  - 跟踪误差、行程距离获取
  - 最大设定值获取
  - 覆盖因子控制

- **controlModeDemo()**: 控制模式演示
  - 运动类型设置
  - 斜坡类型设置
  - 齿轮比修改
  - 控制器模式设置

- **parameterDemo()**: 演示参数读写功能
  - 读取轴参数
  - 写入轴参数
  - 参数验证

- **torqueDemo()**: 演示转矩控制功能
  - 启用转矩控制
  - 设置转矩值
  - 禁用转矩控制

## 注意事项

1. **DLL依赖**: 确保 `mcapi.dll` 在正确的路径下
2. **类型匹配**: Python结构体字段类型必须与C++完全匹配
3. **内存管理**: 使用 `byref()` 传递结构体指针
4. **错误处理**: 检查所有函数调用的返回值
5. **周期运行**: 确保轴和功能块按正确顺序运行周期

## 故障排除

### 常见问题

1. **DLL加载失败**: 检查文件路径和依赖库
2. **访问违规**: 检查结构体字段类型和内存布局
3. **轴运动异常**: 检查轴配置和使能状态
4. **凸轮同步失败**: 检查凸轮表文件和参数设置

### 调试技巧

1. 使用 `print()` 输出关键变量值
2. 检查C++端的日志输出
3. 验证结构体字段的内存布局
4. 逐步测试各个功能模块

## 技术支持

如有问题，请检查：
1. 编译环境配置
2. DLL版本兼容性
3. Python版本兼容性
4. 结构体定义一致性