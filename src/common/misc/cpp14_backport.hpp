#ifndef _ZMC_CPP14_BACKPORT_HPP_
#define _ZMC_CPP14_BACKPORT_HPP_

// C++14 (and selected C++17 metafunction) polyfills for -std=c++11 builds.
// When the toolchain already provides a feature, alias to std:: where possible.

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

// MSVC reports __cplusplus=199711L unless /Zc:__cplusplus; use _MSVC_LANG when set.
#if (defined(_MSVC_LANG) && _MSVC_LANG >= 201402L) || (__cplusplus >= 201402L)
#define ZMC_HAS_CXX14 1
#else
#define ZMC_HAS_CXX14 0
#endif

#if (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || (__cplusplus >= 201703L)
#define ZMC_HAS_CXX17 1
#else
#define ZMC_HAS_CXX17 0
#endif

namespace zmc {

#if ZMC_HAS_CXX17

using std::void_t;

#else

template <typename...>
struct make_void {
    using type = void;
};

template <typename... Ts>
using void_t = typename make_void<Ts...>::type;

#endif

#if ZMC_HAS_CXX14

using std::enable_if_t;
using std::make_unique;

#else

template <bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

// Single-object make_unique only (no size_t overload: avoids MSVC C2668 ambiguity).
template <class T, class... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#endif

#if ZMC_HAS_CXX14

using std::exchange;

#else

template <class T>
T exchange(T& obj, T new_value) {
    T old_value = std::move(obj);
    obj = std::move(new_value);
    return old_value;
}

#endif

}  // namespace zmc

#endif  // _ZMC_CPP14_BACKPORT_HPP_
