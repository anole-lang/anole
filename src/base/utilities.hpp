#pragma once

#include <type_traits>

namespace base
{
template<typename...>
struct conjunction : std::true_type {};
template<typename B>
struct conjunction<B> : B {};
template<typename B1, typename ...Bn>
struct conjunction<B1, Bn...>
  : std::conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};
}
