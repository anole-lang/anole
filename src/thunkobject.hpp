#pragma once

#include "code.hpp"
#include "scope.hpp"
#include "object.hpp"

namespace ice_language
{
class ThunkObject : public Object
{
  public:
    ThunkObject(Ptr<Scope> pre_scope, std::size_t base)
      : scope_(std::make_shared<Scope>(pre_scope)), base_(base) {}

    Ptr<Scope> scope() { return scope_; }
    std::size_t base() { return base_; }

  private:
    Ptr<Scope> scope_;
    std::size_t base_;
};
}
