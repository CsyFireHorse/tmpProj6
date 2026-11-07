#include "misc/TagUnion.hpp"
#include <iostream>
#include <string>

void TagUnion_demo() {
    using namespace zmc;

    std::cout << "=== TagUnion Test ===" << std::endl;
    
    // Test basic tagUnion functionality
    tagUnion nil_val;
    tagUnion bool_val(true);
    tagUnion int_val(42);
    tagUnion double_val(3.14159);
    tagUnion string_val("Hello World");
    
    std::cout << "nil_val.isNil(): " << (nil_val.isNil() ? "true" : "false") << std::endl;
    std::cout << "bool_val.getBool(): " << bool_val.getBool() << std::endl;
    std::cout << "int_val.getInt(): " << int_val.getInt() << std::endl;
    std::cout << "double_val.getDouble(): " << double_val.getDouble() << std::endl;
    std::cout << "string_val.getCharPtr(): " << string_val.getCharPtr() << std::endl;
    
    // Test parse_value_to_tagUnion function
    std::cout << "\n=== Parse Value Test ===" << std::endl;
    
    auto parsed_bool = parse_value_to_tagUnion("true");
    auto parsed_int = parse_value_to_tagUnion("123");
    auto parsed_double = parse_value_to_tagUnion("3.14");
    auto parsed_string = parse_value_to_tagUnion("test_string");
    
    std::cout << "parse_value_to_tagUnion(\"true\"): " << (parsed_bool.getBool() ? "true" : "false") << std::endl;
    std::cout << "parse_value_to_tagUnion(\"123\"): " << parsed_int.getInt() << std::endl;
    std::cout << "parse_value_to_tagUnion(\"3.14\"): " << parsed_double.getDouble() << std::endl;
    std::cout << "parse_value_to_tagUnion(\"test_string\"): " << parsed_string.getCharPtr() << std::endl;
    
    // Test ConfigMap functionality
    std::cout << "\n=== ConfigMap Test ===" << std::endl;
    
    ConfigMap config = create_config_map();
    set_config_value(config, "cycle_time", tagUnion(1000));
    set_config_value(config, "axis_index", tagUnion(0));
    set_config_value(config, "is_axis_virtual", tagUnion(true));
    set_config_value(config, "velocity", tagUnion(1000.5));
    set_config_value(config, "task_name", tagUnion("MyTask"));
    
    std::cout << "cycle_time: " << get_config_value(config, "cycle_time", tagUnion(0)).getInt() << std::endl;
    std::cout << "axis_index: " << get_config_value(config, "axis_index", tagUnion(0)).getInt() << std::endl;
    std::cout << "is_axis_virtual: " << get_config_value(config, "is_axis_virtual", tagUnion(false)).getBool() << std::endl;
    std::cout << "velocity: " << get_config_value(config, "velocity", tagUnion(0.0)).getDouble() << std::endl;
    std::cout << "task_name: " << get_config_value(config, "task_name", tagUnion("")).getCharPtr() << std::endl;
    
    std::cout << "\nAll tests completed successfully!" << std::endl;
}
