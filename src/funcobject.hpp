#pragma once

#include "code.hpp"
#include "scope.hpp"
#include "object.hpp"

namespace ice_language
{
/*
Draft

*/
class FunctionObject : public Object
{
  public:
    FunctionObject(Ptr<Scope> scope,
        Code &code, std::size_t base)
      : scope_(scope), code_(code), base_(base) {}

    Ptr<Scope> scope() { return scope_; }
    Code &code() { return code_; }
    std::size_t base() { return base_; }

  private:
    Ptr<Scope> scope_;
    Code &code_;
    std::size_t base_;
};
}
