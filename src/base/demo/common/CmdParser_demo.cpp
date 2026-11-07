#include <functional>
#include <vector>
#include "common/misc/CmdParser.hpp"


void echo2(const std::string& str){
    printf("echo:%s\n", str.c_str());
}
void hello();
void print_inc(int i);
void print_twice(double v);

static void print_help(){
    printf("print --help\n"
        "print hello\n"
        "print echo some_string\n"
        "print inc 1234567\n"
        "print twice 3.14159\n"
        "print cat filename\n"
    );
}
void StrFuncMatcher_demo(int argc, char** argv){
    zmc::CmdParser cps;
    cps.bind("", print_help);
    cps.bind("--help", print_help);
    cps.bind("hello", hello);
    cps.bind("echo", echo2);
    cps.bind("inc", print_inc);
    cps.bind("twice", print_twice);
    std::function<void(void)> ff = [](){
        printf("hello 2\n");
    };
    cps.bind("hello2", ff);
    int val = 11;
    cps.bind("dec", [&val](){ printf("dec %d\n", val--);});
    cps.match(argc, argv);
}