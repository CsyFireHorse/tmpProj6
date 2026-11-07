# Lua runtime and `zmc.axfb`

日期：2026-10-13
代笔：GPT-6 Codex

This directory builds four separate runtime artifacts. Their boundaries are
intentional and must remain separate.

```text
lua.exe  -> lua55.dll
              ^
              |
axfb.dll ----->+-----> mcapi.dll
```

| Artifact | Responsibility |
| --- | --- |
| `lua55.dll` | The only Lua 5.5 runtime used by this distribution. |
| `lua.exe` | The standard Lua command-line interpreter. It executes Lua scripts. |
| `zmc/axfb.dll` | The native module loaded by `require("zmc.axfb")`. It exports only `luaopen_zmc_axfb`. |
| `mcapi.dll` | The existing Axis FB bridge implementation used by `axfb`. |

`axfb.dll` is not an interpreter and must never contain statically linked Lua C
sources. Both `lua.exe` and `axfb.dll` import the same `lua55.dll`; this guarantees
that the Lua state created by the interpreter is manipulated by the same Lua ABI in
the extension.

## Build

The vendored source under `src/thirdlib/lua` is pinned to Lua 5.5.0. CMake rejects
any other major/minor version.

```powershell
cmake -S . -B build -DENABLE_LUA_AXFB=ON
cmake --build build --config Debug
```

The relevant Windows Debug outputs are:

```text
build/lua/lua.exe
build/lua/lua55.dll
build/lua/zmc/axfb.dll
```

There is no `zmc_lua.exe` target or artifact.

## Run a script

The CLI loads the module through Lua's normal module search path:

```powershell
$env:PATH = "<mcapi-directory>;" + $env:PATH
build/lua/lua.exe -e "package.cpath='./build/lua/?.dll;'..package.cpath" `
  src/wrap/lua/demo/axfb_bridge_demo.lua `
  src/mhelper/data/zmc_motapp_demo_fak0.ini -5 400 `
  src/mhelper/data/pse_servo1.xml
```

## Error checking (`fail_if`)

Demo and integration scripts use `fail_if` only on core statements:

- bridge init: `axfb.new`, `load_eni`, `configure`, `axfb.status`
- time advance: `bridge:run_cycle`

Packet fill helpers (`set_*`, `move_abs`, `power_on`, ...), `push_packet`, and
`status:pop` are not wrapped in `fail_if`. Hard end-state checks belong in
`test/` via `assert_*`, not in `demo/`.

## Integration tests (ctest)

When `ENABLE_LUA_AXFB=ON`, CMake registers Lua tests under `test/`:

| ctest name | script |
| --- | --- |
| `dev_test_lua_axfb_packet_init_reset` | `test/test_packet_init_reset.lua` |
| `dev_test_lua_axfb_bridge_status_pop` | `test/test_bridge_status_pop.lua` |
| `dev_test_lua_axfb_bridge_power_move` | `test/test_bridge_power_move.lua` |
| `dev_test_lua_memory_pou_fields` | `test/test_memory_pou_fields.lua` |

Shared helpers live in `test/helpers.lua`. Memory-POU assembler (fields string
-> live image + Lua `FB` map) lives in `test/memory_pou_assemble.lua`.

## Memory POU (declaration-as-image)

C++ exposes only the image engine (`axfb_memory_pou_*`: layout / configure /
decl offsets / bind / timed script). Dynamic FB assembly is Lua-only:

1. User supplies `MEMORY_POU_DEMO_FIELDS` body (type/name list).
2. Lua allocates the live image, maps `Fbt*` -> `axfb.mc.*_create/exec/destroy`.
3. Script assigns and `MC_*_exec` dual-write the same bytes.

```lua
local assemble = require("memory_pou_assemble")
local pou, image, exec_list, axis = assemble.assemble(fields)
pou:parse_script(script)
for i = 1, 200 do assemble.run_cycle(pou, exec_list, axis) end
```

Run from the build tree:

```powershell
ctest --test-dir build --output-on-failure --parallel 4 -C Debug -R lua_axfb
```

Manual single-test example (repo root as cwd):

```powershell
$env:PATH = "build/src/mcapi/Debug;build/lua;" + $env:PATH
build/lua/lua.exe `
  -e "package.cpath='./build/lua/?.dll;'..package.cpath; package.path='./src/wrap/lua/test/?.lua;'..package.path" `
  src/wrap/lua/test/test_packet_init_reset.lua
```

## Lua bridge lifecycle

The Lua bridge does not expose PDO layout or axis-binding operations. A script loads
the ENI, then configures the bridge from its INI. The bridge owns PDO layout setup;
the INI owns axis binding.

```lua
bridge:load_eni(eni_path)
bridge:configure({ ini_path = ini_path, axis_num = 1 })
```

`bridge:run_cycle(inp, oup)` is the only call that advances motion time. Lua does not
expose `layout`, `apply_layout`, `auto_bind`, or `manual_bind`.

Lua bridge does not own, decode, or consume qcmd status packets. `axfb.status()`
creates an independent status client that owns only its `qcmd::MotionStatusMirror`.
After `bridge:run_cycle()`, the script pops one status packet at a time from the
bridge and applies it into the mirror, then queries typed state.

```lua
local status = assert(axfb.status())
bridge:run_cycle(inp, oup)
while status:pop(bridge) == 0 do end
local ret, state = status:get_axis_state2(0)
local flow_ret, flow = status:get_flow_status()
local seq_ret, seq = status:ack_seq()
```

`status:pop(bridge)` is a single-element helper: one `axfb_bridge_pop_status_packet`
followed by one `axfb_status_apply_packet`. Neither bridge nor status retains the
other. If a script resets or reconfigures its bridge, it explicitly calls
`status:reset()` before the next pop loop.

Status query methods mirror `MotionStatusMirror` and use wire types from
`MotionCmdTypes.hpp` (`AxisState`, `AxisState2`, `FlowCtrlStatus`) and the shared
C/C++ `MotionCmdResult.h` (`CmdResult` for both axis and IO results): `get_axis_state`, `get_axis_state2`, `get_flow_status`,
`get_cmd_results`, `get_io_results`, `current_tick_for_axis`, `current_axis_state`,
`ack_seq`, `ack_tick`.

Both result query methods return the same nine fields: `id`, `command_type`,
`target_index`, `target_kind`, `status`, `io_count`, `error_code`, `tick`, `data`.
`command_type` is the submitted `FBM_*` command, not the implementation FB type.
Non-IO `target_kind` values are Axis `1`, Group `2`, and Controller `3`.
IO kinds DI/DO/AI/AO use bases 0x10/0x14/0x18/0x1c plus group 0..3; each group
contains 256 points. Decode the start as `((target_kind - 0x10) & 3) * 256 +
target_index`, and the end as `start + io_count - 1` only when the count is nonzero.
`data` is signed int32 on the wire. IO values and axis-terminal start cycles retain
their complete unsigned bits; recover these in Lua with `data & 0xffffffff`.
Axis-terminal `tick` is the result cycle and its `data` is the command-binding cycle.
IO writes report the submitted value; reads report their value or zero on failure.
This result schema replaces the previous fields and requires matching binaries.

## qcmd packets

`axfb.packet()` stores an inline `MCCmdPacket` (via `AxFbBridgePacket`) value and
calls `axfb_cmd_packet_init` on construction.

- `packet:init()` clears wire fields.
- `packet:reset()` appends an `FBM_RESET` command item (not a clear).

Typed helpers (`set_axis`, `move_abs`, `multi_move_abs`, `group_set_axes`, ...) fill
that packet; `bridge:push_packet` calls `axfb_bridge_push_cmd_packet` directly.
Wire status types come from `MotionCmdTypes.hpp` and `MotionCmdResult.h`. Lua neither includes qcmd
headers nor links `plcfb`. Lua does not encode `FBM_*` items or implement a command
queue.

An external interpreter may load `axfb` only when it uses the same Lua 5.5 ABI and
the same `lua55.dll`. An arbitrary Lua executable, including a different Lua minor
version, is not a supported host.

## Source layout

- `source/`: implements `axfb`; it separates shared Lua helpers, bridge userdata,
  command userdata, status-client userdata, and module registration.
- `include/`: declares the public Lua loader entry point for `axfb`.
- `runtime/`: builds the Lua shared runtime and CLI used by the root module.
- `demo/`: Lua scripts only; it does not define a runtime target.
- `test/`: multi-file Lua integration tests registered with ctest.

AI modification signature: Composer
