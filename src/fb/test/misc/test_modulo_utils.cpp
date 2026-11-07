#include <gtest/gtest.h>
#include "fbt_fb_base.hpp"
#include "mc_enum_def.h"

using namespace zmc;

class CalcTargetPosByDirectionTest : public ::testing::Test {
protected:
    const double modulo = 360.0;  // Use 360 as modulo for easy understanding
};

TEST_F(CalcTargetPosByDirectionTest, AllScenarios) {
    struct Scenario {
        const char* label;
        MC_DIRECTION direction;
        double bgPos;
        double tgPos;
        double moduloValue;
        bool isNeg;
        double expected;
        double tolerance;
    };

    constexpr double tol = 1e-6;
    const double baseModulo = modulo;
    const Scenario cases[] = {
        {"PositiveForward", mcPositiveDirection, 10.0, 50.0, baseModulo, false, 50.0, tol},
        {"PositiveBackwardWrap", mcPositiveDirection, 50.0, 10.0, baseModulo, false, 370.0, tol},
        {"PositiveSamePosition", mcPositiveDirection, 50.0, 50.0, baseModulo, false, 50.0, tol},
        {"PositiveCrossModulo", mcPositiveDirection, 350.0, 10.0, baseModulo, false, 370.0, tol},
        {"PositiveNegativeInput", mcPositiveDirection, -10.0, 50.0, baseModulo, false, 50.0, tol},
        {"NegativeBackward", mcNegativeDirection, 50.0, 10.0, baseModulo, false, 10.0, tol},
        {"NegativeForwardWrap", mcNegativeDirection, 10.0, 50.0, baseModulo, false, -310.0, tol},
        {"NegativeSamePosition", mcNegativeDirection, 50.0, 50.0, baseModulo, false, 50.0, tol},
        {"NegativeCrossModulo", mcNegativeDirection, 10.0, 350.0, baseModulo, false, -10.0, tol},
        {"ShortestForward", mcShortestWay, 10.0, 50.0, baseModulo, false, 50.0, tol},
        {"ShortestBackwardWrap", mcShortestWay, 50.0, 10.0, baseModulo, false, 370.0, tol},
        {"ShortestLongForwardWrap", mcShortestWay, 10.0, 200.0, baseModulo, false, -160.0, tol},
        {"ShortestLongBackward", mcShortestWay, 200.0, 10.0, baseModulo, false, 10.0, tol},
        {"ShortestHalf", mcShortestWay, 0.0, 180.0, baseModulo, false, 180.0, tol},
        {"FastestForward", mcFastestWay, 10.0, 50.0, baseModulo, false, 50.0, tol},
        {"FastestLongForwardWrap", mcFastestWay, 10.0, 200.0, baseModulo, false, -160.0, tol},
        {"CurrentIsNegFalseForward", mcCurrentDirection, 10.0, 50.0, baseModulo, false, 50.0, tol},
        {"CurrentIsNegFalseBackwardWrap", mcCurrentDirection, 50.0, 10.0, baseModulo, false, 370.0, tol},
        {"CurrentIsNegTrueBackward", mcCurrentDirection, 50.0, 10.0, baseModulo, true, 10.0, tol},
        {"CurrentIsNegTrueForwardWrap", mcCurrentDirection, 10.0, 50.0, baseModulo, true, -310.0, tol},
        {"PositiveNegativeBg", mcPositiveDirection, -50.0, 10.0, baseModulo, false, 10.0, tol},
        {"PositiveNegativeTarget", mcPositiveDirection, 10.0, -50.0, baseModulo, false, 310.0, tol},
        {"PositiveDifferentModulo", mcPositiveDirection, 10.0, 50.0, 100.0, false, 50.0, tol},
        {"PositiveLargeModulo", mcPositiveDirection, 100.0, 500.0, 1000.0, false, 500.0, tol},
        {"PositiveBeyondModulo", mcPositiveDirection, 10.0, 400.0, baseModulo, false, 40.0, tol},
        {"FiniteIgnoresModulo", mcFiniteDirection, 10.0, 400.0, baseModulo, false, 400.0, tol},
        {"PositiveBothBeyondModulo", mcPositiveDirection, 400.0, 500.0, baseModulo, false, 500.0, tol},
        {"PositiveForwardTinyDiff", mcPositiveDirection, 10.0, 50.0 + 1e-7, baseModulo, false, 50.0 + 1e-7, 1e-7},
        {"PositiveForwardTinyNegDiff", mcPositiveDirection, 10.0, 50.0 - 1e-7, baseModulo, false, 50.0 - 1e-7, 1e-7},
        {"PositiveBackwardTinyDiff", mcPositiveDirection, 50.0, 10.0 - 1e-7, baseModulo, false, 370.0 - 1e-7, 1e-7},
        {"NegativeForwardTinyDiff", mcNegativeDirection, 10.0, 50.0 + 1e-7, baseModulo, false, -310.0 + 1e-7, 1e-7},
        {"NegativeBackwardTinyDiff", mcNegativeDirection, 50.0, 10.0 - 1e-7, baseModulo, false, 10.0 - 1e-7, 1e-7},
        {"ShortestTinyDiff", mcShortestWay, -1e-6, 360.0-1e-7, baseModulo, false, -1e-7, 1e-7}
    };

    for (const auto& c : cases) {
        SCOPED_TRACE(c.label);
        double result = calcTargetPosByDirection(c.direction, c.bgPos, c.tgPos, c.moduloValue, c.isNeg);
        EXPECT_NEAR(result, c.expected, c.tolerance);
    }
}
