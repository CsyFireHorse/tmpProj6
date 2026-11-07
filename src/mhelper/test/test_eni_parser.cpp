#include "stdio.h"
#include "string.h"
#include <cstddef>
#include <iostream>
#include <string>
#include "assert.h"
#include <gtest/gtest.h>
#include "fbt_errorcode_def.h"
#include "../demo/pdo_protal3.h"
#include "../demo/pdo_ol.h"
#include "eni_servo_parser.h"
#include "ecat_json.h"
// #include "eni_parse_wrap.h"
#include "pdo_servo_addr.h"

#define LEN(x) sizeof(x)/sizeof(*(x))

using namespace zmc;

static std::string _get_file_path(const char *fname){
    std::string pth(__FILE__);
    pth = pth.substr(0, pth.size() - strlen("test_eni_parser.cpp"));
    pth += fname;
    return pth;
}

TEST(eniParse, missing_eni_returns_canonical_error_code){
    const char* fn = "../data/notfound.xml";
    SlaveInfo slaves;
    int errCode = load_slaves_from_eni_xml_json(_get_file_path(fn).c_str(), slaves);
    EXPECT_EQ(errCode, ZMC_ERR_ENI_FILE_OPEN_FAILED);
}

TEST(eniParse, extract_eni_json_text_from_pse_servo2) {
    const char* fn = "../data/pse_servo2.xml";
    std::string json_text;
    const int errCode = extract_eni_json_text(_get_file_path(fn).c_str(), json_text);
    ASSERT_EQ(errCode, 0);
    EXPECT_NE(std::string::npos, json_text.find("\"slaves\""));
    EXPECT_NE(std::string::npos, json_text.find("\"master\""));
}

TEST(eniParse, extract_eni_json_text_missing_file) {
    std::string json_text;
    const int errCode =
        extract_eni_json_text(_get_file_path("../data/notfound.xml").c_str(), json_text);
    EXPECT_EQ(errCode, ZMC_ERR_ENI_FILE_OPEN_FAILED);
    EXPECT_TRUE(json_text.empty());
}
// TEST(eniParse, eni_parse){
//     const char* fn= "../data/pse_servo3.xml";
//     eni_parse_wrap eni_info;
//     int ret = load_eni_file(_get_file_path(fn).c_str(), eni_info);
//     EXPECT_EQ(ret, 0);
//     EXPECT_EQ(eni_info.master_pdo.cycle_time, 1000);
// }


template<typename T, typename R>
void _test_Eni_servo(EniServoMapper& svMap){
    proc_img_i_addr _pi_i;
    proc_img_o_addr _pi_o;

    T tx;
    R rx;
    int N = svMap.slvInfo.size();
    pdo_get_input( _pi_i, svMap, (const void *)&tx);
    for (int i=0; i<N; i++){
        EXPECT_EQ(_pi_i.servos[i].p, tx.get_pos_ptr(i));
        EXPECT_EQ((uint16_t*) _pi_i.servos[i].stat, tx.get_stat_ptr(i));
    }
    pdo_get_output( _pi_o, svMap, (void *)&rx);
    for (int i=0; i<N; i++){
        EXPECT_EQ(_pi_o.servos[i].p, rx.get_pos_ptr(i));
        EXPECT_EQ((uint16_t*) _pi_o.servos[i].ctrl, rx.get_ctrl_ptr(i));
    }
}
TEST(eniParseServo, eni_servo_map){
    const char* fn= "../data/pse_servo1.xml";
    EniServoMapper svMap;
    int errCode = load_pdo_offset_from_file(_get_file_path(fn).c_str(),  svMap);
    EXPECT_EQ(errCode, 0);
    EXPECT_EQ(svMap.master_pdo.cycle_time, 1000);
    int N = svMap.slvInfo.size();
    EXPECT_EQ(N, 1);
    EXPECT_EQ(1001u, svMap.slvInfo.slaves[0].phys_addr);
    EXPECT_EQ("17185", svMap.slvInfo.slaves[0].vendor_id);
    EXPECT_EQ(svMap.slvInfo.get_rx_end_addr(), 64);
    EXPECT_EQ(svMap.slvInfo.get_tx_end_addr(), 152);
    _test_Eni_servo<PdoTxProtal3, PdoRxProtal3>(svMap);
}

TEST(eniParseServo, prints_parsed_configuration_on_request) {
    EniServoMapper mapper;
    const std::string path = _get_file_path("../data/pse_servo2.xml");
    testing::internal::CaptureStdout();
    const int ret = load_pdo_offset_from_file(path.c_str(), mapper);
    const std::string parse_output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(0, ret);
    EXPECT_TRUE(parse_output.empty());

    testing::internal::CaptureStdout();
    print_eni_servo_mapper(mapper);
    const std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(std::string::npos, output.find("cycle_time="));
    EXPECT_NE(std::string::npos, output.find("slaves=2 servos=2"));
    EXPECT_NE(std::string::npos, output.find("po_ctrl_word_offset:"));
}

TEST(eniParseServo, pio_layout_from_eni_servo_mapper){
    const char* fn= "../data/pse_servo1.xml";
    EniServoMapper svMap;
    int errCode = load_pdo_offset_from_file(_get_file_path(fn).c_str(),  svMap);
    ASSERT_EQ(errCode, 0);

    ZmcPioOffsetLayout layout;
    errCode = pio_layout_from_eni_servo_mapper(&svMap, &layout);

    ASSERT_EQ(errCode, 0);
    EXPECT_EQ(layout.slot_count, 1);
    EXPECT_EQ(layout.descriptor_pod.rx_cap, svMap.slvInfo.get_rx_end_addr() / 8u);
    EXPECT_EQ(layout.descriptor_pod.tx_cap, svMap.slvInfo.get_tx_end_addr() / 8u);
    EXPECT_EQ(layout.descriptor_pod.slave_count, svMap.size());
    EXPECT_EQ(layout.slots[0].slave_idx, svMap.servo_pdo[0].slave_idx);
    EXPECT_EQ(layout.slots[0].pi_stat_offset, svMap.servo_pdo[0].pi_stat_offset);
    EXPECT_EQ(layout.slots[0].po_ctrl_word_offset, svMap.servo_pdo[0].po_ctrl_word_offset);
}

TEST(eniParseServo, zioc_zcpb_servo_uses_eighth_slave) {
    EniServoMapper mapper;
    const std::string eni_path = _get_file_path("../data/zioc-zcpb.xml");
    ASSERT_EQ(0, load_pdo_offset_from_file(eni_path.c_str(), mapper));
    ASSERT_EQ(8u, mapper.slvInfo.slaves.size());
    ASSERT_EQ(1u, mapper.servo_pdo.size());
    EXPECT_EQ(1008u, mapper.slvInfo.slaves[7].phys_addr);
    EXPECT_EQ(65529u, mapper.slvInfo.slaves[7].auto_inc_addr);
    EXPECT_EQ("2103313", mapper.slvInfo.slaves[7].vendor_id);
    EXPECT_EQ("17039362", mapper.slvInfo.slaves[7].product_code);
    EXPECT_EQ("65536", mapper.slvInfo.slaves[7].revision_no);
    EXPECT_EQ(7u, mapper.servo_pdo[0].slave_idx);

    ZmcPioOffsetLayout layout{};
    ASSERT_EQ(0, pio_layout_from_eni_servo_mapper(&mapper, &layout));
    EXPECT_EQ(8u, layout.descriptor_pod.slave_count);
    EXPECT_EQ(7u, layout.slots[0].slave_idx);
}

// Real EtherCAT servo slots are independent from the 64-axis logical graph capacity.
TEST(eniParseServo, pio_layout_rejects_more_than_thirty_two_real_axes) {
    EniServoMapper mapper;
    mapper.servo_pdo.resize(MAX_NUM_SERVOS + 1u);
    ZmcPioOffsetLayout layout{};

    EXPECT_EQ(ZMC_ERR_ENI_AXIS_OVERFLOW, pio_layout_from_eni_servo_mapper(&mapper, &layout));
}

// Keep the Python ctypes mirror aligned with the C-compatible PDO layout contract.
TEST(eniParseServo, pio_layout_binary_contract_uses_thirty_two_servo_slots) {
    EXPECT_EQ(5144u, offsetof(ZmcPioOffsetLayout, slot_count));
    EXPECT_EQ(5148u, sizeof(ZmcPioOffsetLayout));
}

// Non-byte-aligned PDO ends still require the final process-image byte.
TEST(eniParseServo, pio_layout_rounds_bit_capacity_up) {
    EniServoMapper mapper;
    mapper.servo_pdo.push_back(pi_offset_config{});

    SLAVES_INFO_t slave{};
    pdo_info_t output{};
    output.index = 0x7000;
    output.bit_size = 1;
    output.bit_offset = 0;
    slave.outputs.push_back(output);

    pdo_info_t input{};
    input.index = 0x6000;
    input.bit_size = 1;
    input.bit_offset = 0;
    slave.inputs.push_back(input);
    mapper.slvInfo.slaves.push_back(slave);

    ZmcPioOffsetLayout layout{};
    ASSERT_EQ(0, pio_layout_from_eni_servo_mapper(&mapper, &layout));
    EXPECT_EQ(1u, layout.descriptor_pod.tx_cap);
    EXPECT_EQ(1u, layout.descriptor_pod.rx_cap);
    EXPECT_EQ(2u, layout.descriptor_pod.pdo_cell_count);
}

// A pure IO ENI has PDO cells but no CiA402 servo slot.
TEST(eniParseServo, pio_layout_supports_zero_servos) {
    EniServoMapper mapper;

    SLAVES_INFO_t slave{};
    pdo_info_t output{};
    output.index = 0x7000;
    output.bit_size = 8;
    slave.outputs.push_back(output);

    pdo_info_t input{};
    input.index = 0x6000;
    input.bit_size = 8;
    input.bit_offset = 40;
    slave.inputs.push_back(input);
    mapper.slvInfo.slaves.push_back(slave);

    SLAVES_INFO_t output_only_slave{};
    pdo_info_t output_only{};
    output_only.index = 0x7001;
    output_only.bit_size = 8;
    output_only.bit_offset = 56;
    output_only_slave.outputs.push_back(output_only);
    mapper.slvInfo.slaves.push_back(output_only_slave);

    ZmcPioOffsetLayout layout{};
    ASSERT_EQ(0, pio_layout_from_eni_servo_mapper(&mapper, &layout));
    EXPECT_EQ(0u, layout.slot_count);
    EXPECT_EQ(6u, layout.descriptor_pod.tx_cap);
    EXPECT_EQ(8u, layout.descriptor_pod.rx_cap);
    EXPECT_EQ(3u, layout.descriptor_pod.pdo_cell_count);
}

TEST(eniParseServo, eni_servo_map3){
    const char* fn= "../data/pse_servo3.xml";
    EniServoMapper svMap;
    int errCode = load_pdo_offset_from_file(_get_file_path(fn).c_str(),  svMap);
    EXPECT_EQ(errCode, 0);
    EXPECT_EQ(svMap.master_pdo.cycle_time, 1000);
    int N = svMap.slvInfo.size();
    EXPECT_EQ(N, 3);
    EXPECT_EQ(svMap.slvInfo.get_rx_end_addr(), 224);
    EXPECT_EQ(svMap.slvInfo.get_tx_end_addr(), 456);
    _test_Eni_servo<PdoTxProtal3, PdoRxProtal3>(svMap);
}

TEST(eniParseServo, eni_servo_map2){
    const char* fn= "../data/pse_OL3.xml";
    EniServoMapper svMap;
    int errCode = load_pdo_offset_from_file(_get_file_path(fn).c_str(),  svMap);
    EXPECT_EQ(errCode, 0);
    EXPECT_EQ(svMap.master_pdo.cycle_time, 1000);
    int N = svMap.slvInfo.size();
    EXPECT_EQ(N, 1);
    EXPECT_EQ(svMap.slvInfo.get_rx_end_addr(), 96);
    EXPECT_EQ(svMap.slvInfo.get_tx_end_addr(), 152);
    _test_Eni_servo<PdoTxOL, PdoRxOL>(svMap);
}
