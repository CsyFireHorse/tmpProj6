#ifndef _ZMC_TAG_UNION_HPP_
#define _ZMC_TAG_UNION_HPP_

#include <string>
#include <unordered_map>
#include <algorithm>
#include <cctype>

namespace zmc {

// Union type to store different data types
struct tagUnion {
    enum Type {
        NIL,        // null/none value
        BOOL,       // boolean
        INT,        // integer
        DOUBLE,     // double precision float
        VOID_PTR,   // void pointer
        CHAR_PTR    // char pointer (string)
    };
    
    Type type = NIL;
    union {
        bool bool_val;
        int int_val;
        double double_val;
        void* void_ptr;
        char* char_ptr;
    };
    
    // Constructors
    tagUnion() : type(NIL) {}
    tagUnion(bool val) : type(BOOL), bool_val(val) {}
    tagUnion(int val) : type(INT), int_val(val) {}
    tagUnion(double val) : type(DOUBLE), double_val(val) {}
    tagUnion(void* val) : type(VOID_PTR), void_ptr(val) {}
    tagUnion(char* val) : type(CHAR_PTR), char_ptr(val) {}
    tagUnion(const char* val) : type(CHAR_PTR), char_ptr(const_cast<char*>(val)) {}
    
    // Getter methods with type checking
    bool getBool() const {
        if (type == BOOL) return bool_val;
        if (type == INT) return int_val != 0;
        if (type == DOUBLE) return double_val != 0.0;
        return false;
    }
    
    int getInt() const {
        if (type == INT) return int_val;
        if (type == BOOL) return bool_val ? 1 : 0;
        if (type == DOUBLE) return static_cast<int>(double_val);
        return 0;
    }
    
    double getDouble() const {
        if (type == DOUBLE) return double_val;
        if (type == INT) return static_cast<double>(int_val);
        if (type == BOOL) return bool_val ? 1.0 : 0.0;
        return 0.0;
    }
    
    void* getVoidPtr() const {
        if (type == VOID_PTR) return void_ptr;
        return nullptr;
    }
    
    char* getCharPtr() const {
        if (type == CHAR_PTR) return char_ptr;
        return nullptr;
    }
    
    // Check if value is nil
    bool isNil() const { return type == NIL; }
    
    // Type checking methods
    bool isBool() const { return type == BOOL; }
    bool isInt() const { return type == INT; }
    bool isDouble() const { return type == DOUBLE; }
    bool isVoidPtr() const { return type == VOID_PTR; }
    bool isCharPtr() const { return type == CHAR_PTR; }
};

// Configuration map type
using ConfigMap = std::unordered_map<std::string, tagUnion>;

// Helper function to parse string value to tagUnion
inline tagUnion parse_value_to_tagUnion(const std::string& value) {
    if (value.empty()) {
        return tagUnion(); // NIL
    }
    
    // Check for boolean values
    std::string lower_value = value;
    std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);
    if (lower_value == "true" || lower_value == "yes" || lower_value == "on" || lower_value == "1") {
        return tagUnion(true);
    } else if (lower_value == "false" || lower_value == "no" || lower_value == "off" || lower_value == "0") {
        return tagUnion(false);
    }
    
    // Check for integer values
    if (value.find('.') == std::string::npos && 
        (value.find_first_of("+-") == 0 || std::isdigit(value[0]))) {
        try {
            int int_val = std::stoi(value);
            return tagUnion(int_val);
        } catch (...) {
            // If conversion fails, treat as string
            return tagUnion(value.c_str());
        }
    }
    
    // Check for double values
    if (value.find('.') != std::string::npos) {
        try {
            double double_val = std::stod(value);
            return tagUnion(double_val);
        } catch (...) {
            // If conversion fails, treat as string
            return tagUnion(value.c_str());
        }
    }
    
    // Default to string
    return tagUnion(value.c_str());
}

// Utility functions for ConfigMap
inline ConfigMap create_config_map() {
    return ConfigMap();
}

inline void set_config_value(ConfigMap& config, const std::string& key, const tagUnion& value) {
    config[key] = value;
}

inline tagUnion get_config_value(const ConfigMap& config, const std::string& key, const tagUnion& default_value = tagUnion()) {
    auto it = config.find(key);
    if (it != config.end()) {
        return it->second;
    }
    return default_value;
}

}  // namespace zmc

#endif  // _ZMC_TAG_UNION_HPP_
