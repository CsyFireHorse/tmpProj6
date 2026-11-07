#ifndef _AXPOU_TMPL_RUNNER_HPP_
#define _AXPOU_TMPL_RUNNER_HPP_

#include "AxPou.hpp"
#include "common/misc/CmdArgsToy.hpp"
#include <unordered_map>
#include <cmath>
#include <sstream>

namespace zmc{

// Helper function: check if section is valid (base or axis_*)
inline bool is_valid_section(const std::string& section) {
    return section == "base" || section.substr(0, 5) == "axis_";
}

// Helper function: check if arguments contain section.key format
inline bool has_section_key_args(const CmdArgsToy& argsToy) {
    for (const auto& pair : argsToy.params) {
        const std::string& key = pair.first;
        size_t dotPos = key.find('.');
        if (dotPos != std::string::npos && dotPos > 0 && dotPos < key.length() - 1) {
            std::string section = key.substr(0, dotPos);
            if (is_valid_section(section)) {
                return true;
            }
        }
    }
    return false;
}

// Helper function: collect INI args in format --section.key value
inline std::unordered_map<std::string, std::unordered_map<std::string, std::string>> 
collect_ini_args(const CmdArgsToy& argsToy) {
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> result;
    
    for (const auto& pair : argsToy.params) {
        const std::string& key = pair.first;
        
        // Check if key contains dot (section.key format)
        size_t dotPos = key.find('.');
        if (dotPos != std::string::npos && dotPos > 0 && dotPos < key.length() - 1) {
            std::string section = key.substr(0, dotPos);
            std::string innerKey = key.substr(dotPos + 1);
            
            // Only process if section is valid (base or axis_*)
            if (is_valid_section(section)) {
                const char* raw_value = pair.second.get<const char*>();
                if (raw_value) {
                    result[section][innerKey] = raw_value;
                } else {
                    double numeric_value = pair.second.get<double>(0.0);
                    result[section][innerKey] = std::to_string(numeric_value);
                }
            }
        }
    }
    
    return result;
}

// Helper function: generate INI content from map
inline std::string 
generate_ini_content(const std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& iniMap) {
    std::ostringstream oss;
    
    for (const auto& sectionPair : iniMap) {
        const std::string& section = sectionPair.first;
        const auto& entries = sectionPair.second;
        
        if (entries.empty()) continue;
        
        oss << "[" << section << "]\n";
        
        for (const auto& entry : entries) {
            oss << entry.first << " = ";
            
            // Check if value is numeric and whether it is integer
            const std::string& value = entry.second;
            try {
                double numeric_value = std::stod(value);
                if (numeric_value == floor(numeric_value)) {
                    oss << static_cast<long long>(numeric_value);
                } else {
                    oss << numeric_value;
                }
            } catch (...) {
                oss << value;
            }
            
            oss << "\n";
        }
    }
    
    return oss.str();
}

using PouTmplDefaults = std::unordered_map<std::string, std::string>;

inline void inject_default_args(CmdArgsToy& argsToy, const PouTmplDefaults& defaults) {
    if (defaults.empty()) {
        return;
    }
    for (const auto& entry : defaults) {
        if (!argsToy.has_key(entry.first)) {
            argsToy.add_key(entry.first, entry.second.c_str());
        }
    }
}

// TODO: depreciated
inline PouTmplDefaults make_verbose_flag_default(int verbose_flag_default) {
    PouTmplDefaults defaults;
    defaults["verbose_flag"] = std::to_string(verbose_flag_default);
    return defaults;
}


template<typename T>
inline T* make_pou_config(int argc,
                       char** argv,
                       const std::string& iniContentDefault,
                       const PouTmplDefaults& defaults = PouTmplDefaults()) {
    if ((argc>=2 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0))||
    (argc && (strcmp(argv[0], "--help") == 0 || strcmp(argv[0], "-h") == 0))) {
        printf("Usage: --help\n");
        printf("  [--ini_path <string>]\n");
        printf("  [--ini_content <string>]  # Full INI content\n");
        printf("  [--section.key <value>]    # Simplified key-value format (e.g., --base.axis_num 1 --axis_0.id 1)\n");
        printf("  [--cycles <int>]\n");
        printf("  [--verbose_flag <int>]\n");
        return nullptr;
    }
    CmdArgsToySingleton::parse(argc, argv);
    auto& argsToy = CmdArgsToySingleton::get();
    inject_default_args(argsToy, defaults);

    const char* ini_content = nullptr;
    std::string generated_ini;
    
    // Check for --ini_content parameter or section.key format
    if (argsToy.has_key("ini_content")) {
        ini_content = argsToy["ini_content"].get<const char*>("");
    } else if (has_section_key_args(argsToy)) {
        // Collect and generate INI content from section.key format
        auto iniMap = collect_ini_args(argsToy);
        generated_ini = generate_ini_content(iniMap);
        ini_content = generated_ini.c_str();
    }
    
    // Keep original logic: use default if no ini_path and no ini_content
    if (!argsToy["ini_path"].get<const char*>() && !ini_content) {
        ini_content = iniContentDefault.c_str();
    }else if (!ini_content){
        ini_content = "";
    }
    
    T* pRunner = new T();
    if (pRunner->configure(argsToy["ini_path"].get<const char*>(""), ini_content) != 0) {
        delete pRunner;
        return nullptr;
    }
    auto& runner = *pRunner;

    if (argsToy.has_key("cycles")) {
        runner.set_cycles(argsToy["cycles"].get<int>(300));
    }
    if (argsToy.has_key("verbose_flag")) {
        runner.set_verbose_flag(argsToy["verbose_flag"].get<int>());
    }
    if (argsToy.has_key("cycle_time")) {
        runner.set_cycle_time(argsToy["cycle_time"].get<int>());
    }
    return pRunner;
}

template<typename T>
int pou_auto_tmpl_demo(int argc,
                       char** argv,
                       const std::string& iniContentDefault,
                       const PouTmplDefaults& defaults = PouTmplDefaults()) {
    auto pRunner = make_pou_config<AxPouTaskBinderAuto>(argc, argv, iniContentDefault, defaults);
    if (pRunner == nullptr) {
        return -1;
    }
    T task;
    pRunner->bind(0, &task);
    pRunner->run();
    // task.postRun();
    delete pRunner;
    return 0;
}


const std::string iniContentDefault = R"(
[base]
axis_num = 1
[axis_0]
cycle_time = 10000
is_axis_virtual = true
)";


template<typename T>
int pou_auto_default_tmpl(int argc,
                          char** argv,
                          const PouTmplDefaults& defaults = PouTmplDefaults()) {
    return pou_auto_tmpl_demo<T>(argc, argv, iniContentDefault, defaults);
}

}
#endif