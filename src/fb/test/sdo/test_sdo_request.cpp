/*
@ArtifactStatus draft
@ArtifactStatusDate 2026-09-17
@ArtifactStatusAuthority GPT-5 Codex
*/
#include <gtest/gtest.h>

#include "common/zmc/fbt_errorcode_def.h"
#include "SdoMessage.hpp"
#include "sdo_test_helper.hpp"

using fb_test::sdo::make_key;

TEST(SdoRequest, KeepsCompactLayouts) {
  EXPECT_EQ(6u, sizeof(zmc::SdoKey));
  EXPECT_EQ(12u, sizeof(zmc::SdoRequestItem));
}

TEST(SdoRequest, AppendMixedUploadDownloadItems) {
  zmc::SdoRequest req{};
  const zmc::SdoKey upload_key = make_key(1, 0x6061, 0, 1);
  const zmc::SdoKey download_key = make_key(1, 0x607A, 0, 4);

  EXPECT_EQ(0, req.append_item(zmc::SDO_DIR_UPLOAD, upload_key, 1));
  EXPECT_EQ(0,
      req.append_item(zmc::SDO_DIR_DOWNLOAD, download_key, 4, 0x11223344u));

  ASSERT_EQ(2u, req.item_count);
  EXPECT_EQ(zmc::SDO_DIR_UPLOAD, req.items[0].direction);
  EXPECT_EQ(1u, req.items[0].len);
  EXPECT_EQ(0u, req.items[0].data);
  EXPECT_EQ(zmc::SDO_DIR_DOWNLOAD, req.items[1].direction);
  EXPECT_EQ(4u, req.items[1].len);
  EXPECT_EQ(0x11223344u, req.items[1].data);
}

TEST(SdoRequest, AppendMixedLengths) {
  zmc::SdoRequest req{};
  EXPECT_EQ(0,
      req.append_item(zmc::SDO_DIR_DOWNLOAD, make_key(1, 0x6060, 0), 1, 0x08u));
  EXPECT_EQ(0, req.append_item(zmc::SDO_DIR_DOWNLOAD, make_key(1, 0x2000, 0), 2,
                               0xAABBu));
  EXPECT_EQ(0, req.append_item(zmc::SDO_DIR_DOWNLOAD, make_key(1, 0x607A, 0), 4,
                               0x11223344u));

  ASSERT_EQ(3u, req.item_count);
  EXPECT_EQ(1u, req.items[0].len);
  EXPECT_EQ(2u, req.items[1].len);
  EXPECT_EQ(4u, req.items[2].len);
}

TEST(SdoRequest, RejectsInvalidLenAndDirection) {
  zmc::SdoRequest req{};
  EXPECT_EQ(ZMC_ERR_SDO_INVALID_LENGTH,
            req.append_item(zmc::SDO_DIR_UPLOAD, make_key(1, 0x1000, 0), 0));
  EXPECT_EQ(ZMC_ERR_SDO_INVALID_LENGTH,
            req.append_item(zmc::SDO_DIR_UPLOAD, make_key(1, 0x1000, 0), 5));
  EXPECT_EQ(ZMC_ERR_SDO_INVALID_ARG, req.append_item(99u, make_key(1, 0x1000, 0), 4));
  EXPECT_TRUE(req.empty());
}

TEST(SdoRequest, RejectsFullBatch) {
  zmc::SdoRequest req{};
  const zmc::SdoKey key = make_key(1, 0x1000, 0, 4);
  for (uint32_t i = 0; i < SDO_REQUEST_MAX_ITEMS; ++i) {
    EXPECT_EQ(0, req.append_item(zmc::SDO_DIR_UPLOAD, key, 4));
  }
  EXPECT_TRUE(req.full());
  EXPECT_EQ(ZMC_ERR_SDO_CAPACITY, req.append_item(zmc::SDO_DIR_UPLOAD, key, 4));
}

TEST(SdoResult, AppendsItemsInOrderAndResets) {
  zmc::SdoResult res{};
  EXPECT_TRUE(res.empty());

  zmc::SdoResultItem first{};
  first.key = make_key(1, 0x6061, 0);
  first.direction = zmc::SDO_DIR_UPLOAD;
  first.result_code = 0;
  first.data_len = 1;
  first.data = 0x08u;
  first.seq = 3u;
  first.request_id = 2u;

  zmc::SdoResultItem second{};
  second.key = make_key(2, 0x607A, 0);
  second.direction = zmc::SDO_DIR_DOWNLOAD;
  second.result_code = -1;
  second.data_len = 4;
  second.data = 0x11223344u;
  second.seq = 4u;
  second.request_id = 2u;

  ASSERT_EQ(0, res.append_item(first));
  ASSERT_EQ(0, res.append_item(second));
  ASSERT_EQ(2u, res.item_count);
  EXPECT_EQ(first.key, res.items[0].key);
  EXPECT_EQ(first.direction, res.items[0].direction);
  EXPECT_EQ(first.data, res.items[0].data);
  EXPECT_EQ(first.seq, res.items[0].seq);
  EXPECT_EQ(first.request_id, res.items[0].request_id);
  EXPECT_EQ(second.key, res.items[1].key);
  EXPECT_EQ(second.result_code, res.items[1].result_code);
  EXPECT_EQ(second.data_len, res.items[1].data_len);

  res.reset();
  EXPECT_TRUE(res.empty());
}

TEST(SdoResult, RejectsFullPayload) {
  zmc::SdoResult res{};
  zmc::SdoResultItem item{};
  for (uint32_t i = 0; i < SDO_RESULT_MAX_ITEMS; ++i) {
    item.seq = i + 1u;
    EXPECT_EQ(0, res.append_item(item));
  }
  EXPECT_TRUE(res.full());
  EXPECT_EQ(ZMC_ERR_SDO_CAPACITY, res.append_item(item));
}
