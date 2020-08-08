#pragma once

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


} // namespace light
