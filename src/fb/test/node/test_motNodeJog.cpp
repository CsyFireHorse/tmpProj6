#if 0
TEST(motNode, jog) {
    AxisMotEmu axis;
    auto& cfg = axis.get_config_ref();
    cfg.debug.servo_fak_pos = 5;
    cfg.cycle_time = 100000; // 0.1s
    axis.reload_config();
    auto& ax = get_axis_impl_ref((AXIS_REF)&axis);

    FbtJog mov;
    mov.Axis = (AXIS_REF)&axis;
    mov.Velocity = 5;
    mov.Acceleration = 3;
    mov.Deceleration = 3;
    mov.Jerk = 0;
    mov.JogForward = false;
    mov.JogBackward = false;

    VelOnlineChecker chk;
    chk.Ts = cfg.cycle_time / 1e6;

    for (int i = 0; i < 60; i++) {
        if (i == 5) {
            mov.JogForward = true;
        } else if (i == 20) {
            mov.JogForward = false;
            mov.JogBackward = true;
        }
        mov.runCycle();
        axis.runCycle();
        // 断言可根据需要添加
        printf("%d,%f,%f,%f\n", i, ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration);
        EXPECT_EQ(0, chk.check(ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration));
        // 可选：检查方向

    }
}

TEST(motNode, jog_home) {
    AxisMotEmu axis;
    auto& cfg = axis.get_config_ref();
    cfg.debug.servo_fak_pos = 5;
    cfg.cycle_time = 100000; // 0.1s
    axis.reload_config();
    auto& ax = get_axis_impl_ref((AXIS_REF)&axis);

    FbtJog mov;
    mov.Axis = (AXIS_REF)&axis;
    mov.Velocity = 2;
    mov.Acceleration = 3;
    mov.Deceleration = 3;
    mov.Jerk = 0;
    mov.JogForward = false;
    mov.JogBackward = false;

    VelOnlineChecker chk;
    chk.Ts = cfg.cycle_time / 1e6;

    double vm = 2;
    const double Tol = 1e-3;
    bool fl = true;
    const double posZero = 9;

    for (int i = 0; i < 120; i++) {
        if (i == 1) {
            mov.JogForward = true;
        }

        // 根据位置调整速度和方向
        if ((ax.fSetPosition < posZero + Tol) ^ fl) {
            vm *= 0.7;
            mov.Velocity = vm;
        }

        fl = ax.fSetPosition < posZero - Tol;
        mov.JogBackward = ax.fSetPosition > posZero + Tol;
        mov.JogForward = fl;

        mov.runCycle();
        axis.runCycle();

        // 检查运动状态
        if (i > 99) {
            EXPECT_NEAR(ax.fSetPosition, 9, 1e-4);
            EXPECT_NEAR(ax.fSetVelocity, 0, 1e-4);
        }
        printf("%d,%f,%f,%f\n", i, ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration);
        // 检查运动轨迹
        EXPECT_EQ(0, chk.check(ax.fSetPosition, ax.fSetVelocity, ax.fSetAcceleration));
    }
}
#endif