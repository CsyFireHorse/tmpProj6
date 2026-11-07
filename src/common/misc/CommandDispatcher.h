#ifndef COMMAND_DISPATCHER_H
#define COMMAND_DISPATCHER_H

#include <cctype>
#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace zmc {

class CommandDispatcher {
public:
    typedef std::vector<std::string> ArgList;
    typedef std::function<int(const ArgList&)> VectorHandler;
    typedef std::function<int(int, char**)> ArgvHandler;

    enum {
        OK = 0,
        ERR_NOT_FOUND = -1,
        ERR_INVALID_ARG_COUNT = -2,
        ERR_CONVERT_ARG = -3,
        ERR_INVALID_BIND = -4
    };

    int bind(const std::string& cmd, const VectorHandler& handler) {
        if (!handler) {
            return ERR_INVALID_BIND;
        }
        handlers_[cmd] = handler;
        return OK;
    }

    int bind(const std::string& cmd, const ArgvHandler& handler) {
        if (!handler) {
            return ERR_INVALID_BIND;
        }
        handlers_[cmd] = adaptArgvHandler(handler);
        return OK;
    }

    int setDefaultHandler(const VectorHandler& handler) {
        if (!handler) {
            return ERR_INVALID_BIND;
        }
        defaultHandler_ = handler;
        return OK;
    }

    int setDefaultHandler(const ArgvHandler& handler) {
        if (!handler) {
            return ERR_INVALID_BIND;
        }
        defaultHandler_ = adaptArgvHandler(handler);
        return OK;
    }

    int parse(const ArgList& tokens, bool withoutArgv0 = false) const {
        const size_t cmdIndex = withoutArgv0 ? 0 : 1;
        if (tokens.size() <= cmdIndex) {
            return callDefaultHandler();
        }

        const std::string& cmd = tokens[cmdIndex];
        std::map<std::string, VectorHandler>::const_iterator it = handlers_.find(cmd);
        if (it == handlers_.end()) {
            return ERR_NOT_FOUND;
        }

        // Handlers always receive the payload after the outer argv[0] slot.
        ArgList args(tokens.begin() + 1, tokens.end());
        return it->second(args);
    }

    int parse(int argc, char** argv, bool withoutArgv0 = false) const {
        if (argc <= 0 || argv == NULL) {
            return callDefaultHandler();
        }

        ArgList tokens;
        tokens.reserve(static_cast<size_t>(argc));
        for (int i = 0; i < argc; ++i) {
            tokens.push_back(argv[i] == NULL ? std::string() : std::string(argv[i]));
        }
        return parse(tokens, withoutArgv0);
    }

private:
    static VectorHandler adaptArgvHandler(const ArgvHandler& handler) {
        return [handler](const ArgList& args) -> int {
            std::vector<char*> argvView(args.size(), static_cast<char*>(NULL));
            for (size_t i = 0; i < args.size(); ++i) {
                // The argv view is only valid during the handler call.
                argvView[i] = const_cast<char*>(args[i].c_str());
            }
            return handler(static_cast<int>(argvView.size()), argvView.empty() ? static_cast<char**>(NULL) : &argvView[0]);
        };
    }

    int callDefaultHandler() const {
        if (!defaultHandler_) {
            return ERR_NOT_FOUND;
        }
        return defaultHandler_(ArgList());
    }

    std::map<std::string, VectorHandler> handlers_;
    VectorHandler defaultHandler_;
};

namespace detail {

template<typename Signature>
inline bool isCallableValid(const std::function<Signature>& fn) {
    return static_cast<bool>(fn);
}

template<typename F>
inline bool isCallableValid(const F&) {
    return true;
}

inline bool parseIntToken(const std::string& text, int& value) {
    try {
        size_t pos = 0;
        value = std::stoi(text, &pos);
        return pos == text.size();
    } catch (...) {
        value = 0;
        return false;
    }
}

inline bool parseDoubleToken(const std::string& text, double& value) {
    try {
        size_t pos = 0;
        value = std::stod(text, &pos);
        return pos == text.size();
    } catch (...) {
        value = 0.0;
        return false;
    }
}

inline bool parseBoolToken(const std::string& text, bool& value) {
    std::string lower(text);
    for (size_t i = 0; i < lower.size(); ++i) {
        lower[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(lower[i])));
    }

    if (lower == "1" || lower == "true" || lower == "yes" || lower == "y") {
        value = true;
        return true;
    }
    if (lower == "0" || lower == "false" || lower == "no" || lower == "n") {
        value = false;
        return true;
    }

    value = false;
    return false;
}

template<typename F>
inline typename std::enable_if<
    std::is_void<typename std::result_of<F()>::type>::value,
    int>::type
invokeNullary(F fn) {
    fn();
    return CommandDispatcher::OK;
}

template<typename F>
inline typename std::enable_if<
    !std::is_void<typename std::result_of<F()>::type>::value,
    int>::type
invokeNullary(F fn) {
    return static_cast<int>(fn());
}

template<typename F, typename T>
inline typename std::enable_if<
    std::is_void<typename std::result_of<F(const T&)>::type>::value,
    int>::type
invokeUnary(F fn, const T& value) {
    fn(value);
    return CommandDispatcher::OK;
}

template<typename F, typename T>
inline typename std::enable_if<
    !std::is_void<typename std::result_of<F(const T&)>::type>::value,
    int>::type
invokeUnary(F fn, const T& value) {
    return static_cast<int>(fn(value));
}

template<typename F>
inline CommandDispatcher::VectorHandler makeVoidHandlerImpl(F fn) {
    if (!isCallableValid(fn)) {
        return CommandDispatcher::VectorHandler();
    }

    return [fn](const CommandDispatcher::ArgList& args) -> int {
        if (args.size() != 1) {
            return CommandDispatcher::ERR_INVALID_ARG_COUNT;
        }
        return invokeNullary(fn);
    };
}

template<typename F>
inline CommandDispatcher::VectorHandler makeIntHandlerImpl(F fn) {
    if (!isCallableValid(fn)) {
        return CommandDispatcher::VectorHandler();
    }

    return [fn](const CommandDispatcher::ArgList& args) -> int {
        if (args.size() != 2) {
            return CommandDispatcher::ERR_INVALID_ARG_COUNT;
        }

        int value = 0;
        if (!parseIntToken(args[1], value)) {
            return CommandDispatcher::ERR_CONVERT_ARG;
        }
        return invokeUnary(fn, value);
    };
}

template<typename F>
inline CommandDispatcher::VectorHandler makeStringHandlerImpl(F fn) {
    if (!isCallableValid(fn)) {
        return CommandDispatcher::VectorHandler();
    }

    return [fn](const CommandDispatcher::ArgList& args) -> int {
        if (args.size() != 2) {
            return CommandDispatcher::ERR_INVALID_ARG_COUNT;
        }
        return invokeUnary(fn, args[1]);
    };
}

template<typename F>
inline CommandDispatcher::VectorHandler makeDoubleHandlerImpl(F fn) {
    if (!isCallableValid(fn)) {
        return CommandDispatcher::VectorHandler();
    }

    return [fn](const CommandDispatcher::ArgList& args) -> int {
        if (args.size() != 2) {
            return CommandDispatcher::ERR_INVALID_ARG_COUNT;
        }

        double value = 0.0;
        if (!parseDoubleToken(args[1], value)) {
            return CommandDispatcher::ERR_CONVERT_ARG;
        }
        return invokeUnary(fn, value);
    };
}

template<typename F>
inline CommandDispatcher::VectorHandler makeBoolHandlerImpl(F fn) {
    if (!isCallableValid(fn)) {
        return CommandDispatcher::VectorHandler();
    }

    return [fn](const CommandDispatcher::ArgList& args) -> int {
        if (args.size() != 2) {
            return CommandDispatcher::ERR_INVALID_ARG_COUNT;
        }

        bool value = false;
        if (!parseBoolToken(args[1], value)) {
            return CommandDispatcher::ERR_CONVERT_ARG;
        }
        return invokeUnary(fn, value);
    };
}

}  // namespace detail

inline CommandDispatcher::VectorHandler makeVoidHandler(const std::function<void(void)>& fn) {
    return detail::makeVoidHandlerImpl(fn);
}

inline CommandDispatcher::VectorHandler makeVoidHandler(const std::function<int(void)>& fn) {
    return detail::makeVoidHandlerImpl(fn);
}

template<typename F>
inline CommandDispatcher::VectorHandler makeVoidHandler(F fn) {
    return detail::makeVoidHandlerImpl(fn);
}

inline CommandDispatcher::VectorHandler makeIntHandler(const std::function<void(int)>& fn) {
    return detail::makeIntHandlerImpl(fn);
}

inline CommandDispatcher::VectorHandler makeIntHandler(const std::function<int(int)>& fn) {
    return detail::makeIntHandlerImpl(fn);
}

template<typename F>
inline CommandDispatcher::VectorHandler makeIntHandler(F fn) {
    return detail::makeIntHandlerImpl(fn);
}

inline CommandDispatcher::VectorHandler makeStringHandler(const std::function<void(const std::string&)>& fn) {
    return detail::makeStringHandlerImpl(fn);
}

inline CommandDispatcher::VectorHandler makeStringHandler(const std::function<int(const std::string&)>& fn) {
    return detail::makeStringHandlerImpl(fn);
}

template<typename F>
inline CommandDispatcher::VectorHandler makeStringHandler(F fn) {
    return detail::makeStringHandlerImpl(fn);
}

inline CommandDispatcher::VectorHandler makeDoubleHandler(const std::function<void(double)>& fn) {
    return detail::makeDoubleHandlerImpl(fn);
}

inline CommandDispatcher::VectorHandler makeDoubleHandler(const std::function<int(double)>& fn) {
    return detail::makeDoubleHandlerImpl(fn);
}

template<typename F>
inline CommandDispatcher::VectorHandler makeDoubleHandler(F fn) {
    return detail::makeDoubleHandlerImpl(fn);
}

inline CommandDispatcher::VectorHandler makeBoolHandler(const std::function<void(bool)>& fn) {
    return detail::makeBoolHandlerImpl(fn);
}

inline CommandDispatcher::VectorHandler makeBoolHandler(const std::function<int(bool)>& fn) {
    return detail::makeBoolHandlerImpl(fn);
}

template<typename F>
inline CommandDispatcher::VectorHandler makeBoolHandler(F fn) {
    return detail::makeBoolHandlerImpl(fn);
}

}  // namespace zmc

#endif  // COMMAND_DISPATCHER_H
