#ifndef __ANOLE_RUNTIME_VARIABLE_HPP__
#define __ANOLE_RUNTIME_VARIABLE_HPP__

#include "../objects/object.hpp"

#include <memory>

namespace anole
{
class Variable
{
  public:
    Variable() : ptr_(nullptr) {}
    Variable(Object *ptr) : ptr_(ptr) {}

    Variable &operator=(Object *) = delete;

    void bind(Object *ptr)
    {
        ptr_ = ptr;
    }

    Object *ptr()
    {
        return ptr_;
    }

    operator bool()
    {
        return ptr_ != nullptr;
    }

  private:
    Object *ptr_;
};
} // namespace anole

#endif
