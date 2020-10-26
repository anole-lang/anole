#ifndef __LIGHT_TYPETRAITS_HPP__
#define __LIGHT_TYPETRAITS_HPP__

namespace light
{
template<typename ...Ts>
using void_t = void;

template<typename T, T v>
struct integral_constant
{
    using value_type = T;
    using type = integral_constant<T, v>;

    constexpr static T value = v;
};

template<bool v>
using bool_constant = integral_constant<bool, v>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

template<typename T>
struct is_pointer : false_type {};
template<typename T>
struct is_pointer<T *> : true_type {};
template<typename T>
inline constexpr bool is_pointer_v = is_pointer<T>::value;

template<typename T>
struct is_reference : false_type {};
template<typename T>
struct is_reference<T&> : true_type {};
template<typename T>
struct is_reference<T&&> : true_type {};

template<typename T>
struct is_lvalue_reference : false_type {};
template<typename T>
struct is_lvalue_reference<T&> : true_type {};

template<typename T>
struct is_rvalue_reference : false_type {};
template<typename T>
struct is_rvalue_reference<T&&> : true_type {};

template<typename T>
struct is_const : false_type {};
template<typename T>
struct is_const<const T> : true_type {};
template<typename T>
struct is_volatile : false_type {};
template<typename T>
struct is_volatile<volatile T> : true_type {};

template<typename T, typename U>
struct is_same : false_type {};
template<typename T>
struct is_same<T, T> : true_type {};
template<typename T, typename U>
inline constexpr bool is_same_v = is_same<T, U>::value;

template<typename T>
struct remove_cv { using type = T; };
template<typename T>
struct remove_cv<const T> { using type = T; };
template<typename T>
struct remove_cv<volatile T> { using type = T; };
template<typename T>
struct remove_cv<const volatile T> { using type = T; };

template<typename T>
struct remove_reference { using type = T; };
template<typename T>
struct remove_reference<T&> { using type = T; };
template<typename T>
struct remove_reference<T&&> { using type = T; };
template<typename T>
using remove_reference_t = typename remove_reference<T>::type;

template<typename T>
struct is_void : is_same<void, typename remove_cv<T>::type> {};
template<typename T>
inline constexpr bool is_void_v = is_void<T>::value;

template<bool B, typename T = void>
struct enable_if {};
template<typename T>
struct enable_if<true, T>
{
    using type = T;
};
template<bool B, typename T>
using enable_if_t = typename enable_if<B, T>::type;
} // namespace light

#endif
