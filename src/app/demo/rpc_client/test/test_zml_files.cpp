#include "ZmlClientTest.hpp"
#include <memory>

// One service belongs to the file suite; each test owns its client session.
class ZmlFilesTest : public ZmlClientTest {
   protected:
    static std::unique_ptr<SrvTestRuntime>& filesRuntime() {
        static std::unique_ptr<SrvTestRuntime> runtime;
        return runtime;
    }

    static void SetUpTestSuite() {
        auto& runtime = filesRuntime();
        runtime.reset(new SrvTestRuntime());
        ASSERT_EQ(0, runtime->start({std::string("--") + ZML_TEST_MODE,
                                     "--port=" + std::to_string(ZML_TEST_PORT), "--ini",
                                     std::string(ZML_TEST_FIXTURE_DIR) + "/zml_motion.ini"}));
        zml_sleep_ms(100);
        ASSERT_EQ(0, zml_sleep_ms(100));
    }

    static void TearDownTestSuite() {
        auto& runtime = filesRuntime();
        if (runtime) {
            EXPECT_EQ(0, runtime->finish());
            runtime.reset();
        }
    }

    void SetUp() override {
        ASSERT_TRUE(open(handle_));
        ZML_FlowCtrlConfig flow = {};
        flow.period = 10;
        flow.verbose_flag = 1;
        ASSERT_EQ(0, zml_set_flow_config(handle_, &flow));
    }

    void TearDown() override {
        if (second_) EXPECT_EQ(0, zml_close(&second_));
        if (handle_) EXPECT_EQ(0, zml_close(&handle_));
    }
};

// GPT-6 Codex, 2026-10-22. Remote files are created exclusively through public downloads.
TEST_F(ZmlFilesTest, EmptyBinaryAndMultiChunkFileRoundTrip) {
    std::string large(131073, '\0');
    for (size_t i = 0; i < large.size(); ++i) large[i] = static_cast<char>(i % 251);
    const std::string payloads[] = {std::string(), std::string("a\0b", 3), large};
    for (size_t i = 0; i < 3; ++i) {
        SCOPED_TRACE(i);
        const std::string remote = "roundtrip" + std::to_string(i) + ".bin";
        const std::string output = "output" + std::to_string(i) + ".bin";
        ASSERT_NO_FATAL_FAILURE(writeFile("input.bin", payloads[i]));
        ASSERT_EQ(0, zml_download_file(handle_, "input.bin", remote.c_str(), 5000));
        ASSERT_EQ(0, zml_upload_file(handle_, output.c_str(), remote.c_str(), 5000));
        EXPECT_EQ(payloads[i], readFile(output.c_str()));
    }
}

TEST_F(ZmlFilesTest, DownloadReplacesExistingFile) {
    ASSERT_NO_FATAL_FAILURE(writeFile("input.bin", "original"));
    ASSERT_EQ(0, zml_download_file(handle_, "input.bin", "replace.bin", 5000));
    const std::string replacement("new\0bytes", 9);
    ASSERT_NO_FATAL_FAILURE(writeFile("input.bin", replacement));
    ASSERT_EQ(0, zml_download_file(handle_, "input.bin", "replace.bin", 5000));
    ASSERT_EQ(0, zml_upload_file(handle_, "output.bin", "replace.bin", 5000));
    EXPECT_EQ(replacement, readFile("output.bin"));
}

TEST_F(ZmlFilesTest, UploadKeepsExistingClientFile) {
    ASSERT_NO_FATAL_FAILURE(writeFile("input.bin", "remote"));
    ASSERT_EQ(0, zml_download_file(handle_, "input.bin", "keep.bin", 5000));
    ASSERT_NO_FATAL_FAILURE(writeFile("output.bin", "local"));
    EXPECT_NE(0, zml_upload_file(handle_, "output.bin", "keep.bin", 5000));
    EXPECT_EQ("local", readFile("output.bin"));
}
