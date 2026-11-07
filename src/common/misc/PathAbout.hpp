#ifndef _ZMC_PATH_ABOUT_HPP_
#define _ZMC_PATH_ABOUT_HPP_
#include <string>

using std::string;

namespace zmc {
constexpr inline char get_spliter() {
#ifdef __linux__
    return '/';
#elif defined(WIN32)
    return '\\';
#elif defined(__GNUC__)
    return '/';
#else
    return '/';
#endif
}

inline bool startsWith(const std::string& str, const std::string& prefix) {
    return (str.rfind(prefix, 0) == 0);
}

inline bool endsWith(const std::string& str, const std::string& suffix) {
    if (suffix.length() > str.length()) { return false; }
    return (str.rfind(suffix) == (str.length() - suffix.length()));
}

inline void str_replace2(string& buf, const string& fnd, const string& rep) {
    buf = buf.replace(buf.find(fnd), fnd.length(), rep);
}
inline void str_replace(string& buf, const string& fnd, const string& rep) {
    if (startsWith(buf, fnd)){
        buf = rep + buf.substr(fnd.length());
    }
}

// Returns the path-string without the filename part
inline std::string getFolderPath(const std::string& path) {
    // size_t found = path.find_last_of(get_spliter());  // "/\\"
    const size_t found = path.find_last_of("/\\");
    return (found == string::npos) ? "." : path.substr(0, found);
}

// Returns only the filename part (after the last slash)
inline std::string getFileName(const std::string& path) {
    const size_t found = path.find_last_of("/\\");
    return (found == string::npos) ? path : path.substr(found + 1);
}
}  // namespace zmc

#endif