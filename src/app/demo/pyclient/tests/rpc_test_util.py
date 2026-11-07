"""Live RPC fixtures. Date: 2026-10-22. Drafted by: GPT-6 (Codex)."""

import configparser
import contextlib
import os
from pathlib import Path
import socket
import subprocess
import tempfile
import time
import unittest

import zmc_rpc

_server = None
_work = None
_log = None


def live_endpoint():
    port = os.environ.get("ZMC_RPC_PORT")
    if not port:
        raise unittest.SkipTest("ZMC_RPC_PORT is not set")
    return os.environ.get("ZMC_RPC_HOST", "127.0.0.1"), int(port)


def start_live_server():
    global _server, _work, _log
    exe = os.environ.get("ZMC_RPC_SERVER_EXE")
    if not exe:
        return
    host, port = live_endpoint()
    # Refuse to attach these tests to an unrelated listener on the requested port.
    with socket.socket() as probe:
        probe.bind((host, port))
    root = Path(os.environ["ZMC_RPC_TEST_WORK_DIR"])
    root.mkdir(parents=True, exist_ok=True)
    _work = tempfile.TemporaryDirectory(prefix="run-", dir=root)
    data = Path(os.environ["ZMC_RPC_TEST_DATA_DIR"]).resolve()
    config = configparser.ConfigParser(interpolation=None)
    config.read_string((data / "zmc_motapp_demo_fak3.ini").read_text().replace(
        "{{dirname}}", data.as_posix()))
    config.set("base", "count_to_exit", "0")
    for section in config.sections():
        if section.startswith("axis_") and section[5:].isdigit():
            config.set(section, "is_axis_virtual", "1")
            config.set(section, "debug.count_to_exit", "0")
    config["io_binding"] = {
        "di_10.source": "virtual", "do_11.source": "virtual",
        "ai_12.source": "virtual", "ao_13.source": "virtual",
    }
    ini = Path(_work.name) / "rpc.ini"
    # RPC download replaces an existing server entry; creation belongs to the fixture.
    files = Path(_work.name) / "server_files"
    files.mkdir()
    (files / "python_contract.bin").write_bytes(b"fixture")
    with ini.open("w", encoding="utf-8") as stream:
        stream.write("; Date: 2026-10-22. Drafted by: GPT-6 (Codex).\n")
        config.write(stream)
    _log = tempfile.NamedTemporaryFile(prefix="server-", suffix=".log", dir=root, delete=False)
    try:
        options = {"creationflags": subprocess.CREATE_NO_WINDOW} if os.name == "nt" else {}
        _server = subprocess.Popen(
            [exe, "axis_ini", "--rpc", "--port=%d" % port, "--ini", str(ini)],
            cwd=_work.name, stdin=subprocess.DEVNULL, stdout=_log, stderr=_log, **options)
        deadline = time.monotonic() + 10
        while time.monotonic() < deadline:
            if _server.poll() is not None:
                break
            try:
                socket.create_connection((host, port), timeout=0.2).close()
                return
            except OSError:
                time.sleep(0.02)
        raise RuntimeError("RPC server did not start; log: %s" % _log.name)
    except BaseException:
        stop_live_server()
        raise


def stop_live_server():
    global _server, _work, _log
    # Own and reap only the process created above; never kill a PID read from disk.
    try:
        if _server is not None:
            if _server.poll() is None:
                _server.terminate()
            try:
                _server.wait(timeout=5)
            except subprocess.TimeoutExpired:
                _server.kill()
                _server.wait(timeout=5)
    finally:
        _server = None
        if _log is not None:
            _log.close()  # Preserve the log outside the disposable runtime directory.
            _log = None
        if _work is not None:
            _work.cleanup()
            _work = None


def until(predicate, timeout=10):
    """Observe a condition without changing the RPC wait-command semantics."""
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        value = predicate()
        if value:
            return value
        time.sleep(0.01)
    raise AssertionError("RPC observation timed out")


@contextlib.contextmanager
def rpc_handle(device):
    ret, handle = zmc_rpc.zml_open(device)
    if ret != 0:
        raise AssertionError("zml_open failed: %d" % ret)
    try:
        yield handle
    finally:
        ret = zmc_rpc.zml_close(handle)
        if ret != 0:
            raise AssertionError("zml_close failed: %d" % ret)
