#include "stdio.h"
#include "string.h"




static int printHelp(int argc, char **argv){
    printf(" --version\n"
        " cmd\n"
        " cmd2\n"
        " cmd_inter\n"
        " cmd_parse\n"
        " delay_filter\n"
        " axis_check\n"
        " inih\n"
        " tag_union\n"
        " json_merge file_1 file_2\n"
        " --help\n");
    return 0;
}

void CommandInteractor_demo();
void StrFuncMatcher_demo(int argc, char** argv);
void CommandParse_demo(int argc, char** argv);
void TagUnion_demo();


int main(int argc, char** argv){
    // int argc = 3;
    // char* argv[] = {"", "cmd2", "hello2"};
    // testAllocatorBitmap();
    if(argc == 1){
        return printHelp(argc, argv);
    }else if (argc >= 2 && strcmp(argv[1], "--help")==0){
        return printHelp(argc, argv);
    }else if (argc >= 2 && strcmp(argv[1], "cmd")==0){
        // cmdParseItem_demo(argc-2, argv+2);
    }else if (argc >= 2 && strcmp(argv[1], "servo")==0){
        // servoFak_demo();
    }else if (argc >= 2 && strcmp(argv[1], "cmd2")==0){
        StrFuncMatcher_demo(argc-2, argv+2);
    }else if(argc >= 2 && strcmp(argv[1], "cmd_inter")==0){
        CommandInteractor_demo();
    }else if(argc >= 2 && strcmp(argv[1], "cmd_parse")==0){
        CommandParse_demo(argc-2, argv+2);
    }else if(argc >= 2 && strcmp(argv[1], "tag_union")==0){
        TagUnion_demo();
    }
    return 0;
}
