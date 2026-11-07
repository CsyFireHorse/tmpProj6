/*
 * Shared x64 type layout for memory-backed POU fields.
 * @ArtifactStatus draft
 * @ArtifactStatusDate 2026-09-02
 * @ArtifactStatusAuthority GPT-5 Codex
 */
#ifndef ZMC_MEMORY_POU_TYPE_LAYOUT_HPP
#define ZMC_MEMORY_POU_TYPE_LAYOUT_HPP

// Type layout copied from the reference zmc_type_layout.json.
static const char kMemoryPouTypeLayout[] = R"json(
{
  "types": {
    "void*": {"size": 8, "align": 8, "kind": "pointer"},
    "AXIS_REF": {"size": 8, "align": 8, "kind": "pointer"},
    "AxisGrpZmotRef": {"size": 8, "align": 8, "kind": "pointer"},
    "ZMC_BOOL": {"size": 1, "align": 1, "kind": "bool"},
    "ZMC_DINT": {"size": 4, "align": 4, "kind": "int32"},
    "int": {"size": 4, "align": 4, "kind": "int32"},
    "ZMC_UDINT": {"size": 4, "align": 4, "kind": "uint32"},
    "ZMC_DWORD": {"size": 4, "align": 4, "kind": "uint32"},
    "MC_ERROR": {"size": 2, "align": 2, "kind": "uint16"},
    "MC_BUFFER_MODE": {"size": 2, "align": 2, "kind": "uint16"},
    "MC_DIRECTION": {"size": 2, "align": 2, "kind": "uint16"},
    "MC_HOMING_MODE": {"size": 2, "align": 2, "kind": "uint16"},
    "ZMC_LREAL": {"size": 8, "align": 8, "kind": "double"},
    "ZMC_POS_REF": {
      "size": 64, "align": 8, "kind": "pod",
      "fields": {
        "a0": {"type": "ZMC_LREAL", "offset": 0, "size": 8},
        "a1": {"type": "ZMC_LREAL", "offset": 8, "size": 8},
        "a2": {"type": "ZMC_LREAL", "offset": 16, "size": 8},
        "a3": {"type": "ZMC_LREAL", "offset": 24, "size": 8},
        "a4": {"type": "ZMC_LREAL", "offset": 32, "size": 8},
        "a5": {"type": "ZMC_LREAL", "offset": 40, "size": 8},
        "a6": {"type": "ZMC_LREAL", "offset": 48, "size": 8},
        "a7": {"type": "ZMC_LREAL", "offset": 56, "size": 8}
      }
    },
    "FbtPower": {
      "size": 32, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Enable": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "bRegulatorOn": {"type": "ZMC_BOOL", "offset": 17, "size": 1},
        "bDriveStart": {"type": "ZMC_BOOL", "offset": 18, "size": 1},
        "Status": {"type": "ZMC_BOOL", "offset": 19, "size": 1},
        "bRegulatorRealState": {"type": "ZMC_BOOL", "offset": 20, "size": 1},
        "bDriveStartRealState": {"type": "ZMC_BOOL", "offset": 21, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 22, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 23, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 24, "size": 2}
      }
    },
    "FbtReset": {
      "size": 24, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Execute": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Done": {"type": "ZMC_BOOL", "offset": 17, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 18, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 19, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 20, "size": 2}
      }
    },
    "FbtReadActualPosition": {
      "size": 32, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Enable": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Valid": {"type": "ZMC_BOOL", "offset": 17, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 18, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 19, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 20, "size": 2},
        "Position": {"type": "ZMC_LREAL", "offset": 24, "size": 8}
      }
    },
    "FbtReadAxisError": {
      "size": 32, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Enable": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Valid": {"type": "ZMC_BOOL", "offset": 17, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 18, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 19, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 20, "size": 2},
        "AxisError": {"type": "ZMC_BOOL", "offset": 22, "size": 1},
        "AxisErrorID": {"type": "ZMC_DWORD", "offset": 24, "size": 4},
        "SWEndSwitchActive": {"type": "ZMC_BOOL", "offset": 28, "size": 1}
      }
    },
    "FbtReadStatus": {
      "size": 40, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Enable": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Valid": {"type": "ZMC_BOOL", "offset": 17, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 18, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 19, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 20, "size": 2},
        "Disabled": {"type": "ZMC_BOOL", "offset": 22, "size": 1},
        "ErrorStop": {"type": "ZMC_BOOL", "offset": 23, "size": 1},
        "Stopping": {"type": "ZMC_BOOL", "offset": 24, "size": 1},
        "Standstill": {"type": "ZMC_BOOL", "offset": 25, "size": 1},
        "DiscreteMotion": {"type": "ZMC_BOOL", "offset": 26, "size": 1},
        "ContinuousMotion": {"type": "ZMC_BOOL", "offset": 27, "size": 1},
        "SynchronizedMotion": {"type": "ZMC_BOOL", "offset": 28, "size": 1},
        "Homing": {"type": "ZMC_BOOL", "offset": 29, "size": 1},
        "ConstantVelocity": {"type": "ZMC_BOOL", "offset": 30, "size": 1},
        "Accelerating": {"type": "ZMC_BOOL", "offset": 31, "size": 1},
        "Decelerating": {"type": "ZMC_BOOL", "offset": 32, "size": 1},
        "FBErrorOccurred": {"type": "ZMC_BOOL", "offset": 33, "size": 1}
      }
    },
    "FbtSetPosition": {
      "size": 40, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Execute": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Position": {"type": "ZMC_LREAL", "offset": 24, "size": 8},
        "Mode": {"type": "ZMC_BOOL", "offset": 32, "size": 1},
        "Done": {"type": "ZMC_BOOL", "offset": 33, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 34, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 35, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 36, "size": 2}
      }
    },
    "FbtHome": {
      "size": 40, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Execute": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Position": {"type": "ZMC_LREAL", "offset": 24, "size": 8},
        "Done": {"type": "ZMC_BOOL", "offset": 32, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 33, "size": 1},
        "CommandAborted": {"type": "ZMC_BOOL", "offset": 34, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 35, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 36, "size": 2}
      }
    },
    "FbtHoming": {
      "size": 120, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "bExecute": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "fHomePosition": {"type": "ZMC_LREAL", "offset": 24, "size": 8},
        "fVelocitySlow": {"type": "ZMC_LREAL", "offset": 32, "size": 8},
        "fVelocityFast": {"type": "ZMC_LREAL", "offset": 40, "size": 8},
        "fAcceleration": {"type": "ZMC_LREAL", "offset": 48, "size": 8},
        "fDeceleration": {"type": "ZMC_LREAL", "offset": 56, "size": 8},
        "fJerk": {"type": "ZMC_LREAL", "offset": 64, "size": 8},
        "nDirection": {"type": "MC_DIRECTION", "offset": 72, "size": 2},
        "bReferenceSwitch": {"type": "ZMC_BOOL", "offset": 74, "size": 1},
        "fSignalDelay": {"type": "ZMC_LREAL", "offset": 80, "size": 8},
        "nHomingMode": {"type": "MC_HOMING_MODE", "offset": 88, "size": 2},
        "bReturnToZero": {"type": "ZMC_BOOL", "offset": 90, "size": 1},
        "bIndexOccured": {"type": "ZMC_BOOL", "offset": 91, "size": 1},
        "fIndexPosition": {"type": "ZMC_LREAL", "offset": 96, "size": 8},
        "bIgnoreHWLimit": {"type": "ZMC_BOOL", "offset": 104, "size": 1},
        "bDone": {"type": "ZMC_BOOL", "offset": 105, "size": 1},
        "bBusy": {"type": "ZMC_BOOL", "offset": 106, "size": 1},
        "bCommandAborted": {"type": "ZMC_BOOL", "offset": 107, "size": 1},
        "bError": {"type": "ZMC_BOOL", "offset": 108, "size": 1},
        "nErrorID": {"type": "MC_ERROR", "offset": 110, "size": 2},
        "bStartLatchingIndex": {"type": "ZMC_BOOL", "offset": 112, "size": 1}
      }
    },
 )json"
R"json(
    "FbtReadParameter": {
      "size": 40, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Enable": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "ParameterNumber": {"type": "ZMC_DINT", "offset": 20, "size": 4},
        "Valid": {"type": "ZMC_BOOL", "offset": 24, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 25, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 26, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 28, "size": 2},
        "Value": {"type": "ZMC_LREAL", "offset": 32, "size": 8}
      }
    },
    "FbtReadBoolParameter": {
      "size": 32, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Enable": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "ParameterNumber": {"type": "ZMC_DINT", "offset": 20, "size": 4},
        "Valid": {"type": "ZMC_BOOL", "offset": 24, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 25, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 26, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 28, "size": 2},
        "Value": {"type": "ZMC_BOOL", "offset": 30, "size": 1}
      }
    },
    "FbtWriteBoolParameter": {
      "size": 32, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Execute": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "ParameterNumber": {"type": "ZMC_DINT", "offset": 20, "size": 4},
        "Value": {"type": "ZMC_BOOL", "offset": 24, "size": 1},
        "Done": {"type": "ZMC_BOOL", "offset": 25, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 26, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 27, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 28, "size": 2}
      }
    },
    "FbtStop": {
      "size": 48, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Execute": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Deceleration": {"type": "ZMC_LREAL", "offset": 24, "size": 8},
        "Jerk": {"type": "ZMC_LREAL", "offset": 32, "size": 8},
        "Done": {"type": "ZMC_BOOL", "offset": 40, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 41, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 42, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 44, "size": 2}
      }
    },
    "FbtMoveVelocity": {
      "size": 72, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Execute": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Velocity": {"type": "ZMC_LREAL", "offset": 24, "size": 8},
        "Acceleration": {"type": "ZMC_LREAL", "offset": 32, "size": 8},
        "Deceleration": {"type": "ZMC_LREAL", "offset": 40, "size": 8},
        "Jerk": {"type": "ZMC_LREAL", "offset": 48, "size": 8},
        "Direction": {"type": "MC_DIRECTION", "offset": 56, "size": 2},
        "BufferMode": {"type": "MC_BUFFER_MODE", "offset": 58, "size": 2},
        "InVelocity": {"type": "ZMC_BOOL", "offset": 60, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 61, "size": 1},
        "Active": {"type": "ZMC_BOOL", "offset": 62, "size": 1},
        "CommandAborted": {"type": "ZMC_BOOL", "offset": 63, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 64, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 66, "size": 2}
      }
    },
    "FbtJog": {
      "size": 64, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "JogForward": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "JogBackward": {"type": "ZMC_BOOL", "offset": 17, "size": 1},
        "Velocity": {"type": "ZMC_LREAL", "offset": 24, "size": 8},
        "Acceleration": {"type": "ZMC_LREAL", "offset": 32, "size": 8},
        "Deceleration": {"type": "ZMC_LREAL", "offset": 40, "size": 8},
        "Jerk": {"type": "ZMC_LREAL", "offset": 48, "size": 8},
        "Busy": {"type": "ZMC_BOOL", "offset": 56, "size": 1},
        "CommandAborted": {"type": "ZMC_BOOL", "offset": 57, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 58, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 60, "size": 2}
      }
    },
    "FbtInch": {
      "size": 72, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "InchForward": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "InchBackward": {"type": "ZMC_BOOL", "offset": 17, "size": 1},
        "Distance": {"type": "ZMC_LREAL", "offset": 24, "size": 8},
        "Velocity": {"type": "ZMC_LREAL", "offset": 32, "size": 8},
        "Acceleration": {"type": "ZMC_LREAL", "offset": 40, "size": 8},
        "Deceleration": {"type": "ZMC_LREAL", "offset": 48, "size": 8},
        "Jerk": {"type": "ZMC_LREAL", "offset": 56, "size": 8},
        "Busy": {"type": "ZMC_BOOL", "offset": 64, "size": 1},
        "CommandAborted": {"type": "ZMC_BOOL", "offset": 65, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 66, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 68, "size": 2}
      }
    },
    "FbtGearIn": {
      "size": 80, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Master": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Slave": {"type": "AXIS_REF", "offset": 16, "size": 8},
        "Execute": {"type": "ZMC_BOOL", "offset": 24, "size": 1},
        "RatioNumerator": {"type": "ZMC_DINT", "offset": 28, "size": 4},
        "RatioDenominator": {"type": "ZMC_UDINT", "offset": 32, "size": 4},
        "Acceleration": {"type": "ZMC_LREAL", "offset": 40, "size": 8},
        "Deceleration": {"type": "ZMC_LREAL", "offset": 48, "size": 8},
        "Jerk": {"type": "ZMC_LREAL", "offset": 56, "size": 8},
        "BufferMode": {"type": "MC_BUFFER_MODE", "offset": 64, "size": 2},
        "InGear": {"type": "ZMC_BOOL", "offset": 66, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 67, "size": 1},
        "Active": {"type": "ZMC_BOOL", "offset": 68, "size": 1},
        "CommandAborted": {"type": "ZMC_BOOL", "offset": 69, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 70, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 72, "size": 2}
      }
    },
    "FbtGearOut": {
      "size": 24, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Slave": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Execute": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Done": {"type": "ZMC_BOOL", "offset": 17, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 18, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 19, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 20, "size": 2}
      }
    },
    "FbtFollowPosition": {
      "size": 40, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "bExecute": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "fSetPosition": {"type": "ZMC_LREAL", "offset": 24, "size": 8},
        "bBusy": {"type": "ZMC_BOOL", "offset": 32, "size": 1},
        "bCommandAborted": {"type": "ZMC_BOOL", "offset": 33, "size": 1},
        "bError": {"type": "ZMC_BOOL", "offset": 34, "size": 1},
        "iErrorID": {"type": "MC_ERROR", "offset": 36, "size": 2}
      }
    },
    "FbtReadSetValues": {
      "size": 56, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Enable": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Valid": {"type": "ZMC_BOOL", "offset": 17, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 18, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 19, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 20, "size": 2},
        "Position": {"type": "ZMC_LREAL", "offset": 24, "size": 8},
        "Velocity": {"type": "ZMC_LREAL", "offset": 32, "size": 8},
        "Acceleration": {"type": "ZMC_LREAL", "offset": 40, "size": 8},
        "Jerk": {"type": "ZMC_LREAL", "offset": 48, "size": 8}
      }
    },
    "FbtReadActualValues": {
      "size": 56, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Enable": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Valid": {"type": "ZMC_BOOL", "offset": 17, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 18, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 19, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 20, "size": 2},
        "Position": {"type": "ZMC_LREAL", "offset": 24, "size": 8},
        "Velocity": {"type": "ZMC_LREAL", "offset": 32, "size": 8},
        "Acceleration": {"type": "ZMC_LREAL", "offset": 40, "size": 8},
        "Jerk": {"type": "ZMC_LREAL", "offset": 48, "size": 8}
      }
    },
    "FbtReadSetDriveValues": {
      "size": 40, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Enable": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Valid": {"type": "ZMC_BOOL", "offset": 17, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 18, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 19, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 20, "size": 2},
        "Position": {"type": "int", "offset": 24, "size": 4},
        "Velocity": {"type": "int", "offset": 28, "size": 4},
        "Acceleration": {"type": "int", "offset": 32, "size": 4},
        "Jerk": {"type": "int", "offset": 36, "size": 4}
      }
    },
    "FbtGetOverride": {
      "size": 48, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Enable": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Valid": {"type": "ZMC_BOOL", "offset": 17, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 18, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 19, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 20, "size": 2},
        "VelFactor": {"type": "ZMC_LREAL", "offset": 24, "size": 8},
        "AccFactor": {"type": "ZMC_LREAL", "offset": 32, "size": 8},
        "JerkFactor": {"type": "ZMC_LREAL", "offset": 40, "size": 8}
      }
    },
    "FbtSetOverride": {
      "size": 56, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Enable": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "VelFactor": {"type": "ZMC_LREAL", "offset": 24, "size": 8},
        "AccFactor": {"type": "ZMC_LREAL", "offset": 32, "size": 8},
        "JerkFactor": {"type": "ZMC_LREAL", "offset": 40, "size": 8},
        "Enabled": {"type": "ZMC_BOOL", "offset": 48, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 49, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 50, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 52, "size": 2}
      }
    },
 )json"
R"json(
    "FbtHalt": {
      "size": 48, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Execute": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Deceleration": {"type": "ZMC_LREAL", "offset": 24, "size": 8},
        "Jerk": {"type": "ZMC_LREAL", "offset": 32, "size": 8},
        "Done": {"type": "ZMC_BOOL", "offset": 40, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 41, "size": 1},
        "CommandAborted": {"type": "ZMC_BOOL", "offset": 42, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 43, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 44, "size": 2}
      }
    },
    "FbtMoveAbsolute": {
      "size": 80, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Execute": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Position": {"type": "ZMC_LREAL", "offset": 24, "size": 8},
        "Velocity": {"type": "ZMC_LREAL", "offset": 32, "size": 8},
        "Acceleration": {"type": "ZMC_LREAL", "offset": 40, "size": 8},
        "Deceleration": {"type": "ZMC_LREAL", "offset": 48, "size": 8},
        "Jerk": {"type": "ZMC_LREAL", "offset": 56, "size": 8},
        "Direction": {"type": "MC_DIRECTION", "offset": 64, "size": 2},
        "BufferMode": {"type": "MC_BUFFER_MODE", "offset": 66, "size": 2},
        "Done": {"type": "ZMC_BOOL", "offset": 68, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 69, "size": 1},
        "Active": {"type": "ZMC_BOOL", "offset": 70, "size": 1},
        "CommandAborted": {"type": "ZMC_BOOL", "offset": 71, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 72, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 74, "size": 2}
      }
    },
    "FbtMoveAdditive": {
      "size": 72, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Execute": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Distance": {"type": "ZMC_LREAL", "offset": 24, "size": 8},
        "Velocity": {"type": "ZMC_LREAL", "offset": 32, "size": 8},
        "Acceleration": {"type": "ZMC_LREAL", "offset": 40, "size": 8},
        "Deceleration": {"type": "ZMC_LREAL", "offset": 48, "size": 8},
        "Jerk": {"type": "ZMC_LREAL", "offset": 56, "size": 8},
        "Done": {"type": "ZMC_BOOL", "offset": 64, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 65, "size": 1},
        "CommandAborted": {"type": "ZMC_BOOL", "offset": 66, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 67, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 68, "size": 2}
      }
    },
    "FbtMoveRelative": {
      "size": 80, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 8, "size": 8},
        "Execute": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Distance": {"type": "ZMC_LREAL", "offset": 24, "size": 8},
        "Velocity": {"type": "ZMC_LREAL", "offset": 32, "size": 8},
        "Acceleration": {"type": "ZMC_LREAL", "offset": 40, "size": 8},
        "Deceleration": {"type": "ZMC_LREAL", "offset": 48, "size": 8},
        "Jerk": {"type": "ZMC_LREAL", "offset": 56, "size": 8},
        "BufferMode": {"type": "MC_BUFFER_MODE", "offset": 64, "size": 2},
        "Done": {"type": "ZMC_BOOL", "offset": 66, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 67, "size": 1},
        "Active": {"type": "ZMC_BOOL", "offset": 68, "size": 1},
        "CommandAborted": {"type": "ZMC_BOOL", "offset": 69, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 70, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 72, "size": 2}
      }
    },
    "FbtAddAxisToGroup": {
      "size": 32, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "AxisGroup": {"type": "AxisGrpZmotRef", "offset": 8, "size": 8},
        "Axis": {"type": "AXIS_REF", "offset": 16, "size": 8},
        "Execute": {"type": "ZMC_BOOL", "offset": 24, "size": 1},
        "Done": {"type": "ZMC_BOOL", "offset": 25, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 26, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 27, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 28, "size": 2}
      }
    },
    "FbtGroupEnable": {
      "size": 24, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "AxisGroup": {"type": "AxisGrpZmotRef", "offset": 8, "size": 8},
        "Execute": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "Done": {"type": "ZMC_BOOL", "offset": 17, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 18, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 19, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 20, "size": 2}
      }
    },
    "FbtMoveLinear": {
      "size": 136, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "AxisGroup": {"type": "AxisGrpZmotRef", "offset": 8, "size": 8},
        "Execute": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "MotionType": {"type": "MC_BUFFER_MODE", "offset": 18, "size": 2},
        "Position": {"type": "ZMC_POS_REF", "offset": 24, "size": 64},
        "Velocity": {"type": "ZMC_LREAL", "offset": 88, "size": 8},
        "Acceleration": {"type": "ZMC_LREAL", "offset": 96, "size": 8},
        "Deceleration": {"type": "ZMC_LREAL", "offset": 104, "size": 8},
        "Jerk": {"type": "ZMC_LREAL", "offset": 112, "size": 8},
        "BufferMode": {"type": "MC_BUFFER_MODE", "offset": 120, "size": 2},
        "Done": {"type": "ZMC_BOOL", "offset": 122, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 123, "size": 1},
        "Active": {"type": "ZMC_BOOL", "offset": 124, "size": 1},
        "CommandAborted": {"type": "ZMC_BOOL", "offset": 125, "size": 1},
        "CommandAccepted": {"type": "ZMC_BOOL", "offset": 126, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 127, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 128, "size": 2}
      }
    },
    "FbtMoveCircular": {
      "size": 208, "align": 8, "kind": "pod",
      "fields": {
        "pImpl": {"type": "void*", "offset": 0, "size": 8},
        "AxisGroup": {"type": "AxisGrpZmotRef", "offset": 8, "size": 8},
        "Execute": {"type": "ZMC_BOOL", "offset": 16, "size": 1},
        "MotionType": {"type": "MC_BUFFER_MODE", "offset": 18, "size": 2},
        "AuxPoint": {"type": "ZMC_POS_REF", "offset": 24, "size": 64},
        "EndPosition": {"type": "ZMC_POS_REF", "offset": 88, "size": 64},
        "PathChoice": {"type": "MC_BUFFER_MODE", "offset": 152, "size": 2},
        "CircMode": {"type": "MC_BUFFER_MODE", "offset": 154, "size": 2},
        "Velocity": {"type": "ZMC_LREAL", "offset": 160, "size": 8},
        "Acceleration": {"type": "ZMC_LREAL", "offset": 168, "size": 8},
        "Deceleration": {"type": "ZMC_LREAL", "offset": 176, "size": 8},
        "Jerk": {"type": "ZMC_LREAL", "offset": 184, "size": 8},
        "BufferMode": {"type": "MC_BUFFER_MODE", "offset": 192, "size": 2},
        "Done": {"type": "ZMC_BOOL", "offset": 194, "size": 1},
        "Busy": {"type": "ZMC_BOOL", "offset": 195, "size": 1},
        "Active": {"type": "ZMC_BOOL", "offset": 196, "size": 1},
        "CommandAborted": {"type": "ZMC_BOOL", "offset": 197, "size": 1},
        "CommandAccepted": {"type": "ZMC_BOOL", "offset": 198, "size": 1},
        "Error": {"type": "ZMC_BOOL", "offset": 199, "size": 1},
        "ErrorID": {"type": "MC_ERROR", "offset": 200, "size": 2}
      }
    }
  }
}
)json";

#endif
