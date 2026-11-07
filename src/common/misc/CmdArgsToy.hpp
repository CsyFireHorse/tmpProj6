#ifndef _ZMC_CMD_ARGS_TOY_HPP_
#define _ZMC_CMD_ARGS_TOY_HPP_

#include <string>
#include <unordered_map>
#include <cstring>
#include <cstdlib>
#include "VariantToy.hpp"
#include <iostream>
#include <vector>

namespace zmc {
class CmdArgsToy {
   public:
    std::unordered_map<std::string, VariantToy> params;
    bool has_key(const std::string& key) const {
        return params.find(key) != params.end();
    }
    VariantToy& operator[](const std::string& key) {
        return params[key];
    }

    const VariantToy& operator[](const std::string& key) const {
        static const VariantToy defaultValue  = makeVariantToy();
        auto it = params.find(key);
        if (it != params.end()) {
            return it->second;
        }
        return defaultValue;
    }

    // template <typename T>
    // void set(const std::string& key, const T& value) {
    //     VariantToy v;
    //     v.set<T>(value);
    //     params[key] = v;
    // }

    // template <typename T>
    // T get(const std::string& key, const T& defaultVal = T()) const {
    //     auto it = params.find(key);
    //     if (it != params.end()) {
    //         return it->second.get<T>();
    //     }
    //     return defaultVal;
    // }

    // double getDouble(const std::string& key, double defaultVal = 0.0) const {
    //     return get<double>(key, defaultVal);
    // }

    // int getInt(const std::string& key, int defaultVal = 0) const {
    //     return get<int>(key, defaultVal);
    // }

    //     template<typename T>
    //     T get(const std::string& key, T* defaultVal) const {
    //         return params[key].get<T>(key, defaultVal);
    //     }

    void add_key(const std::string& key, const char* value) {
        if (value == nullptr) {
            return;
        }

        char* end = nullptr;
        long lval = strtol(value, &end, 10);
        if (*end == '\0') {
            params[key].set<int>(static_cast<int>(lval));
            return;
        }

        end = nullptr;
        double dval = strtod(value, &end);
        if (*end == '\0') {
            params[key].set<double>(dval);
        } else {
            params[key].set<const char*>(value);
        }
    }
};

// parse args all begin with --, such as --acceleration 100, save to a map["acceleration"] = 100.0
// all args are parse to double , so map struct is map<string, double> type
inline void parse_args(int argc, char** argv, CmdArgsToy& args) {
    for (int i = 0; i < argc; i++) {
        if (strncmp(argv[i], "--", 2) == 0) {
            std::string key = std::string(argv[i] + 2);  // Skip -- prefix
            if (i + 1 < argc ) {
                if (strncmp(argv[i+1], "--", 2) == 0) {
                    // not match
                    continue;
                }
                args.add_key(key, argv[i + 1]);
                i++;
            }
        }
    }
}

// parse args all begin with --, such as --acceleration 100, save to a map["acceleration"] = 100.0
// all args are parse to double , so map is map<string, double> type
// map has default value 0.0
// inline void parse_args(int argc, char** argv, CmdArgsToy& args) {
//     for (int i = 0; i < argc; i++) {
//         if (strncmp(argv[i], "--", 2) == 0) {
//             std::string key = std::string(argv[i] + 2);  // Skip -- prefix
//             if (i + 1 < argc && argv[i + 1][0] != '-') {
//                 args.params[key].set<double>(std::stod(argv[i + 1]));
//                 i++;
//             }
//         }
//     }
// }
class CmdArgsToySingleton {
private:
    static CmdArgsToy& instance() {
        static CmdArgsToy args;
        return args;
    }
    
public:
    static void parse(int argc, char** argv) {
        instance().params.clear();
        parse_args(argc, argv, instance());
    }
    
    static CmdArgsToy& get() { return instance(); }
    static void clear() { instance().params.clear(); }
};


// Parse argv to std::map<std::string, std::string>; key without leading --
inline std::unordered_map<std::string, std::string> argv_to_dict(int argc, char** argv) {
    std::unordered_map<std::string, std::string> result;
    for (int i = 0; i < argc; ++i) {
        if (strncmp(argv[i], "--", 2) == 0) {
            std::string key = std::string(argv[i] + 2);  // Remove '--'
            if (i + 1 < argc && strncmp(argv[i + 1], "--", 2) != 0) {
                result[key] = argv[i + 1];
                ++i;
            }
        }
    }
    return result;
}
// Convert a std::unordered_map<std::string, std::string> (dict) to argv-like array of char*
// The first element (index 0) is the program name (default: "main")
inline std::vector<std::string> dict_to_argv_vec(const std::unordered_map<std::string, std::string>& dict, const std::string& progname = "main") {
    std::vector<std::string> result;
    result.push_back(progname);
    for (const auto& kv : dict) {
        result.push_back("--" + kv.first);
        result.push_back(kv.second);
    }
    return result;
}

// Return std::vector<char*> for passing to main(int, char**) or parse_args
inline std::vector<char*> dict_to_argv(const std::unordered_map<std::string, std::string>& dict, const std::string& progname = "main") {
    static std::vector<std::string> backing_vec; // needs to be static or kept alive for char* pointers
    backing_vec = dict_to_argv_vec(dict, progname);
    std::vector<char*> result;
    for (auto& s : backing_vec) {
        result.push_back(const_cast<char*>(s.c_str())); // c_str() is valid as long as backing_vec lives
    }
    return result;
}

// Helper function: merge two argv arrays into a new one
// argv_result = argv1 + argv2 (argv2 appended to argv1)
inline std::vector<char*> merge_argv(char* argv1[], size_t len1, char* argv2[], size_t len2) {
    std::vector<char*> result;
    
    // Add first element (program name) from argv1
    if (len1 > 0) {
        result.push_back(argv1[0]);
    }

    for (size_t i = 1; i < len1; ++i) {
        result.push_back(argv1[i]);
    }
    
    for (size_t i = 0; i < len2; ++i) {
        result.push_back(argv2[i]);
    }
    
    return result;
}

}  // namespace zmc
#endif