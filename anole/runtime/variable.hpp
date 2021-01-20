#ifndef __ANOLE_RUNTIME_VARIABLE_HPP__
#define __ANOLE_RUNTIME_VARIABLE_HPP__

#include "../objects/object.hpp"

#include <memory>

namespace anole
{
class Variable
{
  public:
    Variable() noexcept(noexcept(String())) : ptr_(nullptr) {}
    Variable(Object *ptr) noexcept(noexcept(String())) : ptr_(ptr) {}

    Variable &operator=(Object *) = delete;

    void bind(Object *ptr) noexcept
    {
        ptr_ = ptr;
    }

    Object *ptr() const noexcept
    {
        return ptr_;
    }

    void set_called_name(String called_name)
    {
        called_name_ = std::move(called_name);
    }

    const String &called_name() const noexcept
    {
      return called_name_;
    }

  private:
    Object *ptr_;
    String called_name_;
};
} // namespace anole

#endif
