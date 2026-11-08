#include "common/misc/CmdArgsToy.hpp"
#include <iostream>

using namespace zmc;



void demo_variantToy() {
    // 1. Use VariantToy.
    VariantToy v1, v2, v3;
    v1.set<double>(10.5);               // Store as double.
    v2.set<int>(42);                    // Store as int.
    v3.set<const char*>("Hello world"); // Store as char*.
    
    std::cout << "v1: " << v1.get<double>() << std::endl;
    std::cout << "v2: " << v2.get<int>() << std::endl;
    std::cout << "v3: " << v3.get<const char*>() << std::endl;

    VariantToy v4;
    std::cout << "v4 default: " << v4.get<double>(10.5) << std::endl;
    std::cout << "v4 default: " << v4.get<int>(42) << std::endl;
    std::cout << "v4 default: " << v4.get<const char*>("Hello world") << std::endl;
}

void demo_variant_args() {
    // 2. Use Args.
    char* argv[] = {"--double_val", "3.14159", "--int_val", "100", "--no_exist", "-1.0"};
    // , "--string_val", "Arguments demo"
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    CmdArgsToy args;
    parse_args(argc, argv, args);
    std::cout << "double_val: " << args.params["double_val"].get<double>() << std::endl;
    std::cout << "int_val: " << args.params["int_val"].get<int>() << std::endl;
    std::cout << "string_val: " << args.params["string_val"].get<const char*>() << std::endl;
    // Args args;
    // args.set<double>("double_val", 3.14159);
    // args.set<int>("int_val", 100);
    // args.set<const char*>("string_val", "Arguments demo");
    
    // std::cout << "double_val: " << args.get<double>("double_val") << std::endl;
    // std::cout << "int_val: " << args.get<int>("int_val") << std::endl;
    // std::cout << "string_val: " << args.get<const char*>("string_val") << std::endl;
    // std::cout << "default_val: " << args.get<double>("no_exist", -1.0) << std::endl;
}

// int main(){
//     demo_variantToy();
//     demo_variant_args();
//     return 0;
// }
