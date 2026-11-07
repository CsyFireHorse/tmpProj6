# zmc_rpc：RPC 公共接口的 Python 绑定

日期：2026-11-06
代笔：GPT-6.1 Codex

## 用户要求和实施边界

- 严格参考 `../rpc_client/include/zml_motion_api.h` 的 72 项有效声明和
  `zml_motion_api_ex.h` 的 22 项有效声明，行为以对应客户端实现为准。
- 直接替换旧 `RpcClient`，提供模块级 `zml_*` 同名函数和显式句柄；不保留旧名称。
- 输入名称、顺序、位宽和语义跟随 RPC；输出指针转换为元组，结构体使用原字段名字典。
- 修改仅限本目录的绑定、构建、两个示例和测试；新增本文件及 `zmc_rpc.pyi`。
  不修改底层 RPC、服务端、qcmd 或共享配置，不引入线程、同步机制或新的完成等待层。

## 接口契约

完整 Python 声明见 `zmc_rpc.pyi`，不在此重复接口清单。

- 无输出参数的函数直接返回原始整数。`zml_get_connection_state` 返回连接状态整数。
- `zml_open*` 返回 `(ret, handle)`，失败返回 `(ret, None)`；打开与等待连接独立。
- 普通输出按 C 声明顺序返回 `(ret, output1, ...)`；失败时普通输出为 `None`。
- `zml_get_cmd_results(handle, inout_len)` 返回 `(ret, results, inout_len)`；容量显式提供，
  返回 C 调用后的长度。一次 Python 调用只调用一次 C API，不自动扩容重试。
- SDO download 保留 `data_len` 和连续字节缓冲区；upload 返回 `(ret, data_len, data)`，
  数据为 `bytes`。输入输出长度始终保留 C 调用后的值。
- 多轴输入保留 `ax_num` 和 `axes`，不推导轴号。结构字段保留 `batch_size`、
  `verbose_flag`、`cycle_time`、`reserved`、`data_ext` 等原名；固定数组使用列表。
- 配置字典必须提供全部声明字段。Python 类型、整数表示范围、数组/缓冲区长度转换失败
  使用 Python 参数异常；业务结果使用原 C API 错误码。
- `ZML_HANDLE` 不允许直接构造。显式 `zml_close(handle)` 返回原始结果并清空原生句柄；
  重复关闭及关闭后调用遵循 C API 行为。析构仅兜底释放；调用方应使用 `try/finally`。
- `zml_ax_wait_ticks` 和 `zml_ax_wait_seconds` 保持 RPC 的脚本排队语义，
  返回成功不表示 Python 已等到运动完成。`zml_ax_wait_idle` 阻塞直至轴到达 Standstill：
  状态 1 返回 0，ErrorStop（低 4 位为 7）返回错误码，其余状态超时返回超时错误码。
  原接口文件和地址解析语义均保持原样。（本条更新由 GLM-5.3（opencode），2026-10-23 代笔。）
- 业务前置条件也继承现有 RPC：协调运动先调用轴组使能；文件 download 替换服务端已有条目，
  不创建新条目。测试在自己的临时服务目录内预置传输目标文件。

## 使用方式

将构建输出目录加入 `PYTHONPATH` 后导入 `zmc_rpc`。例如 Windows Debug 输出为
`build/src/app/demo/pyclient/Debug`。

```python
import zmc_rpc as rpc

ret, handle = rpc.zml_open("127.0.0.1:18082")
if ret == 0:
    try:
        ret, state = rpc.zml_ax_get_state(handle, 0)
        print(ret, state)
    finally:
        close_ret = rpc.zml_close(handle)
        print(close_ret)
else:
    print(ret)
```

示例入口为 `examples/basic_poweron.py --device 127.0.0.1:18082 --axis 0` 和
`examples/basic_motion.py --device 127.0.0.1:18082 --axis 0 --pos 10`。
两者都检查返回码，并在观察截止时间内检查实际状态。结构类型名仅在 `.pyi` 中描述字典，
运行时配置直接传入完整字典；句柄类型由扩展实际导出。

## 实施与迁移

1. 使用现有手写 CPython 扩展，复用公共类型和同签名转换模板，完整绑定 94 个函数。
2. 删除旧类和名称，迁移示例、测试、类型声明；示例改为必填 `--device`、显式 `--axis`，
   检查返回码并在 `finally` 中关闭句柄。
3. live 测试通过 `zmc_motapp_demo axis_ini --rpc --port=<port> --ini <ini>` 启动服务。
   测试配置、日志和传输文件置于构建目录的独立工作目录，复用虚拟轴资源及虚拟 IO。
   helper 持有自己启动的进程，只清理该进程和测试临时文件。
4. CMake 仅调整测试信息、依赖及类型声明安装，保持已有构建方式与选项。
   live 用例使用 `RUN_SERIAL` 单独调度，避免其虚拟运动周期观察与其他集成测试并行；
   CTest 的其他测试仍按仓库的 `--parallel 4` 设置执行。

## 验证和验收

- 提取公共头的有效声明，核对 94 个导出名称、类型声明和转换后的参数/输出顺序。
- 验证打开、连接状态、等待连接、ping、关闭、参数读写、流控与轴状态字段。
- 在虚拟轴上验证上电、绝对/相对运动、多轴指定顺序及最终状态；观察循环带截止时间。
- 验证虚拟数字/模拟输出写读、含零字节的文件传输及命令结果完整字段。
- 累计超过 64 条终态，验证足够容量读取；小容量保留原错误和所需长度，再显式重读。
- 优先 Windows `build/`，执行 `cmake ..` 后启用 Python/live 测试和显式端口 18082，
  构建扩展及 demo。编译等待按仓库要求交由 GPT-luna；Windows/WSL 一侧编译通过即可。
- 成功构建后，在 Windows `build/` 和 WSL `build_wsl/` 分别运行
  `ctest --output-on-failure --parallel 4 -C Debug`。WSL 未启用 Python 时，其结果不代表绑定验证。
- 核对修改白名单、`git diff --check` 及旧入口残留；范围外失败只记录，不顺带修复。
- 真实 EtherCAT/SDO 和驱动回零的硬件成功行为不纳入自动验收，交付时单独标记未验证项。

完成标准：94 个绑定逐项对应公共 RPC，类型声明、示例和测试同步，核心成功调用链通过验证；
最终报告给出两处 CTest 结果、未验证行为和实际修改文件。

## 本次验证记录（2026-10-22）

- Windows Debug 扩展和 demo 编译通过：`build/pyclient_build_verified.log`。
- Python 定向 live 用例 9/9 通过（45.65 秒），包含完整名称/签名形态检查、两个示例、
  生命周期、参数及状态、指定轴顺序运动、虚拟 IO、二进制传输、70 次终态及容量重读。
  日志：`build/pyclient_live_verified.log`。
- Windows 最终全量 CTest：17 项通过、38 项失败、1 项禁用；Python live 用例通过
  （45.90 秒）。38 项分别为 36 个客户端用例找不到运行程序，以及 2 个 protocol 程序缺失。
  日志：`build/pyclient_ctest_windows_final.log`。
- WSL 全量 CTest：16 项通过、38 项失败、1 项禁用；同样是上述 38 项程序缺失。
  WSL 未启用 Python 扩展。日志：`build_wsl/pyclient_ctest_wsl.log`。
- 首轮测试缺少轴组使能和服务端目标文件，已按原 RPC 前置条件补齐测试配置/序列。
  一次全量并行运行出现上电示例观察超时；将 Python 集成用例设为独立调度后，最终全量运行通过该用例。
- 本次未运行真实 EtherCAT/SDO 或驱动回零的硬件成功场景，未修改范围外源码。
