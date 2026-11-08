/*
 * @Author: ChenSy
 * @LastEditors: ChenSy
 * @Date: 2022-03-24 16:01:11
 * @LastEditTime: 2022-07-22 16:06:49
 * @Description: Modify here please
 */

#ifndef MISC_UTILS_H_
#define MISC_UTILS_H_

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

namespace zmc{
// Multiple copies are made; only string_view avoids copying.
std::vector<std::string> stringSplit0(const std::string& strIn, char delim);
std::vector<std::string> stringSplit(const std::string& strIn, char delim);

std::string& stringTrim(std::string &s) ;

std::string& stringTrim(std::string &&s);

void strSplitArgs(const std::string &cmdline, std::string &cmd, std::vector<std::string>& vargs, char sep=' ');

// int cmd_parse_loop(int (*pFunc)(const std::string&));

class CmdInterLooper{
public:
    int exit() { exitFlag = true;}
    void set_prompt(const std::string& str){
        pre_word = str;
    }
    template <typename Func, typename... Args>
    int run2(Func func, Args&... __args){
        char        cmd_org[4096];
        int errCode = 0;
        while (!exitFlag){
            std::cout << pre_word;
            std::cout.flush();

            std::cin.getline(cmd_org, sizeof(cmd_org));
            auto cmd2 = std::string(cmd_org);
            std::string cmd_data = stringTrim(cmd2);
            if (!cmd_data.size()) continue;
            errCode = func(cmd_data, __args...); 
            if (errCode){
                // Exit on error.
                break;
            }
        }
        std::cout<< " exit " << errCode << " " << exitFlag << std::endl;
        return errCode;
    }

    template <typename... Args>
    int run(int (*pFunc)(const std::string&, Args&...), Args&... __args){
        char        cmd_org[4096];
        int errCode = 0;
        while (!exitFlag){
            std::cout << pre_word;
            std::cout.flush();

            std::cin.getline(cmd_org, sizeof(cmd_org));
            auto cmd2 = std::string(cmd_org);
            std::string cmd_data = stringTrim(cmd2);
            if (!cmd_data.size()) continue;
            errCode = pFunc(cmd_data, __args...); 
            if (errCode){
                // Exit on error.
                break;
            }
        }
        std::cout<< " exit " << errCode << " " << exitFlag << std::endl;
        return errCode;
    }
private:
    bool exitFlag = false;
    std::string pre_word = "cmd> ";
};

/** Parse command-line arguments using lvalue references only; forwarding references are not supported.
 * pFunc   Parser function.
 * __args  Variadic arguments.
 * 
*/
template <typename... Args>
int cmd_parse_loop(int (*pFunc)(const std::string&, Args&...), Args&... __args){
    char        cmd_org[4096];
    int errCode = 0;
    while (1){
        std::cout << "cmd> ";
        std::cout.flush();

        std::cin.getline(cmd_org, sizeof(cmd_org));
        auto cmd2 = std::string(cmd_org);
        std::string cmd_data = stringTrim(cmd2);
        if (!cmd_data.size()) continue;

        if ("break" == cmd_data){
            // Exit normally.
            break;
        }else{   
            errCode = pFunc(cmd_data, __args...); 
            if (errCode){
                // Exit on error.
                break;
            }
        }
    }
    std::cout<< " exit " << errCode << std::endl;
    return errCode;
}


int cmd_parse_loopWrap(int (*pFunc)(const std::string&));


template <typename... Args>
int load_file_parse(const std::string& file_path, int (*pFunc)(const std::string&, Args&...), Args&... __args){
    std::ifstream infile(file_path);
    std::string line;
    if(!infile){
        std::cout << "not found file:" << file_path << std::endl;
        std::cout << "load_file_parse exit -1" << std::endl;
        return -1;
    }
    std::cout<< "file:" << file_path << std::endl;

    int i=0;
    int errCode = 0;
    while (std::getline(infile, line)) {
        if (!line.size()) continue;
        std::cout << ++i <<" line," << line << std::endl;
        std::string cmd_data = stringTrim(std::string(line));
        if (!cmd_data.size()) continue;

        if ("break" == cmd_data){
            // Exit normally.
            break;
        }else{   
            errCode = pFunc(cmd_data, __args...); 
            if (errCode){
                // Exit on error.
                break;
            }
        }
    } 
    std::cout<<"load_file_parse exit "<<  errCode << std::endl;
    return 0;
}

template <typename... Args>
int cmd_lines_parse(std::string& lines, int (*pFunc)(const std::string&, Args&...), Args&... __args)
{
    std::stringstream infile(lines);
    int i=0;
    std::string line;
    while (std::getline(infile, line)) {
        if (!line.size()) continue;
        std::cout << ++i <<" line," << line << std::endl;
        std::string cmd_data = stringTrim(std::string(line));
        if (!cmd_data.size()) continue;
        if ("break" == cmd_data){
            // Exit normally.
            break;
        }else{   
            int errCode = pFunc(cmd_data, __args...); 
            if (errCode){
                // Exit on error.
                return errCode;
            }
        }
    }
    return 0;
}

} /* namespace zmc */


#endif /* MISC_UTILS_H_ */
