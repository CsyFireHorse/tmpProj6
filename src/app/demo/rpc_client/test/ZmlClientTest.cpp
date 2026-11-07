#include "ZmlClientTest.hpp"
#include <fstream>
#include <iterator>
#include <vector>

// GPT-6 Codex, 2026-10-22. The first successful open is the test's main session.
void ZmlClientTest::SetUp() { start("zml_motion.ini"); }

void ZmlClientTest::start(const char* ini, const char* script) {
    ASSERT_NO_FATAL_FAILURE(startRuntime(ini, script));
    ASSERT_TRUE(open(handle_));
    ZML_FlowCtrlConfig flow = {};
    flow.period = 10;
    flow.verbose_flag = 1;
    ASSERT_EQ(0, zml_set_flow_config(handle_, &flow));
}

void ZmlClientSupport::startRuntime(const char* ini, const char* script) {
    std::vector<std::string> args = {std::string("--") + ZML_TEST_MODE,
                                     "--port=" + std::to_string(ZML_TEST_PORT), "--ini",
                                     fixture(ini)};
    if (script) {
        args.push_back("--script");
        args.push_back(fixture(script));
    }
    ASSERT_EQ(0, runtime_.start(args));
    zml_sleep_ms(100);
}

bool ZmlClientTest::open(ZML_HANDLE& handle) {
    const std::string endpoint = "127.0.0.1:" + std::to_string(ZML_TEST_PORT);
    ZML_RESULT ret = 0;
    const bool ready = wait([&]() {
        ret = zml_open(&handle, endpoint.c_str());
        return ret == 0;
    });
    if (!ready) ADD_FAILURE() << "open " << endpoint << " ret=" << ret;
    // GPT-6.1 Terra, 2026-11-05. Wait for a motion-cycle confirmation after opening the session.
    return ready && wait([&]() { return zml_ping(handle, 1000) == 0; });
}

void ZmlClientTest::TearDown() {
    if (second_) {
        EXPECT_EQ(0, zml_close(&second_));
    }
    if (handle_) {
        EXPECT_EQ(0, zml_close(&handle_));
    }
    EXPECT_EQ(0, runtime_.finish());
}

void ZmlClientSupport::prepareAxis(int axis) {
    ASSERT_EQ(0, zml_ax_power_on(handle_, axis));
    ASSERT_TRUE(axisState(axis, kStandstill));
    ASSERT_EQ(0, zml_ax_set_position(handle_, axis, 0));
    ASSERT_TRUE(position(axis, 0));
    ASSERT_EQ(0, zml_ax_set_velocity(handle_, axis, 100));
    ASSERT_EQ(0, zml_ax_set_accel(handle_, axis, 1000));
    ASSERT_EQ(0, zml_ax_set_decel(handle_, axis, 1000));
}

// Startup arguments initialize the current runtime; selection governs the next bus start.
void ZmlClientSupport::selectConfig(const char* ini, const char* eni) {
    const std::string eni_path = fixture(eni);
    const std::string ini_path = fixture(ini);
    ASSERT_EQ(0, zml_download_eni(handle_, eni_path.c_str()));
    ASSERT_EQ(0, zml_download_syscfg(handle_, ini_path.c_str()));
}

bool ZmlClientSupport::position(int axis, double expected) {
    ZML_AxisState state = {};
    ZML_RESULT ret = 0;
    const bool reached = wait([&]() {
        ret = zml_ax_get_state(handle_, axis, &state);
        return ret == 0 && state.error_id == 0 &&
               (state.axis_state & kAxisStateMask) == kStandstill &&
               std::fabs(state.act_pos - expected) < 0.01 &&
               std::fabs(state.set_pos - expected) < 0.01;
    });
    if (!reached)
        ADD_FAILURE() << "axis=" << axis << " expected=" << expected << " ret=" << ret
                      << " state=" << unsigned(state.axis_state) << " error=" << state.error_id
                      << " act=" << state.act_pos << " set=" << state.set_pos
                      << " tick=" << state.tick;
    return reached;
}

bool ZmlClientSupport::axisState(int axis, unsigned expected) {
    ZML_AxisState state = {};
    const bool reached = wait([&]() {
        return zml_ax_get_state(handle_, axis, &state) == 0 &&
               (state.axis_state & kAxisStateMask) == expected;
    });
    if (!reached)
        ADD_FAILURE() << "axis=" << axis << " state=" << unsigned(state.axis_state)
                      << " error=" << state.error_id << " expected=" << expected;
    return reached;
}

bool ZmlClientSupport::parameter(ZML_HANDLE handle, int axis,
                                 ZML_RESULT (*getter)(ZML_HANDLE, int32_t, double*),
                                 double expected) {
    double actual = 0;
    ZML_RESULT ret = 0;
    const bool reached = wait([&]() {
        ret = getter(handle, axis, &actual);
        return ret == 0 && std::fabs(actual - expected) < 1e-9;
    });
    if (!reached)
        ADD_FAILURE() << "axis=" << axis << " ret=" << ret << " expected=" << expected
                      << " actual=" << actual;
    return reached;
}

std::string ZmlClientSupport::fixture(const char* name) const {
    return std::string(ZML_TEST_FIXTURE_DIR) + "/" + name;
}

void ZmlClientTest::writeFile(const char* name, const std::string& bytes) {
    std::ofstream file(name, std::ios::binary | std::ios::trunc);
    ASSERT_TRUE(file.is_open());
    file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    file.close();
    ASSERT_TRUE(file.good());
}

std::string ZmlClientTest::readFile(const char* name) {
    std::ifstream file(name, std::ios::binary);
    EXPECT_TRUE(file.is_open()) << name;
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}
