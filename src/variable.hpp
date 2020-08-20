#pragma once

#include <memory>
#include "object.hpp"

namespace anole
{
class Object;
class Variable
{
  public:
    Variable() : sptr_(nullptr) {}
    Variable(ObjectSPtr sptr) : sptr_(sptr) {}

    void bind(ObjectSPtr sptr)
    {
        sptr_.swap(sptr);
    }

    ObjectSPtr &sptr()
    {
        return sptr_;
    }

    ObjectRawPtr rptr()
    {
        return sptr_.get();
    }

    operator bool()
    {
        return sptr_ != nullptr;
    }

  private:
    ObjectSPtr sptr_;
};
} // namespace anole
