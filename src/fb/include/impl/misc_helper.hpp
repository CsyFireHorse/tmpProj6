#ifndef _MISC_HELPER_HPP_
#define _MISC_HELPER_HPP_
#include <type_traits>
#include <cstring>

namespace zmc {

template <typename T>
inline void all_zero_init(T& obj) {
    // Static assertions to prevent initialization of non-trivially copyable types
    static_assert(std::is_trivially_copyable<T>::value, 
        "Cannot zero-initialize non-trivially copyable type");
    
    // Zero out the memory
    std::memset(static_cast<void*>(&obj), 0, sizeof(T));
}

// Specialization for pointers
template <typename T>
void all_zero_init(T* ptr, size_t size) {
    if (ptr) {
        std::memset(static_cast<void*>(ptr), 0, size);
    }
}

} // namespace zmc
#endif