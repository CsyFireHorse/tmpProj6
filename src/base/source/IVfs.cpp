#include "IVfs.hpp"

// Implements platform-independent virtual path normalization. AI modification signature: GPT-6 Codex.

#include <cctype>

namespace zmc {
namespace {

bool validUtf8(const std::string& value) {
    size_t i = 0;
    while (i < value.size()) {
        const unsigned char c = static_cast<unsigned char>(value[i]);
        if (c < 0x80) {
            if (c < 0x20 || c == 0x7f || c == '\\' || c == ':' || c == '<' || c == '>' ||
                c == '"' || c == '|' || c == '?' || c == '*') return false;
            ++i;
            continue;
        }
        size_t count = c >= 0xc2 && c <= 0xdf ? 1 :
                       c >= 0xe0 && c <= 0xef ? 2 :
                       c >= 0xf0 && c <= 0xf4 ? 3 : 0;
        if (!count || i + count >= value.size()) return false;
        for (size_t j = 1; j <= count; ++j) {
            if ((static_cast<unsigned char>(value[i + j]) & 0xc0) != 0x80) return false;
        }
        const unsigned char second = static_cast<unsigned char>(value[i + 1]);
        if ((c == 0xe0 && second < 0xa0) || (c == 0xed && second >= 0xa0) ||
            (c == 0xf0 && second < 0x90) || (c == 0xf4 && second >= 0x90)) return false;
        i += count + 1;
    }
    return true;
}

bool reservedWindowsName(const std::string& name) {
    const size_t dot = name.find('.');
    std::string stem = name.substr(0, dot);
    for (size_t i = 0; i < stem.size(); ++i) {
        stem[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(stem[i])));
    }
    if (stem == "CON" || stem == "PRN" || stem == "AUX" || stem == "NUL") return true;
    return stem.size() == 4 &&
           (stem.compare(0, 3, "COM") == 0 || stem.compare(0, 3, "LPT") == 0) &&
           stem[3] >= '1' && stem[3] <= '9';
}

}  // namespace

VfsResult normalizeVfsPath(const std::string& path, std::string& normalized) {
    if (path.empty() || !validUtf8(path) || path.compare(0, 2, "//") == 0) {
        return VfsResult::InvalidPath;
    }
    std::string result("/");
    size_t start = path[0] == '/' ? 1 : 0;
    while (start <= path.size()) {
        const size_t end = path.find('/', start);
        const size_t length = (end == std::string::npos ? path.size() : end) - start;
        const std::string part = path.substr(start, length);
        if (part == "..") return VfsResult::InvalidPath;
        if (!part.empty() && part != ".") {
            if (part[part.size() - 1] == '.' || part[part.size() - 1] == ' ' ||
                reservedWindowsName(part)) return VfsResult::InvalidPath;
            if (result.size() > 1) result += '/';
            result += part;
        }
        if (end == std::string::npos) break;
        start = end + 1;
    }
    normalized.swap(result);
    return VfsResult::Ok;
}

}  // namespace zmc
