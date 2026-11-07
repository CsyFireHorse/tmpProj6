#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <tuple>
#include <memory>
#include <type_traits>
#include <iostream>
#include <cctype>

#define LEN(x) (sizeof(x)/sizeof(x[0]))
namespace zmc{
// Forward declaration
template<typename T>
T convertArg(const std::string& arg, int& errCode);

// Specialization for std::string type conversion
template<>
inline const std::string& convertArg<const std::string&>(const std::string& arg, int& errCode) {
    errCode = 0;
    return arg;
}
template<>
inline std::string convertArg<std::string>(const std::string& arg, int& errCode) {
    errCode = 0;
    return arg;
}

// Conversion function for bool
inline bool convertBool(const std::string& arg, int& errCode) {
    errCode = 0;
    std::string lower = arg;
    // Convert to lowercase
    for (auto& c : lower) {
        c = std::tolower(c);
    }
    
    if (lower == "true" || lower == "1" || lower == "yes" || lower == "y") {
        return true;
    } else if (lower == "false" || lower == "0" || lower == "no" || lower == "n") {
        return false;
    } else {
        errCode = 1;
        return false;
    }
}

// Specialization for bool type conversion
template<>
inline bool convertArg<bool>(const std::string& arg, int& errCode) {
    return convertBool(arg, errCode);
}

// Conversion function for int
inline int convertInt(const std::string& arg, int& errCode) {
    errCode = 0;
    try {
        return std::stoi(arg);
    } catch (...) {
        errCode = 1;
        return 0;
    }
}

// Specialization for int type conversion
template<>
inline int convertArg<int>(const std::string& arg, int& errCode) {
    return convertInt(arg, errCode);
}
// Conversion function for char*
inline const char* convertPchar(const std::string& arg, int& errCode) {
    // Return a non-const char* pointer; caller must ensure proper lifetime
    errCode = 0;
    // std::cout<< "convertPchar" << errCode<<std::endl;
    return const_cast<const char*>(arg.c_str());
}

// Specialization for char* type conversion
template<>
inline const char* convertArg<const char*>(const std::string& arg, int& errCode) {
    return convertPchar(arg, errCode);
}

// Conversion function for float
inline float convertFloat(const std::string& arg, int& errCode) {
    errCode = 0;
    try {
        return std::stof(arg);
    } catch (...) {
        errCode = 1;
        return 0.0f;
    }
}

// Specialization for float type conversion
template<>
inline float convertArg<float>(const std::string& arg, int& errCode) {
    return convertFloat(arg, errCode);
}

// Conversion function for double
inline double convertDouble(const std::string& arg, int& errCode) {
    errCode = 0;
    try {
        return std::stod(arg);
    } catch (...) {
        errCode = 1;
        return 0.0;
    }
}

// Specialization for double type conversion
template<>
inline double convertArg<double>(const std::string& arg, int& errCode) {
    return convertDouble(arg, errCode);
}

// Return default value of T 
// This function is used for compile generic template, we should use specialization instead to return real value
template<typename T>
T convertArg(const std::string& arg, int& errCode) {
    errCode = 1;
    static typename std::remove_reference<T>::type temp{};
    return static_cast<T>(temp);
}

/**
 * @brief Simple command parser class that matches input strings and executes callbacks
 */
class CommandParser {
public:
    // error code definition
    enum ErrorCode {
        SUCCESS = 0,
        COMMAND_NOT_FOUND = 1,
        INVALID_ARGUMENT_COUNT = 2,
        ARGUMENT_CONVERSION_ERROR = 3,
        EXECUTION_ERROR = 4,
        FATAL_ERROR = 5, // will exit the program?
        UNKNOWN_ERROR = 6,
        EMPTY_COMMAND = 7 // will be ignored
    };

    // Command handler type
    using CommandHandler = std::function<ErrorCode(const std::vector<std::string>&)>;

    /**
     * @brief Constructor
     */
    CommandParser() : exitFlag_(false) {}

    /**
     * @brief Register a command with a callback function
     * This unified interface handles all function types based on their signatures
     * 
     * @param cmdName Command name
     * @param func Callback function (can be regular function, lambda, or function object)
     * @param helpText Optional help text describing the command
     */
    template<typename F>
    void bind(const std::string& cmdName, F&& func, const std::string& helpText = "") {
        registerSlotImpl(cmdName, std::forward<F>(func));
        if (!helpText.empty()) {
            helpTexts_[cmdName] = helpText;
        }
    }

    ErrorCode parseAndExecute(int argc, char** argv) {
        if (argc <= 0 || argv == nullptr) {
            return EMPTY_COMMAND;
        }
        // Build tokens from argv, skipping argv[0]
        std::vector<std::string> tokens;
        for (int i = 0; i < argc; ++i) {
            tokens.emplace_back(argv[i]);
        }
        return parseAndExecute(tokens);
    }

    /**
     * @brief Parse input string and execute matching command callback
     * 
     * @param input The input string to parse: [cmdName, args, ...], no exeName
     * @return ErrorCode indicating success or failure reason
     */
    ErrorCode parseAndExecute(const std::string& input) {
        auto tokens = tokenize(input);
        return parseAndExecute(tokens);
    }
    ErrorCode parseAndExecute(const std::vector<std::string>& tokens) {        
        if (tokens.empty()) {
            return EMPTY_COMMAND;
        }
        
        const std::string& cmdName = tokens[0];
        auto cmdIt = commands_.find(cmdName);
        
        if (cmdIt == commands_.end()) {
            return COMMAND_NOT_FOUND;
        }
        
        // Extract arguments (excluding the command name)
        // std::vector<std::string> args;
        // for (size_t i = 1; i < tokens.size(); ++i) {
        //     args.push_back(tokens[i]);
        // }
        std::vector<std::string> args(tokens.begin() + 1, tokens.end());
        // Execute the command handler
        return cmdIt->second(args);
    }

    /**
     * @brief List all registered command names
     * 
     * @return Vector of registered command names
     */
    std::vector<std::string> getRegisteredCommands() const {
        std::vector<std::string> cmdNames;
        for (const auto& pair : commands_) {
            cmdNames.push_back(pair.first);
        }
        return cmdNames;
    }

    /**
     * @brief Get help text for a specific command
     * 
     * @param cmdName Command name
     * @return Help text for the command, or empty string if command not found
     */
    std::string getCommandHelp(const std::string& cmdName) const {
        auto it = helpTexts_.find(cmdName);
        if (it != helpTexts_.end()) {
            return it->second;
        }
        return "";
    }

    /**
     * @brief Get all commands with their help texts
     * 
     * @return Map of command names to help texts
     */
    std::map<std::string, std::string> getAllCommandsHelp() const {
        return helpTexts_;
    }

    /**
     * @brief Check if exit flag is set
     * 
     * @return true if exit flag is set, false otherwise
     */
    bool isExitFlagSet() const {
        return exitFlag_;
    }

    /**
     * @brief Set the exit flag
     */
    void setExitFlag() {
        exitFlag_ = true;
    }

    void showHelp() const {
        std::cout << "Available commands:" << std::endl;
        auto cmds = getRegisteredCommands();
        auto commandHelp = getAllCommandsHelp();
        for (const auto& cmd : cmds) {
            std::cout << "- " << cmd << ": " << commandHelp[cmd] << std::endl;
        }
    }
    static void printError(CommandParser::ErrorCode errorCode){
        const char* errorMsg[] = {
            "Success",
            "Command not found",
            "Invalid number of arguments",
            "Failed to convert argument(s)",
            "Execution error",
            "fatal error",
            "Unknown error occurred", 
            "Empty command"
        };
        if (errorCode > 0  &&  errorCode < sizeof(errorMsg)/sizeof(errorMsg[0])) {
            std::cout << "Error: " << errorMsg[errorCode] << std::endl;
        }
    }

private:
    template<typename T>
    using DecayT = typename std::decay<T>::type;

    template<typename T>
    struct IsVectorString : std::is_same<DecayT<T>, std::vector<std::string>> {};

    // Helper to detect if type is vector<string> or its reference/const variants (without decay)
    template<typename T>
    struct IsVectorStringExact : std::is_same<
        typename std::remove_cv<typename std::remove_reference<T>::type>::type,
        std::vector<std::string>
    > {};

    template<typename T1, typename T2>
    struct IsArgcArgv : std::integral_constant<bool,
        std::is_same<DecayT<T1>, int>::value && std::is_same<DecayT<T2>, char**>::value> {};

    // Function traits helper to extract argument types and arity
    template<typename F>
    struct FunctionTraits;

    // Function pointer
    template<typename R, typename... Args>
    struct FunctionTraits<R(*)(Args...)> {
        using return_type = R;
        static constexpr std::size_t arity = sizeof...(Args);

        template<std::size_t N>
        struct arg {
            using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
        };
    };

    // Function reference
    template<typename R, typename... Args>
    struct FunctionTraits<R(&)(Args...)> {
        using return_type = R;
        static constexpr std::size_t arity = sizeof...(Args);

        template<std::size_t N>
        struct arg {
            using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
        };
    };

    // Member function pointer
    template<typename R, typename C, typename... Args>
    struct FunctionTraits<R(C::*)(Args...)> {
        using return_type = R;
        static constexpr std::size_t arity = sizeof...(Args);

        template<std::size_t N>
        struct arg {
            using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
        };
    };

    // Const member function pointer
    template<typename R, typename C, typename... Args>
    struct FunctionTraits<R(C::*)(Args...) const> {
        using return_type = R;
        static constexpr std::size_t arity = sizeof...(Args);

        template<std::size_t N>
        struct arg {
            using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
        };
    };

    // Function object (including lambdas)
    template<typename F>
    struct FunctionTraits {
    private:
        using call_type = FunctionTraits<decltype(&F::operator())>;
    public:
        using return_type = typename call_type::return_type;
        static constexpr std::size_t arity = call_type::arity;

        template<std::size_t N>
        struct arg {
            using type = typename call_type::template arg<N>::type;
        };
    };

    // MSVC2015: only inspect arg<0> when arity is 1 (avoid tuple_element on empty tuple)
    template<typename F, typename = void>
    struct LambdaArg0IsVectorString : std::false_type {};

    template<typename F>
    struct LambdaArg0IsVectorString<F, typename std::enable_if<
        (FunctionTraits<typename std::decay<F>::type>::arity == 1)>::type>
        : std::integral_constant<bool,
            IsVectorStringExact<typename FunctionTraits<typename std::decay<F>::type>::template arg<0>::type>::value> {};

    // Implementation for no-argument functions
    template<typename R>
    void registerSlotImpl(const std::string& cmdName, R(*func)()) {
        commands_[cmdName] = [this, func](const std::vector<std::string>& args) -> ErrorCode {
            // if (!args.empty()) {
            //     return INVALID_ARGUMENT_COUNT;
            // }
            func();
            return SUCCESS;
        };
    }

    // Special-case: vector<string> (varargs) function pointer
    template<typename R>
    void registerSlotImpl(const std::string& cmdName, R(*func)(const std::vector<std::string>&)) {
        commands_[cmdName] = [func](const std::vector<std::string>& args) -> ErrorCode {
            func(args);
            return SUCCESS;
        };
    }

    // Special-case: vector<string> (varargs) function pointer by value
    template<typename R>
    void registerSlotImpl(const std::string& cmdName, R(*func)(std::vector<std::string>)) {
        commands_[cmdName] = [func](const std::vector<std::string>& args) -> ErrorCode {
            func(args); // copies if callback takes by value
            return SUCCESS;
        };
    }

    // Special-case: argc/argv function pointer
    template<typename R>
    void registerSlotImpl(const std::string& cmdName, R(*func)(int, char**)) {
        const std::string cmdNameCopy = cmdName;
        commands_[cmdName] = [func, cmdNameCopy](const std::vector<std::string>& args) -> ErrorCode {
            // argv is a view into existing string storage. Do not modify argv[i][j].
            const int argc = static_cast<int>(args.size());
            std::vector<char*> argvStorage(static_cast<size_t>(argc), nullptr);
            // argvStorage[0] = const_cast<char*>(cmdNameCopy.c_str());
            for (int i = 0; i < argc; ++i) {
                argvStorage[static_cast<size_t>(i)] = const_cast<char*>(args[static_cast<size_t>(i)].c_str());
            }
            func(argc, argvStorage.data());
            return SUCCESS;
        };
    }

    // Implementation for one-argument functions
    template<typename R, typename T>
    void registerSlotImpl(const std::string& cmdName, R(*func)(T)) {
        commands_[cmdName] = [this, func](const std::vector<std::string>& args) -> ErrorCode {
            if (args.size() != 1) {
                return INVALID_ARGUMENT_COUNT;
            }
            int errCode = 0;
            T val = convertArg<T>(args[0], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            func(val);
            return SUCCESS;
        };
    }

    // Implementation for two-argument functions
    template<typename R, typename T1, typename T2>
    void registerSlotImpl(const std::string& cmdName, R(*func)(T1, T2)) {
        commands_[cmdName] = [this, func](const std::vector<std::string>& args) -> ErrorCode {
            if (args.size() != 2) {
                return INVALID_ARGUMENT_COUNT;
            }
            int errCode = 0;
            T1 val1 = convertArg<T1>(args[0], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            T2 val2 = convertArg<T2>(args[1], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            func(val1, val2);
            return SUCCESS;
        };
    }

    // Implementation for three-argument functions
    template<typename R, typename T1, typename T2, typename T3>
    void registerSlotImpl(const std::string& cmdName, R(*func)(T1, T2, T3)) {
        commands_[cmdName] = [this, func](const std::vector<std::string>& args) -> ErrorCode {
            if (args.size() != 3) {
                return INVALID_ARGUMENT_COUNT;
            }
            int errCode = 0;
            T1 val1 = convertArg<T1>(args[0], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            T2 val2 = convertArg<T2>(args[1], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            T3 val3 = convertArg<T3>(args[2], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            func(val1, val2, val3);
            return SUCCESS;
        };
    }

    // Implementation for four-argument functions
    template<typename R, typename T1, typename T2, typename T3, typename T4>
    void registerSlotImpl(const std::string& cmdName, R(*func)(T1, T2, T3, T4)) {
        commands_[cmdName] = [this, func](const std::vector<std::string>& args) -> ErrorCode {
            if (args.size() != 4) {
                return INVALID_ARGUMENT_COUNT;
            }
            int errCode = 0;
            T1 val1 = convertArg<T1>(args[0], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            T2 val2 = convertArg<T2>(args[1], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            T3 val3 = convertArg<T3>(args[2], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            T4 val4 = convertArg<T4>(args[3], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            func(val1, val2, val3, val4);
            return SUCCESS;
        };
    }

    // Implementation for lambdas and function objects
    template<typename F>
    void registerSlotImpl(const std::string& cmdName, F&& func) {
        registerSlotLambda(cmdName, std::forward<F>(func), 
                          std::integral_constant<std::size_t, FunctionTraits<typename std::decay<F>::type>::arity>());
    }

    // Implementation for no-argument lambdas
    template<typename F>
    void registerSlotLambda(const std::string& cmdName, F&& func, std::integral_constant<std::size_t, 0>) {
        commands_[cmdName] = [this, func](const std::vector<std::string>& args) -> ErrorCode {
            // if (!args.empty()) {
            //     return INVALID_ARGUMENT_COUNT;
            // }
            func();
            return SUCCESS;
        };
    }

    // Implementation for one-argument lambdas
    template<typename F>
    void registerSlotLambda(const std::string& cmdName, F&& func, std::integral_constant<std::size_t, 1>) {
        registerSlotLambdaArity1(cmdName, std::forward<F>(func), LambdaArg0IsVectorString<typename std::decay<F>::type>());
    }

    template<typename F>
    void registerSlotLambdaArity1(const std::string& cmdName, F&& func, std::true_type) {
        commands_[cmdName] = [func](const std::vector<std::string>& args) -> ErrorCode {
            func(args);
            return SUCCESS;
        };
    }

    template<typename F>
    void registerSlotLambdaArity1(const std::string& cmdName, F&& func, std::false_type) {
        using Traits = FunctionTraits<typename std::decay<F>::type>;
        using ArgType = typename Traits::template arg<0>::type;

        commands_[cmdName] = [this, func](const std::vector<std::string>& args) -> ErrorCode {
            if (args.size() != 1) {
                return INVALID_ARGUMENT_COUNT;
            }
            int errCode = 0;
            auto val = convertArg<ArgType>(args[0], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            func(val);
            return SUCCESS;
        };
    }

    // Implementation for two-argument lambdas
    template<typename F>
    void registerSlotLambda(const std::string& cmdName, F&& func, std::integral_constant<std::size_t, 2>) {
        using Traits = FunctionTraits<typename std::decay<F>::type>;
        using FirstArg = typename Traits::template arg<0>::type;
        using SecondArg = typename Traits::template arg<1>::type;
        
        commands_[cmdName] = [this, func](const std::vector<std::string>& args) -> ErrorCode {
            if (args.size() != 2) {
                return INVALID_ARGUMENT_COUNT;
            }
            int errCode = 0;
            auto val1 = convertArg<FirstArg>(args[0], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            auto val2 = convertArg<SecondArg>(args[1], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            func(val1, val2);
            return SUCCESS;
        };
    }

    // Implementation for three-argument lambdas
    template<typename F>
    void registerSlotLambda(const std::string& cmdName, F&& func, std::integral_constant<std::size_t, 3>) {
        using Traits = FunctionTraits<typename std::decay<F>::type>;
        using FirstArg = typename Traits::template arg<0>::type;
        using SecondArg = typename Traits::template arg<1>::type;
        using ThirdArg = typename Traits::template arg<2>::type;
        
        commands_[cmdName] = [this, func](const std::vector<std::string>& args) -> ErrorCode {
            if (args.size() != 3) {
                return INVALID_ARGUMENT_COUNT;
            }
            int errCode = 0;
            auto val1 = convertArg<FirstArg>(args[0], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            auto val2 = convertArg<SecondArg>(args[1], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            auto val3 = convertArg<ThirdArg>(args[2], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            func(val1, val2, val3);
            return SUCCESS;
        };
    }

    // Implementation for four-argument lambdas
    template<typename F>
    void registerSlotLambda(const std::string& cmdName, F&& func, std::integral_constant<std::size_t, 4>) {
        using Traits = FunctionTraits<typename std::decay<F>::type>;
        using FirstArg = typename Traits::template arg<0>::type;
        using SecondArg = typename Traits::template arg<1>::type;
        using ThirdArg = typename Traits::template arg<2>::type;
        using FourthArg = typename Traits::template arg<3>::type;
        
        commands_[cmdName] = [this, func](const std::vector<std::string>& args) -> ErrorCode {
            if (args.size() != 4) {
                return INVALID_ARGUMENT_COUNT;
            }
            int errCode = 0;
            auto val1 = convertArg<FirstArg>(args[0], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            auto val2 = convertArg<SecondArg>(args[1], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            auto val3 = convertArg<ThirdArg>(args[2], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            auto val4 = convertArg<FourthArg>(args[3], errCode);
            if (errCode != 0) {
                return ARGUMENT_CONVERSION_ERROR;
            }
            func(val1, val2, val3, val4);
            return SUCCESS;
        };
    }

    // Split input string into tokens
    std::vector<std::string> tokenize(const std::string& input) const {
        std::vector<std::string> tokens;
        std::string current;
        bool inQuotes = false;
        
        for (size_t i = 0; i < input.length(); ++i) {
            char c = input[i];
            
            if (c == '"') {
                inQuotes = !inQuotes;
            } else if (c == ' ' && !inQuotes) {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }
        
        if (!current.empty()) {
            tokens.push_back(current);
        }
        
        return tokens;
    }
    
    // Map of command names to handlers
    std::map<std::string, CommandHandler> commands_;
    
    // Map of command names to help texts
    std::map<std::string, std::string> helpTexts_;

    // Exit flag
    bool exitFlag_;
};

}
#endif // COMMAND_PARSER_H 