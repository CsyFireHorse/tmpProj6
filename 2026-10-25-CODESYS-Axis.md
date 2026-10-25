# CODESYS Axis 调研

**日期：** 2026-10-25

## 概述

CODESYS 中的 "Axis"（轴）通常指 **CODESYS SoftMotion**，即 CODESYS PLC 的运动控制附加组件。以下信息来自产品数据手册与经销商页面，未经 codesys.com 官方核实。

## 关键要点

- **按轴数授权：** SoftMotion Axes 授权分多个档位。一份数据手册列出最多 64 个实轴加 64 个虚拟轴，并要求至少具备 CODESYS Control Standard S。另有 32 轴、48 轴版本。
- **CNC / 机器人：** SoftMotion Axis Groups / CNC Interpolators 授权面向 CNC 与机器人，基础版含 4 个轴组或插补器。
- **功能块：** 库采用 PLCopen 风格功能块，如定位用的 `MC_MoveAbsolute`、`MC_MoveRelative`，并支持主从同步运动（如凸轮曲线）。
- **硬件抽象：** 轴通过设备树中的名称引用，更换驱动器时无需修改应用程序代码。
- **驱动集成：** SEW-EURODRIVE 的 MOVIKIT PLCopen MotionControl 在 CODESYS 中提供轴诊断界面，显示轴状态与位置。其轴驱动模板提供使能、故障确认以及 `ActualVelocity`、`ActualPosition` 等实际值。

## 未覆盖内容

- 当前价格
- 最新 SoftMotion 版本
- 开发环境中的详细配置步骤

## 参考来源

- [CODESYS SoftMotion Axes (32)，德文数据手册](https://store.codesys.com/media/n98_media_assets/files/2305000012-F/3/CODESYS%20SoftMotion%20Axes%20(32)_de.pdf)
- [CODESYS SoftMotion Axes (48)，德文数据手册](https://store.codesys.com/media/n98_media_assets/files/2305000013-F/3/CODESYS%20SoftMotion%20Axes%20(48)_de.pdf)
- [CODESYS SoftMotion Axes (64)，GlobalSpec 数据手册](https://datasheets.globalspec.com/ds/codesys/2305000014/b2a88d1a-79ee-4f28-b815-235f5f77d77d)
- [CODESYS SoftMotion Axis Groups / CNC Interpolators (4)，GlobalSpec 数据手册](https://datasheets.globalspec.com/ds/CODESYS/2305000018/e68c05f5-feff-40a7-86f2-72798dd10ab2)
- [SEW-EURODRIVE：集成轴驱动模板](https://download.sew-eurodrive.com/download/html/27794997/en-EN/39123419147.html)
- [SEW-EURODRIVE：MOVIKIT 诊断](https://download.sew-eurodrive.com/download/html/27794997/en-EN/39123222027.html)
- [CODESYS 示例：动态路径（SM CNC 10）](https://content.helpme-codesys.com/fr/CODESYS%20Examples/_ex_sm_cnc10_dynamicpath-2029462.html)
