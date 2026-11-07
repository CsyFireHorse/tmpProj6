/*
 * MemoryPou expect comparison operators and power/move integration tests.
 * @ArtifactStatus garbage
 * @ArtifactStatusDate 2026-09-01
 * @ArtifactStatusAuthority user-request
 * AI modification signature: Cursor Grok 4.6
 */
#include <gtest/gtest.h>

#include "AxPou.hpp"
#include "axPouTmplRunner.hpp"
#include "common/misc/pou_base.hpp"
#include "fb_cpp_def.h"
#include "fbt_axis_morph.h"
#include "memory/MemoryPouEngine.hpp"
#include "memory/MemoryPouTypeLayout.hpp"

using namespace zmc;

namespace {

#define MEMORY_POU_TEST_FIELDS     \
    AXIS_REF axis;                 \
    FbtPower power;                \
    FbtHalt halt;                  \
    FbtMoveRelative moveRel;       \
    FbtMoveAdditive moveAdd;       \
    FbtMoveAbsolute move;          \
    FbtReadActualPosition readPos; \
    FbtReadStatus readStatus

static const char kMemoryPouTestInitConfig[] = R"(
{
    AXIS_REF axis;
    FbtPower power;
    FbtHalt halt;
    FbtMoveRelative moveRel;
    FbtMoveAdditive moveAdd;
    FbtMoveAbsolute move;
    FbtReadActualPosition readPos;
    FbtReadStatus readStatus;
};
)";

static const char kMemoryPouPowerMoveScript[] =
    R"(
power.Enable = true;
power.bRegulatorOn = true;
power.bDriveStart = true;
readPos.Enable = true;
readStatus.Enable = true;
move.Position = 15;
move.Velocity = 20;
move.Acceleration = 40;
move.Deceleration = 40;
delay 6;
expect power.Status = true;
move.Execute = true;
delay 1;
expect readPos.Position = 0;
expect readStatus.Valid = true;
expect readStatus.DiscreteMotion = true;
delay 150;
expect move.Done == true;
expect readPos.Valid = true;
expect readPos.Position >= 15;
expect readStatus.Standstill = true;
)";

/* Self-contained PouTask wrapper (same shape as demo MemoryPouTask). */
class MemoryPouPowerMoveTask : public PouTaskBase {
   public:
    typedef struct PouData {
        MEMORY_POU_TEST_FIELDS;
    } PouData;

    MemoryPouPowerMoveTask() : errorCode_(MEMOP_OK) {
        axis = AxPouTaskBinderAuto::getAxisByIndex(0);

        errorCode_ = engine_.loadTypeLayout(kMemoryPouTypeLayout);
        if (errorCode_ != MEMOP_OK) {
            return;
        }
        errorCode_ = engine_.configure(kMemoryPouTestInitConfig);
        if (errorCode_ != MEMOP_OK) {
            return;
        }
        if (engine_.memoryOperator().rootSize() != sizeof(PouData)) {
            errorCode_ = MEMOP_ERR_LAYOUT;
            return;
        }
        engine_.bindMemory(&axis, sizeof(PouData));
        errorCode_ = engine_.parseScheduledScript(kMemoryPouPowerMoveScript);
        if (errorCode_ != MEMOP_OK) {
            return;
        }

        power.Axis = axis;
        move.Axis = axis;
        readPos.Axis = axis;
        readStatus.Axis = axis;
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
        halt.runCycle();
        moveRel.runCycle();
        moveAdd.runCycle();
        move.runCycle();
        readPos.runCycle();
        readStatus.runCycle();
    }

   private:
    MEMORY_POU_TEST_FIELDS;
    MemoryPouEngine engine_;
    int errorCode_;
};

#undef MEMORY_POU_TEST_FIELDS

static const char kTinyTypeLayout[] = R"json(
{
  "types": {
    "ZMC_BOOL": {"size": 1, "align": 1, "kind": "bool"},
    "ZMC_LREAL": {"size": 8, "align": 8, "kind": "double"}
  }
}
)json";

static const char kTinyInitConfig[] = R"(
{
    ZMC_LREAL x;
    ZMC_BOOL b;
};
)";

struct TinyPouData {
    double x;
    unsigned char b;
};

int run_memory_pou_power_move_demo(int argc, char** argv) {
    AxPouTaskBinderAuto* runner = make_pou_config<AxPouTaskBinderAuto>(
        argc, argv, iniContentDefault, PouTmplDefaults{{"cycles", "200"}});
    if (runner == NULL) {
        return -1;
    }

    MemoryPouPowerMoveTask task;
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

TEST(MemoryPouLayout, read_fb_matches_cpp_pod) {
    ASSERT_EQ(32u, sizeof(FbtReadActualPosition));
    ASSERT_EQ(40u, sizeof(FbtReadStatus));
}

TEST(MemoryPouExpect, ordered_and_ne_without_axis) {
    MemoryPouEngine engine;
    ASSERT_EQ(MEMOP_OK, engine.loadTypeLayout(kTinyTypeLayout));
    ASSERT_EQ(MEMOP_OK, engine.configure(kTinyInitConfig));

    TinyPouData data;
    data.x = 0.0;
    data.b = 0;
    engine.bindMemory(&data, sizeof(data));
    ASSERT_EQ(MEMOP_OK, engine.parseScheduledScript("x = 2; expect x > 1; expect b != true"));

    engine.cmdAdvance();
    EXPECT_EQ(MEMOP_OK, engine.errorCode());
    EXPECT_NEAR(2.0, data.x, 1e-9);
}

TEST(MemoryPouPowerMove, script_expect_power_move_done) {
    char* argv[] = {const_cast<char*>("test_memory_pou_power_move"), NULL};
    const int rc = run_memory_pou_power_move_demo(1, argv);
    ASSERT_EQ(MEMOP_OK, rc);
}
