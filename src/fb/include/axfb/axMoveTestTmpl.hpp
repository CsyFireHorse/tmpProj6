#ifndef _AX_MOVE_TEST_TMPL_HPP_
#define _AX_MOVE_TEST_TMPL_HPP_

#include "AxisArrRunner.hpp"
#include <unordered_map>
#include <string>
#include <algorithm>
#include <cctype>

namespace zmc {

// Configuration dictionary type with helper accessors
struct ConfigDict : public std::unordered_map<std::string, std::string> {
    using Base = std::unordered_map<std::string, std::string>;
    using Base::Base;

    bool has_key(const std::string& key) const {
        return this->find(key) != this->end();
    }

    int get_int(const std::string& key, int default_value) const {
        auto it = this->find(key);
        if (it != this->end() && !it->second.empty()) {
            try {
                return std::stoi(it->second);
            } catch (...) {
            }
        }
        return default_value;
    }

    double get_double(const std::string& key, double default_value) const {
        auto it = this->find(key);
        if (it != this->end() && !it->second.empty()) {
            try {
                return std::stod(it->second);
            } catch (...) {
            }
        }
        return default_value;
    }

    bool get_bool(const std::string& key, bool default_value) const {
        auto it = this->find(key);
        if (it != this->end() && !it->second.empty()) {
            std::string lower = it->second;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            return (lower == "true" || lower == "1" || lower == "yes" || lower == "on");
        }
        return default_value;
    }

    std::string get_string(const std::string& key, const std::string& default_value) const {
        auto it = this->find(key);
        if (it != this->end()) {
            return it->second.empty() ? default_value : it->second;
        }
        return default_value;
    }
    void merge(const ConfigDict& default_config) {
        for (ConfigDict::const_iterator it = default_config.begin(); it != default_config.end(); ++it) {
            const std::string& key = it->first;
            const std::string& value = it->second;
            if (this->find(key) == this->end()) {
                this->insert(std::make_pair(key, value));
            }
        }
    }
};

class AxMoveTestCaseConfig {
private:
    static ConfigDict& instance() {
        static ConfigDict config;
        return config;
    }
    
public:
    // Set configuration dictionary
    static void set_config(const ConfigDict& config) {
        instance() = config;
    }
    
    // Get configuration dictionary
    static ConfigDict& get_config() {
        return instance();
    }
    
    // Clear configuration
    static void clear() {
        instance().clear();
    }
};

// Helper function to configure a single axis from dictionary
template<typename T>
void configure_axis_from_dict(T& ax, int axis_idx, const ConfigDict& config) {
    std::string prefix = "axis_" + std::to_string(axis_idx) + ".";
    auto& cfg = ax.get_config_ref();
    
    // Read cycle_time from axis_N.cycle_time
    if (config.has_key(prefix + "cycle_time")) {
        cfg.cycle_time = config.get_int(prefix + "cycle_time", 1000);
    }
    
    // Read increments_units_in_application_num from axis_N.increments_units_in_application_num
    if (config.has_key(prefix + "increments_units_in_application_num")) {
        cfg.linear.increments_units_in_application_num = 
            config.get_int(prefix + "increments_units_in_application_num", 1);
    }
    
    // Read increments_units_in_application_den from axis_N.increments_units_in_application_den
    if (config.has_key(prefix + "increments_units_in_application_den")) {
        cfg.linear.increments_units_in_application_den = 
            config.get_int(prefix + "increments_units_in_application_den", 1);
    }
    
    // Read servo_fak_pos from axis_N.debug.servo_fak_pos
    if (config.has_key(prefix + "debug.servo_fak_pos")) {
        cfg.debug.servo_fak_pos = config.get_int(prefix + "debug.servo_fak_pos", 0);
    }
    
    // Read bit_width from axis_N.bit_width
    if (config.has_key(prefix + "bit_width")) {
        cfg.bit_width = config.get_int(prefix + "bit_width", 0);
    }
    
    // Set is_axis_virtual if specified
    if (config.has_key(prefix + "is_axis_virtual")) {
        cfg.is_axis_virtual = config.get_bool(prefix + "is_axis_virtual", true);
    } else {
        cfg.is_axis_virtual = true; // default
    }
    
    ax.reload_config();
}

template<typename T>
class AxRunner : public AxisArrRunner<T> {
public:
    AxRunner(const ConfigDict& config_dict) {
        const auto& config = config_dict; // AxMoveTestCaseConfig::get_config();
        int axis_num = 1;
        if (config.has_key("base.axis_num")) {
            axis_num = config.get_int("base.axis_num", static_cast<int>(this->axArr.size()));
            if (axis_num > 0) {
                this->axArr.resize(static_cast<size_t>(axis_num));
            }
        }
        if (config.has_key("base.count_to_exit")) {
            this->count_to_exit = config.get_int("base.count_to_exit", 70);
        }
        if (config.has_key("base.verbose_flag")) {
            this->verbose_flag = config.get_int("base.verbose_flag", 0);
        }
        if (config.has_key("base.verbose_modulo")) {
            this->verbose_modulo = config.get_int("base.verbose_modulo", 1);
        }
        
        // Configure all axes
        for (int i = 0; i < axis_num; ++i) {
            configure_axis_from_dict(this->axArr[i], i, config);
        }
    }

    // void preRun() override {}
};

// New function to replace AxSgRunTmpl, similar to pou_auto_tmpl_demo but using dictionary
// T: Test case type (e.g., AxMoveTestCase<FbtMoveAbsolute>)
// R: Runner type (e.g., AxisCia402Fak)
template<typename T, typename R>
void ax_move_test_tmpl_dict(const ConfigDict& config) {
    // Set configuration to singleton
    AxMoveTestCaseConfig::set_config(config);
    
    // Create runner with dictionary
    AxRunner<R> runner(config);
    
    // Create test case (no-arg constructor, reads from singleton)
    T pou;
    
    // Bind and run
    runner.bind2(0, pou);
    runner.run();
}

} // namespace zmc

#endif // _AX_MOVE_TEST_TMPL_HPP_

