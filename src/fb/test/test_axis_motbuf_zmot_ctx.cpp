#include <gtest/gtest.h>

#include <vector>

#include "motbuf/AxisMotbufZmot.h"
#include "motbuf/FbMoveWrapV3.hpp"
#include "pou/AxPou.hpp"
#include "pou/Ctx.h"

namespace {

// Build the single-index form used by FBM_AXIS_GROUP commands.
zmc::CommandStructDefault make_axis_group_command(int axis_index) {
    zmc::CommandStructDefault cmd;
    cmd.cmdType = zmc::FBM_AXIS_GROUP;
    cmd.paramLen = sizeof(int);
    cmd.param.inlineInt[0] = axis_index;
    return cmd;
}

// Build the scalar form used by the four FBM_GROUP_SET_* commands.
zmc::CommandStructDefault make_group_parameter_command(int command_type, double value) {
    zmc::CommandStructDefault cmd;
    cmd.cmdType = command_type;
    cmd.setDouble(value);
    return cmd;
}

}  // namespace

TEST(AxisMotbufZmotCtx, ResolvesAxisIndexBeforeCallingGroupLayer) {
    zmc::AxPouTaskBinderAuto pou(2);
    std::vector<AXIS_REF> axes;
    zmc::Ctx ctx;
    ctx.set_axis_arr(&axes);

    zmc::AxisMotbufZmot motbuf;
    motbuf.setCtx(&ctx);
    ASSERT_NE(nullptr, motbuf.group(0));

    zmc::CommandStructDefault cmd = make_axis_group_command(0);
    ASSERT_EQ(0, motbuf.pushCommand(cmd));
    motbuf.runCycleTop();
    EXPECT_EQ(0, motbuf.group(0)->axNum());

    axes.push_back(pou.axisRef(1));
    ASSERT_EQ(0, motbuf.pushCommand(cmd));
    motbuf.runCycleTop();
    EXPECT_EQ(1, motbuf.group(0)->axNum());
}

TEST(AxisMotbufZmotCtx, RejectsUnresolvedAxisBeforeMutatingGroup) {
    zmc::AxPouTaskBinderAuto pou(1);
    std::vector<AXIS_REF> axes(1, pou.axisRef(0));
    zmc::Ctx ctx;
    ctx.set_axis_arr(&axes);

    zmc::AxisMotbufZmot motbuf;
    motbuf.setCtx(&ctx);
    ASSERT_NE(nullptr, motbuf.group(0));

    zmc::CommandStructDefault valid_cmd = make_axis_group_command(0);
    ASSERT_EQ(0, motbuf.pushCommand(valid_cmd));
    motbuf.runCycleTop();
    ASSERT_EQ(1, motbuf.group(0)->axNum());

    zmc::CommandStructDefault invalid_cmd = make_axis_group_command(1);
    ASSERT_EQ(0, motbuf.pushCommand(invalid_cmd));
    motbuf.runCycleTop();
    EXPECT_EQ(1, motbuf.group(0)->axNum());

    AXIS_REF invalid_axes[] = {nullptr};
    EXPECT_NE(0, motbuf.group(0)->setAxisRefArr(invalid_axes, 1));
    EXPECT_EQ(1, motbuf.group(0)->axNum());
}

TEST(AxisMotbufZmotCtx, RuntimeResetRestoresDefaultGroupSelection) {
    zmc::AxisMotbufZmot motbuf;

    motbuf.setGroupSelect(3);
    ASSERT_EQ(4u, motbuf.groupCount());
    ASSERT_EQ(3, motbuf.groupSelect());

    motbuf.reset_runtime();

    EXPECT_EQ(1u, motbuf.groupCount());
    EXPECT_EQ(0, motbuf.groupSelect());
    EXPECT_NE(nullptr, motbuf.group(0));
    EXPECT_EQ(nullptr, motbuf.group(1));
}

TEST(AxisMotbufZmotCtx, AppliesEachGroupMotionParameterCommandIndependently) {
    zmc::AxisMotbufZmot motbuf;
    ASSERT_NE(nullptr, motbuf.group(0));

    motbuf.group(0)->setCmdVel(1.0);
    motbuf.group(0)->setCmdAcc(2.0);
    motbuf.group(0)->setCmdDec(3.0);
    motbuf.group(0)->setCmdJerk(4.0);

    ASSERT_EQ(0, motbuf.pushCommand(make_group_parameter_command(zmc::FBM_GROUP_SET_VEL, 10.0)));
    motbuf.runCycleTop();
    EXPECT_DOUBLE_EQ(10.0, motbuf.group(0)->cmdVel());
    EXPECT_DOUBLE_EQ(2.0, motbuf.group(0)->cmdAcc());
    EXPECT_DOUBLE_EQ(3.0, motbuf.group(0)->cmdDec());
    EXPECT_DOUBLE_EQ(4.0, motbuf.group(0)->cmdJerk());

    ASSERT_EQ(0, motbuf.pushCommand(make_group_parameter_command(zmc::FBM_GROUP_SET_ACC, 20.0)));
    motbuf.runCycleTop();
    EXPECT_DOUBLE_EQ(10.0, motbuf.group(0)->cmdVel());
    EXPECT_DOUBLE_EQ(20.0, motbuf.group(0)->cmdAcc());
    EXPECT_DOUBLE_EQ(3.0, motbuf.group(0)->cmdDec());
    EXPECT_DOUBLE_EQ(4.0, motbuf.group(0)->cmdJerk());

    ASSERT_EQ(0, motbuf.pushCommand(make_group_parameter_command(zmc::FBM_GROUP_SET_DEC, 30.0)));
    motbuf.runCycleTop();
    EXPECT_DOUBLE_EQ(10.0, motbuf.group(0)->cmdVel());
    EXPECT_DOUBLE_EQ(20.0, motbuf.group(0)->cmdAcc());
    EXPECT_DOUBLE_EQ(30.0, motbuf.group(0)->cmdDec());
    EXPECT_DOUBLE_EQ(4.0, motbuf.group(0)->cmdJerk());

    ASSERT_EQ(0, motbuf.pushCommand(make_group_parameter_command(zmc::FBM_GROUP_SET_JERK, 40.0)));
    motbuf.runCycleTop();
    EXPECT_DOUBLE_EQ(10.0, motbuf.group(0)->cmdVel());
    EXPECT_DOUBLE_EQ(20.0, motbuf.group(0)->cmdAcc());
    EXPECT_DOUBLE_EQ(30.0, motbuf.group(0)->cmdDec());
    EXPECT_DOUBLE_EQ(40.0, motbuf.group(0)->cmdJerk());
}

TEST(FbMoveWrap2Queue, RoutesNewGroupMotionParameterCommandsToGroupMotbuf) {
    zmc::FbMoveWrap2Queue wrap(0);
    zmc::CmdPacket packet;
    packet.reset();
    const int group_index = 0;
    const double acceleration = 12.5;

    ASSERT_TRUE(packet.appendItem(zmc::FBM_GROUP_SELECT, &group_index, sizeof(group_index)));
    ASSERT_TRUE(packet.appendItem(zmc::FBM_GROUP_SET_ACC, &acceleration, sizeof(acceleration)));
    ASSERT_TRUE(wrap.getCmdQueue().push(packet));

    wrap.runCycle();

    ASSERT_NE(nullptr, wrap.zmotMotbuf.group(0));
    EXPECT_DOUBLE_EQ(acceleration, wrap.zmotMotbuf.group(0)->cmdAcc());
}
