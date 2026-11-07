#include "stdio.h"
#include "string.h"
#include <iostream>
#include "common/misc/CmdParseItem.hpp"

using namespace zmc;

void eni_json_parse_demo();
// void eni_parse_demo();
void eni_parse_map_demo();
void eni_servo_map_demo();
void zspdlog_demo();
void ecat_config_compress_demo();
void auto_generate_eni_from_embedded_demo();

static const char* EXE_NAME = "mhelper_demo";

int printVersion(){
    std::cout << EXE_NAME << ": mhelper demo tool\n"
            << "version 1.0.0\n"
            << "create by csy on 2023/08/26,"
            << "update on 2023/08/26\n";
    return 0;
}

static std::string help_str="";

int printHelp(){
    printf("%s\n", help_str.c_str());
    return 0;
}

int cmdParseItem_demo(int argc, char** argv){
    const static std::vector<CmdParseItem> cmds = {
        {{}, CMD_WRAP(printHelp)},
        {{"--help"}, CMD_WRAP(printHelp)},
        {{"--version"}, CMD_WRAP(printVersion)},
        {{"-V"}, CMD_WRAP(printVersion)},
        {{"eni"}, CMD_WRAP(eni_json_parse_demo)},
        {{"eni_parse"}, CMD_WRAP(eni_json_parse_demo)},
        {{"eni_iomap"}, CMD_WRAP(eni_parse_map_demo)},
        {{"eni_servo"}, CMD_WRAP(eni_servo_map_demo)},
        {{"zspdlog"}, CMD_WRAP(zspdlog_demo)},
        {{"ecat_config_compress"}, CMD_WRAP(ecat_config_compress_demo)},
        {{"auto_generate_eni_from_embedded"}, CMD_WRAP(auto_generate_eni_from_embedded_demo)},
    };
    genHelpStr(cmds, help_str);
    return cmdParseItem_match(cmds, argc, argv);
}

int parse_args(int argc, char** argv){
    return cmdParseItem_demo(argc-1, argv+1);
}

int main(int argc, char** argv){
    return parse_args(argc, argv);
}
