#pragma once

#include "code.hpp"
#include "scope.hpp"
#include "object.hpp"

namespace ice_language
{
class ThunkObject : public Object
{
  public:
    ThunkObject(Ptr<Scope> pre_scope,
        Ptr<Code> code, std::size_t base)
      : scope_(std::make_shared<Scope>(pre_scope)),
        code_(code), base_(base) {}

    Ptr<Scope> scope() { return scope_; }
    Ptr<Code> code() { return code_; }
    std::size_t base() { return base_; }

  private:
    Ptr<Scope> scope_;
    Ptr<Code> code_;
    std::size_t base_;
};
}
