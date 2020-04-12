#pragma once

#include "code.hpp"
#include "scope.hpp"
#include "object.hpp"

namespace ice_language
{
class ThunkObject : public Object
{
  public:
    ThunkObject(SPtr<Scope> pre_scope,
        SPtr<Code> code, std::size_t base)
      : scope_(std::make_shared<Scope>(pre_scope)),
        code_(code), base_(base) {}

    SPtr<Scope> scope() { return scope_; }
    SPtr<Code> code() { return code_; }
    std::size_t base() { return base_; }

  private:
    SPtr<Scope> scope_;
    SPtr<Code> code_;
    std::size_t base_;
};
}
