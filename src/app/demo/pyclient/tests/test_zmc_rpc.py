"""RPC binding contracts and live success paths.

Date: 2026-11-05. Drafted by: GPT-6 (Codex).
"""

import ast
import contextlib
from pathlib import Path
import re
import unittest

import zmc_rpc as rpc

from tests.rpc_test_util import (
    live_endpoint, rpc_handle, start_live_server, stop_live_server, until,
)

ROOT = Path(__file__).resolve().parents[1]
INCLUDE = ROOT.parent / "rpc_client" / "include"


def declarations():
    """Read actual public declarations, excluding commented-out APIs."""
    result = {}
    for file in ("zml_motion_api.h", "zml_motion_api_ex.h"):
        text = re.sub(r"/\*[\s\S]*?\*/|//[^\r\n]*", "", (INCLUDE / file).read_text())
        for name, params in re.findall(
                r"ZML_API\s+ZML_RESULT\s+(zml_\w+)\s*\(([^;]+)\);", text):
            args = []
            for param in params.split(","):
                match = re.fullmatch(r"(.+?)\s*(\w+)\s*", param.strip())
                args.append((" ".join(match[1].split()), match[2]))
            result[name] = args
    return result


def setUpModule():
    start_live_server()


def tearDownModule():
    stop_live_server()


class TestContract(unittest.TestCase):
    def test_all_public_names_and_signature_shapes(self):
        declared = declarations()
        stubs = {f.name: f for f in ast.parse((ROOT / "zmc_rpc.pyi").read_text()).body
                 if isinstance(f, ast.FunctionDef)}
        self.assertEqual(set(declared), set(stubs))
        self.assertEqual(set(declared), {n for n in dir(rpc) if n.startswith("zml_")})
        self.assertFalse(hasattr(rpc, "RpcClient"))
        for name, params in declared.items():
            with self.subTest(api=name):
                expected = []
                output_count = 0
                for typ, field in params:
                    inout = ((name == "zml_get_cmd_results" and field == "inout_len") or
                             (name == "zml_ecat_coe_sdo_upload" and field == "data_len"))
                    output = "*" in typ and not typ.startswith("const ")
                    if name == "zml_close":
                        output = False
                    if not output or inout:
                        expected.append(field)
                    output_count += int(output)
                stub = stubs[name]
                self.assertEqual(expected, [a.arg for a in stub.args.posonlyargs])
                self.assertFalse(stub.args.args or stub.args.defaults or stub.args.kwonlyargs)
                annotation = stub.returns
                if output_count:
                    self.assertEqual("Tuple", annotation.value.id)
                    self.assertEqual(1 + output_count, len(annotation.slice.elts))
                else:
                    self.assertEqual("int", annotation.id)
                self.assertTrue(callable(getattr(rpc, name)))

    def test_public_structure_fields(self):
        classes = {c.name: c for c in ast.parse((ROOT / "zmc_rpc.pyi").read_text()).body
                   if isinstance(c, ast.ClassDef)}
        for file in ("zml_motion_types.h", "zml_types_def.h"):
            text = (INCLUDE / file).read_text()
            for name, body in re.findall(r"typedef struct (ZML_\w+)\s*\{(.*?)\}", text, re.S):
                if name == "ZML_SetupStatus":
                    continue  # No public RPC function uses this structure.
                fields = re.findall(r"\b\w+\s+(\w+)(?:\[\d+\])?\s*;", body)
                self.assertEqual(fields, [f.target.id for f in classes[name].body
                                          if isinstance(f, ast.AnnAssign)])

class TestLive(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        host, port = live_endpoint()
        cls.device = "%s:%d" % (host, port)

    def ok(self, ret):
        self.assertEqual(0, ret, "RPC returned %d" % ret)

    @contextlib.contextmanager
    def client(self):
        with rpc_handle(self.device) as handle:
            cfg = {"period": 10, "batch_size": 0, "verbose_flag": 1, "reserved": [0, 0, 0]}
            self.ok(rpc.zml_set_flow_config(handle, cfg))
            yield handle

    def state(self, handle, axis):
        ret, value = rpc.zml_ax_get_state_ex(handle, axis)
        return value if ret == 0 else None

    def reached(self, handle, axis, position):
        def observed():
            state = self.state(handle, axis)
            return state if (state and state["error_id"] == 0 and
                             state["axis_state"] & 15 == 1 and
                             abs(state["set_pos"] - position) < 0.01 and
                             abs(state["act_pos"] - position) < 0.01) else None
        return until(observed)

    def configure(self, handle, axes):
        for axis in axes:
            # Units carry the explicit num/den ratio: 1/1 = 1.0. GLM-5.3 (opencode).
            self.ok(rpc.zml_ax_set_units(handle, axis, 1, 1))
            until(lambda: rpc.zml_ax_get_units(handle, axis) == (0, 1.0))
            for fn, value in ((rpc.zml_ax_set_velocity, 100.0), (rpc.zml_ax_set_accel, 1000.0),
                              (rpc.zml_ax_set_decel, 1000.0)):
                self.ok(fn(handle, axis, value))
            self.ok(rpc.zml_ax_power_on(handle, axis))
            until(lambda: (self.state(handle, axis) or {}).get("axis_state") == 1)

    def test_lifecycle_and_flow_conversion(self):
        with self.client() as handle:
            self.assertIsInstance(handle, rpc.ZML_HANDLE)
            self.assertEqual(1, rpc.zml_get_connection_state(handle))
            self.ok(rpc.zml_wait_until_connection_ready(handle, 10))
            self.ok(rpc.zml_ping(handle, 3000))
            # Units set the explicit num/den ratio: 2/1 = 2.0. GLM-5.3 (opencode).
            self.ok(rpc.zml_ax_set_units(handle, 0, 2, 1))
            until(lambda: rpc.zml_ax_get_units(handle, 0) == (0, 2.0))
            ret, config = rpc.zml_get_flow_config(handle)
            self.ok(ret)
            self.assertEqual({"period", "batch_size", "verbose_flag", "reserved"}, set(config))
            # The last two bytes contain the processed command sequence.
            self.assertEqual(3, len(config["reserved"]))
            self.assertEqual(0, config["reserved"][0])
            ret, flow = rpc.zml_get_flow_status(handle)
            self.ok(ret)
            self.assertEqual(set(config) | {"latency", "axis_num", "cycle_time"}, set(flow))
            self.assertEqual(3, flow["axis_num"])
        self.assertEqual(0, rpc.zml_get_connection_state(handle))

    def test_explicit_axis_order_and_result_conversion(self):
        with self.client() as handle:
            self.configure(handle, [0, 2])
            # Coordinated motion requires the existing group-enable command first.
            self.ok(rpc.zml_group_select(handle, 0))
            self.ok(rpc.zml_group_enable(handle, 2, [2, 0]))
            self.ok(rpc.zml_ax_wait_ticks(handle, 10, 0))
            self.ok(rpc.zml_moveabs(handle, 2, [2, 0], [2.0, 4.0]))
            self.ok(rpc.zml_ax_wait_idle(handle, 0, 10000))
            self.reached(handle, 2, 2.0)
            extended = self.reached(handle, 0, 4.0)
            ret, basic = rpc.zml_ax_get_state(handle, 0)
            self.ok(ret)
            self.assertEqual({"axis_id", "axis_state", "error_id", "tick", "act_pos", "set_pos"},
                             set(basic))
            self.assertEqual(set(basic) | {"set_vel", "set_acc", "set_position", "act_position"},
                             set(extended))
            self.ok(rpc.zml_group_disable(handle, 2, [2, 0]))
            self.ok(rpc.zml_ax_wait_ticks(handle, 10, 0))

            # Group submissions report acceptance; a single-axis move exposes its terminal result.
            self.ok(rpc.zml_ax_move_abs(handle, 0, 5.0))
            self.reached(handle, 0, 5.0)

            def terminal():
                ret, results, _ = rpc.zml_get_cmd_results(handle, 100)
                if ret != 0:
                    return None
                return next((r for r in results if r["command_type"] == 0x43 and
                             r["target_kind"] == 1 and r["target_index"] == 0 and
                             r["status"] == 4 and r["error_code"] == 0), None)
            result = until(terminal)
            fields = {"id", "command_type", "target_index", "target_kind", "status",
                      "io_count", "error_code", "tick", "data", "data_ext"}
            self.assertEqual(fields, set(result))
            self.assertTrue(all(isinstance(value, int) for value in result.values()))
            ret, missing, needed = rpc.zml_get_cmd_results(handle, 1)
            self.assertNotEqual(0, ret)
            self.assertIsNone(missing)
            self.assertGreater(needed, 1)
            ret, reread, count = rpc.zml_get_cmd_results(handle, needed)
            self.ok(ret)
            self.assertEqual(count, len(reread))
            self.assertIn(result, reread)

    def test_virtual_io_conversion(self):
        with self.client() as handle:
            self.assertEqual((0, False), rpc.zml_read_di(handle, 10))
            self.assertEqual((0, 0), rpc.zml_read_ai(handle, 12))
            self.ok(rpc.zml_write_virtual_di(handle, 10, True))
            until(lambda: rpc.zml_read_di(handle, 10) == (0, True))
            self.ok(rpc.zml_write_virtual_ai(handle, 12, 0xFEDCBA98))
            until(lambda: rpc.zml_read_ai(handle, 12) == (0, 0xFEDCBA98))


if __name__ == "__main__":
    unittest.main()
