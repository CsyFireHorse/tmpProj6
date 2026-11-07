from axfb_pou_runner import AppliableTaskBase, AxPouTaskBinderAuto, InteractivePouRunner
from pymc import FbPower, FbMoveAbsolute

# Example implementation
class MoveAbsTask(AppliableTaskBase):
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



def parse_args(cmds=None):
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--ini_path", '-i', type=str, default="")
    parser.add_argument("--ini_content", type=str, default="")
    parser.add_argument("--cycles", type=int)
    parser.add_argument("--verbose_flag", type=int, default=1)
    return parser.parse_args(cmds)

def main():
    iniContent = """[base]
axis_num = 1
[axis_0]
cycle_time = 10000
is_axis_virtual = true
"""
    args = parse_args()
    # interactive_pou_demo(args.ini_path, args.ini_content, args.cycles, args.verbose_flag)
    
    if not args.ini_path and not args.ini_content:
        args.ini_content = iniContent

    runner = InteractivePouRunner(iniPath=args.ini_path, iniContent=args.ini_content)
    runner.set_verbose_flag(args.verbose_flag)
    if args.cycles is not None:
        runner.set_cycles(args.cycles)
    task = MoveAbsTask()
    runner.bind(0, task)
    runner.run()
    
if __name__ == "__main__":
    # pou_demo()
    main()