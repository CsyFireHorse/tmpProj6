#ifndef _ZMC_FILE_MISC_HPP_
#define _ZMC_FILE_MISC_HPP_

#include <sys/stat.h>
#include <string>
#include <cstdlib>
// #include "eni_zcpb_data.h"
#ifdef _WIN32
#include <direct.h>
#define CREATE_DIR(path) _mkdir(path)
#define PATH_SEP "\\"
#else
#include <sys/types.h>
#define CREATE_DIR(path) mkdir(path, 0755)
#define PATH_SEP "/"
#endif

namespace zmc{
/**
 * Get system temporary directory path (cross-platform)
 * Windows: %TEMP% or %TMP%
 * Linux: $TMPDIR, $TEMP, $TMP, or /tmp
 */
inline std::string get_system_tmp_dir() {
    #ifdef _WIN32
        // Windows: try TEMP, then TMP
        const char* temp = getenv("TEMP");
        if (!temp) temp = getenv("TMP");
        if (temp) {
            std::string path(temp);
            // Normalize path separator
            for (char& c : path) {
                if (c == '/') c = '\\';
            }
            // Ensure trailing separator
            if (path.back() != '\\') path += '\\';
            return path;
        }
        // Fallback
        return "C:\\Windows\\Temp\\";
    #else
        // Linux/Unix: try TMPDIR, TEMP, TMP, or default to /tmp
        const char* temp = getenv("TMPDIR");
        if (!temp) temp = getenv("TEMP");
        if (!temp) temp = getenv("TMP");
        if (!temp) temp = "/tmp";
        
        std::string path(temp);
        // Ensure trailing separator
        if (path.back() != '/') path += '/';
        return path;
    #endif
    }
    
    /**
     * Ensure directory exists (cross-platform)
     */
    inline bool ensure_directory(const std::string& dir_path) {
        struct stat st = {0};
        
        if (stat(dir_path.c_str(), &st) == 0) {
            // Directory exists
            return true;
        }
        
        // Try to create directory
        if (CREATE_DIR(dir_path.c_str()) == 0) {
            return true;
        }
        
        // Check if it exists now (maybe created by another process)
        if (stat(dir_path.c_str(), &st) == 0) {
            return true;
        }
        
        return false;
    }
    

}
#endif