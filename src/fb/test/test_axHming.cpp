/*
 * Script-driven POU regression and motion tests for FbtHome and FbtHoming.
 * @ArtifactStatus draft
 * @ArtifactStatusDate 2026-09-02
 * @ArtifactStatusAuthority GPT-5 Codex
 */

#include <gtest/gtest.h>

#include <cstddef>
#include <string>

#include "AxPou.hpp"
#include "axPouTmplRunner.hpp"
#include "common/misc/pou_base.hpp"
#include "fb_cpp_def.h"
#include "fbt_axis_morph.h"
#include "mc_parameter_enum_def.h"
#include "memory/MemoryPouEngine.hpp"
#include "memory/MemoryPouTypeLayout.hpp"

using namespace zmc;

namespace {

#define MEMORY_POU_HOMING_FIELDS          \
    AXIS_REF axis;                        \
    MC_DIRECTION direction;               \
    MC_HOMING_MODE mode;                  \
    ZMC_DINT factorAccParameter;           \
    ZMC_DINT controllerModeParameter;      \
    ZMC_DINT hwLimitEnableParameter;       \
    ZMC_BOOL simulateReferenceSwitch;     \
    ZMC_LREAL refCenter;                  \
    ZMC_LREAL refWidth;                   \
    FbtPower power;                       \
    FbtSetOverride setOverride;           \
    FbtHome oldHome;                      \
    FbtHome newHome;                      \
    FbtHoming oldHoming;                  \
    FbtHoming newHoming;                  \
    FbtReadParameter readFactorAcc;        \
    FbtReadParameter readControllerMode;   \
    FbtReadBoolParameter readHwLimitEnable; \
    FbtWriteBoolParameter writeHwLimitEnable; \
    FbtReadActualPosition readPos;        \
    FbtReadStatus readStatus;             \
    FbtReadAxisError readAxisError

static const char kMemoryPouHomingInitConfig[] = R"(
{
    AXIS_REF axis;
    MC_DIRECTION direction;
    MC_HOMING_MODE mode;
    ZMC_DINT factorAccParameter;
    ZMC_DINT controllerModeParameter;
    ZMC_DINT hwLimitEnableParameter;
    ZMC_BOOL simulateReferenceSwitch;
    ZMC_LREAL refCenter;
    ZMC_LREAL refWidth;
    FbtPower power;
    FbtSetOverride setOverride;
    FbtHome oldHome;
    FbtHome newHome;
    FbtHoming oldHoming;
    FbtHoming newHoming;
    FbtReadParameter readFactorAcc;
    FbtReadParameter readControllerMode;
    FbtReadBoolParameter readHwLimitEnable;
    FbtWriteBoolParameter writeHwLimitEnable;
    FbtReadActualPosition readPos;
    FbtReadStatus readStatus;
    FbtReadAxisError readAxisError;
};
)";

static const char kMemoryPouHomingCommonScript[] = R"(
power.Axis = axis;
setOverride.Axis = axis;
oldHome.Axis = axis;
newHome.Axis = axis;
oldHoming.Axis = axis;
newHoming.Axis = axis;
readFactorAcc.Axis = axis;
readControllerMode.Axis = axis;
readHwLimitEnable.Axis = axis;
writeHwLimitEnable.Axis = axis;
readPos.Axis = axis;
readStatus.Axis = axis;
readAxisError.Axis = axis;
power.Enable = true;
power.bRegulatorOn = true;
power.bDriveStart = true;
setOverride.VelFactor = 1;
setOverride.AccFactor = 1;
setOverride.JerkFactor = 1;
setOverride.Enable = false;
readFactorAcc.ParameterNumber = factorAccParameter;
readFactorAcc.Enable = true;
readControllerMode.ParameterNumber = controllerModeParameter;
readControllerMode.Enable = true;
readHwLimitEnable.ParameterNumber = hwLimitEnableParameter;
readHwLimitEnable.Enable = true;
writeHwLimitEnable.ParameterNumber = hwLimitEnableParameter;
readPos.Enable = true;
readStatus.Enable = true;
readAxisError.Enable = true;
oldHome.Position = 0;
newHome.Position = 0;
oldHoming.fHomePosition = 0;
oldHoming.fVelocitySlow = 10;
oldHoming.fVelocityFast = 20;
oldHoming.fAcceleration = 100;
oldHoming.fDeceleration = 100;
oldHoming.fJerk = 0;
oldHoming.fSignalDelay = 0;
oldHoming.bReturnToZero = false;
oldHoming.nDirection = direction;
oldHoming.nHomingMode = mode;
oldHoming.bReferenceSwitch = true;
newHoming.fHomePosition = 0;
newHoming.fVelocitySlow = 10;
newHoming.fVelocityFast = 20;
newHoming.fAcceleration = 100;
newHoming.fDeceleration = 100;
newHoming.fJerk = 0;
newHoming.fSignalDelay = 0;
newHoming.bReturnToZero = false;
newHoming.nDirection = direction;
newHoming.nHomingMode = mode;
newHoming.bReferenceSwitch = true;
)";

class MemoryPouHomingTask : public PouTaskBase {
   public:
    typedef struct PouData {
        MEMORY_POU_HOMING_FIELDS;
    } PouData;

    MemoryPouHomingTask(const std::string& script, MC_HOMING_MODE modeValue)
        : axis(NULL),
          direction(mcNegativeDirection),
          mode(modeValue),
          factorAccParameter(PARA_fFactorAcc),
          controllerModeParameter(PARA_byControllerMode),
          hwLimitEnableParameter(PARA_bHWLimitEnable),
          simulateReferenceSwitch(false),
          refCenter(0.0),
          refWidth(0.0),
          errorCode_(MEMOP_OK) {
        axis = AxPouTaskBinderAuto::getAxisByIndex(0);
        if (axis == NULL) {
            errorCode_ = MEMOP_ERR_INIT;
            return;
        }

        errorCode_ = engine_.loadTypeLayout(kMemoryPouTypeLayout);
        if (errorCode_ != MEMOP_OK) {
            return;
        }
        errorCode_ = engine_.configure(kMemoryPouHomingInitConfig);
        if (errorCode_ != MEMOP_OK) {
            return;
        }
        if (engine_.memoryOperator().rootSize() != sizeof(PouData)) {
            errorCode_ = MEMOP_ERR_LAYOUT;
            return;
        }
        engine_.bindMemory(&axis, sizeof(PouData));
        errorCode_ = engine_.parseScheduledScript(script);
        if (errorCode_ != MEMOP_OK) {
            return;
        }
    }

    int errorCode() const {
        if (errorCode_ != MEMOP_OK) {
            return errorCode_;
        }
        return engine_.errorCode();
    }

   protected:
    void runCycle() override {
        engine_.cmdAdvance();

        power.runCycle();
        setOverride.runCycle();
        writeHwLimitEnable.runCycle();
        readFactorAcc.runCycle();
        readControllerMode.runCycle();
        readHwLimitEnable.runCycle();
        readPos.runCycle();
        readStatus.runCycle();
        readAxisError.runCycle();

        if (simulateReferenceSwitch) {
            // Simulate the reference switch from the public actual-position FB output.
            const double pos = readPos.Position;
            const bool inWindow =
                pos >= refCenter - refWidth && pos <= refCenter + refWidth;
            (void)inWindow;
            oldHoming.bReferenceSwitch = pos > refCenter;
            newHoming.bReferenceSwitch = pos > refCenter;
        }

        // Keep the established command order so an already-running FB observes a new request.
        oldHome.runCycle();
        oldHoming.runCycle();
        newHome.runCycle();
        newHoming.runCycle();
    }

   private:
    MEMORY_POU_HOMING_FIELDS;
    MemoryPouEngine engine_;
    int errorCode_;
};

#undef MEMORY_POU_HOMING_FIELDS

std::string makeHomingScript(const std::string& scenarioScript) {
    return std::string(kMemoryPouHomingCommonScript) + scenarioScript;
}

std::string expectErrorCode(const char* symbol, unsigned long errorCode) {
    return std::string("expect ") + symbol + " = " + std::to_string(errorCode) + ";\n";
}

std::string expectPositionRange(double expectedMin, double expectedMax) {
    return std::string("expect readPos.Position >= ") + std::to_string(expectedMin) +
           ";\nexpect readPos.Position <= " + std::to_string(expectedMax) + ";\n";
}

std::string makeMotionScript(double expectedMin, double expectedMax) {
    std::string script = makeHomingScript(R"(
simulateReferenceSwitch = true;
refCenter = 2;
refWidth = 0.5;
setOverride.VelFactor = 1;
setOverride.AccFactor = 0.5;
setOverride.JerkFactor = 1;
setOverride.Enable = true;
delay 6;
expect power.Status = true;
expect setOverride.Enabled = true;
expect setOverride.Busy = true;
expect setOverride.Error = false;
expect readFactorAcc.Valid = true;
expect readFactorAcc.Error = false;
expect readFactorAcc.Value = 0.5;
expect readPos.Valid = true;
oldHoming.bExecute = true;
delay 20;
expect oldHoming.bBusy = true;
expect readStatus.Homing = true;
expect readControllerMode.Value = 3;
expect readPos.Position > 0.5;
delay 160;
expect oldHoming.bDone = true;
expect oldHoming.bError = false;
expect readStatus.Standstill = true;
expect readAxisError.AxisError = false;
expect readControllerMode.Value = 3;
)");
    script += expectPositionRange(expectedMin, expectedMax);
    script += R"(
delay 3;
expect oldHoming.bDone = true;
expect readStatus.Standstill = true;
)";
    script += expectPositionRange(expectedMin, expectedMax);
    return script;
}

int runHomingScenario(const std::string& script, int cycles,
                      MC_HOMING_MODE mode = FAST_BSLOW_S_STOP) {
    char* argv[] = {const_cast<char*>("test_axHming"), NULL};
    AxPouTaskBinderAuto* runner = make_pou_config<AxPouTaskBinderAuto>(
        1, argv, iniContentDefault, PouTmplDefaults{{"cycles", "1"}});
    if (runner == NULL) {
        return MEMOP_ERR_INIT;
    }
    runner->set_cycles(cycles);
    runner->set_verbose_flag(0);

    MemoryPouHomingTask task(script, mode);
    int rc = task.errorCode();
    if (rc == MEMOP_OK) {
        runner->bind(0, &task);
        runner->run();
        rc = task.errorCode();
    }

    delete runner;
    return rc;
}

}  // namespace

TEST(MemoryPouHomingLayout, public_fb_types_match_cpp_pods) {
    ASSERT_EQ(40u, sizeof(FbtHome));
    ASSERT_EQ(120u, sizeof(FbtHoming));
    ASSERT_EQ(40u, sizeof(FbtReadParameter));
    ASSERT_EQ(32u, sizeof(FbtReadBoolParameter));
    ASSERT_EQ(32u, sizeof(FbtWriteBoolParameter));
    ASSERT_EQ(56u, sizeof(FbtSetOverride));
    ASSERT_EQ(32u, sizeof(FbtReadActualPosition));
    ASSERT_EQ(40u, sizeof(FbtReadStatus));
    ASSERT_EQ(32u, sizeof(FbtReadAxisError));
}

TEST(MemoryPouHomingParameters, bool_parameter_write_and_read_round_trip) {
    const int rc = runHomingScenario(
        makeHomingScript(R"(
delay 6;
expect power.Status = true;
expect readHwLimitEnable.Valid = true;
writeHwLimitEnable.Value = false;
writeHwLimitEnable.Execute = true;
delay 2;
expect writeHwLimitEnable.Done = true;
expect writeHwLimitEnable.Error = false;
expect readHwLimitEnable.Value = false;
writeHwLimitEnable.Execute = false;
delay 1;
writeHwLimitEnable.Value = true;
writeHwLimitEnable.Execute = true;
delay 2;
expect writeHwLimitEnable.Done = true;
expect writeHwLimitEnable.Error = false;
expect readHwLimitEnable.Value = true;
)"),
        12);
    ASSERT_EQ(MEMOP_OK, rc);
}

TEST(homing_replacement, home_and_homing_publish_their_own_busy_outputs) {
    const int homeRc = runHomingScenario(
        makeHomingScript(R"(
delay 6;
expect power.Status = true;
expect readFactorAcc.Valid = true;
expect readFactorAcc.Value = 1;
oldHome.Execute = true;
delay 4;
expect oldHome.Busy = true;
expect oldHoming.bBusy = false;
expect readStatus.Homing = true;
expect readControllerMode.Value = 5;
)"),
        11);
    ASSERT_EQ(MEMOP_OK, homeRc);

    const int homingRc = runHomingScenario(
        makeHomingScript(R"(
delay 6;
expect power.Status = true;
expect readFactorAcc.Value = 1;
oldHoming.bExecute = true;
delay 2;
expect oldHome.Busy = false;
expect oldHoming.bBusy = true;
expect readStatus.Homing = true;
expect readControllerMode.Value = 3;
)"),
        9);
    ASSERT_EQ(MEMOP_OK, homingRc);
}

TEST(homing_replacement, software_homing_replaces_software_homing) {
    const int rc = runHomingScenario(
        makeHomingScript(R"(
delay 6;
expect power.Status = true;
expect readFactorAcc.Value = 1;
oldHoming.bExecute = true;
delay 2;
newHoming.bExecute = true;
delay 2;
expect oldHoming.bCommandAborted = true;
expect newHoming.bBusy = true;
expect readStatus.Homing = true;
expect readControllerMode.Value = 3;
)"),
        11);
    ASSERT_EQ(MEMOP_OK, rc);
}

TEST(homing_replacement, invalid_replacement_requests_axis_errorstop) {
    std::string script = makeHomingScript(R"(
delay 6;
expect power.Status = true;
expect readFactorAcc.Value = 1;
oldHoming.bExecute = true;
delay 2;
newHoming.fVelocityFast = 0;
newHoming.bExecute = true;
delay 3;
expect oldHoming.bCommandAborted = true;
expect newHoming.bError = true;
expect readStatus.ErrorStop = true;
expect readAxisError.AxisError = true;
expect readFactorAcc.Value = 1;
)" );
    script += expectErrorCode("newHoming.nErrorID", ZMC_INVALID_VELACCJERK_VALUES);
    script += expectErrorCode("readAxisError.AxisErrorID", ZMC_INVALID_VELACCJERK_VALUES);

    ASSERT_EQ(MEMOP_OK, runHomingScenario(script, 12));
}

TEST(homing_replacement, invalid_start_rolls_back_without_axis_error) {
    std::string script = makeHomingScript(R"(
delay 6;
expect power.Status = true;
expect readFactorAcc.Value = 1;
newHoming.fVelocityFast = 0;
newHoming.bExecute = true;
delay 2;
expect newHoming.bError = true;
expect readStatus.Standstill = true;
expect readStatus.ErrorStop = false;
expect readAxisError.AxisError = false;
expect readControllerMode.Value = 3;
)" );
    script += expectErrorCode("newHoming.nErrorID", ZMC_INVALID_VELACCJERK_VALUES);

    ASSERT_EQ(MEMOP_OK, runHomingScenario(script, 9));
}

TEST(homing_replacement, software_homing_cannot_replace_drive_home) {
    std::string script = makeHomingScript(R"(
delay 6;
expect power.Status = true;
expect readFactorAcc.Value = 1;
oldHome.Execute = true;
delay 4;
newHoming.bExecute = true;
delay 2;
expect oldHome.Busy = true;
expect newHoming.bError = true;
expect readStatus.Homing = true;
expect readControllerMode.Value = 5;
)" );
    script += expectErrorCode("newHoming.nErrorID", ZMC_ERR_H_AXIS_IN_MOVING);

    ASSERT_EQ(MEMOP_OK, runHomingScenario(script, 13));
}

TEST(homing_replacement, drive_home_cannot_replace_software_homing) {
    std::string script = makeHomingScript(R"(
delay 6;
expect power.Status = true;
expect readFactorAcc.Value = 1;
oldHoming.bExecute = true;
delay 2;
newHome.Execute = true;
delay 2;
expect oldHoming.bBusy = true;
expect newHome.Error = true;
expect readStatus.Homing = true;
expect readControllerMode.Value = 3;
)" );
    script += expectErrorCode("newHome.ErrorID", ZMC_ERR_H_AXIS_IN_MOVING);

    ASSERT_EQ(MEMOP_OK, runHomingScenario(script, 11));
}

TEST(homing_replacement, drive_home_reentry_keeps_first_home_busy) {
    std::string script = makeHomingScript(R"(
delay 6;
expect power.Status = true;
expect readFactorAcc.Value = 1;
oldHome.Execute = true;
delay 4;
newHome.Execute = true;
delay 2;
expect oldHome.Busy = true;
expect newHome.Error = true;
expect readStatus.Homing = true;
expect readControllerMode.Value = 5;
)" );
    script += expectErrorCode("newHome.ErrorID", ZMC_ERR_H_AXIS_IN_MOVING);

    ASSERT_EQ(MEMOP_OK, runHomingScenario(script, 13));
}

TEST(MemoryPouHomingMotion, supported_modes_complete_real_motion) {
    // S_STOP modes retain the stopping distance after SetPosition; STOP_S modes do not.
    struct ModeCase {
        MC_HOMING_MODE mode;
        const char* name;
        double expectedMin;
        double expectedMax;
    };
    const ModeCase cases[] = {
        {FAST_BSLOW_S_STOP, "FAST_BSLOW_S_STOP", 0.49, 0.51},
        {FAST_BSLOW_STOP_S, "FAST_BSLOW_STOP_S", -0.01, 0.01},
        {FAST_SLOW_S_STOP, "FAST_SLOW_S_STOP", 0.49, 0.51},
        {FAST_SLOW_STOP_S, "FAST_SLOW_STOP_S", -0.01, 0.01},
    };

    for (std::size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        SCOPED_TRACE(cases[i].name);
        const int rc = runHomingScenario(
            makeMotionScript(cases[i].expectedMin, cases[i].expectedMax), 195,
            cases[i].mode);
        ASSERT_EQ(MEMOP_OK, rc);
    }
}
