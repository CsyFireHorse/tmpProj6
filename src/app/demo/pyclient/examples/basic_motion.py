"""Explicit RPC handle example. Date: 2026-10-22. Drafted by: GPT-6 (Codex)."""

import argparse
import sys
import time

import zmc_rpc as rpc


def observe(handle, axis, position=None):
    """Bounded example observation; RPC wait commands remain unchanged."""
    deadline = time.monotonic() + 10
    while time.monotonic() < deadline:
        ret, state = rpc.zml_ax_get_state(handle, axis)
        if ret == 0 and state["error_id"] == 0 and state["axis_state"] & 15 == 1:
            if position is None or (abs(state["act_pos"] - position) < 0.01 and
                                    abs(state["set_pos"] - position) < 0.01):
                print("zml_ax_get_state", ret, state)
                return 0
        time.sleep(0.01)
    print("axis observation timed out:", ret, state, file=sys.stderr)
    return 1


def run(handle, args):
    for fn, value in ((rpc.zml_ax_set_velocity, 100.0), (rpc.zml_ax_set_accel, 1000.0),
                      (rpc.zml_ax_set_decel, 1000.0)):
        ret = fn(handle, args.axis, value)
        print(fn.__name__, ret)
        if ret != 0:
            return ret
    ret = rpc.zml_ax_power_on(handle, args.axis)
    print("zml_ax_power_on", ret)
    if ret != 0:
        return ret
    ret = observe(handle, args.axis)
    if ret != 0:
        return ret
    ret = rpc.zml_ax_move_abs(handle, args.axis, args.pos)
    print("zml_ax_move_abs", ret)
    if ret != 0:
        return ret
    ret = rpc.zml_ax_wait_idle(handle, args.axis, args.timeout)
    print("zml_ax_wait_idle", ret)
    return ret if ret != 0 else observe(handle, args.axis, args.pos)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--device", required=True)
    parser.add_argument("--axis", required=True, type=int)
    parser.add_argument("--pos", default=10.0, type=float)
    parser.add_argument("--timeout", default=30000, type=int)
    args = parser.parse_args()
    ret, handle = rpc.zml_open(args.device)
    print("zml_open", ret)
    if ret != 0:
        return ret
    try:
        ret = run(handle, args)
    finally:
        close_ret = rpc.zml_close(handle)
        print("zml_close", close_ret)
    return ret or close_ret


if __name__ == "__main__":
    sys.exit(main())
