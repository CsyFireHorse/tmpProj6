#include "ZmlClientTest.hpp"
#include "zml_http_json.h"
#include "common/zmc/fbt_errorcode_def.h"

#include <string>

// GPT-6 Codex, 2026-11-05. Exercise both the C JSON contract and the HTTP session.
namespace {
std::string jsonValue(ZML_JSON json, const char* path, ZML_JSON_TYPE expected) {
    ZML_JSON_TYPE type = ZML_JSON_NULL;
    char* value = nullptr;
    uint32_t length = 0;
    const ZML_RESULT ret = zml_json_get_value(json, path, &type, &value, &length);
    EXPECT_EQ(0, ret);
    EXPECT_EQ(expected, type);
    if (!value) return std::string();
    const std::string result(value, length);
    zml_free_string(value);
    return result;
}
}  // namespace

TEST(ZmlJsonTest, PointerValuesAndArrays) {
    ZML_JSON json = nullptr;
    ASSERT_EQ(0, zml_json_parse(
        "{\"axes\":[{\"id\":7,\"name\":\"a\\u0000b\"}],\"empty\":[],"
        "\"axis\":null,\"large\":18446744073709551615,\"a/b\":true}", &json));
    uint32_t count = 1;
    ASSERT_EQ(0, zml_json_array_size(json, "/axes", &count));
    EXPECT_EQ(1u, count);
    EXPECT_EQ("7", jsonValue(json, "/axes/0/id", ZML_JSON_NUMBER));
    EXPECT_EQ(std::string("a\0b", 3), jsonValue(json, "/axes/0/name", ZML_JSON_STRING));
    EXPECT_EQ("null", jsonValue(json, "/axis", ZML_JSON_NULL));
    EXPECT_EQ("18446744073709551615", jsonValue(json, "/large", ZML_JSON_NUMBER));
    EXPECT_EQ("true", jsonValue(json, "/a~1b", ZML_JSON_BOOL));
    ASSERT_EQ(0, zml_json_array_size(json, "/empty", &count));
    EXPECT_EQ(0u, count);
    char* missing = nullptr;
    uint32_t length = 0;
    ZML_JSON_TYPE type = ZML_JSON_NULL;
    EXPECT_EQ(ZMC_ERR_RPC_JSON_MISSING_FIELD,
              zml_json_get_value(json, "/axes/1/id", &type, &missing, &length));
    EXPECT_EQ(nullptr, missing);
    zml_json_free(json);
    json = nullptr;
    EXPECT_EQ(ZMC_ERR_RPC_JSON_INVALID, zml_json_parse("{", &json));
    EXPECT_EQ(nullptr, json);
}

TEST_F(ZmlClientTest, HttpJsonRequestUsesSession) {
    int32_t status = 0;
    char* response = nullptr;
    ASSERT_EQ(0, zml_http_json_request(handle_, "POST", "/zmot/api/speed",
                                       "{\"axis\":0,\"value\":31.5}", &status, &response));
    ASSERT_EQ(200, status);
    ZML_JSON json = nullptr;
    ASSERT_EQ(0, zml_json_parse(response, &json));
    zml_free_string(response);
    EXPECT_EQ("0", jsonValue(json, "/code", ZML_JSON_NUMBER));
    zml_json_free(json);
    ASSERT_TRUE(parameter(handle_, 0, zml_ax_get_speed, 31.5));

    response = nullptr;
    ASSERT_EQ(0, zml_http_json_request(handle_, "GET", "/zmot/api/speed?axis=0", nullptr,
                                       &status, &response));
    ASSERT_EQ(200, status);
    ASSERT_EQ(0, zml_json_parse(response, &json));
    zml_free_string(response);
    EXPECT_EQ("0", jsonValue(json, "/code", ZML_JSON_NUMBER));
    EXPECT_EQ("31.5", jsonValue(json, "/value", ZML_JSON_NUMBER));
    zml_json_free(json);

    response = nullptr;
    ASSERT_EQ(0, zml_http_json_request(handle_, "GET", "/zmot/api/dashboard?axes=0", nullptr,
                                       &status, &response));
    ASSERT_EQ(200, status);
    ASSERT_EQ(0, zml_json_parse(response, &json));
    zml_free_string(response);
    uint32_t count = 0;
    ASSERT_EQ(0, zml_json_array_size(json, "/axes", &count));
    ASSERT_EQ(1u, count);
    EXPECT_EQ("0", jsonValue(json, "/axes/0/axis_id", ZML_JSON_NUMBER));
    zml_json_free(json);

    response = nullptr;
    ASSERT_EQ(0, zml_http_json_request(handle_, "POST", "/zmot/api/speed", "{}", &status,
                                       &response));
    ASSERT_EQ(200, status);
    ASSERT_EQ(0, zml_json_parse(response, &json));
    zml_free_string(response);
    EXPECT_NE("0", jsonValue(json, "/code", ZML_JSON_NUMBER));
    zml_json_free(json);

    response = nullptr;
    ASSERT_EQ(0, zml_http_json_request(handle_, "GET", "/zmot/api/no_such_route", nullptr,
                                       &status, &response));
    EXPECT_EQ(404, status);
    ASSERT_NE(nullptr, response);
    zml_free_string(response);
}
