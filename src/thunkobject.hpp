#pragma once

#include "code.hpp"
#include "scope.hpp"
#include "object.hpp"
#include "allocator.hpp"

namespace anole
{
class ThunkObject : public Object
{
  public:
    ThunkObject(Scope *pre_scope,
        SPtr<Code> code, Size base)
      : Object(ObjectType::Thunk)
      , computed_(false)
      , scope_(Allocator<Scope>::alloc(pre_scope))
      , code_(code), base_(base)
    {
        // ...
    }

    void set_result(Address res)
    {
        result_ = res;
        computed_ = true;
    }
    bool computed() { return computed_; }
    Address result() { return result_; }
    Scope *scope() { return scope_; }
    SPtr<Code> code() { return code_; }
    Size base() { return base_; }

  private:
    bool computed_;
    Address result_;
    Scope *scope_;
    SPtr<Code> code_;
    Size base_;
};
}
