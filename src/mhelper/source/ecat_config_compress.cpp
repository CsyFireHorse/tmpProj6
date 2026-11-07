#include "ecat_config_compress.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include <sstream>
#include <fstream>
#include "file_misc.hpp"
#include "ecat_config_embedded.h"

namespace zmc {
    static bool parse_init_cmd_to_friendly(const char* xml, size_t start_pos, size_t end_pos, FriendlyInitCmd& cmd) ;
    
// Simple XML parser helper - extracts text between tags
static std::string extract_xml_tag(const char* xml, const char* tag, size_t& pos) {
    std::string open_tag = "<" + std::string(tag) + ">";
    std::string close_tag = "</" + std::string(tag) + ">";
    
    const char* start_ptr = strstr(xml + pos, open_tag.c_str());
    if (!start_ptr) {
        pos = strlen(xml);
        return "";
    }
    
    size_t start = start_ptr - xml;
    if (start < pos) {
        pos = strlen(xml);
        return "";
    }
    
    start += open_tag.length();
    const char* end_ptr = strstr(xml + start, close_tag.c_str());
    if (!end_ptr) {
        pos = strlen(xml);
        return "";
    }
    
    size_t end = end_ptr - xml;
    if (end < start) {
        pos = strlen(xml);
        return "";
    }
    
    pos = end + close_tag.length();
    
    std::string content(xml + start, end - start);
    
    // Remove CDATA wrapper if present
    if (content.find("<![CDATA[") == 0) {
        size_t cdata_start = 9;
        size_t cdata_end = content.find("]]>");
        if (cdata_end != std::string::npos) {
            content = content.substr(cdata_start, cdata_end - cdata_start);
        }
    }
    
    return content;
}

// Bounded version with end position limit
static std::string extract_xml_tag_bounded(const char* xml, const char* tag, size_t start_pos, size_t end_pos) {
    std::string open_tag = "<" + std::string(tag) + ">";
    std::string close_tag = "</" + std::string(tag) + ">";
    
    const char* start_ptr = strstr(xml + start_pos, open_tag.c_str());
    if (!start_ptr || (start_ptr - xml) >= (long)end_pos) {
        return "";
    }
    
    size_t start = start_ptr - xml;
    if (start < start_pos) {
        return "";
    }
    
    start += open_tag.length();
    const char* end_ptr = strstr(xml + start, close_tag.c_str());
    if (!end_ptr || (end_ptr - xml) >= (long)end_pos) {
        return "";
    }
    
    size_t end = end_ptr - xml;
    if (end < start) {
        return "";
    }
    
    std::string content(xml + start, end - start);
    
    // Remove CDATA wrapper if present
    if (content.find("<![CDATA[") == 0) {
        size_t cdata_start = 9;
        size_t cdata_end = content.find("]]>");
        if (cdata_end != std::string::npos) {
            content = content.substr(cdata_start, cdata_end - cdata_start);
        }
    }
    
    return content;
}

// Extract attribute value from tag (e.g., <Slave Physics="YY"> -> "YY")
static std::string extract_xml_attribute(const char* tag_start, const char* attr_name) {
    const char* attr_pos = strstr(tag_start, attr_name);
    if (!attr_pos) return "";
    
    const char* quote1 = strchr(attr_pos, '"');
    if (!quote1) return "";
    
    const char* quote2 = strchr(quote1 + 1, '"');
    if (!quote2) return "";
    
    return std::string(quote1 + 1, quote2 - quote1 - 1);
}

// Extract multiple occurrences of a tag
static std::vector<std::string> extract_xml_tags(const char* xml, const char* tag, size_t& pos) {
    std::vector<std::string> results;
    size_t search_pos = pos;
    
    while (true) {
        std::string result = extract_xml_tag(xml, tag, search_pos);
        if (result.empty() && search_pos >= strlen(xml)) break;
        if (!result.empty()) {
            results.push_back(result);
        } else {
            break;
        }
    }
    
    pos = search_pos;
    return results;
}

// Parse hex string to number
static uint32_t parse_hex(const char* str) {
    if (!str || !*str) return 0;
    return (uint32_t)strtoul(str, nullptr, 16);
}

// Parse decimal string to number
static uint32_t parse_dec(const char* str) {
    if (!str || !*str) return 0;
    return (uint32_t)atoi(str);
}

// Parse MAC address (12 hex chars) to 6 bytes
static void parse_mac(const char* mac_str, uint8_t* mac_bytes) {
    if (!mac_str || strlen(mac_str) < 12) {
        memset(mac_bytes, 0, 6);
        return;
    }
    for (int i = 0; i < 6; i++) {
        char hex_byte[3] = {mac_str[i*2], mac_str[i*2+1], 0};
        mac_bytes[i] = (uint8_t)strtoul(hex_byte, nullptr, 16);
    }
}

TransitionState parse_transition(const char* trans_str) {
    if (!trans_str) return TransitionState::IP;
    
    if (strcmp(trans_str, "IP") == 0) return TransitionState::IP;
    if (strcmp(trans_str, "PI") == 0) return TransitionState::PI;
    if (strcmp(trans_str, "BI") == 0) return TransitionState::BI;
    if (strcmp(trans_str, "SI") == 0) return TransitionState::SI;
    if (strcmp(trans_str, "OI") == 0) return TransitionState::OI;
    if (strcmp(trans_str, "SP") == 0) return TransitionState::SP;
    if (strcmp(trans_str, "OP") == 0) return TransitionState::OP;
    if (strcmp(trans_str, "OS") == 0) return TransitionState::OS;
    if (strcmp(trans_str, "PS") == 0) return TransitionState::PS;
    if (strcmp(trans_str, "SO") == 0) return TransitionState::SO;
    if (strcmp(trans_str, "IB") == 0) return TransitionState::IB;
    if (strcmp(trans_str, "II") == 0) return TransitionState::II;
    
    return TransitionState::IP;
}

const char* transition_to_string(TransitionState state) {
    switch (state) {
        case TransitionState::IP: return "IP";
        case TransitionState::PI: return "PI";
        case TransitionState::BI: return "BI";
        case TransitionState::SI: return "SI";
        case TransitionState::OI: return "OI";
        case TransitionState::SP: return "SP";
        case TransitionState::OP: return "OP";
        case TransitionState::OS: return "OS";
        case TransitionState::PS: return "PS";
        case TransitionState::SO: return "SO";
        case TransitionState::IB: return "IB";
        case TransitionState::II: return "II";
        default: return "IP";
    }
}

uint8_t parse_cmd(const char* cmd_str) {
    if (!cmd_str) return 1;
    int cmd = atoi(cmd_str);
    return (uint8_t)cmd;
}

// Timeout compression: {100, 200, 3000, 5000, 10000} => {1, 2, 30, 50, 100}
uint8_t timeout_to_compact(uint16_t timeout) {
    if (timeout == 100) return 1;
    if (timeout == 200) return 2;
    if (timeout == 3000) return 30;
    if (timeout == 5000) return 50;
    if (timeout == 10000) return 100;
    return (uint8_t)(timeout / 100); // fallback
}

uint16_t timeout_from_compact(uint8_t val) {
    if (val == 1) return 100;
    if (val == 2) return 200;
    if (val == 30) return 3000;
    if (val == 50) return 5000;
    if (val == 100) return 10000;
    return val * 100; // fallback
}

// Pack transitions: 每2个Transition拼成1个字节
void pack_transitions(const uint8_t* transitions, uint8_t count, uint8_t* out) {
    for (uint8_t i = 0; i < count; i += 2) {
        uint8_t low = transitions[i] & 0x0F;
        uint8_t high = (i + 1 < count) ? (transitions[i + 1] & 0x0F) : 0;
        out[i / 2] = (high << 4) | low;
    }
}

void unpack_transitions(const uint8_t* packed, uint8_t count, uint8_t* out) {
    for (uint8_t i = 0; i < count; i += 2) {
        uint8_t byte = packed[i / 2];
        out[i] = byte & 0x0F;
        if (i + 1 < count) {
            out[i + 1] = (byte >> 4) & 0x0F;
        }
    }
}

// Parse InitCmd from XML and write to compressed buffer
static int parse_and_write_init_cmd(const char* xml, size_t& pos, std::vector<uint8_t>& buf) {
    size_t init_pos = pos;
    
    // Find InitCmd block boundaries
    const char* initcmd_start = strstr(xml + init_pos, "<InitCmd");
    if (!initcmd_start) return -1;
    
    const char* initcmd_end = strstr(initcmd_start, "</InitCmd>");
    if (!initcmd_end) return -1;
    
    size_t start_offset = initcmd_start - xml;
    size_t end_offset = initcmd_end - xml + strlen("</InitCmd>");
    
    // Parse to FriendlyInitCmd
    FriendlyInitCmd cmd;
    if (!parse_init_cmd_to_friendly(xml, start_offset, end_offset, cmd)) {
        printf("Error: Failed to parse InitCmd\n");
        return -1;
    }
    
    // Serialize using new variable-length protocol
    serialize_init_cmd(cmd, buf);
    
    // Update position
    pos = end_offset;
    return 0;
}

// Main parsing function using new serialization API
int parse_xml_to_compact(const char* xml_file, std::vector<uint8_t>& compressed_data) {
    // Parse XML to FriendlyEtherCATConfig
    FriendlyEtherCATConfig config;
    if (parse_xml_to_friendly(xml_file, config) != 0) {
        printf("Failed to parse XML to friendly structure\n");
        return -1;
    }
    
    
    // Write header: CRC_len + master_count + slave_count + process_image
    uint8_t crc_len = std::min((size_t)255, config.crc.length());
    compressed_data.push_back(crc_len);
    compressed_data.push_back((uint8_t)config.masters.size());
    compressed_data.push_back((uint8_t)config.slaves.size());
    
    // Write process_image
    compressed_data.push_back(config.process_image.input_byte_size & 0xFF);
    compressed_data.push_back((config.process_image.input_byte_size >> 8) & 0xFF);
    compressed_data.push_back(config.process_image.output_byte_size & 0xFF);
    compressed_data.push_back((config.process_image.output_byte_size >> 8) & 0xFF);
    
    // Write CRC
    if (crc_len > 0) {
        compressed_data.insert(compressed_data.end(), config.crc.begin(), config.crc.begin() + crc_len);
    }
    
    // Serialize masters
    for (const auto& master : config.masters) {
        serialize_master(master, compressed_data);
    }
    
    // Serialize slaves
    for (const auto& slave : config.slaves) {
        serialize_slave(slave, compressed_data);
    }
    
    printf("Compressed size: %zu bytes (compression ratio: variable-length)\n",
           compressed_data.size());
    
    return 0;
}

int save_compressed_config(const char* output_file, const std::vector<uint8_t>& compressed_data) {
    FILE* fp = fopen(output_file, "wb");
    if (!fp) {
        printf("Failed to create output file: %s\n", output_file);
        return -1;
    }
    
    size_t written = fwrite(compressed_data.data(), 1, compressed_data.size(), fp);
    fclose(fp);
    
    if (written != compressed_data.size()) {
        printf("Failed to write all data\n");
        return -1;
    }
    
    printf("Saved compressed config to: %s (%zu bytes)\n", output_file, written);
    return 0;
}

int load_compressed_config(const char* input_file, std::vector<uint8_t>& compressed_data) {
    FILE* fp = fopen(input_file, "rb");
    if (!fp) {
        printf("Failed to open compressed file: %s\n", input_file);
        return -1;
    }
    
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    compressed_data.resize(file_size);
    size_t read = fread(compressed_data.data(), 1, file_size, fp);
    fclose(fp);
    
    if (read != file_size) {
        printf("Failed to read all data\n");
        return -1;
    }
    
    printf("Loaded compressed config from: %s (%zu bytes)\n", input_file, read);
    return 0;
}

int decompress_to_xml(const std::vector<uint8_t>& compressed_data, const char* output_xml) {
    const uint8_t* data = compressed_data.data();
    size_t len = compressed_data.size();
    
    // Read header
    if (len < 7) {
        printf("Compressed data too small\n");
        return -1;
    }
    
    size_t offset = 0;
    uint8_t crc_len = data[offset++];
    uint8_t master_count = data[offset++];
    uint8_t slave_count = data[offset++];
    
    // Read process_image
    CompactProcessImage process_image;
    process_image.input_byte_size = data[offset] | (data[offset+1] << 8); offset += 2;
    process_image.output_byte_size = data[offset] | (data[offset+1] << 8); offset += 2;
    
    // Read CRC
    std::string crc;
    if (crc_len > 0) {
        if (offset + crc_len > len) return -1;
        crc.assign((const char*)(data + offset), crc_len);
        offset += crc_len;
    }
    
    // Deserialize masters
    FriendlyEtherCATConfig config;
    config.crc = crc;
    config.process_image = process_image;
    
    for (uint8_t i = 0; i < master_count; i++) {
        FriendlyMasterConfig master;
        if (!deserialize_master(data, len, offset, master)) {
            printf("Failed to deserialize master %d\n", i);
            return -1;
        }
        config.masters.push_back(master);
    }
    
    // Deserialize slaves
    for (uint8_t i = 0; i < slave_count; i++) {
        FriendlySlaveConfig slave;
        if (!deserialize_slave(data, len, offset, slave)) {
            printf("Failed to deserialize slave %d\n", i);
            return -1;
        }
        config.slaves.push_back(slave);
    }
    
    // Export to XML
    return export_friendly_to_xml(config, output_xml);
}

// ============================================================================
// Friendly Structure Implementation - XML parsing/export
// ============================================================================

// Parse single InitCmd to friendly structure
static bool parse_init_cmd_to_friendly(const char* xml, size_t start_pos, size_t end_pos, FriendlyInitCmd& cmd) {
    // Parse Transitions
    cmd.transitions.clear();
    size_t trans_pos = start_pos;
    const char* initcmd_end_ptr = xml + end_pos;
    
    const char* trans_tag = strstr(xml + start_pos, "<Transition>");
    while (trans_tag && trans_tag < initcmd_end_ptr) {
        size_t tag_pos = trans_tag - xml;
        std::string trans_str = extract_xml_tag(xml, "Transition", tag_pos);
        if (!trans_str.empty()) {
            cmd.transitions.push_back(parse_transition(trans_str.c_str()));
        }
        trans_tag = strstr(xml + tag_pos + 1, "<Transition>");
    }
    
    // Parse BeforeSlave
    std::string before_slave = extract_xml_tag_bounded(xml, "BeforeSlave", start_pos, end_pos);
    cmd.before_slave = !before_slave.empty() && parse_dec(before_slave.c_str()) == 1;
    
    // Parse Cmd
    std::string cmd_str = extract_xml_tag_bounded(xml, "Cmd", start_pos, end_pos);
    cmd.cmd = parse_cmd(cmd_str.c_str());
    
    // Parse Adp
    std::string adp_str = extract_xml_tag_bounded(xml, "Adp", start_pos, end_pos);
    cmd.adp = (uint16_t)parse_dec(adp_str.c_str());
    
    // Parse Ado
    std::string ado_str = extract_xml_tag_bounded(xml, "Ado", start_pos, end_pos);
    cmd.ado = (uint16_t)parse_dec(ado_str.c_str());
    
    // Parse Retries
    std::string retries_str = extract_xml_tag_bounded(xml, "Retries", start_pos, end_pos);
    cmd.retries = (uint16_t)parse_dec(retries_str.c_str());
    if (cmd.retries == 0) cmd.retries = 3;  // default
    
    // Parse Comment (optional)
    cmd.comment = extract_xml_tag_bounded(xml, "Comment", start_pos, end_pos);
    
    // Parse Requires (optional)
    cmd.requires = extract_xml_tag_bounded(xml, "Requires", start_pos, end_pos);
    
    // Parse Data or DataLength
    std::string data_str = extract_xml_tag_bounded(xml, "Data", start_pos, end_pos);
    std::string data_len_str = extract_xml_tag_bounded(xml, "DataLength", start_pos, end_pos);
    
    cmd.data.clear();
    if (!data_len_str.empty()) {
        cmd.has_data_length = true;
        cmd.data_length = (uint16_t)parse_dec(data_len_str.c_str());
    } else if (!data_str.empty()) {
        cmd.has_data_length = false;
        // Convert hex string to bytes
        for (size_t i = 0; i + 1 < data_str.length(); i += 2) {
            char hex_byte[3] = {data_str[i], data_str[i+1], 0};
            cmd.data.push_back((uint8_t)strtoul(hex_byte, nullptr, 16));
        }
    }
    
    // Parse Validate (optional)
    const char* validate_start = strstr(xml + start_pos, "<Validate>");
    const char* validate_end = strstr(xml + start_pos, "</Validate>");
    if (validate_start && validate_end && validate_start < initcmd_end_ptr) {
        cmd.has_validate = true;
        size_t val_start_pos = validate_start - xml;
        size_t val_end_pos = validate_end - xml;
        
        std::string val_data = extract_xml_tag_bounded(xml, "Data", val_start_pos, val_end_pos);
        std::string val_mask = extract_xml_tag_bounded(xml, "DataMask", val_start_pos, val_end_pos);
        std::string val_timeout = extract_xml_tag_bounded(xml, "Timeout", val_start_pos, val_end_pos);
        
        cmd.validate.data.clear();
        cmd.validate.mask.clear();
        
        if (!val_data.empty()) {
            for (size_t i = 0; i + 1 < val_data.length(); i += 2) {
                char hex_byte[3] = {val_data[i], val_data[i+1], 0};
                cmd.validate.data.push_back((uint8_t)strtoul(hex_byte, nullptr, 16));
            }
        }
        
        if (!val_mask.empty()) {
            for (size_t i = 0; i + 1 < val_mask.length(); i += 2) {
                char hex_byte[3] = {val_mask[i], val_mask[i+1], 0};
                cmd.validate.mask.push_back((uint8_t)strtoul(hex_byte, nullptr, 16));
            }
        }
        
        cmd.validate.timeout = !val_timeout.empty() ? (uint16_t)parse_dec(val_timeout.c_str()) : 0;
    } else {
        cmd.has_validate = false;
    }
    
    return true;
}

// Parse XML file to friendly structure
int parse_xml_to_friendly(const char* xml_file, FriendlyEtherCATConfig& config) {
    FILE* fp = fopen(xml_file, "rb");
    if (!fp) {
        printf("Failed to open XML file: %s\n", xml_file);
        return -1;
    }
    
    fseek(fp, 0, SEEK_END);
    size_t file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    std::vector<char> xml_buffer(file_size + 1);
    fread(xml_buffer.data(), 1, file_size, fp);
    xml_buffer[file_size] = 0;
    fclose(fp);
    
    const char* xml = xml_buffer.data();
    
    // Clear config
    config.masters.clear();
    config.slaves.clear();
    
    // Parse CRC
    const char* crc_start = strstr(xml, "<CRC>");
    const char* crc_end = strstr(xml, "</CRC>");
    if (crc_start && crc_end) {
        config.crc = std::string(crc_start + 5, crc_end - crc_start - 5);
    }
    
    // Find Config section
    const char* config_start = strstr(xml, "<Config>");
    if (!config_start) {
        printf("Config section not found\n");
        return -1;
    }
    
    // Parse Master
    const char* master_start = strstr(config_start, "<Master>");
    if (master_start) {
        FriendlyMasterConfig master;
        size_t master_pos = master_start - xml;
        size_t temp_pos;
        
        // Parse Master fields
        temp_pos = master_pos;
        master.nic = extract_xml_tag(xml, "NIC", temp_pos);
        temp_pos = master_pos;
        master.rnic = extract_xml_tag(xml, "RNIC", temp_pos);
        temp_pos = master_pos;
        master.initial_state = (uint8_t)parse_dec(extract_xml_tag(xml, "InitialMasterState", temp_pos).c_str());
        temp_pos = master_pos;
        master.cycle_time = (uint16_t)parse_dec(extract_xml_tag(xml, "CycleTime", temp_pos).c_str());
        temp_pos = master_pos;
        master.mailbox_cycle_time = (uint16_t)parse_dec(extract_xml_tag(xml, "MailboxCycleTime", temp_pos).c_str());
        temp_pos = master_pos;
        master.auto_recovery_timeout = (uint8_t)parse_dec(extract_xml_tag(xml, "AutoRecoveryTimeout", temp_pos).c_str());
        temp_pos = master_pos;
        master.process_image_update_period = (uint16_t)parse_dec(extract_xml_tag(xml, "ProcessImageUpdatePeriod", temp_pos).c_str());
        temp_pos = master_pos;
        master.statistics_update_period = (uint16_t)parse_dec(extract_xml_tag(xml, "StatisticsUpdatePeriod", temp_pos).c_str());
        temp_pos = master_pos;
        master.max_frame_size = (uint16_t)parse_dec(extract_xml_tag(xml, "MaxFrameSize", temp_pos).c_str());
        temp_pos = master_pos;
        master.separate_io_update = parse_dec(extract_xml_tag(xml, "SeparateIOUpdate", temp_pos).c_str()) != 0;
        temp_pos = master_pos;
        master.dc_user_shift_time = parse_dec(extract_xml_tag(xml, "DCUserShiftTime", temp_pos).c_str());
        
        // Parse MasterInfo
        const char* info_start = strstr(xml + master_pos, "<Info>");
        if (info_start) {
            size_t info_pos = info_start - xml;
            temp_pos = info_pos;
            master.info.name = extract_xml_tag(xml, "Name", temp_pos);
            temp_pos = info_pos;
            std::string dest_mac = extract_xml_tag(xml, "Destination", temp_pos);
            parse_mac(dest_mac.c_str(), master.info.dest_mac);
            temp_pos = info_pos;
            std::string src_mac = extract_xml_tag(xml, "Source", temp_pos);
            parse_mac(src_mac.c_str(), master.info.src_mac);
            temp_pos = info_pos;
            master.info.ether_type = (uint16_t)parse_hex(extract_xml_tag(xml, "EtherType", temp_pos).c_str());
        }
        
        // Parse Master InitCmds
        const char* initcmds_start = strstr(xml + master_pos, "<InitCmds>");
        const char* master_end = strstr(master_start, "</Master>");
        if (initcmds_start && master_end) {
            const char* initcmd = strstr(initcmds_start, "<InitCmd>");
            while (initcmd && initcmd < master_end) {
                const char* initcmd_end = strstr(initcmd, "</InitCmd>");
                const char* next_initcmd = strstr(initcmd + 1, "<InitCmd>");
                
                if (initcmd_end && (!next_initcmd || next_initcmd > initcmd_end)) {
                    FriendlyInitCmd cmd;
                    if (parse_init_cmd_to_friendly(xml, initcmd - xml, initcmd_end - xml, cmd)) {
                        master.init_cmds.push_back(cmd);
                    }
                }
                
                initcmd = next_initcmd;
            }
        }
        
        config.masters.push_back(master);
    }
    
    // Parse Slaves
    const char* slave = strstr(config_start, "<Slave");
    while (slave) {
        const char* slave_end = strstr(slave, "</Slave>");
        if (!slave_end) break;
        
        FriendlySlaveConfig slave_cfg;
        size_t slave_pos = slave - xml;
        size_t temp_pos;
        
        // Parse Slave attributes from opening tag
        slave_cfg.physics = extract_xml_attribute(slave, "Physics");
        if (slave_cfg.physics.empty()) slave_cfg.physics = "N";
        
        std::string hotplug_attr = extract_xml_attribute(slave, "HotPlug");
        slave_cfg.hot_plug = !hotplug_attr.empty() && parse_dec(hotplug_attr.c_str()) != 0;
        
        std::string segment_attr = extract_xml_attribute(slave, "Segment");
        slave_cfg.segment = !segment_attr.empty() ? (uint8_t)parse_dec(segment_attr.c_str()) : 0;
        
        std::string seg_sub_attr = extract_xml_attribute(slave, "SegmentSubIndex");
        slave_cfg.segment_sub_index = !seg_sub_attr.empty() ? (uint8_t)parse_dec(seg_sub_attr.c_str()) : 0;
        
        // Parse SlaveInfo
        const char* info_start = strstr(xml + slave_pos, "<Info>");
        if (info_start && info_start < slave_end) {
            size_t info_pos = info_start - xml;
            temp_pos = info_pos;
            slave_cfg.info.name = extract_xml_tag(xml, "Name", temp_pos);
            temp_pos = info_pos;
            slave_cfg.info.phys_addr = (uint16_t)parse_dec(extract_xml_tag(xml, "PhysAddr", temp_pos).c_str());
            temp_pos = info_pos;
            slave_cfg.info.auto_inc_addr = (uint16_t)parse_dec(extract_xml_tag(xml, "AutoIncAddr", temp_pos).c_str());
            temp_pos = info_pos;
            slave_cfg.info.physics = extract_xml_tag(xml, "Physics", temp_pos);
            if (slave_cfg.info.physics.empty()) slave_cfg.info.physics = "N";
            temp_pos = info_pos;
            slave_cfg.info.vendor_id = parse_dec(extract_xml_tag(xml, "VendorId", temp_pos).c_str());
            temp_pos = info_pos;
            slave_cfg.info.product_code = parse_dec(extract_xml_tag(xml, "ProductCode", temp_pos).c_str());
            temp_pos = info_pos;
            slave_cfg.info.revision_no = parse_dec(extract_xml_tag(xml, "RevisionNo", temp_pos).c_str());
            temp_pos = info_pos;
            slave_cfg.info.serial_no = parse_dec(extract_xml_tag(xml, "SerialNo", temp_pos).c_str());
            temp_pos = info_pos;
            slave_cfg.info.product_revision = extract_xml_tag(xml, "ProductRevision", temp_pos);
        }
        
        // Parse Slave InitCmds
        const char* initcmds_start = strstr(xml + slave_pos, "<InitCmds>");
        if (initcmds_start && initcmds_start < slave_end) {
            const char* initcmd = strstr(initcmds_start, "<InitCmd>");
            while (initcmd && initcmd < slave_end) {
                const char* initcmd_end = strstr(initcmd, "</InitCmd>");
                const char* next_initcmd = strstr(initcmd + 1, "<InitCmd>");
                
                if (initcmd_end && (!next_initcmd || next_initcmd > initcmd_end)) {
                    FriendlyInitCmd cmd;
                    if (parse_init_cmd_to_friendly(xml, initcmd - xml, initcmd_end - xml, cmd)) {
                        slave_cfg.init_cmds.push_back(cmd);
                    }
                }
                
                initcmd = next_initcmd;
            }
        }
        
        config.slaves.push_back(slave_cfg);
        slave = strstr(slave_end, "<Slave");
    }
    
    // Parse ProcessImage
    const char* pi_start = strstr(config_start, "<ProcessImage>");
    if (pi_start) {
        size_t pi_pos = pi_start - xml;
        size_t temp_pos;
        
        temp_pos = pi_pos;
        const char* inputs = strstr(xml + pi_pos, "<Inputs>");
        if (inputs) {
            temp_pos = inputs - xml;
            config.process_image.input_byte_size = (uint16_t)parse_dec(extract_xml_tag(xml, "ByteSize", temp_pos).c_str());
        }
        
        temp_pos = pi_pos;
        const char* outputs = strstr(xml + pi_pos, "<Outputs>");
        if (outputs) {
            temp_pos = outputs - xml;
            config.process_image.output_byte_size = (uint16_t)parse_dec(extract_xml_tag(xml, "ByteSize", temp_pos).c_str());
        }
    }
    
    printf("Parsed XML to friendly structure: %zu masters, %zu slaves\n", 
           config.masters.size(), config.slaves.size());
    return 0;
}

// Export single InitCmd to XML string
static std::string export_init_cmd_to_xml(const FriendlyInitCmd& cmd, int indent_level = 3) {
    std::string indent(indent_level * 2, ' ');
    std::string inner_indent = indent + "  ";
    std::string result;
    
    result += indent + "<InitCmd>\n";
    
    // Transitions
    for (const auto& trans : cmd.transitions) {
        result += inner_indent + "<Transition>" + transition_to_string(trans) + "</Transition>\n";
    }
    
    // BeforeSlave
    if (cmd.before_slave) {
        result += inner_indent + "<BeforeSlave>1</BeforeSlave>\n";
    }
    
    // Comment (optional)
    if (!cmd.comment.empty()) {
        result += inner_indent + "<Comment><![CDATA[" + cmd.comment + "]]></Comment>\n";
    }
    
    // Requires (optional)
    if (!cmd.requires.empty()) {
        result += inner_indent + "<Requires>" + cmd.requires + "</Requires>\n";
    }
    
    // Cmd
    char cmd_buf[32];
    snprintf(cmd_buf, sizeof(cmd_buf), "%d", cmd.cmd);
    result += inner_indent + "<Cmd>" + cmd_buf + "</Cmd>\n";
    
    // Adp
    snprintf(cmd_buf, sizeof(cmd_buf), "%u", cmd.adp);
    result += inner_indent + "<Adp>" + cmd_buf + "</Adp>\n";
    
    // Ado
    snprintf(cmd_buf, sizeof(cmd_buf), "%u", cmd.ado);
    result += inner_indent + "<Ado>" + cmd_buf + "</Ado>\n";
    
    // Data or DataLength
    if (cmd.has_data_length) {
        snprintf(cmd_buf, sizeof(cmd_buf), "%u", cmd.data_length);
        result += inner_indent + "<DataLength>" + cmd_buf + "</DataLength>\n";
    } else if (!cmd.data.empty()) {
        result += inner_indent + "<Data>";
        for (uint8_t byte : cmd.data) {
            snprintf(cmd_buf, sizeof(cmd_buf), "%02X", byte);
            result += cmd_buf;
        }
        result += "</Data>\n";
    }
    
    // Retries (always output)
    snprintf(cmd_buf, sizeof(cmd_buf), "%u", cmd.retries);
    result += inner_indent + "<Retries>" + cmd_buf + "</Retries>\n";
    
    // Validate (optional)
    if (cmd.has_validate) {
        result += inner_indent + "<Validate>\n";
        std::string val_indent = inner_indent + "  ";
        
        // Data
        if (!cmd.validate.data.empty()) {
            result += val_indent + "<Data>";
            for (uint8_t byte : cmd.validate.data) {
                snprintf(cmd_buf, sizeof(cmd_buf), "%02X", byte);
                result += cmd_buf;
            }
            result += "</Data>\n";
        }
        
        // DataMask (optional)
        if (!cmd.validate.mask.empty()) {
            result += val_indent + "<DataMask>";
            for (uint8_t byte : cmd.validate.mask) {
                snprintf(cmd_buf, sizeof(cmd_buf), "%02X", byte);
                result += cmd_buf;
            }
            result += "</DataMask>\n";
        }
        
        // Timeout
        snprintf(cmd_buf, sizeof(cmd_buf), "%u", cmd.validate.timeout);
        result += val_indent + "<Timeout>" + cmd_buf + "</Timeout>\n";
        
        result += inner_indent + "</Validate>\n";
    }
    
    result += indent + "</InitCmd>\n";
    return result;
}

// Export friendly structure to XML file
int export_friendly_to_xml(const FriendlyEtherCATConfig& config, const char* output_xml) {
    FILE* fp = fopen(output_xml, "w");
    if (!fp) {
        printf("Failed to create output XML file: %s\n", output_xml);
        return -1;
    }
    
    // Write XML header
    fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
    fprintf(fp, "<EtherCATConfig>\n");
    
    // Write CRC
    if (!config.crc.empty()) {
        fprintf(fp, "  <CRC>%s</CRC>\n", config.crc.c_str());
    }
    
    fprintf(fp, "  <Config>\n");
    
    // Write Masters
    for (const auto& master : config.masters) {
        fprintf(fp, "    <Master>\n");
        
        // Master fields - always output NIC/RNIC even if empty
        fprintf(fp, "      <NIC><![CDATA[%s]]></NIC>\n", master.nic.c_str());
        fprintf(fp, "      <RNIC><![CDATA[%s]]></RNIC>\n", master.rnic.c_str());
        fprintf(fp, "      <InitialMasterState>%u</InitialMasterState>\n", master.initial_state);
        fprintf(fp, "      <CycleTime>%u</CycleTime>\n", master.cycle_time);
        fprintf(fp, "      <MailboxCycleTime>%u</MailboxCycleTime>\n", master.mailbox_cycle_time);
        fprintf(fp, "      <AutoRecoveryTimeout>%u</AutoRecoveryTimeout>\n", master.auto_recovery_timeout);
        fprintf(fp, "      <ProcessImageUpdatePeriod>%u</ProcessImageUpdatePeriod>\n", master.process_image_update_period);
        fprintf(fp, "      <StatisticsUpdatePeriod>%u</StatisticsUpdatePeriod>\n", master.statistics_update_period);
        fprintf(fp, "      <MaxFrameSize>%u</MaxFrameSize>\n", master.max_frame_size);
        fprintf(fp, "      <SeparateIOUpdate>%d</SeparateIOUpdate>\n", master.separate_io_update ? 1 : 0);
        fprintf(fp, "      <DCUserShiftTime>%u</DCUserShiftTime>\n", master.dc_user_shift_time);
        
        // MasterInfo
        fprintf(fp, "      <Info>\n");
        fprintf(fp, "        <Name><![CDATA[%s]]></Name>\n", master.info.name.c_str());
        fprintf(fp, "        <Destination>%02X%02X%02X%02X%02X%02X</Destination>\n",
                master.info.dest_mac[0], master.info.dest_mac[1], master.info.dest_mac[2],
                master.info.dest_mac[3], master.info.dest_mac[4], master.info.dest_mac[5]);
        fprintf(fp, "        <Source>%02X%02X%02X%02X%02X%02X</Source>\n",
                master.info.src_mac[0], master.info.src_mac[1], master.info.src_mac[2],
                master.info.src_mac[3], master.info.src_mac[4], master.info.src_mac[5]);
        fprintf(fp, "        <EtherType>%04x</EtherType>\n", master.info.ether_type);
        fprintf(fp, "      </Info>\n");
        
        // Master InitCmds
        if (!master.init_cmds.empty()) {
            fprintf(fp, "      <InitCmds>\n");
            for (const auto& cmd : master.init_cmds) {
                std::string cmd_xml = export_init_cmd_to_xml(cmd, 4);  // indent level 4
                fprintf(fp, "%s", cmd_xml.c_str());
            }
            fprintf(fp, "      </InitCmds>\n");
        }
        
        fprintf(fp, "    </Master>\n");
    }
    
    // Write Slaves
    for (const auto& slave : config.slaves) {
        // Slave tag with attributes
        fprintf(fp, "    <Slave Physics=\"%s\" HotPlug=\"%d\" Segment=\"%u\" SegmentSubIndex=\"%u\">\n",
                slave.physics.c_str(),
                slave.hot_plug ? 1 : 0,
                slave.segment,
                slave.segment_sub_index);
        
        // SlaveInfo
        fprintf(fp, "      <Info>\n");
        fprintf(fp, "        <Name><![CDATA[%s]]></Name>\n", slave.info.name.c_str());
        fprintf(fp, "        <PhysAddr>%u</PhysAddr>\n", slave.info.phys_addr);
        fprintf(fp, "        <AutoIncAddr>%u</AutoIncAddr>\n", slave.info.auto_inc_addr);
        fprintf(fp, "        <Physics>%s</Physics>\n", slave.info.physics.c_str());
        fprintf(fp, "        <VendorId>%u</VendorId>\n", slave.info.vendor_id);
        fprintf(fp, "        <ProductCode>%u</ProductCode>\n", slave.info.product_code);
        fprintf(fp, "        <RevisionNo>%u</RevisionNo>\n", slave.info.revision_no);
        fprintf(fp, "        <SerialNo>%u</SerialNo>\n", slave.info.serial_no);
        fprintf(fp, "        <ProductRevision>%s</ProductRevision>\n", slave.info.product_revision.c_str());
        fprintf(fp, "      </Info>\n");
        
        // Slave InitCmds
        if (!slave.init_cmds.empty()) {
            fprintf(fp, "      <InitCmds>\n");
            for (const auto& cmd : slave.init_cmds) {
                std::string cmd_xml = export_init_cmd_to_xml(cmd, 4);  // indent level 4
                fprintf(fp, "%s", cmd_xml.c_str());
            }
            fprintf(fp, "      </InitCmds>\n");
        }
        
        fprintf(fp, "    </Slave>\n");
    }
    
    // Write ProcessImage
    fprintf(fp, "    <ProcessImage>\n");
    fprintf(fp, "      <Inputs>\n");
    fprintf(fp, "        <ByteSize>%u</ByteSize>\n", config.process_image.input_byte_size);
    fprintf(fp, "      </Inputs>\n");
    fprintf(fp, "      <Outputs>\n");
    fprintf(fp, "        <ByteSize>%u</ByteSize>\n", config.process_image.output_byte_size);
    fprintf(fp, "      </Outputs>\n");
    fprintf(fp, "    </ProcessImage>\n");
    
    fprintf(fp, "  </Config>\n");
    fprintf(fp, "</EtherCATConfig>\n");
    
    fclose(fp);
    printf("Exported friendly structure to XML: %s\n", output_xml);
    return 0;
}

// Test XML roundtrip: parse -> export -> parse -> compare
int test_xml_roundtrip(const char* input_xml, const char* output_xml) {
    printf("\n=== XML Roundtrip Test ===\n");
    printf("Input:  %s\n", input_xml);
    printf("Output: %s\n\n", output_xml);
    
    // Step 1: Parse input XML to friendly structure
    FriendlyEtherCATConfig config1;
    int ret = parse_xml_to_friendly(input_xml, config1);
    if (ret != 0) {
        printf("ERROR: Failed to parse input XML\n");
        return -1;
    }
    
    printf("Step 1: Parsed input XML\n");
    printf("  Masters: %zu\n", config1.masters.size());
    if (!config1.masters.empty()) {
        printf("    Master[0] InitCmds: %zu\n", config1.masters[0].init_cmds.size());
    }
    printf("  Slaves: %zu\n", config1.slaves.size());
    for (size_t i = 0; i < config1.slaves.size(); ++i) {
        printf("    Slave[%zu] InitCmds: %zu\n", i, config1.slaves[i].init_cmds.size());
    }
    
    // Step 2: Export friendly structure to XML
    ret = export_friendly_to_xml(config1, output_xml);
    if (ret != 0) {
        printf("ERROR: Failed to export to XML\n");
        return -1;
    }
    
    printf("\nStep 2: Exported to XML\n");
    
    // Step 3: Parse output XML to friendly structure
    FriendlyEtherCATConfig config2;
    ret = parse_xml_to_friendly(output_xml, config2);
    if (ret != 0) {
        printf("ERROR: Failed to parse output XML\n");
        return -1;
    }
    
    printf("\nStep 3: Parsed output XML\n");
    printf("  Masters: %zu\n", config2.masters.size());
    if (!config2.masters.empty()) {
        printf("    Master[0] InitCmds: %zu\n", config2.masters[0].init_cmds.size());
    }
    printf("  Slaves: %zu\n", config2.slaves.size());
    for (size_t i = 0; i < config2.slaves.size(); ++i) {
        printf("    Slave[%zu] InitCmds: %zu\n", i, config2.slaves[i].init_cmds.size());
    }
    
    // Step 4: Compare structures
    printf("\n=== Comparison ===\n");
    
    bool passed = true;
    
    // Compare basic counts
    if (config1.masters.size() != config2.masters.size()) {
        printf("FAIL: Master count mismatch (%zu vs %zu)\n", 
               config1.masters.size(), config2.masters.size());
        passed = false;
    }
    
    if (config1.slaves.size() != config2.slaves.size()) {
        printf("FAIL: Slave count mismatch (%zu vs %zu)\n", 
               config1.slaves.size(), config2.slaves.size());
        passed = false;
    }
    
    // Compare Master InitCmds
    if (!config1.masters.empty() && !config2.masters.empty()) {
        const auto& m1 = config1.masters[0];
        const auto& m2 = config2.masters[0];
        
        if (m1.init_cmds.size() != m2.init_cmds.size()) {
            printf("FAIL: Master InitCmd count mismatch (%zu vs %zu)\n",
                   m1.init_cmds.size(), m2.init_cmds.size());
            passed = false;
        } else {
            printf("OK: Master InitCmd count: %zu\n", m1.init_cmds.size());
            
            // Compare each InitCmd
            for (size_t i = 0; i < m1.init_cmds.size(); ++i) {
                const auto& cmd1 = m1.init_cmds[i];
                const auto& cmd2 = m2.init_cmds[i];
                
                if (cmd1.transitions.size() != cmd2.transitions.size()) {
                    printf("FAIL: Master InitCmd[%zu] transition count mismatch (%zu vs %zu)\n",
                           i, cmd1.transitions.size(), cmd2.transitions.size());
                    passed = false;
                }
                
                if (cmd1.cmd != cmd2.cmd || cmd1.adp != cmd2.adp || cmd1.ado != cmd2.ado) {
                    printf("FAIL: Master InitCmd[%zu] field mismatch\n", i);
                    passed = false;
                }
                
                if (cmd1.has_data_length != cmd2.has_data_length) {
                    printf("FAIL: Master InitCmd[%zu] has_data_length mismatch\n", i);
                    passed = false;
                }
                
                if (cmd1.has_data_length && cmd1.data_length != cmd2.data_length) {
                    printf("FAIL: Master InitCmd[%zu] data_length mismatch (%u vs %u)\n",
                           i, cmd1.data_length, cmd2.data_length);
                    passed = false;
                }
                
                if (!cmd1.has_data_length && cmd1.data != cmd2.data) {
                    printf("FAIL: Master InitCmd[%zu] data mismatch\n", i);
                    passed = false;
                }
                
                if (cmd1.has_validate != cmd2.has_validate) {
                    printf("FAIL: Master InitCmd[%zu] has_validate mismatch\n", i);
                    passed = false;
                }
            }
        }
    }
    
    // Compare Slave InitCmds
    for (size_t s = 0; s < std::min(config1.slaves.size(), config2.slaves.size()); ++s) {
        const auto& s1 = config1.slaves[s];
        const auto& s2 = config2.slaves[s];
        
        if (s1.init_cmds.size() != s2.init_cmds.size()) {
            printf("FAIL: Slave[%zu] InitCmd count mismatch (%zu vs %zu)\n",
                   s, s1.init_cmds.size(), s2.init_cmds.size());
            passed = false;
        } else {
            printf("OK: Slave[%zu] InitCmd count: %zu\n", s, s1.init_cmds.size());
            
            // Compare each InitCmd
            for (size_t i = 0; i < s1.init_cmds.size(); ++i) {
                const auto& cmd1 = s1.init_cmds[i];
                const auto& cmd2 = s2.init_cmds[i];
                
                if (cmd1.transitions.size() != cmd2.transitions.size()) {
                    printf("FAIL: Slave[%zu] InitCmd[%zu] transition count mismatch (%zu vs %zu)\n",
                           s, i, cmd1.transitions.size(), cmd2.transitions.size());
                    passed = false;
                }
                
                if (cmd1.cmd != cmd2.cmd || cmd1.adp != cmd2.adp || cmd1.ado != cmd2.ado) {
                    printf("FAIL: Slave[%zu] InitCmd[%zu] field mismatch\n", s, i);
                    passed = false;
                }
            }
        }
    }
    
    // Final result
    printf("\n=== Test Result: %s ===\n", passed ? "PASSED" : "FAILED");
    return passed ? 0 : -1;
}

// ============================================================================
// Variable-Length Serialization Implementation
// ============================================================================

// Serialize InitCmd to byte stream
size_t serialize_init_cmd(const FriendlyInitCmd& cmd, std::vector<uint8_t>& buffer) {
    size_t start = buffer.size();
    
    // 1. Calculate flags
    uint8_t flags = 0;
    if (cmd.before_slave) flags |= INITCMD_FLAG_BEFORE_SLAVE;
    
    // DataLength vs Data (mutually exclusive)
    if (cmd.has_data_length) {
        flags |= INITCMD_FLAG_HAS_DATA_LENGTH;
        // DataLength value will be written explicitly as uint16_t
    }
    // else: Data mode will need length byte + data bytes later
    
    // Requires field (only "cycle" is supported via flag)
    if (cmd.requires == "cycle") {
        flags |= INITCMD_FLAG_REQUIRES_CYCLE;
    }
    
    if (cmd.adp == 1001) flags |= INITCMD_FLAG_ADP_1001;
    if (cmd.retries > 3) flags |= INITCMD_FLAG_RETRIES_5000;
    if (cmd.has_validate) flags |= INITCMD_FLAG_HAS_VALIDATE;
    if (cmd.has_validate && !cmd.validate.mask.empty()) 
        flags |= INITCMD_FLAG_HAS_DATA_MASK;
    
    buffer.push_back(flags);
    
    // 2. cmd (4 bits) + transition_len (4 bits)
    uint8_t trans_len = std::min((size_t)15, cmd.transitions.size());
    uint8_t cmd_trans_byte = (trans_len << 4) | (cmd.cmd & 0x0F);
    buffer.push_back(cmd_trans_byte);
    
    // 3. Packed transitions (variable length, always present)
    size_t packed_len = (trans_len + 1) / 2;
    for (size_t i = 0; i < packed_len; i++) {
        uint8_t packed = (uint8_t)cmd.transitions[i*2];
        if (i*2+1 < trans_len) {
            packed |= ((uint8_t)cmd.transitions[i*2+1]) << 4;
        }
        buffer.push_back(packed);
    }
    
    // 4. ado (uint16_t, little-endian)
    buffer.push_back(cmd.ado & 0xFF);
    buffer.push_back((cmd.ado >> 8) & 0xFF);
    
    // 5. DataLength vs Data mode
    if (cmd.has_data_length) {
        // DataLength mode: write explicit uint16_t (little-endian)
        buffer.push_back(cmd.data_length & 0xFF);
        buffer.push_back((cmd.data_length >> 8) & 0xFF);
    } else {
        // Data mode: write length byte + data
        uint8_t data_len = cmd.data.size();
        buffer.push_back(data_len);  // Length descriptor (no constraints!)
        
        if (cmd.has_validate) {
            buffer.push_back(timeout_to_compact(cmd.validate.timeout));
            buffer.insert(buffer.end(), cmd.data.begin(), cmd.data.end());
            buffer.insert(buffer.end(), cmd.validate.data.begin(), cmd.validate.data.end());
            if (!cmd.validate.mask.empty()) {
                buffer.insert(buffer.end(), cmd.validate.mask.begin(), cmd.validate.mask.end());
            }
        } else {
            buffer.insert(buffer.end(), cmd.data.begin(), cmd.data.end());
        }
    }
    
    return buffer.size() - start;
}

// Deserialize InitCmd from byte stream
bool deserialize_init_cmd(const uint8_t* data, size_t len, size_t& offset, FriendlyInitCmd& cmd) {
    if (offset >= len) return false;
    
    // 1. Read flags
    uint8_t flags = data[offset++];
    cmd.before_slave = flags & INITCMD_FLAG_BEFORE_SLAVE;
    cmd.has_data_length = flags & INITCMD_FLAG_HAS_DATA_LENGTH;
    cmd.has_validate = flags & INITCMD_FLAG_HAS_VALIDATE;
    
    // Requires field (comment is not serialized, always empty)
    cmd.requires = (flags & INITCMD_FLAG_REQUIRES_CYCLE) ? "cycle" : "";
    cmd.comment = "";  // Comment is never saved to binary stream
    
    // 2. Read cmd + transition_len
    if (offset >= len) return false;
    uint8_t cmd_trans_byte = data[offset++];
    cmd.cmd = cmd_trans_byte & 0x0F;
    uint8_t trans_len = (cmd_trans_byte >> 4) & 0x0F;
    
    // 3. Read packed transitions
    size_t packed_len = (trans_len + 1) / 2;
    cmd.transitions.clear();
    for (size_t i = 0; i < packed_len; i++) {
        if (offset >= len) return false;
        uint8_t packed = data[offset++];
        cmd.transitions.push_back((TransitionState)(packed & 0x0F));
        if (i*2+1 < trans_len) {
            cmd.transitions.push_back((TransitionState)((packed >> 4) & 0x0F));
        }
    }
    
    // 4. Read ado
    if (offset + 2 > len) return false;
    cmd.ado = data[offset] | (data[offset+1] << 8);
    offset += 2;
    
    // 5. Read data based on mode
    if (cmd.has_data_length) {
        // DataLength mode: read explicit uint16_t (little-endian)
        if (offset + 2 > len) return false;
        cmd.data_length = data[offset] | (data[offset+1] << 8);
        offset += 2;
        cmd.data.clear();  // No actual data in DataLength mode
    } else {
        // Data mode: read length + data
        if (offset >= len) return false;
        uint8_t data_len = data[offset++];  // Read length descriptor
        
        if (cmd.has_validate) {
            if (offset >= len) return false;
            uint8_t timeout_compact = data[offset++];
            cmd.validate.timeout = timeout_from_compact(timeout_compact);
            
            // Calculate total bytes needed
            bool has_mask = flags & INITCMD_FLAG_HAS_DATA_MASK;
            size_t total_bytes = data_len * (2 + (has_mask ? 1 : 0));
            if (offset + total_bytes > len) return false;
            
            cmd.data.assign(data + offset, data + offset + data_len);
            offset += data_len;
            
            cmd.validate.data.assign(data + offset, data + offset + data_len);
            offset += data_len;
            
            if (has_mask) {
                cmd.validate.mask.assign(data + offset, data + offset + data_len);
                offset += data_len;
            } else {
                cmd.validate.mask.clear();
            }
        } else {
            if (offset + data_len > len) return false;
            cmd.data.assign(data + offset, data + offset + data_len);
            offset += data_len;
        }
        
        cmd.has_data_length = false;
        cmd.data_length = 0;
    }
    
    // 6. Decode compressed fields
    cmd.adp = (flags & INITCMD_FLAG_ADP_1001) ? 1001 : 0;
    cmd.retries = (flags & INITCMD_FLAG_RETRIES_5000) ? 5000 : 3;
    
    return true;
}

// Serialize Master to byte stream
size_t serialize_master(const FriendlyMasterConfig& master, std::vector<uint8_t>& buffer) {
    size_t start = buffer.size();
    
    // Write NIC length + content
    uint8_t nic_len = std::min((size_t)255, master.nic.length());
    buffer.push_back(nic_len);
    if (nic_len > 0) {
        buffer.insert(buffer.end(), master.nic.begin(), master.nic.begin() + nic_len);
    }
    
    // Write RNIC length + content
    uint8_t rnic_len = std::min((size_t)255, master.rnic.length());
    buffer.push_back(rnic_len);
    if (rnic_len > 0) {
        buffer.insert(buffer.end(), master.rnic.begin(), master.rnic.begin() + rnic_len);
    }
    
    // Write master configuration fields
    buffer.push_back(master.initial_state);
    buffer.push_back(master.cycle_time & 0xFF);
    buffer.push_back((master.cycle_time >> 8) & 0xFF);
    buffer.push_back(master.mailbox_cycle_time & 0xFF);
    buffer.push_back((master.mailbox_cycle_time >> 8) & 0xFF);
    buffer.push_back(master.auto_recovery_timeout);
    buffer.push_back(master.process_image_update_period & 0xFF);
    buffer.push_back((master.process_image_update_period >> 8) & 0xFF);
    buffer.push_back(master.statistics_update_period & 0xFF);
    buffer.push_back((master.statistics_update_period >> 8) & 0xFF);
    buffer.push_back(master.max_frame_size & 0xFF);
    buffer.push_back((master.max_frame_size >> 8) & 0xFF);
    buffer.push_back(master.separate_io_update ? 1 : 0);
    buffer.push_back(master.dc_user_shift_time & 0xFF);
    buffer.push_back((master.dc_user_shift_time >> 8) & 0xFF);
    buffer.push_back((master.dc_user_shift_time >> 16) & 0xFF);
    buffer.push_back((master.dc_user_shift_time >> 24) & 0xFF);
    
    // Write Info: name length + dest_mac + src_mac + ether_type
    uint8_t name_len = std::min((size_t)255, master.info.name.length());
    buffer.push_back(name_len);
    buffer.insert(buffer.end(), master.info.dest_mac, master.info.dest_mac + 6);
    buffer.insert(buffer.end(), master.info.src_mac, master.info.src_mac + 6);
    buffer.push_back(master.info.ether_type & 0xFF);
    buffer.push_back((master.info.ether_type >> 8) & 0xFF);
    if (name_len > 0) {
        buffer.insert(buffer.end(), master.info.name.begin(), master.info.name.begin() + name_len);
    }
    
    // Write InitCmd count
    uint8_t init_cmd_count = std::min((size_t)255, master.init_cmds.size());
    buffer.push_back(init_cmd_count);
    
    // Write each InitCmd
    for (size_t i = 0; i < init_cmd_count; i++) {
        serialize_init_cmd(master.init_cmds[i], buffer);
    }
    
    return buffer.size() - start;
}

// Deserialize Master from byte stream
bool deserialize_master(const uint8_t* data, size_t len, size_t& offset, FriendlyMasterConfig& master) {
    // Read NIC
    if (offset >= len) return false;
    uint8_t nic_len = data[offset++];
    if (offset + nic_len > len) return false;
    master.nic.assign((const char*)(data + offset), nic_len);
    offset += nic_len;
    
    // Read RNIC
    if (offset >= len) return false;
    uint8_t rnic_len = data[offset++];
    if (offset + rnic_len > len) return false;
    master.rnic.assign((const char*)(data + offset), rnic_len);
    offset += rnic_len;
    
    // Read configuration fields
    if (offset + 19 > len) return false;
    master.initial_state = data[offset++];
    master.cycle_time = data[offset] | (data[offset+1] << 8); offset += 2;
    master.mailbox_cycle_time = data[offset] | (data[offset+1] << 8); offset += 2;
    master.auto_recovery_timeout = data[offset++];
    master.process_image_update_period = data[offset] | (data[offset+1] << 8); offset += 2;
    master.statistics_update_period = data[offset] | (data[offset+1] << 8); offset += 2;
    master.max_frame_size = data[offset] | (data[offset+1] << 8); offset += 2;
    master.separate_io_update = data[offset++] != 0;
    master.dc_user_shift_time = data[offset] | (data[offset+1] << 8) | 
                                (data[offset+2] << 16) | (data[offset+3] << 24); 
    offset += 4;
    
    // Read Info
    if (offset + 15 > len) return false;
    uint8_t name_len = data[offset++];
    memcpy(master.info.dest_mac, data + offset, 6); offset += 6;
    memcpy(master.info.src_mac, data + offset, 6); offset += 6;
    master.info.ether_type = data[offset] | (data[offset+1] << 8); offset += 2;
    if (offset + name_len > len) return false;
    master.info.name.assign((const char*)(data + offset), name_len);
    offset += name_len;
    
    // Read InitCmd count and commands
    if (offset >= len) return false;
    uint8_t init_cmd_count = data[offset++];
    master.init_cmds.clear();
    for (uint8_t i = 0; i < init_cmd_count; i++) {
        FriendlyInitCmd cmd;
        if (!deserialize_init_cmd(data, len, offset, cmd)) return false;
        master.init_cmds.push_back(cmd);
    }
    
    return true;
}

// Serialize Slave to byte stream
size_t serialize_slave(const FriendlySlaveConfig& slave, std::vector<uint8_t>& buffer) {
    size_t start = buffer.size();
    
    // Write physics (variable length string)
    uint8_t physics_len = std::min((size_t)255, slave.physics.length());
    buffer.push_back(physics_len);
    if (physics_len > 0) {
        buffer.insert(buffer.end(), slave.physics.begin(), slave.physics.begin() + physics_len);
    }
    
    buffer.push_back(slave.hot_plug ? 1 : 0);
    buffer.push_back(slave.segment);
    buffer.push_back(slave.segment_sub_index);
    
    // Write SlaveInfo
    uint8_t name_len = std::min((size_t)255, slave.info.name.length());
    buffer.push_back(name_len);
    buffer.push_back(slave.info.phys_addr & 0xFF);
    buffer.push_back((slave.info.phys_addr >> 8) & 0xFF);
    buffer.push_back(slave.info.auto_inc_addr & 0xFF);
    buffer.push_back((slave.info.auto_inc_addr >> 8) & 0xFF);
    
    // Write info.physics (variable length string)
    uint8_t info_physics_len = std::min((size_t)255, slave.info.physics.length());
    buffer.push_back(info_physics_len);
    if (info_physics_len > 0) {
        buffer.insert(buffer.end(), slave.info.physics.begin(), slave.info.physics.begin() + info_physics_len);
    }
    
    // Write IDs (little-endian)
    buffer.push_back(slave.info.vendor_id & 0xFF);
    buffer.push_back((slave.info.vendor_id >> 8) & 0xFF);
    buffer.push_back((slave.info.vendor_id >> 16) & 0xFF);
    buffer.push_back((slave.info.vendor_id >> 24) & 0xFF);
    buffer.push_back(slave.info.product_code & 0xFF);
    buffer.push_back((slave.info.product_code >> 8) & 0xFF);
    buffer.push_back((slave.info.product_code >> 16) & 0xFF);
    buffer.push_back((slave.info.product_code >> 24) & 0xFF);
    buffer.push_back(slave.info.revision_no & 0xFF);
    buffer.push_back((slave.info.revision_no >> 8) & 0xFF);
    buffer.push_back((slave.info.revision_no >> 16) & 0xFF);
    buffer.push_back((slave.info.revision_no >> 24) & 0xFF);
    buffer.push_back(slave.info.serial_no & 0xFF);
    buffer.push_back((slave.info.serial_no >> 8) & 0xFF);
    buffer.push_back((slave.info.serial_no >> 16) & 0xFF);
    buffer.push_back((slave.info.serial_no >> 24) & 0xFF);
    
    // Write product_revision length
    uint8_t prod_rev_len = std::min((size_t)255, slave.info.product_revision.length());
    buffer.push_back(prod_rev_len);
    
    // Write name
    if (name_len > 0) {
        buffer.insert(buffer.end(), slave.info.name.begin(), slave.info.name.begin() + name_len);
    }
    
    // Write product_revision
    if (prod_rev_len > 0) {
        buffer.insert(buffer.end(), slave.info.product_revision.begin(), 
                     slave.info.product_revision.begin() + prod_rev_len);
    }
    
    // Write InitCmd count
    uint8_t init_cmd_count = std::min((size_t)255, slave.init_cmds.size());
    buffer.push_back(init_cmd_count);
    
    // Write each InitCmd
    for (size_t i = 0; i < init_cmd_count; i++) {
        serialize_init_cmd(slave.init_cmds[i], buffer);
    }
    
    return buffer.size() - start;
}

// Deserialize Slave from byte stream
bool deserialize_slave(const uint8_t* data, size_t len, size_t& offset, FriendlySlaveConfig& slave) {
    // Read physics (variable length string)
    if (offset >= len) return false;
    uint8_t physics_len = data[offset++];
    if (offset + physics_len > len) return false;
    slave.physics.assign((const char*)(data + offset), physics_len);
    offset += physics_len;
    
    // Read slave config fields
    if (offset + 3 > len) return false;
    slave.hot_plug = data[offset++] != 0;
    slave.segment = data[offset++];
    slave.segment_sub_index = data[offset++];
    
    // Read SlaveInfo header
    if (offset + 5 > len) return false;
    uint8_t name_len = data[offset++];
    slave.info.phys_addr = data[offset] | (data[offset+1] << 8); offset += 2;
    slave.info.auto_inc_addr = data[offset] | (data[offset+1] << 8); offset += 2;
    
    // Read info.physics (variable length string)
    if (offset >= len) return false;
    uint8_t info_physics_len = data[offset++];
    if (offset + info_physics_len > len) return false;
    slave.info.physics.assign((const char*)(data + offset), info_physics_len);
    offset += info_physics_len;
    
    // Read IDs (4 x uint32_t = 16 bytes)
    if (offset + 16 > len) return false;
    slave.info.vendor_id = data[offset] | (data[offset+1] << 8) | 
                           (data[offset+2] << 16) | (data[offset+3] << 24); offset += 4;
    slave.info.product_code = data[offset] | (data[offset+1] << 8) | 
                              (data[offset+2] << 16) | (data[offset+3] << 24); offset += 4;
    slave.info.revision_no = data[offset] | (data[offset+1] << 8) | 
                             (data[offset+2] << 16) | (data[offset+3] << 24); offset += 4;
    slave.info.serial_no = data[offset] | (data[offset+1] << 8) | 
                           (data[offset+2] << 16) | (data[offset+3] << 24); offset += 4;
    
    if (offset >= len) return false;
    uint8_t prod_rev_len = data[offset++];
    
    // Read name
    if (offset + name_len > len) return false;
    slave.info.name.assign((const char*)(data + offset), name_len);
    offset += name_len;
    
    // Read product_revision
    if (offset + prod_rev_len > len) return false;
    slave.info.product_revision.assign((const char*)(data + offset), prod_rev_len);
    offset += prod_rev_len;
    
    // Read InitCmd count and commands
    if (offset >= len) return false;
    uint8_t init_cmd_count = data[offset++];
    slave.init_cmds.clear();
    for (uint8_t i = 0; i < init_cmd_count; i++) {
        FriendlyInitCmd cmd;
        if (!deserialize_init_cmd(data, len, offset, cmd)) return false;
        slave.init_cmds.push_back(cmd);
    }
    
    return true;
}



    /**
     * Auto-generate ENI file from embedded data to system temp directory
     * @param filename Output filename (nullptr = use default "auto_generated_zcpb.xml")
     * @return Full path to generated file, or empty string on error
     */
    std::string auto_generate_eni_from_embedded(const char* filename) {
        printf("\n=== Auto-Generating ENI from Embedded Data ===\n");
        
        // Use default filename if not provided
        const char* output_filename = filename ? filename : "auto_generated_zcpb.xml";
        
        // Get embedded data
        size_t data_size = 0;
        const uint8_t* embedded_data = zmc::get_ECAT_CONFIG_ZCPB(&data_size);
        
        printf("Embedded data: %zu bytes\n", data_size);
        
        // Get system temp directory + ecat_configs subdirectory
        std::string base_tmp = get_system_tmp_dir();
        std::string tmp_dir = base_tmp + "ecat_configs";
        
        printf("System temp: %s\n", base_tmp.c_str());
        printf("Output dir: %s\n", tmp_dir.c_str());
        
        // Ensure directory exists
        if (!ensure_directory(tmp_dir)) {
            printf("✗ Error: Cannot create output directory\n\n");
            return "";
        }
        
        // Build full output path
    #ifdef _WIN32
        std::string output_path = tmp_dir + "\\" + output_filename;
    #else
        std::string output_path = tmp_dir + "/" + output_filename;
    #endif
        
        printf("Output file: %s\n", output_path.c_str());
        
        // Convert to vector and decompress
        std::vector<uint8_t> config_data(embedded_data, embedded_data + data_size);
        int ret = decompress_to_xml(config_data, output_path.c_str());
        
        if (ret == 0) {
            // Verify file and get size
            FILE* fp = fopen(output_path.c_str(), "rb");
            if (fp) {
                fseek(fp, 0, SEEK_END);
                size_t xml_size = ftell(fp);
                fclose(fp);
                printf("✓ Generated: %zu bytes (ratio: %.1f%%)\n", 
                       xml_size, (100.0 * data_size) / xml_size);
            }
            printf("\n");
            return output_path;
        } else {
            printf("✗ Failed to generate (error: %d)\n\n", ret);
            return "";
        }
    }
    
} // namespace zmc
