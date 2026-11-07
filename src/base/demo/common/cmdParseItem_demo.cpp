
#include "common/misc/CmdParseItem.hpp"
using namespace zmc;

void echo(char* str){
    printf("echo:%s\n", str);
}
void hello(){
    printf("hello world\n");
}
static void print_help(){
    printf("print help\n"
        "print hello\n"
        "print echo some_string\n"
        "print inc 1234567\n"
        "print twice 3.14159\n"
        "print cat filename\n"
    );
}
void print_inc(int i){
    printf("int:%d\n", ++i);
}
void print_twice(double v){
    printf("twice:%f\n", v*2);
}
int cat_content(char* str){
    printf("cat:%s\n", str);
    return 0;
}

int cmdParseItem_demo(int argc, char** argv){
    // CmdParseItem cmd = {{}, (void*) print_help, 5};
    // printf("functype=%d,%d,%d\n", functype2int<decltype(print_help)>(), functype2int<decltype(hello)>(), functype2int<decltype(echo)>());
    
    std::vector<CmdParseItem> cmds = {
        {{},  (void*) print_help, functype2int<decltype(print_help)>()},
        {{"--help"}, CMD_WRAP(print_help)},
        {{"hello"}, CMD_WRAP(hello) },
        {{"echo", ""},  CMD_WRAP(echo)},
        {{"inc", ""}, CMD_WRAP(print_inc) },
        {{"twice", ""}, CMD_WRAP(print_twice)  },
        {{"cat", ""}, CMD_WRAP(cat_content) },
    };
    return cmdParseItem_match(cmds, argc, argv);
}
