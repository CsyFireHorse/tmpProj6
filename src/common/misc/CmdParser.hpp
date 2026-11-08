#ifndef _CMD_PARSER_HPP_
#define _CMD_PARSER_HPP_

#include <string>
#include <vector>
#include "stdio.h"
#include <functional>
#include <unordered_map>
#include <type_traits>
#include <stdexcept>

namespace zmc{


inline int stoiWrap(const std::string& cmds, int& errCode){
    errCode = 0;
    int val;
    try {
        val = std::stoi(cmds);
    }catch (const std::invalid_argument& e) {
        // std::cerr << "Invalid argument: " << e.what() << '\n';
        errCode = -1;
        return 0;
    }catch (const std::out_of_range& e) {
        // std::cerr << "Value out of range: " << e.what() << '\n';
        errCode = -1;
        return 0;
    }
    return val;
}
inline double stodWrap(const std::string& cmds, int& errCode){
    errCode = 0;
    double val;
    try {
        val = std::stod(cmds);
    }catch (const std::invalid_argument& e) {
        // std::cerr << "Invalid argument: " << e.what() << '\n';
        errCode = -1;
        return 0;
    }catch (const std::out_of_range& e) {
        // std::cerr << "Value out of range: " << e.what() << '\n';
        errCode = -1;
        return 0;
    }
    return val;
}

template<typename F>
int call(F& f, const std::string& cmds){
    if (std::is_same<F, std::function<void(void)>>::value)
        printf("F is function\n");
    else{
        printf("F is else \n");
        f();
    }
    return -1;
}
using VoidVoid = void(void);
template<>
int call(VoidVoid& f, const std::string& cmds){
    f();
    return 0;
}
template<>
int call(std::function<void(void)>& f, const std::string& cmds){
    printf("call std function\n");
    f(); 
    // std::forward<std::function<void(void)>>(f)();
    return 0;
}
template<>
int call(std::function<void(double)>& f, const std::string& cmds){
    printf("call double function\n");
    int errCode;
    double val = stodWrap(cmds, errCode);
    if (errCode) return errCode;
    f(val);
    return 0;
}
template<>
int call(std::function<void(int)>& f, const std::string& cmds){
    printf("call function<void(int)\n");
    int errCode;
    int val = stoiWrap(cmds, errCode);
    if (errCode) return errCode;
    f(val);
    return 0;
}

using IntVoid = int(void);
template<>
int call(IntVoid& f, const std::string& cmds){
    return f();
}


template<>
int call(void(&f)(int), const std::string& cmds){
    int errCode;
    int val = stoiWrap(cmds, errCode);
    if (errCode) return errCode;
    f(val);    
    return 0;
}
template<>
int call(int (&f)(int), const std::string& cmds){
    int errCode;
    int val = stoiWrap(cmds, errCode);
    if (errCode) return errCode;
    return f(val);    
}

template<>
int call(void (&f)(double), const std::string& cmds){
    printf("call double fun pointer\n");
    int errCode;
    double val = stodWrap(cmds, errCode);
    if (errCode) return errCode;
    f(val);
    return 0;
}

template<>
int call(int (&f)(double), const std::string& cmds){
    int errCode;
    double val = stodWrap(cmds, errCode);
    if (errCode) return errCode;
    return f(val);
}

template<>
int call(void (&f)(const std::string&), const std::string& cmds){
    f(cmds);
    return 0;
}

template<>
int call(int (&f)(const std::string&), const std::string& cmds){
    return f(cmds);
}

// class StrFuncMatcher{
struct CmdParser{
    // CmdParser(){ }
    template<typename F>
    void bind(const std::string& str, F&& f){
        // : str_arr(strs)
        auto lf = [&](const std::string& cmds){
            return call(f, cmds);
        };
        map[str] = lf;
    }
    // std::vector<std::string> str_arr;
    std::unordered_map<std::string, std::function<int(const std::string&)>> map;

    void genHelpStr(std::string& help_str)const{
        for (const auto& s:map){
            if (s.first.size())
                help_str += s.first + "\n";
        }
    }
    int match(const std::vector<std::string>& args){
        int ret = 0;
        int fd = -1;
        int argc = args.size();
        for (const auto& s:map){
        // for (auto& cmd:cmds){
            if (argc == 0 && s.first.size() == 0 ){
                ret = s.second("");
                fd = 0;
                break;
            }
            if (argc && s.first == args[0]){
                std::string arg(argc>=2?args[1]:"");
                ret = s.second(arg);
                fd = 0;
                break;
            }
        }
        if (fd == -1){
            printf("not found\n");
            ret = -1;
        }
        return ret;
    }
    int match(int argc, char** argv){
        int ret = 0;
        int fd = -1;
        for (const auto& s:map){
        // for (auto& cmd:cmds){
            if (argc == 0 && s.first.size() == 0 ){
                ret = s.second("");
                fd = 0;
                break;
            }
            if (argc && s.first == argv[0]){
                std::string arg(argc>=2?argv[1]:"");
                ret = s.second(arg);
                fd = 0;
                break;
            }
        }
        if (fd == -1){
            printf("not found\n");
            ret = -1;
        }
        return ret;
    }
};





} /* namespace zmc */

#endif
