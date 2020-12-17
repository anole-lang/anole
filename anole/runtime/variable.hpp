#ifndef __ANOLE_RUNTIME_VARIABLE_HPP__
#define __ANOLE_RUNTIME_VARIABLE_HPP__

#include "../objects/object.hpp"

#include <memory>

namespace anole
{
class Variable
{
  public:
    constexpr Variable() noexcept : ptr_(nullptr) {}
    constexpr Variable(Object *ptr) noexcept : ptr_(ptr) {}

    Variable &operator=(Object *) = delete;

    void bind(Object *ptr) noexcept
    {
        ptr_ = ptr;
    }

    constexpr Object *ptr() const noexcept
    {
        return ptr_;
    }

    constexpr operator bool() const noexcept
    {
        return ptr_ != nullptr;
    }

  private:
    Object *ptr_;
};
} // namespace anole

#endif
