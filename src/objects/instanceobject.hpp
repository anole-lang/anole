#pragma once

#include "object.hpp"

namespace anole
{
class InstanceObject : public Object
{
  public:
    InstanceObject();

    SPtr<Scope> &scope() { return scope_; }

  private:
    SPtr<Scope> scope_;
};
} // namespace anole
