#pragma once

namespace anole
{
class Object;

class Variable
{
  public:
    using Pointer = Object *;

    Variable() : obj_(nullptr) {}
    Variable(Pointer obj) : obj_(obj) {}

    void bind(Pointer obj)
    {
        obj_ = obj;
    }

    Pointer obj()
    {
        return obj_;
    }

    operator bool()
    {
        return obj_ != nullptr;
    }

  private:
    Pointer obj_;
};

using Address = Variable *;
} // namespace anole
