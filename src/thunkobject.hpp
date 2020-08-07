#pragma once

#include "code.hpp"
#include "scope.hpp"
#include "object.hpp"

namespace anole
{
class ThunkObject : public Object
{
  public:
    ThunkObject(SPtr<Scope> pre_scope,
        SPtr<Code> code, Size base)
      : Object(ObjectType::Thunk)
      , computed_(false)
      , scope_(std::make_shared<Scope>(pre_scope))
      , code_(code), base_(base) {}

    void set_result(Address res)
    {
        result_ = res;
        computed_ = true;
    }
    bool computed() { return computed_; }
    Address result() { return result_; }
    SPtr<Scope> scope() { return scope_; }
    SPtr<Code> code() { return code_; }
    Size base() { return base_; }

  private:
    bool computed_;
    Address result_;
    SPtr<Scope> scope_;
    SPtr<Code> code_;
    Size base_;
};
}
