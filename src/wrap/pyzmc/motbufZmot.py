from typing import List, Any, Optional, Tuple
from enum import IntEnum

try:
    from .pymc import (
        FbAxisWrap,
        FbPower,
        FbReadParameter,
        FbReadStatus as PymcFbReadStatus,
        FbReadSetValues as PymcFbReadSetValues,
        FbReadActualValues as PymcFbReadActualValues,
        FbFollowPositionVelocity as PymcFbFollowPositionVelocity,
        McTrajWrap,
        McVelWrap,
        zmc_dll,
    )
except ImportError:
    from pymc import (
        FbAxisWrap,
        FbPower,
        FbReadParameter,
        FbReadStatus as PymcFbReadStatus,
        FbReadSetValues as PymcFbReadSetValues,
        FbReadActualValues as PymcFbReadActualValues,
        FbFollowPositionVelocity as PymcFbFollowPositionVelocity,
        McTrajWrap,
        McVelWrap,
        zmc_dll,
    )


# Command type constants (placeholder, align with fbmEnum.h)
FBM_BASE = 100          # select axis group base
FBM_GROUP_ENABLE = 101  # enable axis group
FBM_GROUP_DISABLE = 102  # disable axis group
FBM_MOVE_LINEAR = 103   # multi-axis linear move (group)

# FB status constants (placeholder, map to FB_STAT_xxx)
FB_STAT_IDLE = 0
FB_STAT_BUSY = 1
FB_STAT_DONE = 2
FB_STAT_ERROR = 3
FB_STAT_ABORTED = 4


class SMC_AXIS_GROUP_STATE(IntEnum):
    """
    Axis group state enum (PLCopen MC Part 4).
    """

    Disabled = 0
    Standby = 1
    Homing = 2
    Moving = 3
    Stopping = 6
    ErrorStop = 7

class ZMC_ERROR(IntEnum):
    UNGROUP_FAIL = 1000
    INVALID_GROUP = 1001
    INVALID_GROUP_STATE = 1002
    MISMATCH_AXIS_NUM = 1003
    TRAJ_BUILD_FAIL = 1004
    VEL_BUILD_FAIL = 1005
    VELOCITY_INVALID = 1006
    ACCEL_DECEL_INVALID = 1007
    
PARA_fTaskCycle = 1015


class AxisEnhanced():
    """
    Axis wrapper with FbReadStatus and FbFollowPosition helpers.
    """

    def __init__(self, axis_wrap: FbAxisWrap) -> None:
        self.axis_wrap = axis_wrap
        self.axis_ref = axis_wrap.ref

        self.fb_read_status = PymcFbReadStatus()
        self.fb_read_status.Axis = self.axis_ref

        self.fb_read_set_values = PymcFbReadSetValues()
        self.fb_read_set_values.Axis = self.axis_ref

        self.fb_read_act_values = PymcFbReadActualValues()
        self.fb_read_act_values.Axis = self.axis_ref

        self.fb_follow_pos_vel = PymcFbFollowPositionVelocity()
        self.fb_follow_pos_vel.Axis = self.axis_ref
        self._pending_follow_position: float = 0.0
        self._pending_follow_velocity: float = 0.0

        self.fb_read_parameter = FbReadParameter()
        self.fb_read_parameter.Axis = self.axis_ref
        self.fb_read_parameter.Enable = True
        self.fb_read_parameter.ParameterNumber = PARA_fTaskCycle

        self.fb_power = FbPower()
        self.fb_power.Axis = self.axis_ref
        self.fb_power.Enable = True
        self.fb_power.bRegulatorOn = True
        self.fb_power.bDriveStart = True

    def _warmup_axis(self, cycles: int = 10) -> None:
        for _ in range(cycles):
            self.fb_power.runCycle()
            self.axis_wrap.run_cycle()

    def read_cycle_time(self) -> float:
        self._warmup_axis()
        for _ in range(20):
            self.fb_power.runCycle()
            self.fb_read_parameter.runCycle()
            value = float(self.fb_read_parameter.Value)
            if value > 0.0:
                # PARA_fTaskCycle may be reported in milliseconds on some targets.
                # Normalize to seconds for trajectory dt integration.
                if value >= 1.0:
                    value = value / 1000.0
                print(f"Cycle time: {value} seconds")
                return value
            self.axis_wrap.run_cycle()
        return 0.0

    def read_status(self) -> dict:
        """
        Read axis status using FbReadStatus pseudo FB.
        """
        self.fb_read_status.Enable = True
        self.fb_read_status.runCycle()
        return {
            "Valid": self.fb_read_status.Valid,
            "Error": self.fb_read_status.Error,
            "ErrorID": self.fb_read_status.ErrorID,
            "Status": self.fb_read_status.SynchronizedMotion*5 + self.fb_read_status.Stopping*4 + self.fb_read_status.Homing*2 + self.fb_read_status.Standstill*1 + self.fb_read_status.DiscreteMotion*3 + self.fb_read_status.ContinuousMotion*4 + self.fb_read_status.ErrorStop*7 + self.fb_read_status.Disabled*0,

            # "ConstantVelocity": self.fb_read_status.ConstantVelocity,
            # "Accelerating": self.fb_read_status.Accelerating,
            # "Decelerating": self.fb_read_status.Decelerating,
            # "FBErrorOccurred": self.fb_read_status.FBErrorOccurred,
        }

    def read_set_values(self) -> dict:
        """
        Read set values (position/velocity/acceleration) from axis.
        """
        self.fb_read_set_values.Enable = True
        self.fb_read_set_values.runCycle()
        return {
            "Valid": self.fb_read_set_values.Valid,
            "Error": self.fb_read_set_values.Error,
            "Position": self.fb_read_set_values.Position,
            "Velocity": self.fb_read_set_values.Velocity,
            "Acceleration": self.fb_read_set_values.Acceleration,
        }

    def read_act_values(self) -> dict:
        """
        Read actual values (position/velocity/acceleration) from axis.
        """
        self.fb_read_act_values.Enable = True
        self.fb_read_act_values.runCycle()
        return {
            "Valid": self.fb_read_act_values.Valid,
            "Error": self.fb_read_act_values.Error,
            "Position": self.fb_read_act_values.Position,
            "Velocity": self.fb_read_act_values.Velocity,
            "Acceleration": self.fb_read_act_values.Acceleration,
        }

    def write_follow_position(self, position: float) -> None:
        """
        Write target (follow) position using FbFollowPosition pseudo FB.
        only write to buffer, not write to axis, so can write to buffer multiple times
        """
        self._pending_follow_position = float(position)

    def write_follow_velocity(self, velocity: float) -> None:
        """
        Write target velocity directly to native axis setpoint.
        """
        self._pending_follow_velocity = float(velocity)

    def write_follow_pos_vel(self, position: float, velocity: float) -> None:
        """
        Write target position and velocity together in one call.
        """
        self._pending_follow_position = float(position)
        self._pending_follow_velocity = float(velocity)
    
    def preRunCycle(self):
        # FollowPosition and FollowVelocity are mutually exclusive.
        # Use a single FollowPositionVelocity FB and write pos/vel together.
        self.fb_follow_pos_vel.fSetPosition = self._pending_follow_position
        self.fb_follow_pos_vel.fSetVelocity = self._pending_follow_velocity
        self.fb_follow_pos_vel.bExecute = True
        self.fb_follow_pos_vel.runCycle()
        self.fb_follow_pos_vel.bExecute = False

    def run_axis_cycle(self) -> None:
        self.axis_wrap.run_cycle()


class AxisManager:
    """
    Native axis manager (conceptual mirror of C++ AxisManager / SgAxisArr).
    Owns only native axis references (AXIS_REF placeholders), not wrappers.
    """

    _axis_refs: List[FbAxisWrap] = []

    @classmethod
    def get_axis_num(cls) -> int:
        return len(cls._axis_refs)
    
    @classmethod
    def add_axis_ref(cls, ref: FbAxisWrap) -> None:
        """
        Register a native axis reference (placeholder for AXIS_REF).
        """
        cls._axis_refs.append(ref)

    @classmethod
    def get_axis_by_index(cls, index: int) -> Optional[FbAxisWrap]:
        """
        Get native axis reference by index.
        """
        if 0 <= index < len(cls._axis_refs):
            return cls._axis_refs[index]
        return None

    @classmethod
    def get_axis_default(cls) -> Optional[Any]:
        """
        Get default native axis reference (index 0 if available).
        """
        if cls._axis_refs:
            return cls._axis_refs[0]
        return None

    @classmethod
    def run_all_cycles(cls) -> None:
        for axis_wrap in cls._axis_refs:
            axis_wrap.run_cycle()


class AxGroup:
    """
    Axis group (base). One group corresponds to one motion base.
    """

    def __init__(self, idx: int) -> None:
        self.group_id: int = idx
        self.cycle_time: float = 0.01

        # Axis configuration
        self.ax_idx_arr: List[int] = []  # axis indices in this group
        self.ax_num: int = 0
        # Optional axis enhancement wrappers (built from native AXIS_REF)
        self.axes_enhanced: List[AxisEnhanced] = []
        self.tick: int = 0
        
        # Group-level motion state (target/output side)
        # These vectors are aligned with ax_idx_arr.
        self.target_pos: List[float] = []   # group target position
        self.target_vel: List[float] = []   # group target velocity
        self.target_acc: List[float] = []   # group target acceleration

        # Axis group state
        self.group_state: int = SMC_AXIS_GROUP_STATE.Disabled
        self.ErrorID: int = 0
        # Active motion FB list
        self.fb_list: List["FbMotion"] = []

    def enable(self):
        if self.group_state == SMC_AXIS_GROUP_STATE.Disabled:
            self.group_state = SMC_AXIS_GROUP_STATE.Standby
    def disable(self):
        if self.group_state == SMC_AXIS_GROUP_STATE.Standby:
            self.group_state = SMC_AXIS_GROUP_STATE.Disabled
    
    def reset(self):
        if self.group_state == SMC_AXIS_GROUP_STATE.ErrorStop:
            self.group_state = SMC_AXIS_GROUP_STATE.Standby
    
    def ungroupAllAxes(self):
        if self.group_state == SMC_AXIS_GROUP_STATE.Disabled:
            self.axes_enhanced = []
            self.ax_num = 0
        else:
            self.group_state = SMC_AXIS_GROUP_STATE.ErrorStop
            self.ErrorID = ZMC_ERROR.UNGROUP_FAIL
            

    def ensure_size(self) -> None:
        """
        Ensure target arrays have the same length as ax_num.
        """
        n = self.ax_num
        if len(self.target_pos) != n:
            self.target_pos = [0.0] * n
        if len(self.target_vel) != n:
            self.target_vel = [0.0] * n
        if len(self.target_acc) != n:
            self.target_acc = [0.0] * n

    def add_axis_to_group(self, axis_ref: FbAxisWrap) -> None:
        """
        Bind a native axis (by index) into this group.
        The native axis reference comes from AxisManager; this group then
        creates an AxisEnhanced wrapper as a high-level helper.
        """
        if self.group_state != SMC_AXIS_GROUP_STATE.Disabled:
            return
        if axis_ref is None:
            return
        axis_index = 0 # TODO: get axis index from axis_ref
        self.ax_idx_arr.append(axis_index)
        # In real C++ code, AxisEnhanced would wrap AXIS_REF; here we reuse ref.
        enhanced = AxisEnhanced(axis_ref)
        self.axes_enhanced.append(enhanced)
        self.ax_num = len(self.ax_idx_arr)
        cycle_time = enhanced.read_cycle_time()
        if cycle_time <= 0.0:
            raise RuntimeError("Invalid axis task cycle time from FbReadParameter")
        self.cycle_time = cycle_time
        self.ensure_size()

    def runCycle(self):
        self.tick += 1
        # Push group target positions to each axis wrapper before one cycle update.
        for i, axis in enumerate(self.axes_enhanced):
            if 1:
                pos = self.target_pos[i]
                vel = self.target_vel[i]
                axis.write_follow_pos_vel(pos, vel)
            axis.preRunCycle()
    
    def printAll(self):
        print(f"Group {self.group_id} state: {self.group_state}, ErrorID: {self.ErrorID}")
        for i in range(self.ax_num):
            self.print(i)

    def print(self, i: int) -> None:
        act_values = self.axes_enhanced[i].read_act_values()
        set_values = self.axes_enhanced[i].read_set_values()
        status = self.axes_enhanced[i].read_status()
        print(
            f"Cycle {self.tick:03d} axis {i} "
            f"act_pos={act_values['Position']:.6f} "
            f"act_vel={act_values['Velocity']:.6f} "
            f"set_pos={set_values['Position']:.6f} "
            f"status(S={status['Status']},E={status['Error']},EID={status['ErrorID']})"
        )

class Cmd:
    def __init__(self, m_type: int = 0, params: Optional[List[Any]] = None) -> None:
        self.m_type: int = m_type
        self._params: List[Any] = params if params is not None else []

    def get_param(self, idx: int) -> Any:
        return self._params[idx]

    def get_param_list(self) -> List[Any]:
        return self._params

    def __repr__(self) -> str:
        return f"Cmd(type={self.m_type}, params={self._params})"



class FbMotion:
    """
    Base motion FB interface (pseudo).
    """

    def __init__(self) -> None:
        self.execute: bool = False
        self.executeBak: bool = False
        self.status: int = FB_STAT_IDLE
        self.ErrorId: int = 0
        self.group: Optional[AxGroup] = None

    # -------- control helpers --------
    def onError(self, errorId: int) -> None:
        self.status = FB_STAT_ERROR
        self.ErrorId = errorId
    
    def set_axis_group(self, group: AxGroup) -> None:
        """
        Bind this FB to an axis group.
        """
        self.group = group

    def set_execute(self, on: bool) -> None:
        """
        Set execute flag and initialize/reset internal state.
        """
        self.execute = on

    # -------- lifecycle hooks --------

    def on_start(self) -> None:
        """
        Called when execute goes from False to True.
        Override in subclass for initialization.
        """
        if self.status == FB_STAT_IDLE:
            self.status = FB_STAT_BUSY
        else:
            return
        if self.group is None:
            return self.onError(ZMC_ERROR.INVALID_GROUP)
        if self.group.group_state in (SMC_AXIS_GROUP_STATE.Disabled, SMC_AXIS_GROUP_STATE.ErrorStop):
            return self.onError(ZMC_ERROR.INVALID_GROUP_STATE)
            # FB activates group: set group state to Moving
        self.group.group_state = SMC_AXIS_GROUP_STATE.Moving

    def on_stop(self) -> None:
        """
        Called when execute goes from True to False.
        Override in subclass for clean up or abort.
        """
        if self.status in (FB_STAT_ERROR, FB_STAT_ABORTED, FB_STAT_DONE):
            self.status = FB_STAT_IDLE

    # -------- abstract interface --------

    def bind_cmd(self, cmd: Cmd) -> None:
        """Bind command parameters to this FB (override in subclass)."""
        raise NotImplementedError

    def run_cycle(self) -> None:
        """
        Single cycle execution (override in subclass).
        """
        if self.execute and not self.executeBak:
            # rising edge: start motion
            self.on_start()
        elif not self.execute and self.executeBak:
            # falling edge: stop/abort motion
            self.on_stop()
        self.run_cycle_impl()
        self.executeBak = self.execute

    def run_cycle_impl(self) -> None:
        """
        Single cycle execution (override in subclass).
        """
        raise NotImplementedError
    
    def get_stat(self) -> int:
        """Return FB status (FB_STAT_XXX)."""
        return self.status

class FbMotionLinear(FbMotion):
    """
    Multi-axis linear motion FB (pseudo).
    It holds:
      - axis count
      - target position vector for this group
      - simple constant-velocity trajectory generator (TrajLinear)
    """

    def __init__(self) -> None:
        super().__init__()
        self.axis_num: int = 0
        self.target_position: List[float] = []  # one target position per axis in group

        # velocity profile parameters (simplified)
        self.cmd_vel: float = 0.0   # scalar speed along path
        self.cmd_acc: float = 0.0   # currently unused (for future extension)
        self.cmd_dec: float = 0.0   # currently unused
        self.cmd_jerk: float = 0.0  # currently unused
        # trajectory state: McTrajWrap + McVelWrap
        self.traj_wrap = McTrajWrap()
        self.vel_wrap = McVelWrap()
        self.traj_len: float = 0.0
        self.elapsed: float = 0.0
        self.end_time: float = 0.0
        self.start_pos: List[float] = []

        
    def bind_cmd(self, cmd: Cmd) -> None:
        # Example: params = [axis_num, [p0, p1, ...], vel, acc, dec, jerk]
        self.axis_num = int(cmd.get_param(0))
        self.target_position = list(cmd.get_param(1))
        self.cmd_vel = float(cmd.get_param(2))
        self.cmd_acc = float(cmd.get_param(3))
        self.cmd_dec = float(cmd.get_param(4))
        self.cmd_jerk = float(cmd.get_param(5))

    def on_start(self) -> None:
        super().on_start()
        
        # reset trajectory state
        self.traj_len = 0.0
        self.elapsed = 0.0
        self.end_time = 0.0
        self.start_pos = []
        

        # ensure group target arrays size
        self.group.ensure_size()
        if self.axis_num != self.group.ax_num:
            # dimension mismatch
            return self.onError(ZMC_ERROR.MISMATCH_AXIS_NUM)

        # configure geometry using McTrajWrap and timing using McVelWrap
        self.start_pos = list(self.group.target_pos)
        diff_arr: List[float] = [
            t - s for s, t in zip(self.start_pos, self.target_position)
        ]
        print(self.start_pos, diff_arr)
        # build geometric trajectory
        # explicitly initialize axis dimension for trajectory core
        self.traj_wrap.init_axis_num(self.axis_num)
        self.traj_wrap.set_begin_pos(self.start_pos)
        # use relative line parameters; diff_arr is delta per axis
        self.traj_wrap.set_line_param(diff_arr, True)
        traj_ret = self.traj_wrap.build()
        if traj_ret != 0:
            self.onError(ZMC_ERROR.TRAJ_BUILD_FAIL)
            return

        self.traj_len = float(self.traj_wrap.get_plan())
        if self.cmd_vel <= 0.0:
            return self.onError(ZMC_ERROR.VELOCITY_INVALID)

        print("self.traj_len=", self.traj_len, self.axis_num, self.traj_wrap.get_axis_num())
        # build velocity plan along path length
        if self.cmd_acc <= 0.0 or self.cmd_dec <= 0.0 or self.cmd_jerk < 0.0:
            return self.onError(ZMC_ERROR.ACCEL_DECEL_INVALID)
        acc_cfg = self.cmd_acc
        dec_cfg = self.cmd_dec
        jerk_cfg = self.cmd_jerk
        self.vel_wrap.set_config(self.cmd_vel, acc_cfg, dec_cfg, jerk_cfg)
        self.vel_wrap.set_begin_pva(0.0, 0.0, 0.0)
        self.vel_wrap.set_end_pva(self.traj_len, 0.0, 0.0)
        vel_ret = self.vel_wrap.build()
        if vel_ret != 0:
            self.onError(ZMC_ERROR.VEL_BUILD_FAIL)
            return
        self.end_time = float(self.vel_wrap.get_end_time())
        print("end_time=", self.end_time)
        
    def run_cycle_impl(self) -> None:
        if self.status not in (FB_STAT_BUSY, FB_STAT_IDLE):
            return


        if self.status == FB_STAT_ERROR:
            # keep group in error stop
            self.group.group_state = SMC_AXIS_GROUP_STATE.ErrorStop
            return

        # step trajectory and update group target_pos / target_vel / target_acc
        dt = self.group.cycle_time
        self.elapsed += max(dt, 0.0)

        ret, d, v, a = self.vel_wrap.get_slice(self.elapsed)
        if 0 and ret != 0:
            self.status = FB_STAT_ERROR
            self.group.group_state = SMC_AXIS_GROUP_STATE.ErrorStop
            return

        # use trajectory slice3 from McTrajWrap to get per-axis position / velocity / acceleration
        cur_pos: List[float] = [0.0] * self.axis_num
        cur_vel: List[float] = [0.0] * self.axis_num
        cur_acc: List[float] = [0.0] * self.axis_num
        if 1:
            abs_pos, dif_pos, dif2_pos = self.traj_wrap.get_slice3(d, v, a, self.axis_num)
            cur_pos = abs_pos
            cur_vel = dif_pos
            cur_acc = dif2_pos

            for i in range(self.axis_num):
                self.group.target_pos[i] = cur_pos[i]
                self.group.target_vel[i] = cur_vel[i]
                self.group.target_acc[i] = cur_acc[i]

        if self.elapsed >= self.end_time:
            self.status = FB_STAT_DONE
            self.group.group_state = SMC_AXIS_GROUP_STATE.Standby


class MotbufZmot:
    """
    Multi-axis motion buffer:
      - manages axis groups (base selection)
      - accepts commands (base select + group linear move)
      - allocates and schedules FbMotion instances
    """

    def __init__(self, max_groups: int = 32) -> None:
        # global command queue
        self.cmd_queue: List[Cmd] = []
        # all active FBs (optional global list)
        self.fb_list: List[FbMotion] = []
        # per-group sentinel
        self.sentinel_arr: List[AxGroup] = [AxGroup(i) for i in range(max_groups)]
        # current selected base group id; -1 means no selection
        self.base_id: int = -1
    # ------------ command queue API ------------

    def push(self, cmd: Cmd) -> None:
        """Push command into global queue."""
        self.cmd_queue.append(cmd)

    # ------------ FB allocation helpers ------------

    def allocate_fb_linear(self) -> FbMotionLinear:
        """
        Allocate a new linear motion FB.
        In C++ you may use object pools; here we simply construct a new instance.
        """
        fb = FbMotionLinear()
        self.fb_list.append(fb)
        return fb

    # ------------ command processing ------------

    def _handle_base_cmd(self, cmd: Cmd) -> None:
        """
        Handle FBM_BASE: select base group and bind its axis indices.
        Params example: [base_id:int, [ax_idx0, ax_idx1, ...]]
        """
        base_id = int(cmd.get_param(0))
        axis_idx_list = list(cmd.get_param(1))

        self.base_id = base_id
        group = self.sentinel_arr[base_id]
        # rebind group axes using AxisManager for each index
        group.ax_idx_arr = []
        group.axes_enhanced = []
        group.ax_num = 0
        for idx in axis_idx_list:
            axis_ref = AxisManager.get_axis_by_index(int(idx))
            group.add_axis_to_group(axis_ref)
    def _handle_group_enable_cmd(self, cmd: Cmd) -> None:
        """
        Handle FBM_GROUP_ENABLE: enable axis group.
        Params example: [group_id:int]
        """
        group_id = int(cmd.get_param(0))
        group = self.sentinel_arr[group_id]
        group.enable()

    def _handle_group_disable_cmd(self, cmd: Cmd) -> None:
        """
        Handle FBM_GROUP_DISABLE: disable axis group.
        Params example: [group_id:int]
        """
        group_id = int(cmd.get_param(0))
        group = self.sentinel_arr[group_id]
        group.disable()
        
    def _handle_move_linear_cmd(self, cmd: Cmd) -> int:
        """
        Handle FBM_MOVE_LINEAR:
        - require base_id already selected
        - allocate a linear FB
        - bind parameters and attach to group fb_list
        Params example (pseudo):
          [target_positions:list[float], vel, acc, dec, jerk]
        """
        if self.base_id < 0:
            # no base group selected
            return -1

        group = self.sentinel_arr[self.base_id]
        if group.ax_num <= 0:
            # group has no axes
            return -1

        # enrich command with axis_num and reorder params for FbMotionLinear
        target_positions = list(cmd.get_param(0))
        vel = float(cmd.get_param(1))
        acc = float(cmd.get_param(2))
        dec = float(cmd.get_param(3))
        jerk = float(cmd.get_param(4))

        # build a new command for FbMotionLinear
        fb_cmd = Cmd(
            m_type=FBM_MOVE_LINEAR,
            params=[
                group.ax_num,         # param 0: axis_num
                target_positions,     # param 1: position vector
                vel,                  # param 2
                acc,                  # param 3
                dec,                  # param 4
                jerk,                 # param 5
            ],
        )

        fb = self.allocate_fb_linear()
        fb.set_axis_group(group)
        fb.bind_cmd(fb_cmd)
        fb.set_execute(True)  # start motion
        group.fb_list.append(fb)

        return 0

    def process_commands(self) -> None:
        """
        Process all pending commands in cmd_queue.
        This is similar to C++ runCycleTop's command decoding part.
        """
        while self.cmd_queue:
            cmd = self.cmd_queue.pop(0)

            if cmd.m_type == FBM_BASE:
                self._handle_base_cmd(cmd)
            elif cmd.m_type == FBM_GROUP_ENABLE:
                self._handle_group_enable_cmd(cmd)
            elif cmd.m_type == FBM_GROUP_DISABLE:
                self._handle_group_disable_cmd(cmd)
            elif cmd.m_type == FBM_MOVE_LINEAR:
                ret = self._handle_move_linear_cmd(cmd)
                if ret != 0:
                    # error handling placeholder
                    # e.g. log or push result
                    pass

            else:
                # other command types are ignored for now
                pass

    # ------------ cycle interface ------------

    def run_cycle_top(self) -> None:
        """
        High-level: process commands and update FB internal states.
        """
        self.process_commands()
        # group-level FB pre-run operations can be added here


    def run_cycle_bottom(self) -> None:
        """
        Execute FBs for each group.
        In C++ this would iterate intrusive lists, manage pools, and free finished FBs.
        Here we use a simplified list iteration.
        """
        for group in self.sentinel_arr:
            if group.ax_num <= 0:
                continue

            # traverse a copy to allow removal during iteration
            fb_list_copy = list(group.fb_list)

            for fb in fb_list_copy:
                # Run this FB one cycle
                fb.run_cycle()
                fb_stat = fb.get_stat()

                if fb_stat in (FB_STAT_DONE, FB_STAT_ERROR, FB_STAT_ABORTED):
                    # stop and remove finished FB
                    fb.set_execute(False)
                    group.fb_list.remove(fb)
                    # here you could pushCmdResult(...) like in C++
            group.runCycle()


def toy_demo() -> None:
    """
    Demo for MotbufZmot with real pymc wrappers.
    Requires mcapi.dll and fb wrappers in pymc.py.
    """
    if not zmc_dll:
        raise RuntimeError("mcapi.dll is required for motbufRobot toy_demo")

    # Register two virtual axes (axis 0 and axis 1)
    AxisManager.add_axis_ref(FbAxisWrap())
    AxisManager.add_axis_ref(FbAxisWrap())
    axisNum = AxisManager.get_axis_num()
    motbuf = MotbufZmot(max_groups=2)

    # Select base 0 with axes [0, 1]
    base_cmd = Cmd(m_type=FBM_BASE, params=[0, [0, 1]])
    motbuf.push(base_cmd)
    enable_cmd = Cmd(m_type=FBM_GROUP_ENABLE, params=[0])
    motbuf.push(enable_cmd)
    # Command a simple linear move to positions [10.0, 24.0]
    move_cmd = Cmd(m_type=FBM_MOVE_LINEAR, params=[[10.0, 24.0], 100.0, 200.0, 200.0, 0.0])
    move_cmd_pushed = False

    # Build power FB for each axis, and enable drive first.
    power_fbs: List[FbPower] = []
    for i in range(axisNum):
        axis_wrap = AxisManager.get_axis_by_index(i)
        if axis_wrap is None:
            continue
        power = FbPower()
        power.Axis = axis_wrap.ref
        power.Enable = True
        power.bRegulatorOn = True
        power.bDriveStart = True
        power_fbs.append(power)

    # Run a limited number of cycles to simulate execution
    max_cycles = 1000
    print_every = 20
    for cycle in range(max_cycles):
        # Execute power enabling first.
        for power in power_fbs:
            power.runCycle()

        # Push motion command only after all axes are powered on.
        if (not move_cmd_pushed) and power_fbs and all(bool(power.Status) for power in power_fbs):
            motbuf.push(move_cmd)
            move_cmd_pushed = True
            print(f"Cycle {cycle:03d} power enabled, move_cmd pushed.")

        motbuf.run_cycle_top()
        motbuf.run_cycle_bottom()
        AxisManager.run_all_cycles()

        # Print periodically to avoid flooding terminal output.
        if (cycle % print_every == 0) or (cycle == max_cycles - 1):
            group = motbuf.sentinel_arr[0]
            group.printAll()

        # Stop once all groups have no active FBs.
        if all(len(group.fb_list) == 0 for group in motbuf.sentinel_arr) and cycle > 100:
            print(f"Motion finished at cycle {cycle}.")
            motbuf.sentinel_arr[0].printAll()
            break
    else:
        print("Motion did not finish within max_cycles.")
    
def main():
    toy_demo()
    

if __name__ == "__main__":
    main()    