#include "ServoFakFileState.h"
#include "ServoFakAlloc.hpp"
#include <gtest/gtest.h>

using namespace zmc;

TEST(ServoFakFileState, EnvFileEnabled) {
    EXPECT_TRUE(servo_fak_state_enabled());
}

TEST(ServoFakFileState, ProcessEnvWins) {
    ServoSgFaker sv;
    ASSERT_EQ(servo_fak_state_load(&sv, 1, 1e-3), 0);
    EXPECT_EQ(sv.diActPosition, 111);
}

TEST(ServoFakFileState, LoadSparseAxes) {
    ServoSgFaker sv[4];
    ASSERT_EQ(servo_fak_state_load(sv, 4, 1e-3), 0);
    EXPECT_EQ(sv[0].diActPosition, 0);
    EXPECT_EQ(sv[3].diActPosition, 30003);
}

TEST(ServoFakFileState, SaveWritesSnapshot) {
    ServoSgFaker sv;
    sv.diActPosition = 4242;
    sv.diActVelocity = 0;
    sv.wStatusword = 567;
    sv.errCode = 0;
    sv.byOpModeDisplay = 8;
    EXPECT_EQ(servo_fak_state_save(&sv, 1), 0);
}
