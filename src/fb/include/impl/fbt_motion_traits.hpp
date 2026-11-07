#ifndef _ZMC_FBT_MOTION_TRAITS_HPP_
#define _ZMC_FBT_MOTION_TRAITS_HPP_

#include <type_traits>
#include <utility>  // for std::declval
#include "misc/cpp14_backport.hpp"

namespace zmc{


// Done SFINAE 
template<typename T>
struct has_Done {
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().Done, std::true_type{});
    
    template<typename U>
    static std::false_type test(...);
public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T>
struct has_Active {
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().Active, std::true_type{});
    
    template<typename U>
    static std::false_type test(...);
public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T>
struct has_Aborted {
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().CommandAborted, std::true_type{});
    
    template<typename U>
    static std::false_type test(...);
public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

//  InVelocity SFINAE
template<typename T>
struct has_InVelocity {
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().InVelocity, std::true_type{});
    
    template<typename U>
    static std::false_type test(...);
public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

// InEndVelocity SFINAE 
template<typename T>
struct has_InEndVelocity {
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().InEndVelocity, std::true_type{});
    
    template<typename U>
    static std::false_type test(...);
public:
    static constexpr bool value = decltype(test<T>(0))::value;
};


template<typename T, typename = void>
struct getMotionTypeActTmpl {
    static constexpr MC_AXIS_STATES value = mcSynchronizedMotion;
};

// Done 
template<typename T>
struct getMotionTypeActTmpl<T, enable_if_t<has_Done<T>::value>> {
    static constexpr MC_AXIS_STATES value = mcDiscreteMotion;
};

// InVelocity or InEndVelocity 
template<typename T>
struct getMotionTypeActTmpl<T, enable_if_t<
    !has_Done<T>::value && 
    (has_InVelocity<T>::value || has_InEndVelocity<T>::value)
>> {
    static constexpr MC_AXIS_STATES value = mcContinuousMotion;
};

template<typename T>
constexpr MC_AXIS_STATES getMotionTypeAct() {
    return getMotionTypeActTmpl<T>::value;
}

inline void test_getStatAct() {
    struct TestDone { int Done; };
    struct TestInVel { bool InVelocity; };
    struct TestInEndVel { bool InEndVelocity; };
    struct TestBothVel { int InVelocity; int InEndVelocity; };
    struct TestNone {};

    static_assert(getMotionTypeAct<TestDone>() == mcDiscreteMotion, "");
    static_assert(getMotionTypeAct<TestInVel>() == mcContinuousMotion, "");
    static_assert(getMotionTypeAct<TestInEndVel>() == mcContinuousMotion, "");
    static_assert(getMotionTypeAct<TestBothVel>() == mcContinuousMotion, "");
    static_assert(getMotionTypeAct<TestNone>() == mcSynchronizedMotion, "");
}

typedef struct  FbtMoveAbsoluteStruct FbtMoveAbsoluteStruct; 
typedef struct FbtMoveRelativeStruct FbtMoveRelativeStruct; 
typedef struct FbtMoveContinuousAbsoluteStruct FbtMoveContinuousAbsoluteStruct; 
typedef struct FbtStopStruct FbtStopStruct;
class FbtStop;

template <typename T, typename std::enable_if<!has_Aborted<T>::value, int>::type = 0>
inline int get_move_stat(const T& fb) {
    return fb.Busy * 3 + fb.Done * 4 + fb.Error * 5;
}

template <typename T,
          typename std::enable_if<!has_Active<T>::value && has_Aborted<T>::value, int>::type = 0>
inline int get_move_stat(const T& fb) {
    return fb.Busy * 3 + fb.Done * 4 + fb.Error * 5 + fb.CommandAborted * 6;
}

template <typename T,
          typename std::enable_if<
              has_Active<T>::value && has_Done<T>::value && has_Aborted<T>::value, int>::type = 0>
inline int get_move_stat(const T& fb) {
    return fb.Busy * 2 + fb.Active + fb.Done * 4 + fb.Error * 5 + fb.CommandAborted * 6;
}

template <typename T,
          typename std::enable_if<
              has_Active<T>::value && !has_Done<T>::value && has_Aborted<T>::value, int>::type = 0>
inline int get_move_stat(const T& fb) {
    return fb.Busy * 2 + fb.Active + fb.Error * 5 + fb.CommandAborted * 6;
}

// struct FbtHomeStruct;
// template<>
// inline int get_move_stat<FbtHomeStruct>(const FbtHomeStruct& fb){
//     return fb.Busy*3 + fb.Done * 4 + fb.Error * 5 + fb.CommandAborted * 6;
// }


template <typename T>
inline bool is_fb_has_absolute() {
    return false;
}
template <>
inline bool is_fb_has_absolute<FbtMoveAbsoluteStruct>() {
    return true;
}
template <>
inline bool is_fb_has_absolute<FbtMoveContinuousAbsoluteStruct>() {
    return true;
}

template <typename T>
inline bool isFbBlendable(){
    return false;
}
template <>
inline bool isFbBlendable<FbtMoveAbsoluteStruct>(){
    return true;
}
template <>
inline bool isFbBlendable<FbtMoveRelativeStruct>(){
    return true;
}

template <typename T>
inline double get_pos_or_dist(const T& obj) {
    return obj.Distance;
}
template <>
inline double get_pos_or_dist(const FbtMoveAbsoluteStruct& obj) {
    return obj.Position;
}
template <>
inline double get_pos_or_dist(const FbtMoveContinuousAbsoluteStruct& obj) {
    return obj.Position;
}

}
#endif