#pragma once

#include <type_traits>

namespace base
{
template<typename ...Types>
class variant
{
    static_assert(sizeof...(Types) > 0,
        "variant must have at least one alternative");
    static_assert(!(std::is_reference_v<Types> || ...),
        "variant must have no reference alternative");
    static_assert(!(std::is_void_v<Types> || ...),
        "variant must have no void alternative");

  public:
    constexpr variant() noexcept;
    constexpr variant(const variant &other);
    constexpr variant(variant &&other) noexcept;
    template<typename T>
    constexpr variant(T &&t) noexcept;

  private:
};
}
