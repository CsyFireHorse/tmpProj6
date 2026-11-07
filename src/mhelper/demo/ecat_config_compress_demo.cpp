#include "ecat_config_compress.h"

#include <stdio.h>
#include <iostream>
#include <string>
#include <cstdlib>


using namespace zmc;

static std::string get_file_path(const char* fname) {
    std::string path(__FILE__);
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        path = path.substr(0, pos + 1);
    }
    // Go up to data directory
    path += "../data/";
    return path + fname;
}

void xml_roundtrip_demo(){
    // Input XML file
    std::string xml_file = get_file_path("EtherCAT_master_zcpb_eni.xml");
    // === NEW: Test XML roundtrip (parse -> export -> parse -> compare) ===
    printf("\n*** Testing XML Roundtrip (Friendly Structure) ***\n");
    std::string roundtrip_xml = get_file_path("EtherCAT_master_zcpb_eni_roundtrip.xml");
    int ret = test_xml_roundtrip(xml_file.c_str(), roundtrip_xml.c_str());
    if (ret != 0) {
        printf("WARNING: XML roundtrip test failed\n");
    }
    return;
}

void ecat_config_compress_demo() {
    printf("=== EtherCAT Config Compression Demo ===\n\n");
    
    // Input XML file
    std::string xml_file = get_file_path("EtherCAT_master_zcpb_eni.xml");
    printf("Input XML: %s\n", xml_file.c_str());
    
    // === Original compression test ===
    printf("\n*** Testing Compression (Compact Structure) ***\n");
    
    // Parse and compress
    std::vector<uint8_t> compressed_data;
    int ret = parse_xml_to_compact(xml_file.c_str(), compressed_data);
    if (ret != 0) {
        printf("Failed to parse XML: %d\n", ret);
        return;
    }
    
    printf("\nCompression Statistics:\n");
    printf("  Compressed size: %zu bytes\n", compressed_data.size());
    
    // Save compressed format
    std::string compressed_file = get_file_path("EtherCAT_master_zcpb_eni.compressed");
    ret = save_compressed_config(compressed_file.c_str(), compressed_data);
    if (ret != 0) {
        printf("Failed to save compressed config: %d\n", ret);
        return;
    }
    
    // Load compressed format
    std::vector<uint8_t> loaded_data;
    ret = load_compressed_config(compressed_file.c_str(), loaded_data);
    if (ret != 0) {
        printf("Failed to load compressed config: %d\n", ret);
        return;
    }
    
    printf("\nVerification:\n");
    printf("  Original compressed size: %zu bytes\n", compressed_data.size());
    printf("  Loaded compressed size: %zu bytes\n", loaded_data.size());
    printf("  Data match: %s\n", (compressed_data == loaded_data) ? "YES" : "NO");
    
    // Decompress to XML (basic)
    std::string decompressed_xml = get_file_path("EtherCAT_master_zcpb_eni_decompressed.xml");
    ret = decompress_to_xml(loaded_data, decompressed_xml.c_str());
    if (ret != 0) {
        printf("Failed to decompress: %d\n", ret);
        return;
    }
    
    printf("\n=== Demo completed successfully ===\n");
}

void auto_generate_eni_from_embedded_demo(){
    std::string tmp_dir = auto_generate_eni_from_embedded();
    printf("tmp_dir: %s\n", tmp_dir.c_str());
}