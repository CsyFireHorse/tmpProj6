#ifndef _ZMC_FBT_TRAITS_HPP_
#define _ZMC_FBT_TRAITS_HPP_

#include "fbt_common.h"
#include "fbt_impl_enum_def.h"
#include <type_traits>
#include <utility>  // for std::declval
#include "misc/cpp14_backport.hpp"
#include "fbt_motion_traits.hpp"

namespace zmc{

template <typename T>
struct has_buffer_mode {
private:
    // Test for BufferMode member
    template <typename C>
    static constexpr auto test(int) -> decltype(std::declval<C>().BufferMode, bool()) {
        return true;
    }
    
    // Fallback
    template <typename>
    static constexpr bool test(...) {
        return false;
    }
    
public:
    static constexpr bool value = test<T>(0);
};

//  bufferMode 
template <typename T, enable_if_t<has_buffer_mode<T>::value, int> = 0>
MC_BUFFER_MODE get_buffer_mode_or_default(const T& obj) {
    return obj.BufferMode;
}

// bufferMode
template <typename T, enable_if_t<!has_buffer_mode<T>::value, int> = 0>
MC_BUFFER_MODE get_buffer_mode_or_default(const T&) {
    return mcAborting;
}

template <typename T>
struct has_direction {
private:
    // Test for Direction member
    template <typename C>
    static constexpr auto test(int) -> decltype(std::declval<C>().Direction, bool()) {
        return true;
    }
    
    // Fallback
    template <typename>
    static constexpr bool test(...) {
        return false;
    }
    
public:
    static constexpr bool value = test<T>(0);
};


template <typename T, enable_if_t<has_direction<T>::value, int> = 0>
MC_DIRECTION get_direction_or_default(const T& obj) {
    return obj.Direction;
}

template <typename T, enable_if_t<!has_direction<T>::value, int> = 0>
MC_DIRECTION get_direction_or_default(const T&) {
    return mcShortestWay;
}

template <typename T, typename = void>
struct has_in_end_velocity {
private:
    // Test for InEndVelocity member
    template <typename C>
    static constexpr auto test(int) -> decltype(std::declval<C>().InEndVelocity, bool()) {
        return true;
    }
    
    // Fallback
    template <typename>
    static constexpr bool test(...) {
        return false;
    }
    
public:
    static constexpr bool value = test<T>(0);
};

template <typename T>
constexpr bool is_fb_has_in_end_velocity() {
    return has_in_end_velocity<T>::value;
}

template <typename T>
struct has_done{
private:
    // Test for Axis member
    template <typename C>
    static constexpr auto test(int) -> decltype(std::declval<C>().Done, bool()) {
        return true;
    }
    
    // Fallback
    template <typename>
    static constexpr bool test(...) {
        return false;
    }
    
public:
    static constexpr bool value = test<T>(0);
};

template <typename P>
inline MC_AXIS_STATES get_axis_stat_when_fb_activated() {
    return has_done<P>::value ? mcDiscreteMotion
                              : (is_fb_has_in_end_velocity<P>() ? mcContinuousMotion
                                                                : mcSynchronizedMotion);
}

class FbtJogStruct;
template <>
inline MC_AXIS_STATES get_axis_stat_when_fb_activated<FbtJogStruct>() {
    return mcContinuousMotion;
}
class FbtStopStruct;
template <>
inline MC_AXIS_STATES get_axis_stat_when_fb_activated<FbtStopStruct>() {
    return mcStopping;
}
class FbtHomeStruct;
template <>
inline MC_AXIS_STATES get_axis_stat_when_fb_activated<FbtHomeStruct>() {
    return mcHoming;
}

class FbtMoveVelocityStruct;
template <>
inline MC_AXIS_STATES get_axis_stat_when_fb_activated<FbtMoveVelocityStruct>() {
    return mcContinuousMotion;
}

// Check if type has Axis member
template <typename T>
struct has_axis {
private:
    // Test for Axis member
    template <typename C>
    static constexpr auto test(int) -> decltype(std::declval<C>().Axis, bool()) {
        return true;
    }
    
    // Fallback
    template <typename>
    static constexpr bool test(...) {
        return false;
    }
    
public:
    static constexpr bool value = test<T>(0);
};

// Check if type has Slave member
// template <typename T, typename = void>
// struct has_slave : std::false_type {};

// template <typename T>
// struct has_slave<T, void_t<decltype(std::declval<T>().Slave)>>
//     : std::true_type {};

// Check if type has Slave member - SFINAE implementation
template <typename T>
struct has_slave {
private:
    // Test for Slave member
    template <typename C>
    static constexpr auto test(int) -> decltype(std::declval<C>().Slave, bool()) {
        return true;
    }
    
    // Fallback
    template <typename>
    static constexpr bool test(...) {
        return false;
    }
    
public:
    static constexpr bool value = test<T>(0);
};

template <typename T>
constexpr bool has_axis_or_slave(){
    return has_slave<T>::value || has_axis<T>::value;
}

// Get Slave when available
template <typename T, enable_if_t<has_slave<T>::value, int> = 0>
auto get_axis_or_slave(const T& obj) -> decltype(obj.Slave) {
    return obj.Slave;
}

// Get Axis as default fallback
template <typename T, enable_if_t<!has_slave<T>::value, int> = 0>
auto get_axis_or_slave(const T& obj) -> decltype(obj.Axis) {
    return obj.Axis;
}

inline void test_hasBufferMode() {
    struct TestHasBufferMode { MC_BUFFER_MODE BufferMode; };
    struct TestNoBufferMode {};
    // TestHasSlave obj;
    static_assert(has_buffer_mode<TestHasBufferMode>::value == true, "");
    static_assert(has_buffer_mode<TestNoBufferMode>::value == false, "");
}

inline void test_hasAxis() {
    struct TestHasAxis { AXIS_REF Axis; };
    struct TestNoAxis { int abc; };
    static_assert(has_axis<TestHasAxis>::value == true, "");
    static_assert(has_axis<TestNoAxis>::value == false, "");
}

inline void test_getAxisOrSlave() {
    struct TestHasSlave { AXIS_REF Slave; };
    struct TestNoSlave { AXIS_REF axis; };
    // TestHasSlave obj;
    static_assert(has_slave<TestHasSlave>::value == true, "");
    static_assert(has_slave<TestNoSlave>::value == false, "");
}

inline void test_hasInEndVelocity() {
    struct TestHasInEndVelocity { bool InEndVelocity; };
    struct TestNoInEndVelocity { int abc;};
    static_assert(has_in_end_velocity<TestHasInEndVelocity>::value == true, "");
    static_assert(has_in_end_velocity<TestNoInEndVelocity>::value == false, "");
    static_assert(is_fb_has_in_end_velocity<TestHasInEndVelocity>() == true, "");
    static_assert(is_fb_has_in_end_velocity<TestNoInEndVelocity>() == false, "");
}

}

#endif