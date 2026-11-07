#include "ZmlClientTest.hpp"

// GPT-6 Codex, 2026-10-29. Shared client contract tests.

TEST_F(ZmlClientTest, NumericSdoRoundTrips) {
    const uint32_t sizes[] = {1, 2, 4};
    const uint32_t values[] = {0x78, 0x5678, 0x12345678};
    for (size_t i = 0; i < 3; ++i) {
        ASSERT_EQ(0, zml_ecat_coe_sdo_download(handle_, 0, 0x2000, 0, sizes[i], &values[i]));
        uint32_t actual = 0, size = sizes[i];
        ASSERT_EQ(0, zml_ecat_coe_sdo_upload(handle_, 0, 0x2000, 0, &size, &actual));
        EXPECT_EQ(sizes[i], size);
        EXPECT_EQ(values[i], actual);
    }
}
