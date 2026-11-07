#pragma once

namespace zmc{
constexpr unsigned int strhash(const char* str, int h = 0) {
    return !str[h] ? 5381 : (strhash(str, h+1) * 33) ^ str[h];
}

constexpr unsigned int constLength(const char* str){
    return (*str == 0) ? 0 : constLength(str + 1) + 1;
}

}
