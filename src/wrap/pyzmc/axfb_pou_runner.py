"""
AxPouTaskBinderAuto Python Wrapper
==================================

This module provides Python bindings for AxPouTaskBinderAuto C API.
It wraps the C API functions and provides a Python-friendly interface.

Usage:
    from axfb_pou_runner import AxPouTaskBinderAuto, PouTaskBase
    from pymc import FbPower, FbMoveAbsolute
    
    class MyTask(PouTaskBase):
        def __init__(self):
            self.axis = AxPouTaskBinderAuto.getAxisByIndex(0)
            self.power = FbPower()
            self.power.Axis = self.axis
        
        def runCycle(self):
            self.power.Enable = True
            self.power.runCycle()
    
    runner = AxPouTaskBinderAuto(iniPath="", iniContent="...")
    task = MyTask()
    runner.bind(0, task)
    runner.run()
"""

from ctypes import *
from abc import ABC, abstractmethod
from collections import defaultdict
import os
import sys
import threading
import queue

# Import from pymc
try:
    from pymc import (
        zmc_dll, AXIS_REF, FbPower, FbMoveAbsolute,
        Direction, BufferMode
    )
except ImportError:
    # If pymc is not available, define minimal types
    print("Warning: pymc module not found. Some features may not work.")
    zmc_dll = None
    AXIS_REF = c_void_p

# Define handle type
axfb_pou_runner_handle_t = c_void_p

# Bind C API functions if DLL is available
if zmc_dll:
    # Create and destroy
    axfb_pou_runner_create = zmc_dll.axfb_pou_runner_create
    axfb_pou_runner_create.argtypes = [c_char_p, c_char_p]
    axfb_pou_runner_create.restype = axfb_pou_runner_handle_t
    
    axfb_pou_runner_destroy = zmc_dll.axfb_pou_runner_destroy
    axfb_pou_runner_destroy.argtypes = [axfb_pou_runner_handle_t]
    axfb_pou_runner_destroy.restype = None
    
    # Configuration setters
    axfb_pou_runner_set_cycles = zmc_dll.axfb_pou_runner_set_cycles
    axfb_pou_runner_set_cycles.argtypes = [axfb_pou_runner_handle_t, c_int]
    axfb_pou_runner_set_cycles.restype = None
    
    axfb_pou_runner_set_verbose_flag = zmc_dll.axfb_pou_runner_set_verbose_flag
    axfb_pou_runner_set_verbose_flag.argtypes = [axfb_pou_runner_handle_t, c_int]
    axfb_pou_runner_set_verbose_flag.restype = None
    
    axfb_pou_runner_set_cycle_time = zmc_dll.axfb_pou_runner_set_cycle_time
    axfb_pou_runner_set_cycle_time.argtypes = [axfb_pou_runner_handle_t, c_int]
    axfb_pou_runner_set_cycle_time.restype = None
    
    # Task binding (not used in Python, but kept for completeness)
    axfb_pou_runner_bind_task = zmc_dll.axfb_pou_runner_bind_task
    axfb_pou_runner_bind_task.argtypes = [axfb_pou_runner_handle_t, c_int, c_void_p]
    axfb_pou_runner_bind_task.restype = None
    
    # Run control
    axfb_pou_runner_run = zmc_dll.axfb_pou_runner_run
    axfb_pou_runner_run.argtypes = [axfb_pou_runner_handle_t]
    axfb_pou_runner_run.restype = None
    
    axfb_pou_runner_run_cycle = zmc_dll.axfb_pou_runner_run_cycle
    axfb_pou_runner_run_cycle.argtypes = [axfb_pou_runner_handle_t]
    axfb_pou_runner_run_cycle.restype = None

    axfb_pou_runner_pre_run = zmc_dll.axfb_pou_runner_pre_run
    axfb_pou_runner_pre_run.argtypes = [axfb_pou_runner_handle_t]
    axfb_pou_runner_pre_run.restype = None

    axfb_pou_runner_post_run_cycle = zmc_dll.axfb_pou_runner_post_run_cycle
    axfb_pou_runner_post_run_cycle.argtypes = [axfb_pou_runner_handle_t]
    axfb_pou_runner_post_run_cycle.restype = None
    
    # Static methods for axis access
    axfb_pou_runner_get_axis_by_index = zmc_dll.axfb_pou_runner_get_axis_by_index
    axfb_pou_runner_get_axis_by_index.argtypes = [c_uint32]
    axfb_pou_runner_get_axis_by_index.restype = c_void_p
    
    axfb_pou_runner_get_axis_count = zmc_dll.axfb_pou_runner_get_axis_count
    axfb_pou_runner_get_axis_count.argtypes = []
    axfb_pou_runner_get_axis_count.restype = c_int

    axfb_pou_runner_get_verbose_flag = zmc_dll.axfb_pou_runner_get_verbose_flag
    axfb_pou_runner_get_verbose_flag.argtypes = [axfb_pou_runner_handle_t]
    axfb_pou_runner_get_verbose_flag.restype = c_int

    axfb_pou_runner_get_cycle_time = zmc_dll.axfb_pou_runner_get_cycle_time
    axfb_pou_runner_get_cycle_time.argtypes = [axfb_pou_runner_handle_t]
    axfb_pou_runner_get_cycle_time.restype = c_int

class PouTaskBase(ABC):
    """
    Base class for POU tasks.
    Subclasses must implement runCycle() method.
    """
    
    @abstractmethod
    def runCycle(self):
        """
        Execute one cycle of the task.
        This method must be implemented by subclasses.
        """
        pass
    
    def postRun(self):
        """
        Called after the runner finishes.
        Override this method if needed.
        """
        pass


class AxPouTaskBinderAuto:
    """
    Python wrapper for AxPouTaskBinderAuto C API.
    Provides a Python-friendly interface for creating and running POU tasks.
    """
    
    def __init__(self, iniPath="", iniContent=""):
        """
        Create a new AxPouTaskBinderAuto instance.
        
        Args:
            iniPath: Path to INI configuration file (can be empty)
            iniContent: INI configuration content as string
        """
        if not zmc_dll:
            raise RuntimeError("DLL not loaded. Cannot create AxPouTaskBinderAuto.")
        
        # Convert strings to bytes
        ini_path_bytes = iniPath.encode('utf-8') if iniPath else b""
        ini_content_bytes = iniContent.encode('utf-8') if iniContent else b""
        
        self._handle = axfb_pou_runner_create(ini_path_bytes, ini_content_bytes)
        if not self._handle:
            raise RuntimeError("Failed to create AxPouTaskBinderAuto instance")
        
        # Store bound tasks in Python (since C bind is not used)
        self._tasks = []
        self._count_to_exit = 0
        self._idx = 0
        self._exit_flag = False
    
    def __del__(self):
        """Destroy the runner instance."""
        if hasattr(self, '_handle') and self._handle and zmc_dll:
            axfb_pou_runner_destroy(self._handle)
            self._handle = None
    
    def get_verbose_flag(self):
        """
        Get the verbose flag for printing.
        
        Returns:
            int: Verbose flag value
        """
        if zmc_dll and self._handle:
            return axfb_pou_runner_get_verbose_flag(self._handle)
        return 0
    
    def get_cycle_time(self):
        """
        Get the cycle time in microseconds.
        
        Returns:
            int: Cycle time in microseconds
        """
        if zmc_dll and self._handle:
            return axfb_pou_runner_get_cycle_time(self._handle)
        return 0
    
    def set_cycles(self, cycles):
        """
        Set the number of cycles to run before exiting.
        
        Args:
            cycles: Number of cycles (0 means run indefinitely)
        """
        if zmc_dll and self._handle:
            axfb_pou_runner_set_cycles(self._handle, cycles)
        self._count_to_exit = cycles
    
    def set_verbose_flag(self, flag):
        """
        Set the verbose flag for printing.
        
        Args:
            flag: Verbose flag value
        """
        if zmc_dll and self._handle:
            axfb_pou_runner_set_verbose_flag(self._handle, flag)
    
    def set_cycle_time(self, us):
        """
        Set the cycle time in microseconds.
        
        Args:
            us: Cycle time in microseconds
        """
        if zmc_dll and self._handle:
            axfb_pou_runner_set_cycle_time(self._handle, us)
    
    def bind(self, index, task):
        """
        Bind a task to the runner.
        
        Args:
            index: Task index (currently not used, can be 0)
            task: PouTaskBase instance to bind
        """
        if not isinstance(task, PouTaskBase):
            raise TypeError("Task must be an instance of PouTaskBase")
        
        # Store task in Python list
        # Extend list if needed
        while len(self._tasks) <= index:
            self._tasks.append(None)
        self._tasks[index] = task

    def pre_run(self):
        """Call AxPou::preRun via C API."""
        if zmc_dll and self._handle:
            axfb_pou_runner_pre_run(self._handle)

    def pre_run_cycle(self):
        pass 
    
    def post_run_cycle(self):
        """Call AxPou::postRunCycle via C API."""
        if zmc_dll and self._handle:
            axfb_pou_runner_post_run_cycle(self._handle)

    def runCycle(self):
        for task in self._tasks:
            if task is not None:
                task.runCycle()
        if zmc_dll and self._handle:
            axfb_pou_runner_run_cycle(self._handle)
            
    
    def run_once(self):
        """
        Python equivalent of RunnerBase::run_once.
        """
        if self._exit_flag:
            return
        if self._idx > 0:
            self.pre_run_cycle()
            self.runCycle()
        self.post_run_cycle()
        
        # Update cycle counter and check exit condition
        self._idx += 1
        if self._count_to_exit > 0 and self._idx > self._count_to_exit:
            self._exit_flag = True

    def post_run(self):
        # Post-run: call postRun() on all tasks
        for task in self._tasks:
            if task is not None:
                task.postRun()

    def run(self, count=0):
        """
        Run the runner until exit condition is met.
        
        Args:
            count: Optional cycle count (if > 0, sets count_to_exit)
        """
        if count > 0:
            self.set_cycles(count)
        
        self._idx = 0
        self._exit_flag = False
        self.pre_run()

        while not self._exit_flag:
            self.run_once()
        self.post_run()

    @staticmethod
    def getAxisByIndex(index):
        """
        Get axis reference by index (static method).
        
        Args:
            index: Axis index
            
        Returns:
            AXIS_REF: Axis reference pointer
        """
        if not zmc_dll:
            return None
        axis_ptr = axfb_pou_runner_get_axis_by_index(index)
        return cast(axis_ptr, AXIS_REF) if axis_ptr else None
    
    @staticmethod
    def getAxisCount():
        """
        Get the number of registered axes (static method).
        
        Returns:
            int: Number of axes
        """
        if not zmc_dll:
            return 0
        return axfb_pou_runner_get_axis_count()


class InteractivePouRunner(AxPouTaskBinderAuto):
    """
    AxPouTaskBinderAuto enhancement that accepts runtime commands from stdin.
    """

    def __init__(self, iniPath="", iniContent=""):
        super().__init__(iniPath, iniContent)
        self.current_task_index = 0
        self.pending_commands = defaultdict(list)
        self._command_queue = queue.Queue()
        self._input_thread = None
        self._stop_input = threading.Event()
        self.paused = False
        # self.autorun = True
        self.tick_target = None
        self.tick_action = None
        self.saved_verbose_flag = None
        self.interval_ms = 100
        self.skip_sleep = False
        
    def _start_input_thread(self):
        if self._input_thread is not None:
            return
        self._input_thread = threading.Thread(target=self._input_worker, daemon=True)
        self._input_thread.start()

    def _input_worker(self):
        while not self._stop_input.is_set():
            try:
                line = input().strip()
            except EOFError:
                break
            if not line:
                continue
            self._command_queue.put(line)

    def _drain_command_queue(self):
        while not self._command_queue.empty():
            line = self._command_queue.get()
            self._handle_command(line)

    def _handle_command(self, line: str):
        lower = line.lower()
        if lower in {"quit", "exit"}:
            print("Exiting runner...")
            self._exit_flag = True
            return
        if lower in {"help", "?"}:
            self._print_help()
            return
        if lower.startswith("task"):
            self._handle_task_switch(line)
            return
        if lower == "pause":
            self.paused = True
            # self.autorun = False
            print("Paused")
            return
        if lower == "resume":
            self.paused = False
            # self.autorun = True
            print("Resumed autorun")
            return
        # if lower == "autorun":
        #     self.paused = False
        #     self.autorun = True
        #     self.tick_target = None
        #     self.tick_action = None
        #     print("Autorun enabled")
        #     return
        if lower == "s" or lower == "step":
            self._handle_step(line)
            return
        if lower.startswith("steps"):
            self._handle_steps(line)
            return
        if lower.startswith("tick"):
            self._handle_tick(line)
            return
        if lower.startswith("delay"):
            self._handle_delay(line)
            return
        if lower.startswith("time"):
            self._handle_time_to(line)
            return
        if lower.startswith("jtime"):
            self._handle_jtime(line)
            return
        if lower.startswith("jsteps"):
            self._handle_jsteps(line)
            return
        if lower.startswith("jtick"):
            self._handle_jtick(line)
            return
        if lower.startswith("jdelay"):
            self._handle_jdelay(line)
            return        
        if lower.startswith("verbose"):
            self._handle_verbose(line)
            return
        if lower.startswith("interval"):
            self._handle_interval(line)
            return
        if lower == "status":
            self._print_status()
            return
        self.pending_commands[self.current_task_index].append(line)
        print(f"Queued for task {self.current_task_index}: {line}")

    def _handle_task_switch(self, line: str):
        parts = line.split()
        if len(parts) != 2 or not parts[1].isdigit():
            print("Usage: task <index>")
            return
        idx = int(parts[1])
        self.current_task_index = idx
        print(f"Switched to task {idx}")

    def _print_help(self):
        print("Commands:")
        print("  task <index>       - switch current task")
        print("  move.Position=10   - queue assignment for current task")
        print("  pause/resume       - toggle execution state")
        print("  s | step           - run one cycle")
        print("  steps <count>      - run count additional cycles (relative)")
        print("  tick <count>       - run count ticks from current index")
        print("  tickTo <idx>       - run until absolute cycle index")
        print("  delay <ms>         - run for ms relative to current index")
        print("  timeTo <ms>        - run until absolute runtime (ms)")
        print("  jdelay <ms>        - jump relative ms with verbose off")
        print("  jtime <ms>         - jump to absolute ms with verbose off")
        print("  jsteps <count>     - jump relative ticks with verbose off")
        print("  jtick <idx>        - jump to absolute tick with verbose off")
        print("  verbose on/off     - toggle verbose printing")
        print("  interval <ms>      - set per-cycle sleep interval")
        print("  status             - show runner status")
        print("  quit/exit          - stop runner")
        print("  help/?             - show this message")

    def _print_status(self):
        verbose_flag = self.get_verbose_flag()
        cycle_time = self.get_cycle_time()
        print("Runner status:")
        print(f"  index       : {self._idx}")
        print(f"  paused      : {self.paused}")
        print(f"  tick_target : {self.tick_target}")
        print(f"  tick_action : {self.tick_action}")
        print(f"  interval_ms : {self.interval_ms}")
        print(f"  verbose_flag: {verbose_flag}")
        print(f"  cycle_time  : {cycle_time} us")

    def _handle_step(self, line: str):
        self._set_tick_target(self._idx + 1, action="pause")
        self.paused = False
        self.skip_sleep = True
        print(f"Step requested")
        
    def _handle_steps(self, line: str):
        self._run_until(line, isAbs=False, cmd="steps", isTick=True)

    def _handle_tick(self, line: str):
        self._run_until(line, isAbs=True, cmd="tick", isTick=True)

    def _handle_delay(self, line: str):
        return self._run_until(line, isAbs=False, cmd="delay", isTick=False)

    def _handle_time(self, line: str):
        return self._run_until(line, isAbs=True, cmd="time", isTick=False)

    def _run_until(self, line: str, isAbs:bool, cmd, isTick:bool):
        parts = line.split()
        if len(parts) != 2 or not parts[1].isdigit():
            # cmd = "timeTo"
            print(f"Usage: {cmd} <ms>")
            return
        ms = int(parts[1])
        if not isTick:
            target = self.time_to_ticks(ms)
        else:
            target = ms
        if not isAbs:
            target += self.getIndex()
        if target <= self.getIndex():
            print("Target must be greater than current index")
            return
        self._set_tick_target(target, action="pause")
        self.paused = False
        self.skip_sleep = True
        print(f"Running to tick {target}")

    def _handle_jdelay(self, line: str):
        self._jump_until(line, cmd="jdelay", isAbs=False, isTick=False)
    
    def _handle_jtime(self, line: str):
        self._jump_until(line, cmd="jumpTo", isAbs=True, isTick=False)
       
    def _handle_jsteps(self, line: str):
        self._jump_until(line, cmd="jsteps", isAbs=False, isTick=True)

    def _handle_jtick(self, line: str):
        self._jump_until(line, cmd="jtickTo", isAbs=True, isTick=True)

    def _jump_until(self, line, cmd, isAbs:bool, isTick:bool):
        parts = line.split()
        if len(parts) != 2 or not parts[1].isdigit():
            print(f"Usage: {cmd} <ms>")
            return
        ms = int(parts[1])
        if not isTick:
            target = self.time_to_ticks(ms)
        else:
            target = ms
        if not isAbs:
            target += self.getIndex()
        if target <= self.getIndex():
            print("Target must be greater than current index")
            return
        self._set_tick_target(target, action="verbose_restore")
        self.paused = False
        self.skip_sleep = True
        self.saved_verbose_flag = self.get_verbose_flag()
        self.set_verbose_flag(0)
        print(f"Jumping to tick {target}")


    def time_to_ticks(self, ms):
        cycle_time = self.get_cycle_time()
        return max(0, ms * 1000 // cycle_time) if cycle_time else ms


    def _handle_verbose(self, line: str):
        parts = line.split()
        if len(parts) != 2 or parts[1].lower() not in {"on", "off"}:
            print("Usage: verbose on|off")
            return
        flag = 1 if parts[1].lower() == "on" else 0
        self.set_verbose_flag(flag)
        print(f"Verbose set to {flag}")

    def _handle_interval(self, line: str):
        parts = line.split()
        if len(parts) != 2 or not parts[1].isdigit():
            print("Usage: interval <ms>")
            return
        self.interval_ms = int(parts[1])
        print(f"Per-cycle interval set to {self.interval_ms} ms")

    def _set_tick_target(self, target: int, action: str):
        if target < self.getIndex():
            return
        self.tick_target = target
        self.tick_action = action

    def pre_run_cycle(self):
        commands = self.pending_commands.pop(self.current_task_index, [])
        if not commands:
            return
        task = self._get_task(self.current_task_index)
        if task is None:
            print(f"No task bound at index {self.current_task_index}")
            return
        if not hasattr(task, "apply_assignment"):
            print(f"Task {self.current_task_index} does not support assignments")
            return
        for cmd in commands:
            try:
                task.apply_assignment(cmd)
                print(f"Applied: {cmd}")
            except Exception as exc:
                print(f"Failed '{cmd}': {exc}")

    def _get_task(self, index: int):
        if 0 <= index < len(self._tasks):
            return self._tasks[index]
        return None

    def run_once(self):
        self._drain_command_queue()
        if self.paused: # and self.tick_target is None and not self.autorun
            # import time
            # if self.interval_ms > 0 :
            #     time.sleep(self.interval_ms / 1000.0)
            return
        super().run_once()
        if not self.skip_sleep and self.interval_ms > 0:
            import time
            time.sleep(self.interval_ms / 1000.0)
        if self.tick_target is not None and self._idx >= self.tick_target:
            if self.tick_action == "verbose_restore" and self.saved_verbose_flag is not None:
                self.set_verbose_flag(self.saved_verbose_flag)
                self.saved_verbose_flag = None
                self.skip_sleep = False
            elif self.tick_action == "pause":
                self.paused = True
                self.skip_sleep = False
            else:
                self.skip_sleep = False
            self.tick_target = None
            self.tick_action = None
            

    def run(self, count=0):
        self._start_input_thread()
        try:
            super().run(count)
        finally:
            self._stop_input.set()

    
    def getIndex(self):
        """
        Get the current cycle index.
        
        Returns:
            int: Current cycle index
        """
        return self._idx


# Example implementation
class AutoTask(PouTaskBase):
    """
    Example task implementation similar to C++ AutoTask.
    """
    
    def __init__(self, move_position=100.0, move_velocity=10.0, 
                 move_acceleration=20.0, move_deceleration=20.0, move_jerk=0.0):
        """
        Initialize the task.
        
        Args:
            move_position: Target position for move
            move_velocity: Velocity for move
            move_acceleration: Acceleration for move
            move_deceleration: Deceleration for move
            move_jerk: Jerk for move
        """
        # Get axis reference
        self.axis = AxPouTaskBinderAuto.getAxisByIndex(0)
        if not self.axis:
            raise RuntimeError("Failed to get axis by index 0")
        # Create function blocks
        self.power = FbPower()
        self.move = FbMoveAbsolute()
        
        # Set axis references
        self.power.Axis = self.axis
        self.move.Axis = self.axis
        
        # Set move parameters
        self.move.Position = move_position
        self.move.Velocity = move_velocity
        self.move.Acceleration = move_acceleration
        self.move.Deceleration = move_deceleration
        self.move.Jerk = move_jerk
        self.power.bDriveStart = True
        self.power.bRegulatorOn = True

    def runCycle(self):
        """Execute one cycle of the task."""
        # Enable power
        self.power.Enable = True

        self.power.runCycle()
        
        # Execute move when power is ready
        # self.move.Execute = self.power.Status
        self.move.runCycle()


class AppliableTaskBase(PouTaskBase):
    def _parse_literal(self, raw: str):
        """Parse string into int/float/bool/str."""
        text = raw.strip()
        if not text:
            raise ValueError("Empty value")
        lower = text.lower()
        if lower in ("true", "false"):
            return lower == "true"
        if text.isdigit() or (text.startswith("-") and text[1:].isdigit()):
            return int(text)
        if self._is_float(text):
            return float(text)
        if (text.startswith('"') and text.endswith('"')) or (text.startswith("'") and text.endswith("'")):
            return text[1:-1]
        raise ValueError(f"Unsupported literal: {raw}")

    def _is_float(self, text: str):
        try:
            float(text)
            return True
        except ValueError:
            return False

    def apply_assignment(self, line: str):
        """Apply assignment like 'move.Position = 5' or 'index = 1'."""
        if "=" not in line:
            raise ValueError("Assignment must contain '='")
        path_str, value_str = line.split("=", 1)
        path_str = path_str.strip()
        value_str = value_str.strip()
        if not path_str or not value_str:
            raise ValueError("Invalid assignment")

        segments = [seg.strip() for seg in path_str.split(".") if seg.strip()]
        if not segments:
            raise ValueError("Invalid attribute path")

        target = self
        for segment in segments[:-1]:
            target = getattr(target, segment)
        attr = segments[-1]
        value = self._parse_literal(value_str)
        setattr(target, attr, value)

def pou_auto_demo(iniPath="", iniContent=None, cycles=300, verbose_flag=1, cycle_time=10000):
    """
    Simple demo function similar to pou_auto_demo in C++.
    
    Args:
        iniContent: INI configuration content (default if None)
        cycles: Number of cycles to run
        verbose_flag: Verbose flag
        cycle_time: Cycle time in microseconds
    """
    if iniContent is None:
        iniContent = """[base]
axis_num = 1
[axis_0]
cycle_time = 10000
is_axis_virtual = true
"""
    # Create runner
    runner = AxPouTaskBinderAuto(iniPath=iniPath, iniContent=iniContent)
    # Set configuration
    runner.set_cycles(cycles)
    runner.set_verbose_flag(verbose_flag)
    runner.set_cycle_time(cycle_time)
    
    # Create and bind task
    task = AutoTask()
    runner.bind(0, task)
    # Run
    runner.run()


def pou_demo():
    iniContent = """
[base]
axis_num = 1
[axis_0]
verbose_flag = 1
cycle_time = 10000
is_axis_virtual = true
"""
    iniPath = ""  # r"C:\Project\works\plcProj2\src\mhelper\data\zmc_motapp_demo_fak.ini"
    pou_auto_demo(iniPath=iniPath, iniContent=iniContent, cycles=220, verbose_flag=1)


if __name__ == "__main__":
    pou_demo()
