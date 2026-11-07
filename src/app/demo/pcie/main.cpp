/*
 * @Author: ChenSy
 * @LastEditors: ChenSy
 * @Date: 2024-03-21 17:57:29
 * @LastEditTime: 2024-03-21 17:57:30
 * @Description: Modify here please
 */
#include "common/misc/CommandParser.h"
#include <cstring>
#include <sstream>
#include "common/get_file_path.hpp"


using namespace zmc;

int pci_main(int argc, char** argv);

static std::string help_str="";

int printHelp(){
    printf("%s\n", help_str.c_str());
    return 0;
}

static const char* EXE_NAME = "zmc_motapp_demo";

int printVersion(){
    printf( "%s: plcfb demo tool\n"
        "version 1.0.0\n"
        "create by csy on 2024/04/07,"
        "update on 2024/04/07\n", 
        EXE_NAME);
    return 0;
}

// Helper function to convert argc/argv to string
static std::string argvToString(int argc, char** argv) {
    std::ostringstream oss;
    for (int i = 0; i < argc; ++i) {
        if (i > 0) oss << " ";
        oss << argv[i];
    }
    return oss.str();
}


static int _main(int argc, char** argv){
    static CommandParser parser;
    
    parser.bind("--help", [](){
        parser.showHelp();
    });
    parser.bind("-h", [](){
        parser.showHelp();
    });
    parser.bind("--version", printVersion);
    parser.bind("-V", printVersion);
    parser.bind("pci", pci_main);

    CommandParser::ErrorCode result = parser.parseAndExecute(argc-1, argv+1);
    
    if (result == CommandParser::COMMAND_NOT_FOUND || result == CommandParser::EMPTY_COMMAND) {
        parser.showHelp();
        return result;
    }
    return result;
}

int main(int argc, char** argv){
    if (argc == 1){
    #if  1 // _DEBUG2
        // char* argv2[] = {"zmc_motapp_demo", "power_move", new_get_file_path_pchar("mhelper/data/pse_servo1.xml")};
        // char* argv2[] = {"zmc_motapp_demo", "cmd", "--base.axis_num", "2"};
        char* argv2[] = {"zmc_pcie_demo", "axis_ini", new_get_file_path_pchar("mhelper/data/zmc_motapp_demo_fak.ini"), "--init-opmode=8"};
        int argc2 = LEN(argv2);
        return _main(argc2, argv2);
    #endif
    }
    return _main(argc, argv);
}