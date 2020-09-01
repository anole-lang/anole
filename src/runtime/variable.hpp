#pragma once

#include "../objects/object.hpp"

#include <memory>

namespace anole
{
class Variable
{
  public:
    Variable() : sptr_(nullptr) {}
    Variable(ObjectSPtr sptr) : sptr_(sptr) {}

    Variable &operator=(ObjectSPtr) = delete;

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
