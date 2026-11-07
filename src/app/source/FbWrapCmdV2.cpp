#include "FbWrapCmdV2.h"

#include "common/misc/CommandInteractor.h"
#include "common/misc/CommandParser.h"
#include "common/utils/double2frac.hpp"
#include "fbt_errorcode_def.h"
#include "qcmd/MotionCmdInterface.hpp"
#include "motbuf/MotionCmdQueueBase.hpp"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace zmc {

namespace {

struct CommandParserContext {
    MotionCmdInterface& cmd;
    std::mutex* mutex;
    int base;
};

template <typename F>
int call_cmd_locked(std::mutex* cmdMutex, F fn) {
    if (cmdMutex) {
        std::lock_guard<std::mutex> lock(*cmdMutex);
        return fn();
    }
    return fn();
}

template <typename F>
void run_cmd_locked(std::mutex* cmdMutex, F fn) {
    if (cmdMutex) {
        std::lock_guard<std::mutex> lock(*cmdMutex);
        fn();
        return;
    }
    fn();
}

void print_motion_ret(int ret) {
    if (ret == static_cast<int>(ZMC_NO_ERROR)) {
        printf("OK\n");
    } else if (ret == static_cast<int>(ZMC_ERR_MOTCMD_SCRIPT_BUSY_DISCARD)) {
        printf("Script busy\n");
    } else if (ret == static_cast<int>(ZMC_ERR_MOTCMD_ABORT_PUSH_FAILED)) {
        printf("FAIL: abort push failed\n");
    } else {
        printf("FAIL: %d\n", ret);
    }
}

void pump_cmd_heartbeat(MotionCmdInterface& cmdIf, std::mutex& cmdMutex,
                        std::atomic<bool>& stopFlag) {
    while (!stopFlag.load()) {
        {
            std::lock_guard<std::mutex> lock(cmdMutex);
            cmdIf.heartbeat();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

}  // namespace

inline int CommandParserBindCmdIf(CommandParser &cps, CommandParserContext &c) {
    cps.bind("--help", [&cps]() { cps.showHelp(); });

    // ====================================================================
    // Motion Commands
    // ====================================================================

    cps.bind("moveabs", [&c](double pos) {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.moveAbs(c.base, pos); });
        printf("moveabs %f ", pos);
        print_motion_ret(ret);
    });

    cps.bind("moverel", [&c](double pos) {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.moveRel(c.base, pos); });
        printf("moverel %f ", pos);
        print_motion_ret(ret);
    });

    cps.bind("move", [&c](double pos) {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.moveRel(c.base, pos); });
        printf("move %f ", pos);
        print_motion_ret(ret);
    });

    cps.bind("moveadd", [&c](double pos) {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.moveAdd(c.base, pos); });
        printf("moveadd %f ", pos);
        print_motion_ret(ret);
    });

    cps.bind("vmove", [&c](int dir) {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.moveVelocity(c.base, dir); });
        printf("vmove %d ", dir);
        print_motion_ret(ret);
    });

    cps.bind("forward", [&c]() {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.forward(c.base); });
        printf("forward ");
        print_motion_ret(ret);
    });

    cps.bind("reverse", [&c]() {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.reverse(c.base); });
        printf("reverse ");
        print_motion_ret(ret);
    });

    // ====================================================================
    // Control Commands
    // ====================================================================

    cps.bind("stop", [&c]() {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.stop(c.base); });
        printf("stop ");
        print_motion_ret(ret);
    });

    cps.bind("halt", [&c]() {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.halt(c.base); });
        printf("halt ");
        print_motion_ret(ret);
    });

    cps.bind("reset", [&c]() {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.reset(c.base); });
        printf("reset ");
        print_motion_ret(ret);
    });

    cps.bind("home", [&c](const std::vector<std::string> &args) {
        double pos = 0.0;
        if (!args.empty()) {
            int errCode = 0;
            pos = convertArg<double>(args[0], errCode);
            if (errCode) pos = 0.0;
        }
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.home(c.base, pos); });
        printf("home %f ", pos);
        print_motion_ret(ret);
    });

    cps.bind("homing", [&c]() {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.homing(c.base); });
        printf("homing ");
        print_motion_ret(ret);
    });

    cps.bind("datum", [&c](int mode) {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.datum(c.base, mode); });
        printf("datum %d ", mode);
        print_motion_ret(ret);
    });

    cps.bind("defpos", [&c](double pos) {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.setPosition(c.base, pos); });
        printf("defpos %f ", pos);
        print_motion_ret(ret);
    });

    // ====================================================================
    // Power Commands
    // ====================================================================

    cps.bind("powerOn", [&c]() {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.powerOn(c.base); });
        printf("powerOn ");
        print_motion_ret(ret);
    });

    cps.bind("powerOff", [&c]() {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.powerOff(c.base); });
        printf("powerOff ");
        print_motion_ret(ret);
    });

    cps.bind("axis_enable", [&c](int enable) {
        int ret =
            call_cmd_locked(c.mutex, [&]() { return c.cmd.axisEnable(c.base, enable != 0); });
        printf("axis_enable %d ", enable);
        print_motion_ret(ret);
    });

    // ====================================================================
    // Parameter Commands
    // ====================================================================

    cps.bind("speed", [&c](double v) {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.setSpeed(c.base, v); });
        printf("speed %f ", v);
        print_motion_ret(ret);
    });

    cps.bind("accel", [&c](double v) {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.setAccel(c.base, v); });
        printf("accel %f ", v);
        print_motion_ret(ret);
    });

    cps.bind("decel", [&c](double v) {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.setDecel(c.base, v); });
        printf("decel %f ", v);
        print_motion_ret(ret);
    });

    cps.bind("sparam", [&c](double v) {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.setJerk(c.base, v); });
        printf("sparam %f ", v);
        print_motion_ret(ret);
    });

    cps.bind("fastdecel", [&c](double v) {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.setFastDecel(c.base, v); });
        printf("fastdecel %f ", v);
        print_motion_ret(ret);
    });

    cps.bind("units", [&c](double v) {
        int ret = call_cmd_locked(c.mutex, [&]() {
            // The legacy script keyword keeps fractional double input.
            // GLM-5.3 (opencode), 2026-10-23.
            NumDenUnion numDen = {{1, 1}};
            doubleToFraction(v, numDen.num, numDen.den);
            return c.cmd.setUnitsNumDen(c.base, numDen.num, numDen.den);
        });
        printf("units %f ", v);
        print_motion_ret(ret);
    });

    cps.bind("direction", [&c](int v) {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.setDirection(c.base, v); });
        printf("direction %d ", v);
        print_motion_ret(ret);
    });

    cps.bind("buffermode", [&c](int v) {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.setBufferMode(c.base, v); });
        printf("buffermode %d ", v);
        print_motion_ret(ret);
    });

    // ====================================================================
    // Axis Selection Commands
    // ====================================================================

    cps.bind("base", [&c](int axis) {
        c.base = axis;
        printf("base %d ", axis);
        print_motion_ret(0);
    });

    cps.bind("axis", [&c](int axis) {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.setAxis(axis); });
        printf("axis %d ", axis);
        print_motion_ret(ret);
    });

    cps.bind("master", [&c](int axis) {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.setMasterAxis(c.base, axis); });
        printf("master %d ", axis);
        print_motion_ret(ret);
    });

    // ====================================================================
    // Gear Commands
    // ====================================================================

    cps.bind("gear_in", [&c]() {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.gearIn(c.base); });
        printf("gear_in ");
        print_motion_ret(ret);
    });

    cps.bind("gear_out", [&c]() {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.gearOut(c.base); });
        printf("gear_out ");
        print_motion_ret(ret);
    });

    // ====================================================================
    // Status Query Commands
    // ====================================================================

    cps.bind("show", [&c](const std::vector<std::string> &args) {
        int axis = 0;
        if (!args.empty()) {
            int errCode = 0;
            axis = convertArg<int>(args[0], errCode);
            if (errCode) axis = 0;
        }
        run_cmd_locked(c.mutex, [&]() {
            c.cmd.updateStatusCache();
            AxisState st;
            if (c.cmd.getAxisState(axis, st) == 0) {
                printf("AxisState[%d]: state=%d err=%d set_pos=%.4f act_pos=%.4f tick=%u\n",
                       st.axis_id, st.axis_state, st.error_id, st.set_pos, st.act_pos,
                       st.tick);
            } else {
                printf("AxisState[%d]: N/A\n", axis);
            }
        });
    });

    cps.bind("p", [&c]() {
        run_cmd_locked(c.mutex, [&]() {
            c.cmd.updateStatusCache();
            AxisState st;
            if (c.cmd.getAxisState(0, st) == 0) {
                printf("AxisState[%d]: state=%d err=%d set_pos=%.4f act_pos=%.4f tick=%u\n",
                       st.axis_id, st.axis_state, st.error_id, st.set_pos, st.act_pos,
                       st.tick);
            }
        });
    });

    cps.bind("state", [&c](const std::vector<std::string> &args) {
        int axis = 0;
        if (!args.empty()) {
            int errCode = 0;
            axis = convertArg<int>(args[0], errCode);
            if (errCode) axis = 0;
        }
        run_cmd_locked(c.mutex, [&]() {
            c.cmd.updateStatusCache();
            AxisState2 st2;
            if (c.cmd.getAxisState2(axis, st2) == 0) {
                printf(
                    "AxisState2[%d]: state=%d err=%d set_pos=%.4f act_pos=%.4f vel=%.4f "
                    "acc=%.4f raw_set=%d raw_act=%d tick=%u\n",
                    st2.axis_id, st2.axis_state, st2.error_id, st2.set_pos, st2.act_pos,
                    st2.set_vel, st2.set_acc, st2.set_position, st2.act_position, st2.tick);
            } else {
                printf("AxisState2[%d]: N/A\n", axis);
            }
        });
    });

    cps.bind("cmdresult", [&c]() {
        run_cmd_locked(c.mutex, [&]() {
            c.cmd.updateStatusCache();
            std::vector<CmdResultInfo> results;
            if (c.cmd.getCmdResults(results) == 0) {
                for (const auto &r : results) {
                    printf("CmdResult id=%u cmd=%u kind=%u target=%u status=%u err=%u "
                           "count=%u tick=%u data=%d data_ext=%d\n",
                           r.id, r.command_type, r.target_kind, r.target_index, r.status,
                           r.error_code, r.io_count, r.tick, r.data, r.data_ext);
                }
            } else {
                printf("CmdResult: empty\n");
            }
        });
    });

    cps.bind("flow", [&c]() {
        run_cmd_locked(c.mutex, [&]() {
            c.cmd.updateStatusCache();
            FlowCtrlStatus fs;
            int ret = c.cmd.getFlowStatus(fs);
            if (ret == static_cast<int>(ZMC_NO_ERROR)) {
                printf(
                    "FlowStatus: period=%u batchSize=%u verboseFlag=%u latency=%u "
                    "axis_num=%u cycleTime=%u\n",
                    fs.period, fs.batchSize, fs.verboseFlag, fs.latency, fs.axis_num,
                    fs.cycleTime);
            } else {
                printf("flow ");
                print_motion_ret(ret);
            }
        });
    });

    cps.bind("flow_config", [&c](const std::vector<std::string> &args) {
        if (args.empty()) {
            run_cmd_locked(c.mutex, [&]() {
                FlowCtrlConfig cfg;
                int ret = c.cmd.getFlowConfig(cfg);
                if (ret == static_cast<int>(ZMC_NO_ERROR)) {
                    printf("FlowConfig: period=%u batchSize=%u verboseFlag=%u\n", cfg.period,
                           cfg.batchSize, cfg.verboseFlag);
                } else {
                    printf("flow_config ");
                    print_motion_ret(ret);
                }
            });
            return;
        }
        int ret = call_cmd_locked(c.mutex, [&]() {
            FlowCtrlConfig cfg;
            int cfgRet = c.cmd.getFlowConfig(cfg);
            if (cfgRet != static_cast<int>(ZMC_NO_ERROR)) {
                return cfgRet;
            }
            for (size_t i = 0; i + 1 < args.size(); i += 2) {
                const std::string &key = args[i];
                int errCode = 0;
                if (key == "period") {
                    cfg.period = static_cast<uint32_t>(convertArg<int>(args[i + 1], errCode));
                } else if (key == "batchSize") {
                    cfg.batchSize = static_cast<uint32_t>(convertArg<int>(args[i + 1], errCode));
                } else if (key == "verboseFlag") {
                    cfg.verboseFlag = static_cast<uint8_t>(convertArg<int>(args[i + 1], errCode));
                }
            }
            return c.cmd.setFlowConfig(cfg);
        });
        printf("flow_config ");
        print_motion_ret(ret);
    });

    // ====================================================================
    // Blocking Wait Commands
    // ====================================================================

    cps.bind("wait_tick", [&c](int nTick) {
        int ret = call_cmd_locked(c.mutex,
                                  [&]() { return c.cmd.waitTicks(static_cast<uint32_t>(nTick)); });
        printf("wait_tick %d -> %d\n", nTick, ret);
    });

    cps.bind("wait_time", [&c](double seconds) {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.waitSeconds(seconds); });
        printf("wait_time %f -> %d\n", seconds, ret);
    });

    cps.bind("wait_idle", [&c](const std::vector<std::string> &args) {
        int axis = 0;
        int timeoutMs = 0;
        if (!args.empty()) {
            int errCode = 0;
            axis = convertArg<int>(args[0], errCode);
            if (errCode) axis = 0;
        }
        if (args.size() >= 2) {
            int errCode = 0;
            timeoutMs = convertArg<int>(args[1], errCode);
            if (errCode) timeoutMs = 0;
        }
        int ret = call_cmd_locked(
            c.mutex, [&]() { return c.cmd.waitAxisIdle(axis, static_cast<uint32_t>(timeoutMs)); });
        printf("wait_idle axis=%d timeoutMs=%d -> %d\n", axis, timeoutMs, ret);
    });

    cps.bind("autoflush", [&c](int enable) {
        run_cmd_locked(c.mutex, [&]() { c.cmd.setAutoFlushEnabled(enable != 0); });
        printf("autoflush %d\n", enable ? 1 : 0);
    });

    // ====================================================================
    // Flush & Exit
    // ====================================================================

    cps.bind("flush", [&c]() {
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.flush(); });
        printf("flush ");
        print_motion_ret(ret);
    });

    cps.bind("exit", [&cps]() {
        printf("exit\n");
        cps.setExitFlag();
    });

    // ====================================================================
    // Fallback: forward unknown commands to executeCommand
    // ====================================================================

    cps.bind("exec", [&c](const std::vector<std::string> &args) {
        std::string line;
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) line += ' ';
            line += args[i];
        }
        int ret = call_cmd_locked(c.mutex, [&]() { return c.cmd.executeCommand(line.c_str()); });
        printf("exec %s ", line.c_str());
        print_motion_ret(ret);
    });

    return 0;
}

int runCmdLoopV2(MotionCmdQueueBase &cmdBase) {
  return runCmdLoopV2(cmdBase, "", false);
}

int runCmdLoopV2(MotionCmdQueueBase& cmdBase, const std::string& startupScript, bool scriptExit) {
    MotionCmdInterface cmdIf(cmdBase);

    std::mutex cmdMutex;
    if (!startupScript.empty()) {
        printf("[cmd-script] startup script: enqueue\n");
        const int ret =
            call_cmd_locked(&cmdMutex,
                            [&]() { return cmdIf.executeCommand(startupScript.c_str()); });
        if (ret != 0) {
            printf("[cmd-script] startup script enqueue failed, ret=%d\n", ret);
            cmdIf.sendExitFlag();
            return ret;
        }
    }

    std::atomic<bool> pumpStop(false);
    std::thread pumpThread([&]() { pump_cmd_heartbeat(cmdIf, cmdMutex, pumpStop); });

    if (scriptExit && !startupScript.empty()) {
        printf("[cmd-script] waiting startup script to finish\n");
        while (true) {
            const int busy = call_cmd_locked(&cmdMutex, [&]() {
                return cmdIf.isScriptBusy() ? 1 : 0;
            });
            if (!busy) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        printf("[cmd-script] startup script finished\n");
    } else {
        CommandParser cps;
        CommandParserContext context = {cmdIf, &cmdMutex, 0};
        CommandInteractor<CommandParser> clp(cps);
        CommandParserBindCmdIf(cps, context);
        clp.setErrorHandlerDefault();
        clp.run();
    }

    pumpStop.store(true);
    if (pumpThread.joinable()) {
        pumpThread.join();
    }

    cmdIf.sendExitFlag();
    return 0;
}

}  // namespace zmc
