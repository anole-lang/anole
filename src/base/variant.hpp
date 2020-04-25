#pragma once

namespace base
{
template<typename ...Types>
class variant
{
  public:
    constexpr variant() noexcept;
    constexpr variant(const variant &other);
    constexpr variant(variant &&other) noexcept;
    template<typename T>
    constexpr variant(T &&t) noexcept;

  private:
};
}
