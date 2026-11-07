#include "ZmlClientTest.hpp"

// Date: 2026-11-05; Drafted by: GPT-6 (Codex).
// Both transports observe metadata through the public ZML API.
class ZmlIoMetadataTest : public testing::Test, protected ZmlClientSupport {
   protected:
    void SetUp() override {
        ASSERT_EQ(0, runtime_.start({std::string("--") + ZML_TEST_MODE,
                                     "--port=" + std::to_string(ZML_TEST_PORT)}));
        const std::string endpoint = "127.0.0.1:" + std::to_string(ZML_TEST_PORT);
        ASSERT_TRUE(wait([&]() { return zml_open(&handle_, endpoint.c_str()) == 0; }));
    }

    void TearDown() override {
        if (handle_) EXPECT_EQ(0, zml_close(&handle_));
        EXPECT_EQ(0, runtime_.finish());
    }
};

TEST_F(ZmlIoMetadataTest, MetadataFollowsBusLifecycle) {
    ZML_IoMetadataEntry entries[4] = {};
    uint32_t count = 0;
    ASSERT_EQ(0, zml_get_io_metadata(handle_, entries, 4, &count));
    EXPECT_EQ(0u, count);

    const std::string eni = fixture("pse_servo1.xml");
    ASSERT_EQ(0, zml_download_eni(handle_, eni.c_str()));
    ASSERT_EQ(0, zml_ecat_start(handle_));
    ASSERT_EQ(0, zml_get_io_metadata(handle_, entries, 4, &count));
    EXPECT_EQ(0u, count);

    ASSERT_EQ(0, zml_ecat_stop(handle_));
    const std::string ini = fixture("zmc_motapp_http_io_metadata_test.ini");
    ASSERT_EQ(0, zml_download_syscfg(handle_, ini.c_str()));
    ASSERT_EQ(0, zml_ecat_start(handle_));
    ASSERT_EQ(0, zml_get_io_metadata(handle_, entries, 4, &count));
    ASSERT_EQ(4u, count);

    const struct {
        uint8_t type;
        uint16_t number;
        uint16_t index;
        uint32_t mask;
    } expected[] = {{ZML_IO_TYPE_DI, 10, 0x60FD, 8},
                    {ZML_IO_TYPE_DO, 11, 0x6040, 4},
                    {ZML_IO_TYPE_AI, 12, 0x6064, 0},
                    {ZML_IO_TYPE_AO, 13, 0x607A, 0}};
    // Match aliases by identity; the API does not require a particular array order.
    for (const auto& item : expected) {
        SCOPED_TRACE(item.number);
        const ZML_IoMetadataEntry* found = nullptr;
        for (const auto& entry : entries)
            if (entry.io_type == item.type && entry.io_number == item.number) found = &entry;
        ASSERT_NE(nullptr, found);
        EXPECT_EQ(ZML_IO_SOURCE_KIND_PDO, found->source_kind);
        EXPECT_EQ(item.index, found->index);
        EXPECT_EQ(0u, found->subindex);
        EXPECT_EQ(item.mask, found->mask);
    }

    ASSERT_EQ(0, zml_ecat_stop(handle_));
    ASSERT_EQ(0, zml_get_io_metadata(handle_, entries, 4, &count));
    EXPECT_EQ(0u, count);
}
