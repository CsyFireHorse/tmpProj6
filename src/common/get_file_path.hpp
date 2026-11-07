#ifndef _ZMC_GET_FILE_PATH_HPP_
#define _ZMC_GET_FILE_PATH_HPP_
#include <string>

namespace zmc {
inline std::string _get_file_path(const char *fname) {
    std::string pth(__FILE__);
    // strlen("common/get_file_path.hpp") = 25
    // src/
    return pth.substr(0, pth.size() - 24) + fname;
}

inline std::string* new_get_file_path(const char* fname){
    return new std::string(_get_file_path(fname));
}
inline char* new_get_file_path_pchar(const char* fname){
    return (char*)new_get_file_path(fname)->c_str();
}

}  // namespace  zmc
#endif