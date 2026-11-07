"""
Minimal AxFbBridge HTTP demo.

This file provides a Python-only HTTP wrapper around the existing
``axfb_bridge_demo.py`` bridge helpers. It is a demo service for a single
virtual axis and does not extend ``pymc.py`` or any C/C++ API.
"""

from __future__ import print_function

import argparse
import json
import mimetypes
import os
import sys
import threading
import time
from ctypes import Structure
from ctypes import byref
from ctypes import c_uint8
from ctypes import c_uint16
from ctypes import c_uint32
from ctypes import sizeof
from http.server import BaseHTTPRequestHandler
from http.server import ThreadingHTTPServer
from urllib.parse import parse_qs
from urllib.parse import urlsplit

from axfb_bridge_demo import AxFbBridge
from axfb_bridge_demo import AxisState
from axfb_bridge_demo import AxisState2
from axfb_bridge_demo import DemoError
from axfb_bridge_demo import append_item
from axfb_bridge_demo import default_ini_path
from axfb_bridge_demo import decode_status_packet
from axfb_bridge_demo import ensure_dll_loaded
from axfb_bridge_demo import iter_packet_items
from axfb_bridge_demo import make_one_axis_layout
from axfb_bridge_demo import pack_f64
from axfb_bridge_demo import pack_flow_cfg
from axfb_bridge_demo import pack_i32
from axfb_bridge_demo import validate_layout_shape


ZMC_NO_ERROR = 0
ZMC_ERR_MOTCMD_SCRIPT_BUSY_DISCARD = 17104
ZMC_ERR_MOTCMD_ABORT_PUSH_FAILED = 17105
ZMC_ERR_MOTCMD_INVALID_ARG = 17108

MC_FINITE_DIRECTION = 4

FBM_AXIS = 0x0002
FBM_SET_SPEED = 0x0023
FBM_SET_ACCEL = 0x0024
FBM_SET_DECEL = 0x0025
FBM_SET_JERK = 0x0026
FBM_SET_DIRECTION = 0x002A
FBM_POWER = 0x0030
FBM_STOP = 0x0040
FBM_HALT = 0x0041
FBM_MOVE_RELATIVE = 0x0042
FBM_MOVE_ABSOLUTE = 0x0043
FBM_BLOCK_TICKS = 0x00E7
FBM_FLOWCTRL_CFG = 0x00F1

STAT_FLOWCTRL_STATUS = 0x8007


class FlowCtrlStatus(Structure):
    """Runtime flow-status payload copied from the motion status wire layout."""

    _fields_ = [
        ("period", c_uint16),
        ("batchSize", c_uint16),
        ("verboseFlag", c_uint8),
        ("reserved", c_uint8 * 3),
        ("latency", c_uint16),
        ("axis_num", c_uint16),
        ("cycleTime", c_uint32),
    ]


class DemoHttpError(Exception):
    """Raised when one HTTP request cannot be satisfied."""

    def __init__(self, code, message, http_status=200):
        Exception.__init__(self, message)
        self.code = int(code)
        self.message = str(message)
        self.http_status = int(http_status)


def build_flow_packet(period, batch_size, verbose_flag):
    """Build the flow-control packet using the shared bridge wire format."""

    payload = bytearray()
    append_item(payload, FBM_FLOWCTRL_CFG, pack_flow_cfg(period, batch_size, verbose_flag))
    return bytes(payload)


def build_profile_packet(axis, velocity, acceleration, deceleration, jerk):
    """Build one single-axis profile packet."""

    payload = bytearray()
    append_item(payload, FBM_AXIS, pack_i32(axis))
    append_item(payload, FBM_SET_SPEED, pack_f64(velocity))
    append_item(payload, FBM_SET_ACCEL, pack_f64(acceleration))
    append_item(payload, FBM_SET_DECEL, pack_f64(deceleration))
    append_item(payload, FBM_SET_JERK, pack_f64(jerk))
    append_item(payload, FBM_SET_DIRECTION, pack_i32(MC_FINITE_DIRECTION))
    return bytes(payload)


def build_power_packet(axis, enable):
    """Build the single-axis power packet."""

    payload = bytearray()
    append_item(payload, FBM_AXIS, pack_i32(axis))
    append_item(payload, FBM_POWER, pack_f64(1.0 if enable else 0.0))
    return bytes(payload)


def build_move_abs_packet(axis, pos, profile):
    """Build one absolute-move packet with an inline profile update."""

    payload = bytearray(build_profile_packet(
        axis,
        profile["velocity"],
        profile["acceleration"],
        profile["deceleration"],
        profile["jerk"],
    ))
    append_item(payload, FBM_MOVE_ABSOLUTE, pack_f64(pos))
    return bytes(payload)


def build_move_rel_packet(axis, delta, profile):
    """Build one relative-move packet with an inline profile update."""

    payload = bytearray(build_profile_packet(
        axis,
        profile["velocity"],
        profile["acceleration"],
        profile["deceleration"],
        profile["jerk"],
    ))
    append_item(payload, FBM_MOVE_RELATIVE, pack_f64(delta))
    return bytes(payload)


def build_stop_packet(axis):
    """Build the single-axis stop packet."""

    payload = bytearray()
    append_item(payload, FBM_AXIS, pack_i32(axis))
    append_item(payload, FBM_STOP, None)
    return bytes(payload)


def build_halt_packet(axis):
    """Build the single-axis halt packet."""

    payload = bytearray()
    append_item(payload, FBM_AXIS, pack_i32(axis))
    append_item(payload, FBM_HALT, None)
    return bytes(payload)


def build_block_ticks_packet(ticks):
    """Build the exclusive block-ticks packet used after power-on."""

    payload = bytearray()
    append_item(payload, FBM_BLOCK_TICKS, pack_i32(ticks))
    return bytes(payload)


def make_initial_status_cache(args):
    """Create the mutable shared status cache for the HTTP runtime."""

    return {
        "axis_state": None,
        "axis_state2": None,
        "axis_state_by_axis": {},
        "axis_state2_by_axis": {},
        "flow_status": {
            "period": 1,
            "batchSize": 20,
            "verboseFlag": 1,
            "latency": 0,
            "axis_num": 1,
            "cycleTime": int(float(args.cycle_period_ms) * 1000.0),
        },
        "flow_config": {
            "period": 1,
            "batchSize": 20,
            "verboseFlag": 1,
        },
        "last_cycle": 0,
    }


def decode_flow_status(packet):
    """Decode the last flow-status item carried by one status packet."""

    flow_status = None
    for item_type, item_payload in iter_packet_items(packet):
        if item_type == STAT_FLOWCTRL_STATUS and len(item_payload) >= sizeof(FlowCtrlStatus):
            flow_status = FlowCtrlStatus.from_buffer_copy(item_payload[: sizeof(FlowCtrlStatus)])
    return flow_status


def drain_status_packets_with_flow(bridge):
    """Drain all queued status packets and keep the latest axis and flow data."""

    latest = {
        "axis_state": None,
        "axis_state2": None,
        "axis_state_by_axis": {},
        "axis_state2_by_axis": {},
        "flow_status": None,
        "count": 0,
    }
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
        flow_status = decode_flow_status(packet)
        if flow_status is not None:
            latest["flow_status"] = flow_status
    return latest


def envelope(code, message, fields=None):
    """Build the shared JSON envelope used by the demo HTTP routes."""

    body = {"code": int(code), "message": str(message)}
    if fields:
        body.update(fields)
    return body


def write_json(handler, http_status, body):
    """Write one JSON response."""

    payload = json.dumps(body, indent=2, sort_keys=True).encode("utf-8")
    handler.send_response(int(http_status))
    handler.send_header("Content-Type", "application/json")
    handler.send_header("Content-Length", str(len(payload)))
    handler.end_headers()
    handler.wfile.write(payload)


def reply_ok(handler, fields=None):
    """Reply with the standard success envelope."""

    write_json(handler, 200, envelope(0, "ok", fields))


def reply_error(handler, code, message, http_status=500, fields=None):
    """Reply with the standard error envelope."""

    write_json(handler, http_status, envelope(code, message, fields))


def reply_invalid_arg(handler, message):
    """Reply with the standard invalid-argument envelope."""

    reply_error(handler, ZMC_ERR_MOTCMD_INVALID_ARG, message, 200)


def read_json_body(handler, allow_empty=False):
    """Parse one JSON request body and convert parse failures into DemoHttpError."""

    body_len = int(handler.headers.get("Content-Length", "0") or "0")
    raw = handler.rfile.read(body_len) if body_len > 0 else b""
    if not raw:
        if allow_empty:
            return {}
        raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "missing json body")
    try:
        body = json.loads(raw.decode("utf-8"))
    except Exception as exc:
        raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "invalid json body: %s" % exc)
    if not isinstance(body, dict):
        raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "json body must be an object")
    return body


def get_axis_or_default_zero(body):
    """Validate the optional single-axis selector used by the demo."""

    axis = body.get("axis", 0)
    try:
        axis = int(axis)
    except Exception:
        raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "invalid 'axis' field")
    if axis != 0:
        raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "only axis 0 is supported")
    return axis


def get_single_pos(body, field_names):
    """Read one numeric position from a list of compatible request-body field names."""

    for field_name in field_names:
        if field_name not in body:
            continue
        value = body[field_name]
        if isinstance(value, (int, float)):
            return float(value)
        if isinstance(value, str):
            parts = [item.strip() for item in value.split(",") if item.strip()]
            if len(parts) != 1:
                raise DemoHttpError(
                    ZMC_ERR_MOTCMD_INVALID_ARG,
                    "invalid single '%s' field" % field_name,
                )
            try:
                return float(parts[0])
            except Exception:
                raise DemoHttpError(
                    ZMC_ERR_MOTCMD_INVALID_ARG,
                    "invalid '%s' field" % field_name,
                )
        if isinstance(value, list):
            if len(value) != 1:
                raise DemoHttpError(
                    ZMC_ERR_MOTCMD_INVALID_ARG,
                    "invalid single '%s' field" % field_name,
                )
            try:
                return float(value[0])
            except Exception:
                raise DemoHttpError(
                    ZMC_ERR_MOTCMD_INVALID_ARG,
                    "invalid '%s' field" % field_name,
                )
        raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "invalid '%s' field" % field_name)
    if len(field_names) == 1:
        raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "missing '%s' field" % field_names[0])
    raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "missing numeric field")


def parse_axis_query(query_map):
    """Parse one dashboard axes query into a filtered single-axis list."""

    if "axes" not in query_map or not query_map["axes"]:
        return [0]
    axes = []
    raw = query_map["axes"][0]
    for item in str(raw).split(","):
        item = item.strip()
        if not item:
            continue
        try:
            axis = int(item)
        except Exception:
            continue
        if axis == 0 and axis not in axes:
            axes.append(axis)
    if not axes:
        return [0]
    return axes


def parse_query_bool(query_map, field_name, default_value):
    """Parse one optional boolean query flag."""

    if field_name not in query_map or not query_map[field_name]:
        return bool(default_value)
    value = str(query_map[field_name][0]).strip().lower()
    return value in ("1", "true", "yes", "on")


def parse_int_value(value, field_name):
    """Parse one integer field and raise a request-level invalid-arg error."""

    try:
        return int(value)
    except Exception:
        raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "invalid '%s' field" % field_name)


def parse_float_value(value, field_name):
    """Parse one float field and raise a request-level invalid-arg error."""

    try:
        return float(value)
    except Exception:
        raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "invalid '%s' field" % field_name)


def default_web_root():
    """Resolve the default source web-client directory for this demo server."""

    here = os.path.abspath(os.path.dirname(__file__))
    src_dir = os.path.abspath(os.path.join(here, "..", ".."))
    return os.path.join(src_dir, "app", "demo", "web_client")


def guess_content_type(path):
    """Guess the content type for one static file path."""

    content_type, _ = mimetypes.guess_type(path)
    if content_type:
        return content_type
    return "application/octet-stream"


def map_bridge_ret(ret):
    """Translate one bridge-layer return code into the HTTP error contract."""

    if ret == ZMC_NO_ERROR:
        return 200, "ok"
    if ret == ZMC_ERR_MOTCMD_SCRIPT_BUSY_DISCARD:
        return 503, "script busy"
    if ret == ZMC_ERR_MOTCMD_ABORT_PUSH_FAILED:
        return 500, "abort push failed"
    return 500, "fail"


class BridgeHttpRuntime(object):
    """Own the bridge handle, cycle thread, and status cache for the HTTP demo."""

    def __init__(self, args):
        self.args = args
        self.profile = {
            "velocity": float(args.velocity),
            "acceleration": float(args.acceleration),
            "deceleration": float(args.deceleration),
            "jerk": float(args.jerk),
        }
        self.bridge = None
        self.bridge_lock = threading.Lock()
        self.state_lock = threading.Lock()
        self.stop_event = threading.Event()
        self.shutdown_requested = threading.Event()
        self.httpd = None
        self.cycle_thread = None
        self.runtime_error = None
        self.web_root = os.path.abspath(args.web_root)
        self.layout = make_one_axis_layout()
        self.inp_buf = (c_uint8 * 16)()
        self.oup_buf = (c_uint8 * 16)()
        self.status_cache = make_initial_status_cache(args)

    def set_httpd(self, httpd):
        """Attach the live HTTP server so runtime failures can stop it."""

        self.httpd = httpd
        if self.shutdown_requested.is_set():
            threading.Thread(
                target=self.httpd.shutdown,
                name="axfb-bridge-http-shutdown",
                daemon=True,
            ).start()

    def start(self):
        """Create and configure the bridge, then start the background cycle loop."""

        ensure_dll_loaded()
        validate_layout_shape()
        if not os.path.exists(self.args.ini_path):
            raise DemoError("INI file not found: %s" % self.args.ini_path)

        self.bridge = AxFbBridge()
        try:
            with self.bridge_lock:
                self._require_zero(
                    "configure",
                    self.bridge.configure(ini_path=self.args.ini_path, ini_content="", axis_num=1),
                )
                self._require_zero(
                    "apply_pio_layout",
                    self.bridge.apply_pio_layout(byref(self.layout)),
                )
                self._require_zero("auto_bind", self.bridge.auto_bind(0, 1))
                self._require_zero(
                    "push_flow_packet",
                    self.bridge.push_cmd_packet(build_flow_packet(1, 20, 1)),
                )
            self.cycle_thread = threading.Thread(
                target=self._cycle_loop,
                name="axfb-bridge-http-cycle",
                daemon=True,
            )
            self.cycle_thread.start()
        except Exception:
            self.close()
            raise

    def close(self):
        """Stop the cycle thread and release the bridge handle."""

        self.stop_event.set()
        if self.cycle_thread is not None and self.cycle_thread.is_alive():
            if self.cycle_thread is not threading.current_thread():
                self.cycle_thread.join(timeout=2.0)
        with self.bridge_lock:
            if self.bridge is not None:
                self.bridge.close()
                self.bridge = None

    def snapshot_axis_state2(self, axis):
        """Read the latest cached AxisState2 for the requested axis."""

        with self.state_lock:
            axis_state2 = self.status_cache["axis_state2_by_axis"].get(axis)
            if axis_state2 is None:
                return None
            return {
                "axis_id": int(axis_state2.axis_id),
                "axis_state": int(axis_state2.axis_state),
                "error_id": int(axis_state2.error_id),
                "tick": int(axis_state2.tick),
                "act_pos": float(axis_state2.act_pos),
                "set_pos": float(axis_state2.set_pos),
                "set_vel": float(axis_state2.set_vel),
                "set_acc": float(axis_state2.set_acc),
                "set_position": int(axis_state2.set_position),
                "act_position": int(axis_state2.act_position),
            }

    def snapshot_flow_status(self):
        """Read the latest cached flow status."""

        with self.state_lock:
            return dict(self.status_cache["flow_status"])

    def snapshot_flow_config(self):
        """Read the latest requested flow configuration."""

        with self.state_lock:
            return dict(self.status_cache["flow_config"])

    def snapshot_dashboard(self, axes, include_flow, include_cmd_results):
        """Build the compact dashboard payload used by the web client."""

        response = {"axes": []}
        with self.state_lock:
            for axis in axes:
                axis_obj = self._make_dashboard_axis_locked(axis)
                if axis_obj is not None:
                    response["axes"].append(axis_obj)
            if include_flow:
                response["flow_status"] = dict(self.status_cache["flow_status"])
        if include_cmd_results:
            response["cmd_results"] = []
        return response

    def push_packet(self, packet_payload):
        """Push one command packet through the bridge queue."""

        self._raise_if_runtime_failed()
        with self.bridge_lock:
            ret = self.bridge.push_cmd_packet(packet_payload)
        if ret != 0:
            http_status, message = map_bridge_ret(ret)
            raise DemoHttpError(ret, message, http_status)

    def power_on(self, axis):
        """Push the standard power-on packet sequence."""

        self.push_packet(build_profile_packet(
            axis,
            self.profile["velocity"],
            self.profile["acceleration"],
            self.profile["deceleration"],
            self.profile["jerk"],
        ))
        self.push_packet(build_power_packet(axis, True))
        self.push_packet(build_block_ticks_packet(self.args.power_block_ticks))

    def power_off(self, axis):
        """Push the standard power-off packet."""

        self.push_packet(build_power_packet(axis, False))

    def axis_enable(self, axis, enable):
        """Push the enable/disable packet used by the HTTP alias route."""

        self.push_packet(build_power_packet(axis, bool(enable)))

    def move_abs(self, axis, pos):
        """Push one absolute move packet."""

        self.push_packet(build_move_abs_packet(axis, pos, self.profile))

    def move_rel(self, axis, delta):
        """Push one relative move packet."""

        self.push_packet(build_move_rel_packet(axis, delta, self.profile))

    def stop(self, axis):
        """Push one stop packet."""

        self.push_packet(build_stop_packet(axis))

    def halt(self, axis):
        """Push one halt packet."""

        self.push_packet(build_halt_packet(axis))

    def update_profile(self, axis, field_name, value):
        """Update the in-memory profile and push it to the bridge."""

        if field_name not in self.profile:
            raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "unsupported profile field")
        self.profile[field_name] = float(value)
        self.push_packet(build_profile_packet(
            axis,
            self.profile["velocity"],
            self.profile["acceleration"],
            self.profile["deceleration"],
            self.profile["jerk"],
        ))

    def set_flow_config(self, period=None, batch_size=None, verbose_flag=None):
        """Update the flow configuration and push it to the bridge."""

        cfg = self.snapshot_flow_config()
        if period is not None:
            cfg["period"] = parse_int_value(period, "period")
        if batch_size is not None:
            cfg["batchSize"] = parse_int_value(batch_size, "batchSize")
        if verbose_flag is not None:
            cfg["verboseFlag"] = parse_int_value(verbose_flag, "verboseFlag")
        self.push_packet(build_flow_packet(cfg["period"], cfg["batchSize"], cfg["verboseFlag"]))
        with self.state_lock:
            self.status_cache["flow_config"] = dict(cfg)

    def execute_command_string(self, cmd_text):
        """Execute a minimal single-line command sequence for the web client."""

        if not isinstance(cmd_text, str):
            raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "invalid 'cmd' field")
        steps = []
        current_axis = 0
        for segment in self._split_command_segments(cmd_text):
            parts = segment.split()
            if not parts:
                continue
            cmd = parts[0].strip().lower()
            args = parts[1:]
            current_axis = self._execute_single_command(current_axis, cmd, args)
            steps.append(segment)
        if not steps:
            raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "empty command")
        return steps

    def request_shutdown(self):
        """Request HTTP-server shutdown and stop the background cycle thread."""

        if self.shutdown_requested.is_set():
            return
        self.shutdown_requested.set()
        self.stop_event.set()
        if self.httpd is not None:
            threading.Thread(
                target=self.httpd.shutdown,
                name="axfb-bridge-http-shutdown",
                daemon=True,
            ).start()

    def _cycle_loop(self):
        """Run the bridge PDO cycle and mirror the latest status packets."""

        cycle_period_s = max(0.0005, float(self.args.cycle_period_ms) / 1000.0)
        cycle_idx = 0
        next_tick = time.time()
        try:
            while not self.stop_event.is_set():
                cycle_idx += 1
                with self.bridge_lock:
                    ret = self.bridge.run_cycle(self.inp_buf, self.oup_buf)
                    if ret != 0:
                        raise DemoError("run_cycle failed at cycle=%d ret=%d" % (cycle_idx, ret))
                    drained = drain_status_packets_with_flow(self.bridge)
                self._merge_status_update(cycle_idx, drained)
                next_tick += cycle_period_s
                wait_s = next_tick - time.time()
                if wait_s > 0:
                    self.stop_event.wait(wait_s)
                else:
                    next_tick = time.time()
        except Exception as exc:
            self.runtime_error = exc
            self.request_shutdown()

    def _merge_status_update(self, cycle_idx, drained):
        """Update the shared cache with one drained batch of status packets."""

        with self.state_lock:
            self.status_cache["last_cycle"] = cycle_idx
            if drained["axis_state"] is not None:
                self.status_cache["axis_state"] = drained["axis_state"]
            if drained["axis_state2"] is not None:
                self.status_cache["axis_state2"] = drained["axis_state2"]
            self.status_cache["axis_state_by_axis"].update(drained["axis_state_by_axis"])
            self.status_cache["axis_state2_by_axis"].update(drained["axis_state2_by_axis"])
            if drained["flow_status"] is not None:
                flow_status = drained["flow_status"]
                self.status_cache["flow_status"] = {
                    "period": int(flow_status.period),
                    "batchSize": int(flow_status.batchSize),
                    "verboseFlag": int(flow_status.verboseFlag),
                    "latency": int(flow_status.latency),
                    "axis_num": int(flow_status.axis_num),
                    "cycleTime": int(flow_status.cycleTime),
                }

    def _raise_if_runtime_failed(self):
        """Convert a background-cycle failure into a request-level error."""

        if self.runtime_error is not None:
            raise DemoHttpError(-1, str(self.runtime_error), 500)

    def _make_dashboard_axis_locked(self, axis):
        """Combine AxisState2 and AxisState into one dashboard row."""

        if axis != 0:
            return None
        axis_state2 = self.status_cache["axis_state2_by_axis"].get(axis)
        axis_state = self.status_cache["axis_state_by_axis"].get(axis)
        if axis_state2 is None and axis_state is None:
            return None
        axis_obj = {"axis_id": axis}
        if axis_state2 is not None:
            axis_obj["axis_id"] = int(axis_state2.axis_id)
            axis_obj["axis_state"] = int(axis_state2.axis_state)
            axis_obj["error_id"] = int(axis_state2.error_id)
            axis_obj["tick"] = int(axis_state2.tick)
            axis_obj["set_pos"] = float(axis_state2.set_pos)
            axis_obj["act_pos"] = float(axis_state2.act_pos)
            axis_obj["set_vel"] = float(axis_state2.set_vel)
            axis_obj["set_acc"] = float(axis_state2.set_acc)
            axis_obj["set_position"] = int(axis_state2.set_position)
            axis_obj["act_position"] = int(axis_state2.act_position)
        if axis_state is not None:
            axis_obj["axis_id"] = int(axis_state.axis_id)
            axis_obj["axis_state"] = int(axis_state.axis_state)
            axis_obj["error_id"] = int(axis_state.error_id)
            axis_obj["tick"] = int(axis_state.tick)
            axis_obj["act_pos"] = float(axis_state.act_pos)
            axis_obj["set_pos"] = float(axis_state.set_pos)
        return axis_obj

    @staticmethod
    def _split_command_segments(cmd_text):
        """Split one command line into comma or newline separated segments."""

        normalized = str(cmd_text).replace("\r", "\n").replace(";", "\n").replace(",", "\n")
        return [item.strip() for item in normalized.split("\n") if item.strip()]

    def _execute_single_command(self, current_axis, cmd, args):
        """Execute one parsed command segment and return the selected axis."""

        if cmd in ("base", "axis"):
            if len(args) != 1:
                raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "missing axis value")
            axis = parse_int_value(args[0], "axis")
            if axis != 0:
                raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "only axis 0 is supported")
            return axis
        if cmd == "poweron":
            self.power_on(current_axis)
            return current_axis
        if cmd == "poweroff":
            self.power_off(current_axis)
            return current_axis
        if cmd in ("stop", "halt"):
            if cmd == "stop":
                self.stop(current_axis)
            else:
                self.halt(current_axis)
            return current_axis
        if cmd in ("enable", "axisenable", "axis_enable"):
            if len(args) != 1:
                raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "missing enable value")
            enable = parse_int_value(args[0], "enable")
            self.axis_enable(current_axis, enable != 0)
            return current_axis
        if cmd == "disable":
            self.axis_enable(current_axis, False)
            return current_axis
        if cmd in ("moveabs", "movesgabs"):
            if len(args) != 1:
                raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "missing position value")
            self.move_abs(current_axis, parse_float_value(args[0], "pos"))
            return current_axis
        if cmd in ("moverel", "move", "movesg", "movesgrel"):
            if len(args) != 1:
                raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "missing delta value")
            self.move_rel(current_axis, parse_float_value(args[0], "delta"))
            return current_axis
        if cmd == "speed":
            if len(args) != 1:
                raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "missing speed value")
            self.update_profile(current_axis, "velocity", parse_float_value(args[0], "speed"))
            return current_axis
        if cmd == "accel":
            if len(args) != 1:
                raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "missing accel value")
            self.update_profile(current_axis, "acceleration", parse_float_value(args[0], "accel"))
            return current_axis
        if cmd == "decel":
            if len(args) != 1:
                raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "missing decel value")
            self.update_profile(current_axis, "deceleration", parse_float_value(args[0], "decel"))
            return current_axis
        if cmd in ("jerk", "sparam"):
            if len(args) != 1:
                raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "missing jerk value")
            self.update_profile(current_axis, "jerk", parse_float_value(args[0], "jerk"))
            return current_axis
        raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "unsupported command: %s" % cmd)

    @staticmethod
    def _require_zero(step_name, ret):
        """Raise a DemoError when the bridge initialization fails."""

        if ret != 0:
            raise DemoError("%s failed with ret=%d" % (step_name, ret))


class DemoHttpHandler(BaseHTTPRequestHandler):
    """Standard-library HTTP handler bound to one BridgeHttpRuntime."""

    runtime = None

    def log_message(self, fmt, *args):
        """Write one compact access log line to stdout."""

        sys.stdout.write("[http] %s - %s\n" % (self.address_string(), fmt % args))

    def do_GET(self):
        """Dispatch all supported GET routes."""

        try:
            path = urlsplit(self.path)
            if path.path == "/" or path.path == "/index.html":
                self._handle_static_file("index.html")
                return
            if path.path.startswith("/css/") or path.path.startswith("/js/"):
                self._handle_static_file(path.path.lstrip("/"))
                return
            if path.path == "/zmot/api/axis_state2":
                self._handle_axis_state2(parse_qs(path.query, keep_blank_values=True))
                return
            if path.path == "/zmot/api/flow_status":
                self._handle_flow_status()
                return
            if path.path == "/zmot/api/flow_config":
                self._handle_flow_config_get()
                return
            if path.path == "/zmot/api/dashboard":
                self._handle_dashboard(parse_qs(path.query, keep_blank_values=True))
                return
            reply_error(self, 404, "not found", 404, {"path": path.path})
        except DemoHttpError as exc:
            reply_error(self, exc.code, exc.message, exc.http_status)
        except Exception as exc:
            reply_error(self, -1, str(exc), 500)

    def do_POST(self):
        """Dispatch all supported POST routes."""

        try:
            path = urlsplit(self.path)
            if path.path == "/zmot/api/poweron":
                self._handle_poweron()
                return
            if path.path == "/zmot/api/poweroff":
                self._handle_poweroff()
                return
            if path.path == "/zmot/api/axisenable":
                self._handle_axisenable()
                return
            if path.path == "/zmot/api/flow_config":
                self._handle_flow_config_post()
                return
            if path.path == "/zmot/api/movesg":
                self._handle_movesg()
                return
            if path.path == "/zmot/api/movesgrel":
                self._handle_movesg()
                return
            if path.path == "/zmot/api/movesgabs":
                self._handle_movesgabs()
                return
            if path.path == "/zmot/api/execute_command":
                self._handle_execute_command()
                return
            if path.path == "/zmot/api/stop":
                self._handle_stop()
                return
            if path.path == "/zmot/api/halt":
                self._handle_halt()
                return
            if path.path == "/zmot/api/server_exit":
                self._handle_server_exit()
                return
            reply_error(self, 404, "not found", 404, {"path": path.path})
        except DemoHttpError as exc:
            reply_error(self, exc.code, exc.message, exc.http_status)
        except Exception as exc:
            reply_error(self, -1, str(exc), 500)

    def _handle_axis_state2(self, query_map):
        """Serve the latest AxisState2 snapshot for axis 0."""

        axis = 0
        if "axis" in query_map and query_map["axis"]:
            try:
                axis = int(query_map["axis"][0])
            except Exception:
                raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "invalid 'axis' field")
        if axis != 0:
            raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "only axis 0 is supported")
        axis_state2 = self.runtime.snapshot_axis_state2(axis)
        reply_ok(self, {"axis": axis_state2})

    def _handle_static_file(self, rel_path):
        """Serve one static file from the configured web root."""

        safe_rel_path = rel_path.replace("/", os.sep)
        full_path = os.path.abspath(os.path.join(self.runtime.web_root, safe_rel_path))
        if not full_path.startswith(self.runtime.web_root + os.sep) and full_path != self.runtime.web_root:
            reply_error(self, 404, "not found", 404, {"path": rel_path})
            return
        if not os.path.isfile(full_path):
            reply_error(self, 404, "not found", 404, {"path": rel_path})
            return
        with open(full_path, "rb") as fp:
            payload = fp.read()
        self.send_response(200)
        self.send_header("Content-Type", guess_content_type(full_path))
        self.send_header("Content-Length", str(len(payload)))
        self.end_headers()
        self.wfile.write(payload)

    def _handle_flow_status(self):
        """Serve the latest flow-status snapshot."""

        reply_ok(self, self.runtime.snapshot_flow_status())

    def _handle_flow_config_get(self):
        """Serve the latest requested flow configuration."""

        reply_ok(self, self.runtime.snapshot_flow_config())

    def _handle_dashboard(self, query_map):
        """Serve the aggregated dashboard payload used by the web client."""

        axes = parse_axis_query(query_map)
        include_flow = parse_query_bool(query_map, "include_flow", True)
        include_cmd_results = parse_query_bool(query_map, "include_cmd_results", True)
        reply_ok(self, self.runtime.snapshot_dashboard(axes, include_flow, include_cmd_results))

    def _handle_poweron(self):
        """Handle the standard power-on route."""

        body = read_json_body(self, allow_empty=True)
        axis = get_axis_or_default_zero(body)
        self.runtime.power_on(axis)
        reply_ok(self)

    def _handle_poweroff(self):
        """Handle the standard power-off route."""

        body = read_json_body(self, allow_empty=True)
        axis = get_axis_or_default_zero(body)
        self.runtime.power_off(axis)
        reply_ok(self)

    def _handle_axisenable(self):
        """Handle the single-axis enable route."""

        body = read_json_body(self, allow_empty=False)
        axis = get_axis_or_default_zero(body)
        if "enable" not in body:
            raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "missing 'enable' field")
        try:
            enable = int(body["enable"])
        except Exception:
            raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "invalid 'enable' field")
        self.runtime.axis_enable(axis, enable != 0)
        reply_ok(self)

    def _handle_flow_config_post(self):
        """Handle one flow-configuration update request."""

        body = read_json_body(self, allow_empty=False)
        period = body.get("period")
        batch_size = body.get("batchSize")
        verbose_flag = body.get("verboseFlag")
        self.runtime.set_flow_config(period, batch_size, verbose_flag)
        reply_ok(self)

    def _handle_movesg(self):
        """Handle the single-axis relative move routes."""

        body = read_json_body(self, allow_empty=False)
        axis = get_axis_or_default_zero(body)
        delta = get_single_pos(body, ("pos", "delta", "value"))
        self.runtime.move_rel(axis, delta)
        reply_ok(self)

    def _handle_movesgabs(self):
        """Handle the single-axis absolute move route."""

        body = read_json_body(self, allow_empty=False)
        axis = get_axis_or_default_zero(body)
        pos = get_single_pos(body, ("pos",))
        self.runtime.move_abs(axis, pos)
        reply_ok(self)

    def _handle_execute_command(self):
        """Handle one simple single-line command sequence."""

        body = read_json_body(self, allow_empty=False)
        if "cmd" not in body:
            raise DemoHttpError(ZMC_ERR_MOTCMD_INVALID_ARG, "missing 'cmd' field")
        steps = self.runtime.execute_command_string(body["cmd"])
        reply_ok(self, {"result": "ok", "steps": steps})

    def _handle_stop(self):
        """Handle the single-axis stop route."""

        body = read_json_body(self, allow_empty=True)
        axis = get_axis_or_default_zero(body)
        self.runtime.stop(axis)
        reply_ok(self)

    def _handle_halt(self):
        """Handle the single-axis halt route."""

        body = read_json_body(self, allow_empty=True)
        axis = get_axis_or_default_zero(body)
        self.runtime.halt(axis)
        reply_ok(self)

    def _handle_server_exit(self):
        """Handle the asynchronous server-exit route."""

        read_json_body(self, allow_empty=True)
        reply_ok(self)
        self.runtime.request_shutdown()


def parse_args(argv=None):
    """Parse the CLI arguments for the HTTP demo."""

    parser = argparse.ArgumentParser(description="Minimal AxFbBridge single-axis HTTP demo")
    parser.add_argument("--ini_path", default=default_ini_path())
    parser.add_argument("--web_root", default=default_web_root())
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8080)
    parser.add_argument("--velocity", type=float, default=20.0)
    parser.add_argument("--acceleration", type=float, default=100.0)
    parser.add_argument("--deceleration", type=float, default=100.0)
    parser.add_argument("--jerk", type=float, default=0.0)
    parser.add_argument("--power_block_ticks", type=int, default=100)
    parser.add_argument("--cycle_period_ms", type=float, default=1.0)
    return parser.parse_args(argv)


def make_http_server(host, port, runtime):
    """Create one HTTP server bound to the supplied runtime."""

    handler_cls = type("BoundDemoHttpHandler", (DemoHttpHandler,), {})
    handler_cls.runtime = runtime
    httpd = ThreadingHTTPServer((host, int(port)), handler_cls)
    httpd.daemon_threads = True
    return httpd


def main(argv=None):
    """Run the Python AxFbBridge HTTP demo."""

    args = parse_args(argv)
    runtime = BridgeHttpRuntime(args)
    httpd = None
    try:
        runtime.start()
        httpd = make_http_server(args.host, args.port, runtime)
        runtime.set_httpd(httpd)
        print("Listening on http://%s:%d" % (args.host, args.port))
        httpd.serve_forever(poll_interval=0.2)
    except KeyboardInterrupt:
        runtime.request_shutdown()
    except DemoError as exc:
        print("DemoError: %s" % exc)
        return 1
    except Exception as exc:
        print("Error: %s" % exc)
        return 1
    finally:
        if httpd is not None:
            httpd.server_close()
        runtime.close()
    if runtime.runtime_error is not None:
        print("RuntimeError: %s" % runtime.runtime_error)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
