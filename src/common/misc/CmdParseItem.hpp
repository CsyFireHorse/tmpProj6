#ifndef _CMD_PARSE_ITEM_HPP_
#define _CMD_PARSE_ITEM_HPP_

#include <string>
#include <vector>

namespace zmc{

#define CMD_WRAP(Func) (void*)Func, functype2int<decltype(Func)>()

enum FuncType{
    VOID_VOID,
    INT_VOID,
    VOID_PCHAR,
    VOID_INT,
    INT_INT,
    VOID_DOUBLE,
    INT_PCHAR,
    INT_INT_PCHAR
};

typedef void(*pVoid)(void);
typedef int(*pIntVoid)(void);
typedef void(*pVoidPchar)(const char*);
typedef void(*pVoidInt)(int);
typedef void(*pVoidDouble)(double);
typedef int(*pIntPchar)(const char*);
typedef int(*pIntIntPchar)(int, char**);

template<typename F>
int functype2int(){
    if (std::is_same<F, void(void)>::value)
        return VOID_VOID;
    else if (std::is_same<F, void(const char*)>::value)
        return VOID_PCHAR;
    else if (std::is_same<F, void(int)>::value)
        return VOID_INT;
    else if (std::is_same<F, int(void)>::value)
        return INT_VOID;
    else if (std::is_same<F, int(const char*)>::value)
        return INT_PCHAR;
    else if (std::is_same<F, void(double)>::value)
        return VOID_DOUBLE;
    else if (std::is_same<F, int(int, char**)>::value)
        return INT_INT_PCHAR;
    return -1;
}

struct CmdParseItem{
    std::vector<std::string> str_arr;
    void*   pFunc;
    int argsType;
    std::string helpStr;
    bool match(int argc, char** argv)const {
        if (argc != str_arr.size()) return false;
        int i=0;
        for(auto& s:str_arr){
            if (s.size() && argv[i++] != s)
                return false;
        }
        return true;
    };
    int call(int argc, char** argv)const{
        int ret = 0;
        switch (argsType){
        case VOID_VOID:{
            pVoid pV = (pVoid) pFunc;
            pV();
            break;
        }case INT_VOID:{
            pIntVoid pV = (pIntVoid) pFunc;
            ret = pV();
            break;
        }case VOID_PCHAR:{
            pVoidPchar pV = (pVoidPchar) pFunc;
            pV(argv[1]);
            break;
        }case VOID_INT:{
            pVoidInt pV = (pVoidInt) pFunc;
            pV(std::stoi(argv[1]));
            break;
        }case VOID_DOUBLE:{
            pVoidDouble pV = (pVoidDouble) pFunc;
            pV(std::stod(argv[1]));
            break;
        }case INT_PCHAR:{
            pIntPchar pV = (pIntPchar) pFunc;
            ret = pV(argv[1]);
            break;
        }case INT_INT_PCHAR:{
            pIntIntPchar pV = (pIntIntPchar) pFunc;
            ret = pV(argc, argv);
            break;
        }default:{
            ret = -1;
            break;
        }
        }
        return ret;
    }
};

inline void genHelpStr(const std::vector<CmdParseItem>& cmds, std::string& help_str){
    for (const auto& s:cmds){
        if (s.str_arr.size()){
            for (const auto& arg:s.str_arr)
                help_str += arg + " ";
            help_str += s.helpStr + "\n";
        }
    }
}

inline int cmdParseItem_match(const std::vector<CmdParseItem>& cmds, int argc, char** argv){
    int ret = 0;
    int fd = -1;
    for (auto& cmd:cmds){
        if (cmd.match(argc, argv)){
            ret = cmd.call(argc, argv);
            fd = 0;
            break;
        }
    }
    if (fd == -1){
        printf("not found\n");
        cmds[0].call(0, nullptr);
    }
    return ret;
}

} /* namespace zmc */

#endif