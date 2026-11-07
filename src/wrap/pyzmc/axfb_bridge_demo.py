"""
Minimal AxFbBridge virtual-axis demo.

This demo intentionally keeps all bridge-specific helpers private to this file.
It does not extend ``pymc.py`` or add new C/C++ exports. The missing reusable
Python interfaces that are intentionally not added in this patch are:

1. ``ZmcPioOffsetLayout`` / ``pi_offset_config`` / ``FlowCtrlConfig`` /
   ``AxisState2`` are not formally exposed by ``pymc.py``.
2. Python-side ``FBM_*`` / ``STAT_*`` constants and typed packet helpers are
   not formally exposed by ``pymc.py``.
3. The bridge wrapper does not expose ``AXIS_REF``, so this demo cannot reuse
   ``FbPower`` / ``FbMoveAbsolute`` directly.
4. There is no reusable Python helper that turns ENI content into
   ``ZmcPioOffsetLayout``.
"""

from __future__ import print_function

import argparse
import csv
import os
import struct
import sys
from ctypes import Structure
from ctypes import byref
from ctypes import c_double
from ctypes import c_uint8
from ctypes import c_uint16
from ctypes import c_uint32
from ctypes import c_int32
from ctypes import sizeof

try:
    from pymc import AXFB_BRIDGE_PACKET_PAYLOAD_SIZE
    from pymc import AxFbBridge
    from pymc import zmc_dll
except Exception as exc:
    print("Failed to import pymc AxFbBridge wrapper: %s" % exc)
    raise SystemExit(1)


MAX_NUM_SERVOS = 32

ENI_DESCRIPTOR_SIZE = 3608
PI_OFFSET_CONFIG_SIZE = 48
PIO_LAYOUT_SLOT_COUNT_OFFSET = 5144
PIO_LAYOUT_TOTAL_SIZE = 5148

MC_FINITE_DIRECTION = 4
POWER_ON_BLOCK_TICKS = 100

FBM_AXIS = 0x0002
FBM_SET_SPEED = 0x0023
FBM_SET_ACCEL = 0x0024
FBM_SET_DECEL = 0x0025
FBM_SET_JERK = 0x0026
FBM_SET_DIRECTION = 0x002A
FBM_POWER = 0x0030
FBM_MOVE_ABSOLUTE = 0x0043
FBM_BLOCK_TICKS = 0x00E7
FBM_FLOWCTRL_CFG = 0x00F1

STAT_STATE = 0x8002
STAT_STATE2 = 0x8006

CSV_COLUMNS = (
    "cycle",
    "axis",
    "state",
    "error_id",
    "act_pos",
    "set_pos",
    "set_vel",
    "set_acc",
    "set_position",
    "act_position",
    "tick",
    "ret",
    "value",
    "note",
    "kind",
    "step",
)


class DemoError(RuntimeError):
    """Raised when the bridge demo cannot complete successfully."""


class DemoLayoutDescriptor(Structure):
    """Keep only the ENI descriptor bytes that the bridge hot path reads."""

    _fields_ = [
        ("_master_reserved", c_uint8 * 12),
        ("slave_count", c_uint16),
        ("_pdo_cell_count_reserved", c_uint16),
        ("tx_cap", c_uint32),
        ("rx_cap", c_uint32),
        ("_tail_reserved", c_uint8 * (ENI_DESCRIPTOR_SIZE - 24)),
    ]


class DemoLayoutSlot(Structure):
    """Keep one full slot because the demo binds exactly one servo."""

    _fields_ = [
        ("slave_idx", c_uint8),
        ("slave_channel_idx", c_uint8),
        ("po_mode_offset", c_int32),
        ("po_ctrl_word_offset", c_int32),
        ("po_p_offset", c_int32),
        ("po_v_offset", c_int32),
        ("po_t_offset", c_int32),
        ("pi_mode_offset", c_int32),
        ("pi_stat_offset", c_int32),
        ("pi_error_offset", c_int32),
        ("pi_p_offset", c_int32),
        ("pi_v_offset", c_int32),
        ("pi_t_offset", c_int32),
    ]


class ZmcPioOffsetLayout(Structure):
    """Single-axis view over the full PDO layout binary contract."""

    _fields_ = [
        ("descriptor_pod", DemoLayoutDescriptor),
        ("slot0", DemoLayoutSlot),
        ("_remaining_slot_bytes", c_uint8 * (PI_OFFSET_CONFIG_SIZE * (MAX_NUM_SERVOS - 1))),
        ("slot_count", c_uint16),
        ("_tail_reserved", c_uint8 * 2),
    ]


class FlowCtrlConfig(Structure):
    """Flow-control configuration payload."""

    _fields_ = [
        ("period", c_uint16),
        ("batchSize", c_uint16),
        ("verboseFlag", c_uint8),
        ("reserved", c_uint8 * 3),
    ]

class AxisState(Structure):
    """Compact axis status payload."""

    _fields_ = [
        ("axis_id", c_uint8),
        ("axis_state", c_uint8),
        ("error_id", c_uint16),
        ("tick", c_uint32),
        ("act_pos", c_double),
        ("set_pos", c_double),
    ]


class AxisState2(Structure):
    """Extended axis status payload."""

    _fields_ = [
        ("axis_id", c_uint8),
        ("axis_state", c_uint8),
        ("error_id", c_uint16),
        ("tick", c_uint32),
        ("act_pos", c_double),
        ("set_pos", c_double),
        ("set_vel", c_double),
        ("set_acc", c_double),
        ("set_position", c_int32),
        ("act_position", c_int32),
    ]


def validate_layout_shape():
    """Validate the reduced ctypes layout against the source-derived binary shape."""

    if sizeof(AxisState) != 24 or sizeof(AxisState2) != 48:
        raise DemoError("Axis state payload size mismatch")
    if (AxisState.tick.offset, AxisState.act_pos.offset, AxisState.set_pos.offset) != (4, 8, 16):
        raise DemoError("AxisState field offset mismatch")
    if (AxisState2.tick.offset, AxisState2.act_pos.offset, AxisState2.set_pos.offset,
            AxisState2.set_vel.offset, AxisState2.set_acc.offset,
            AxisState2.set_position.offset, AxisState2.act_position.offset) != (
                4, 8, 16, 24, 32, 40, 44
            ):
        raise DemoError("AxisState2 field offset mismatch")
    if sizeof(DemoLayoutDescriptor) != ENI_DESCRIPTOR_SIZE:
        raise DemoError(
            "DemoLayoutDescriptor size mismatch: got=%d expect=%d"
            % (sizeof(DemoLayoutDescriptor), ENI_DESCRIPTOR_SIZE)
        )
    if sizeof(DemoLayoutSlot) != PI_OFFSET_CONFIG_SIZE:
        raise DemoError(
            "DemoLayoutSlot size mismatch: got=%d expect=%d"
            % (sizeof(DemoLayoutSlot), PI_OFFSET_CONFIG_SIZE)
        )
    if ZmcPioOffsetLayout.slot_count.offset != PIO_LAYOUT_SLOT_COUNT_OFFSET:
        raise DemoError(
            "slot_count offset mismatch: got=%d expect=%d"
            % (ZmcPioOffsetLayout.slot_count.offset, PIO_LAYOUT_SLOT_COUNT_OFFSET)
        )
    if sizeof(ZmcPioOffsetLayout) != PIO_LAYOUT_TOTAL_SIZE:
        raise DemoError(
            "ZmcPioOffsetLayout size mismatch: got=%d expect=%d"
            % (sizeof(ZmcPioOffsetLayout), PIO_LAYOUT_TOTAL_SIZE)
        )


def ensure_dll_loaded():
    """Fail early when the wrapped motion DLL is not available."""

    if not zmc_dll:
        raise DemoError("zmc_dll is not loaded")


def default_ini_path():
    """Resolve the default fake-axis INI path from the current demo file."""

    here = os.path.abspath(os.path.dirname(__file__))
    src_dir = os.path.abspath(os.path.join(here, "..", ".."))
    return os.path.join(src_dir, "mhelper", "data", "zmc_motapp_demo_fak2.ini")


def pack_i32(value):
    """Pack one signed 32-bit integer using little-endian layout."""

    return struct.pack("<i", int(value))


def pack_f64(value):
    """Pack one float64 using little-endian layout."""

    return struct.pack("<d", float(value))


def pack_flow_cfg(period, batch_size, verbose_flag):
    """Pack FlowCtrlConfig using the same wire layout as the C struct."""

    return struct.pack("<HHB3x", int(period), int(batch_size), int(verbose_flag))


def append_item(packet_payload, item_type, item_payload):
    """Append one motion item with the shared {type,len,payload} wire format."""

    if item_payload is None:
        item_payload = b""
    item_header = struct.pack("<HH", int(item_type), len(item_payload))
    item_size = len(item_header) + len(item_payload)
    if len(packet_payload) + item_size > AXFB_BRIDGE_PACKET_PAYLOAD_SIZE:
        raise DemoError(
            "packet payload overflow: current=%d add=%d max=%d"
            % (len(packet_payload), item_size, AXFB_BRIDGE_PACKET_PAYLOAD_SIZE)
        )
    packet_payload.extend(item_header)
    packet_payload.extend(item_payload)
    return packet_payload


def iter_packet_items(packet):
    """Iterate typed items stored in an AxFbBridge status packet."""

    raw = bytes(packet.payload[: packet.payload_len])
    offset = 0
    while offset + 4 <= len(raw):
        item_type, item_len = struct.unpack_from("<HH", raw, offset)
        offset += 4
        item_end = offset + item_len
        if item_end > len(raw):
            break
        yield item_type, raw[offset:item_end]
        offset = item_end


def make_one_axis_layout():
    """Build only the layout bytes that the bridge actually reads in this demo."""

    layout = ZmcPioOffsetLayout()
    layout.slot_count = 1
    layout.descriptor_pod.tx_cap = 16
    layout.descriptor_pod.rx_cap = 16
    layout.descriptor_pod.slave_count = 1
    layout.slot0.slave_idx = 1
    layout.slot0.slave_channel_idx = 0
    layout.slot0.po_ctrl_word_offset = 0
    layout.slot0.po_p_offset = 2
    layout.slot0.po_mode_offset = -1
    layout.slot0.po_v_offset = -1
    layout.slot0.po_t_offset = -1
    layout.slot0.pi_stat_offset = 0
    layout.slot0.pi_p_offset = 2
    layout.slot0.pi_mode_offset = -1
    layout.slot0.pi_error_offset = -1
    layout.slot0.pi_v_offset = -1
    layout.slot0.pi_t_offset = -1
    return layout


def build_flow_packet():
    """Build the flow-control command packet."""

    payload = bytearray()
    append_item(payload, FBM_FLOWCTRL_CFG, pack_flow_cfg(1, 8, 1))
    return bytes(payload)


def build_power_packet(args):
    """Build the profile and power-on command packet."""

    payload = bytearray()
    append_item(payload, FBM_AXIS, pack_i32(0))
    append_item(payload, FBM_SET_SPEED, pack_f64(args.velocity))
    append_item(payload, FBM_SET_ACCEL, pack_f64(args.acceleration))
    append_item(payload, FBM_SET_DECEL, pack_f64(args.deceleration))
    append_item(payload, FBM_SET_JERK, pack_f64(args.jerk))
    append_item(payload, FBM_SET_DIRECTION, pack_i32(MC_FINITE_DIRECTION))
    append_item(payload, FBM_POWER, pack_f64(1.0))
    return bytes(payload)


def build_block_ticks_packet(block_ticks):
    """Build the exclusive block-ticks packet used after power-on."""

    payload = bytearray()
    append_item(payload, FBM_BLOCK_TICKS, pack_i32(block_ticks))
    return bytes(payload)


def build_move_packet(args):
    """Build the absolute-move command packet."""

    payload = bytearray()
    append_item(payload, FBM_MOVE_ABSOLUTE, pack_f64(args.target_pos))
    return bytes(payload)


def push_demo_cmd_packets(bridge, writer, args):
    """Push the demo command packets using the fixed setup order."""

    require_zero(writer, "push_flow_packet", bridge.push_cmd_packet(build_flow_packet()))
    require_zero(writer, "push_power_packet", bridge.push_cmd_packet(build_power_packet(args)))
    require_zero(
        writer,
        "push_block_ticks_packet",
        bridge.push_cmd_packet(build_block_ticks_packet(POWER_ON_BLOCK_TICKS)),
    )
    require_zero(writer, "push_move_packet", bridge.push_cmd_packet(build_move_packet(args)))


def decode_status_packet(packet):
    """Decode only the status items that this demo cares about."""

    decoded = {
        "axis_states": [],
        "axis_state2_list": [],
    }
    for item_type, item_payload in iter_packet_items(packet):
        if item_type == STAT_STATE and len(item_payload) >= sizeof(AxisState):
            decoded["axis_states"].append(AxisState.from_buffer_copy(item_payload[: sizeof(AxisState)]))
        elif item_type == STAT_STATE2 and len(item_payload) >= sizeof(AxisState2):
            decoded["axis_state2_list"].append(
                AxisState2.from_buffer_copy(item_payload[: sizeof(AxisState2)])
            )
    return decoded


def make_status_snapshot():
    """Create one mutable status snapshot shared by drain and run loops."""

    return {
        "axis_state": None,
        "axis_state2": None,
        "axis_state_by_axis": {},
        "axis_state2_by_axis": {},
        "count": 0,
    }


def drain_status_packets(bridge):
    """Drain the status queue and keep the latest decoded items."""

    latest = make_status_snapshot()
    while True:
        ret, packet = bridge.pop_status_packet()
        if ret != 0:
            break
        decoded = decode_status_packet(packet)
        latest["count"] += 1
        for axis_state in decoded["axis_states"]:
            latest["axis_state"] = axis_state
            latest["axis_state_by_axis"][axis_state.axis_id] = axis_state
        for axis_state2 in decoded["axis_state2_list"]:
            latest["axis_state2"] = axis_state2
            latest["axis_state2_by_axis"][axis_state2.axis_id] = axis_state2
    return latest


def make_csv_writer():
    """Create one stdout CSV writer with a stable line terminator."""

    return csv.writer(sys.stdout, lineterminator="\n")


def write_csv_row(writer, **fields):
    """Write one CSV row using a fixed, demo-friendly schema."""

    writer.writerow([fields.get(column, "") for column in CSV_COLUMNS])


def write_axis_state2_rows(writer, cycle_idx, axis_state2_by_axis, note=""):
    """Write one CSV row per axis using the latest extended status snapshot."""

    if not axis_state2_by_axis:
        write_csv_row(
            writer,
            kind="status",
            cycle=cycle_idx,
            note=note or "no AxisState2 yet",
        )
        return
    for axis_id in sorted(axis_state2_by_axis):
        axis_state2 = axis_state2_by_axis[axis_id]
        write_csv_row(
            writer,
            kind="status",
            cycle=cycle_idx,
            axis=axis_state2.axis_id,
            state=axis_state2.axis_state,
            error_id=axis_state2.error_id,
            act_pos="%.6f" % axis_state2.act_pos,
            set_pos="%.6f" % axis_state2.set_pos,
            set_vel="%.6f" % axis_state2.set_vel,
            set_acc="%.6f" % axis_state2.set_acc,
            set_position=axis_state2.set_position,
            act_position=axis_state2.act_position,
            tick=axis_state2.tick,
            note=note,
        )


def write_step_ret(writer, step_name, ret, value=""):
    """Write one step result row."""

    write_csv_row(writer, kind="step", step=step_name, ret=ret, value=value)


def require_zero(writer, step_name, ret):
    """Raise a DemoError when a bridge API returns a non-zero code."""

    write_step_ret(writer, step_name, ret)
    if ret != 0:
        raise DemoError("%s failed with ret=%d" % (step_name, ret))


def parse_args(argv=None):
    """Parse command line arguments for the bridge demo."""

    parser = argparse.ArgumentParser(description="Minimal AxFbBridge single-axis virtual demo")
    parser.add_argument("--ini_path", default=default_ini_path())
    parser.add_argument("--target_pos", type=float, default=-52.0)
    parser.add_argument("--velocity", type=float, default=20.0)
    parser.add_argument("--acceleration", type=float, default=100.0)
    parser.add_argument("--deceleration", type=float, default=100.0)
    parser.add_argument("--jerk", type=float, default=0.0)
    parser.add_argument("--move_cycles", type=int, default=500)
    parser.add_argument("--print_every", type=int, default=10)
    return parser.parse_args(argv)


def main(argv=None):
    """Run the minimal AxFbBridge virtual-axis demo."""

    args = parse_args(argv)
    writer = make_csv_writer()
    writer.writerow(CSV_COLUMNS)
    write_csv_row(writer, kind="meta", step="ini_path", value=args.ini_path)
    write_csv_row(writer, kind="meta", step="packet_payload_max", value=AXFB_BRIDGE_PACKET_PAYLOAD_SIZE)
    write_csv_row(writer, kind="meta", step="finite_direction", value=MC_FINITE_DIRECTION)
    write_csv_row(writer, kind="meta", step="target_pos", value="%.6f" % args.target_pos)
    write_csv_row(writer, kind="meta", step="velocity", value="%.6f" % args.velocity)
    write_csv_row(writer, kind="meta", step="acceleration", value="%.6f" % args.acceleration)
    write_csv_row(writer, kind="meta", step="deceleration", value="%.6f" % args.deceleration)
    write_csv_row(writer, kind="meta", step="jerk", value="%.6f" % args.jerk)
    write_csv_row(writer, kind="meta", step="block_ticks_after_power", value=POWER_ON_BLOCK_TICKS)
    write_csv_row(writer, kind="meta", step="move_cycles", value=args.move_cycles)
    write_csv_row(writer, kind="meta", step="print_every", value=args.print_every)

    ensure_dll_loaded()
    validate_layout_shape()
    if not os.path.exists(args.ini_path):
        raise DemoError("INI file not found: %s" % args.ini_path)

    layout = make_one_axis_layout()
    inp_buf = (c_uint8 * 16)()
    oup_buf = (c_uint8 * 16)()

    with AxFbBridge() as bridge:
        require_zero(
            writer,
            "configure",
            bridge.configure(ini_path=args.ini_path, ini_content="", axis_num=1),
        )
        require_zero(writer, "apply_pio_layout", bridge.apply_pio_layout(byref(layout)))
        require_zero(writer, "auto_bind", bridge.auto_bind(0, 1))

        ret, value = bridge.axis_count()
        write_step_ret(writer, "axis_count", ret, value)
        ret, value = bridge.servo_count()
        write_step_ret(writer, "servo_count", ret, value)
        ret, value = bridge.pdo_layout_ready()
        write_step_ret(writer, "pdo_layout_ready", ret, value)

        push_demo_cmd_packets(bridge, writer, args)

        latest = make_status_snapshot()
        total_cycles = POWER_ON_BLOCK_TICKS + args.move_cycles
        for cycle_idx in range(1, total_cycles + 1):
            ret = bridge.run_cycle(inp_buf, oup_buf)
            if ret != 0:
                raise DemoError("run_cycle failed at cycle=%d ret=%d" % (cycle_idx, ret))
            drained = drain_status_packets(bridge)
            latest["count"] += drained["count"]
            for key in ("axis_state", "axis_state2"):
                if drained[key] is not None:
                    latest[key] = drained[key]
            latest["axis_state_by_axis"].update(drained["axis_state_by_axis"])
            latest["axis_state2_by_axis"].update(drained["axis_state2_by_axis"])
            if cycle_idx == 1 or cycle_idx == total_cycles or cycle_idx % max(1, args.print_every) == 0:
                write_axis_state2_rows(writer, cycle_idx, latest["axis_state2_by_axis"], "sample")

    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except DemoError as exc:
        write_csv_row(make_csv_writer(), kind="result", note=str(exc))
        raise SystemExit(1)
